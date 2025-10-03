#include "ref_rule.hpp"
#include "../rule_factory.hpp"
#include "../schema_registry.hpp"
#include "../validation_context.hpp"

#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

// Simple thread-local depth counter to prevent infinite recursion
thread_local int validation_depth = 0;
const int MAX_VALIDATION_DEPTH = 50; // Reasonable limit

RefRule::RefRule(const String &ref_uri, const Schema *schema) :
		reference_uri(ref_uri), source_schema(schema), resolution_attempted(false) {}

bool RefRule::validate(const Variant &target, ValidationContext &context) const {
	// Simple depth-based recursion protection
	if (validation_depth >= MAX_VALIDATION_DEPTH) {
		return true; // Assume valid to break potential infinite recursion
	}

	// Lazy resolution
	if (!resolution_attempted) {
		cached_schema = source_schema->resolve_reference(reference_uri);
		resolution_attempted = true;

		if (!cached_schema.is_valid()) {
			context.add_error(vformat("Could not resolve reference: %s", reference_uri), "ref", reference_uri);
			return false;
		}
	}

	if (!cached_schema.is_valid()) {
		return false;
	}

	// Increment depth counter
	validation_depth++;

	bool validation_result = false;

	try {
		// Ensure the schema is compiled
		cached_schema->compilation_mutex->lock();
		bool needs_compilation = !cached_schema->is_compiled;
		cached_schema->compilation_mutex->unlock();

		if (needs_compilation) {
			cached_schema->compile();
		}

		// Get the compiled rules safely
		cached_schema->compilation_mutex->lock();

		if (!cached_schema->is_compiled || !cached_schema->rules) {
			cached_schema->compilation_mutex->unlock();
			context.add_error(vformat("Referenced schema '%s' is not compiled", reference_uri), "ref", reference_uri);
			validation_result = false;
		} else {
			// Copy rules reference so we can unlock
			auto rules_to_validate = cached_schema->rules;
			cached_schema->compilation_mutex->unlock();

			// Create child context for the reference validation
			ValidationContext ref_context = context.create_child_schema(vformat("$ref:%s", reference_uri));

			// Validate using the resolved schema's rules
			validation_result = rules_to_validate->validate(target, ref_context);

			// Merge any errors from the reference validation
			context.merge_errors(ref_context);
		}
	} catch (...) {
		// Ensure we clean up the depth counter even if an exception occurs
		validation_depth--;
		throw;
	}

	// Decrement depth counter
	validation_depth--;

	return validation_result;
}
