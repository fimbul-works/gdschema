#include "min_length_rule.hpp"
#include "../validation_context.hpp"

using namespace godot;

bool MinLengthRule::validate(const Variant &target, ValidationContext &context) const {
	// Only validate strings
	if (target.get_type() != Variant::STRING && target.get_type() != Variant::STRING_NAME) {
		return true; // Rule doesn't apply to non-strings
	}

	String str = target.operator String();
	int64_t actual_length = str.length(); // Use character count (Unicode code points) as required by JSON Schema spec

	if (actual_length < min_length) {
		context.add_error(vformat("String length %d is less than minimum %d", actual_length, min_length), "minLength", target);
		return false;
	}

	return true;
}
