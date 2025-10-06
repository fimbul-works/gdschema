#include "const_rule.hpp"
#include "../validation_context.hpp"

using namespace godot;

bool ConstRule::validate(const Variant &target, ValidationContext &context) const {
	// Check for exact equality using Variant's built-in comparison
	if (target != constant_value) {
		// Build descriptive error message
		String target_str = target.stringify();
		String constant_str = constant_value.stringify();

		context.add_error(vformat("Value %s does not equal required constant %s", target_str, constant_str), "const", target);
		return false;
	}

	return true;
}
