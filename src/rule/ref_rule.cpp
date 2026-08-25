#include "ref_rule.hpp"
#include "../rule_factory/rule_factory.hpp"
#include "../schema_registry.hpp"
#include "../validation_context.hpp"

#include "../util.hpp"
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

RefRule::RefRule(const String &ref_uri, const Schema *schema) :
		reference_uri(ref_uri), source_schema(schema) {}

bool RefRule::validate(const Variant &target, ValidationContext &context) const {
	// Simple depth-based recursion protection
	if (context.get_validation_depth() >= MAX_VALIDATION_DEPTH) {
		return true; // Assume valid to break potential infinite recursion
	}

	if (!source_schema) {
		context.add_error(vformat("Source schema is null for reference: %s", reference_uri), "ref", reference_uri);
		return false;
	}

	Ref<Schema> resolved = source_schema->resolve_reference(reference_uri);
	if (!resolved.is_valid()) {
		context.add_error(vformat("Could not resolve reference: %s", reference_uri), "ref", reference_uri);
		return false;
	}

	bool validation_result = false;

	try {
		// Ensure the schema is compiled
		if (!resolved->is_compiled) {
			resolved->compile();
		}

		if (!resolved->is_compiled || !resolved->rules) {
			context.add_error(vformat("Referenced schema '%s' is not compiled", reference_uri), "ref", reference_uri);
			return false;
		}

		auto rules_to_validate = resolved->rules;

		// Create child context for the reference validation, incrementing depth
		ValidationContext ref_context = context.create_child_schema(vformat("$ref:%s", reference_uri)).with_incremented_depth();
		ref_context.push_dynamic_scope(resolved);

		// Validate using the resolved schema's rules
		validation_result = rules_to_validate->validate(target, ref_context);

		// Merge results from the reference validation
		context.merge_errors(ref_context);
		context.merge_evaluation_data(ref_context);
	} catch (...) {
		throw;
	}

	return validation_result;
}
