#include "required_properties_rule.hpp"
#include "../validation_context.hpp"

using namespace godot;

bool RequiredPropertiesRule::validate(const Variant &target, ValidationContext &context) const {
	// Only validate objects/dictionaries
	if (target.get_type() != Variant::DICTIONARY) {
		return true; // Rule doesn't apply to non-objects
	}

	Dictionary dict = target.operator Dictionary();
	bool all_present = true;

	// Check each required property
	for (const String &required_prop : required_props) {
		StringName prop_name = StringName(required_prop);

		if (!dict.has(prop_name)) {
			context.add_error(vformat("Required property \"%s\" is missing", required_prop), "required");
			all_present = false;
		}
	}

	return all_present;
}

String RequiredPropertiesRule::get_description() const {
	String desc = "required(";
	for (size_t i = 0; i < required_props.size(); i++) {
		if (i > 0) {
			desc += ", ";
		}
		desc += required_props[i];
	}
	desc += ")";
	return desc;
}
