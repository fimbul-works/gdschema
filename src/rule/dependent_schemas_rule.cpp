#include "dependent_schemas_rule.hpp"
#include "../validation_context.hpp"

#include <godot_cpp/variant/dictionary.hpp>

using namespace godot;

bool DependentSchemasRule::validate(const Variant &target, ValidationContext &context) const {
	if (target.get_type() != Variant::DICTIONARY) {
		return true;
	}

	Dictionary dict = target.operator Dictionary();
	bool all_valid = true;
	std::vector<ValidationContext> sub_contexts;

	for (const auto &pair : dependent_schemas) {
		if (dict.has(pair.first)) {
			ValidationContext sub_context = context.create_child_schema(vformat("dependentSchemas/%s", pair.first));

			if (!pair.second->validate(target, sub_context)) {
				all_valid = false;
			}

			// Merge errors
			context.merge_errors(sub_context);
			sub_contexts.push_back(std::move(sub_context));
		}
	}

	if (all_valid) {
		for (const auto &sub_ctx : sub_contexts) {
			context.merge_evaluation_data(sub_ctx);
		}
	}

	return all_valid;
}

String DependentSchemasRule::get_description() const {
	return "dependentSchemas";
}
