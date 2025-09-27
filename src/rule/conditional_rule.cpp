#include "conditional_rule.hpp"
#include "../validation_context.hpp"

using namespace godot;

bool ConditionalRule::validate(const Variant &target, ValidationContext &context) const {
	if (!if_rule) {
		context.add_error("Conditional rule missing 'if' schema", "conditional");
		return false;
	}

	// Create a temporary context to test the 'if' condition
	// We don't want to propagate errors from the 'if' test to the main context
	ValidationContext if_context = context.create_child_schema("if");
	bool if_result = if_rule->validate(target, if_context);

	if (if_result) {
		// 'if' condition passed, validate against 'then' schema
		if (then_rule) {
			ValidationContext then_context = context.create_child_schema("then");
			bool then_result = then_rule->validate(target, then_context);
			if (!then_result) {
				context.merge_errors(then_context);
				return false;
			}
		}
		// If no 'then' schema, validation passes
		return true;
	} else {
		// 'if' condition failed, validate against 'else' schema
		if (else_rule) {
			ValidationContext else_context = context.create_child_schema("else");
			bool else_result = else_rule->validate(target, else_context);
			if (!else_result) {
				context.merge_errors(else_context);
				return false;
			}
		}
		// If no 'else' schema, validation passes
		return true;
	}
}

String ConditionalRule::get_description() const {
	String desc = "conditional(if: ";
	if (if_rule) {
		desc += if_rule->get_description();
	} else {
		desc += "null";
	}

	if (then_rule) {
		desc += ", then: " + then_rule->get_description();
	}

	if (else_rule) {
		desc += ", else: " + else_rule->get_description();
	}

	desc += ")";
	return desc;
}
