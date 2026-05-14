#include "dependent_required_rule.hpp"
#include "../validation_context.hpp"

#include <godot_cpp/variant/dictionary.hpp>

using namespace godot;

bool DependentRequiredRule::validate(const Variant &target, ValidationContext &context) const {
	if (target.get_type() != Variant::DICTIONARY) {
		return true;
	}

	Dictionary dict = target.operator Dictionary();
	bool all_valid = true;

	for (const auto &pair : dependencies) {
		if (dict.has(pair.first)) {
			for (const auto &req : pair.second) {
				if (!dict.has(req)) {
					context.add_error(vformat("Property '%s' is present, which requires property '%s' to be present", pair.first, req), "dependentRequired");
					all_valid = false;
				}
			}
		}
	}

	return all_valid;
}

String DependentRequiredRule::get_description() const {
	return "dependentRequired";
}
