#include "multiple_of_rule.hpp"

using namespace godot;

bool MultipleOfRule::validate(const Variant &target, ValidationContext &context) const {
	if (target.get_type() != Variant::INT && target.get_type() != Variant::FLOAT) {
		return true; // Rule doesn't apply to non-numeric values
	}

	if (multiple_of <= 0) {
		context.add_error("multipleOf must be greater than 0", "multipleOf");
		return false;
	}

	double actual_value;
	if (target.get_type() == Variant::INT) {
		actual_value = static_cast<double>(target.operator int64_t());
	} else {
		actual_value = target.operator double();
	}

	double division = actual_value / multiple_of;
	double remainder = fmod(actual_value, multiple_of);

	// Check if the remainder is close to zero (accounting for floating point precision)
	if (abs(remainder) > 1e-10 && abs(remainder - multiple_of) > 1e-10) {
		context.add_error(
				vformat("Value %f is not a multiple of %f", actual_value, multiple_of),
				"multipleOf",
				target);
		return false;
	}

	return true;
}
