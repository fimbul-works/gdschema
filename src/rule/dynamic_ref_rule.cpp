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

	// Ensure the schema is compiled
	resolved->compile();

	if (!resolved->rules) {
		context.add_error(vformat("Referenced schema '%s' is not compiled", reference_uri), "dynamicRef");
		return false;
	}

	bool result = false;

	// Create child context for the reference validation, incrementing depth
	ValidationContext ref_context = context.create_child_schema(vformat("$dynamicRef:%s", reference_uri)).with_incremented_depth();

	// Note: The DynamicScopeRule inside the resolved schema will push itself to the scope if needed.
	// However, the spec says $dynamicRef itself adds to the dynamic scope.
	ref_context.push_dynamic_scope(resolved);

	// Validate using the resolved schema's rules
	result = resolved->rules->validate(target, ref_context);

	// Merge results
	context.merge_errors(ref_context);
	context.merge_evaluation_data(ref_context);

	return result;
}

String DynamicRefRule::get_description() const {
	return "$dynamicRef:" + reference_uri;
}
