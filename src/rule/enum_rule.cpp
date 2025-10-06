#include "enum_rule.hpp"
#include "../validation_context.hpp"

using namespace godot;

bool EnumRule::validate(const Variant &target, ValidationContext &context) const {
	// Check if target matches any of the allowed values
	for (int i = 0; i < allowed_values.size(); i++) {
		if (target == allowed_values[i]) {
			return true; // Found a match
		}
	}

	// No match found - build error message
	String target_str = target.stringify();
	String allowed_str;
	for (int i = 0; i < allowed_values.size(); i++) {
		if (i > 0) {
			allowed_str += ", ";
		}
		allowed_str += allowed_values[i].stringify();
	}

	context.add_error(vformat("Value %s is not one of the allowed values: %s", target_str, allowed_str), "enum", target);

	return false;
}

String EnumRule::get_description() const {
	String desc = "enum(";
	for (int i = 0; i < allowed_values.size(); i++) {
		if (i > 0) {
			desc += ", ";
		}
		desc += allowed_values[i].stringify();
	}
	desc += ")";
	return desc;
}
