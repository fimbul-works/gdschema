#include "true_rule.hpp"
#include "../validation_context.hpp"

using namespace godot;

bool TrueRule::validate(const Variant &target, ValidationContext &context) const {
	return true; // Always passes
}
