#include "exclusive_maximum_rule.hpp"

using namespace godot;

bool ExclusiveMaximumRule::validate(const Variant &target, ValidationContext &context) const {
	if (target.get_type() != Variant::INT && target.get_type() != Variant::FLOAT) {
		return true; // Rule doesn't apply to non-numeric values
	}

	double actual_value;
	if (target.get_type() == Variant::INT) {
		actual_value = static_cast<double>(target.operator int64_t());
	} else {
		actual_value = target.operator double();
	}

	if (actual_value >= exclusive_maximum) {
		context.add_error(
				vformat("Value %d is not less than exclusive maximum %d", actual_value, exclusive_maximum),
				"exclusiveMaximum",
				target);
		return false;
	}

	return true;
}
