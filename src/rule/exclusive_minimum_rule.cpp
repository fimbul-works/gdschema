#include "exclusive_minimum_rule.hpp"

using namespace godot;

bool ExclusiveMinimumRule::validate(const Variant &target, ValidationContext &context) const {
	if (target.get_type() != Variant::INT && target.get_type() != Variant::FLOAT) {
		return true; // Rule doesn't apply to non-numeric values
	}

	double actual_value;
	if (target.get_type() == Variant::INT) {
		actual_value = static_cast<double>(target.operator int64_t());
	} else {
		actual_value = target.operator double();
	}

	if (actual_value <= exclusive_minimum) {
		context.add_error(vformat("Value %f is not greater than exclusive minimum %f", actual_value, exclusive_minimum),
				"exclusiveMinimum", target);
		return false;
	}

	return true;
}
