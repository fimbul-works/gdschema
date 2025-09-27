#include "max_length_rule.hpp"
#include "../validation_context.hpp"

using namespace godot;

bool MaxLengthRule::validate(const Variant &target, ValidationContext &context) const {
	// Only validate strings
	if (target.get_type() != Variant::STRING && target.get_type() != Variant::STRING_NAME) {
		return true; // Rule doesn't apply to non-strings
	}

	String str = target.operator String();
	int64_t actual_length = str.utf8().length(); // Use UTF-8 byte length for proper Unicode handling

	if (actual_length > max_length) {
		context.add_error(
				vformat("String length %d exceeds maximum %d", actual_length, max_length),
				"maxLength",
				target);
		return false;
	}

	return true;
}
