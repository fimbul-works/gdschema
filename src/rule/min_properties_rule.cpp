#include "min_properties_rule.hpp"

using namespace godot;

bool MinPropertiesRule::validate(const Variant &target, ValidationContext &context) const {
	if (target.get_type() != Variant::DICTIONARY) {
		return true; // Rule doesn't apply to non-objects
	}

	Dictionary dict = target.operator Dictionary();
	int64_t actual_count = dict.size();

	if (actual_count < min_properties) {
		context.add_error(vformat("Object has %d properties but minimum is %d", actual_count, min_properties), "minProperties", target);
		return false;
	}

	return true;
}
