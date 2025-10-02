#include "false_rule.hpp"
#include "../validation_context.hpp"

using namespace godot;

bool FalseRule::validate(const Variant &target, ValidationContext &context) const {
	context.add_error("Value is not allowed by schema: false", "false", target);
	return false; // Always fails
}
