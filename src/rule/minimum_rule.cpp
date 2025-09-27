#include "minimum_rule.hpp"
#include "../validation_context.hpp"

using namespace godot;

bool MinimumRule::validate(const Variant &target, ValidationContext &context) const {
	// Only validate numeric types
	if (target.get_type() != Variant::INT && target.get_type() != Variant::FLOAT) {
		return true; // Rule doesn't apply to non-numeric values
	}

	double actual_value;
	if (target.get_type() == Variant::INT) {
		actual_value = static_cast<double>(target.operator int64_t());
	} else {
		actual_value = target.operator double();
	}

	if (actual_value < minimum) {
		context.add_error(
				vformat("Value %d is less than minimum %d", actual_value, minimum),
				"minimum",
				target);
		return false;
	}

	return true;
}
