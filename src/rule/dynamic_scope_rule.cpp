#include "dynamic_scope_rule.hpp"
#include "../schema.hpp"
#include "../validation_context.hpp"
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

bool DynamicScopeRule::validate(const Variant &target, ValidationContext &context) const {
	// Create a child context to ensure we get a new context with an empty error list
	ValidationContext child_context = context.create_child_schema("");
	if (schema) {
		child_context.push_dynamic_scope(Ref<Schema>(const_cast<Schema *>(schema)));
	}

	bool result = wrapped_rule->validate(target, child_context);

	// Merge results back
	context.merge_errors(child_context);
	context.merge_evaluation_data(child_context);

	return result;
}

String DynamicScopeRule::get_description() const {
	return schema ? vformat("dynamicScope(%s)", schema->get_schema_path()) : "dynamicScope(null)";
}
