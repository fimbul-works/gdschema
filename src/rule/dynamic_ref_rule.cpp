#include "dynamic_ref_rule.hpp"
#include "../schema.hpp"
#include "../validation_context.hpp"

#include "../util.hpp"

using namespace godot;

bool DynamicRefRule::validate(const Variant &target, ValidationContext &context) const {
	if (context.get_validation_depth() >= MAX_VALIDATION_DEPTH) {
		return true; // Break potential infinite recursion
	}

	Ref<Schema> resolved = source_schema->resolve_dynamic_reference(reference_uri, context);

	if (!resolved.is_valid()) {
		context.add_error(vformat("Could not resolve dynamic reference: %s", reference_uri), "dynamicRef");
		return false;
	}

	bool validation_result = false;

	try {
		// Ensure the schema is compiled
		if (!resolved->is_compiled) {
			resolved->compile();
		}

		if (!resolved->is_compiled || !resolved->rules) {
			context.add_error(vformat("Referenced schema '%s' is not compiled", reference_uri), "dynamicRef");
			return false;
		}

		auto rules_to_validate = resolved->rules;

		// Create child context for the reference validation, incrementing depth
		ValidationContext ref_context = context.create_child_schema(vformat("$dynamicRef:%s", reference_uri)).with_incremented_depth();
		ref_context.push_dynamic_scope(resolved);

		// Validate using the resolved schema's rules
		validation_result = rules_to_validate->validate(target, ref_context);

		// Merge results
		context.merge_errors(ref_context);
		context.merge_evaluation_data(ref_context);
	} catch (...) {
		throw;
	}

	return validation_result;
}

String DynamicRefRule::get_description() const {
	return "$dynamicRef:" + reference_uri;
}
