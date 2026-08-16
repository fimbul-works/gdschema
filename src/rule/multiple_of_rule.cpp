#include "multiple_of_rule.hpp"

#include <cmath>

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
	double rounded = std::round(division);

	// Check if division is close to an integer (accounting for 32-bit / 64-bit floating point precision)
	if (std::abs(division - rounded) > 1e-5) {
		context.add_error(vformat("Value %f is not a multiple of %f", actual_value, multiple_of), "multipleOf", target);
		return false;
	}

	return true;
}
