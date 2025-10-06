#include "not_rule.hpp"
#include "../validation_context.hpp"

using namespace godot;

NotRule::NotRule(std::shared_ptr<ValidationRule> rule) :
		sub_rule(rule) {}

bool NotRule::validate(const Variant &target, ValidationContext &context) const {
	if (!sub_rule) {
		context.add_error("NotRule has no sub-rule to negate", "not");
		return false;
	}

	// Create a temporary context to capture sub-rule errors
	// We don't want to propagate these errors to the main context
	ValidationContext temp_context = context.create_child_schema("not");

	bool sub_rule_passed = sub_rule->validate(target, temp_context);

	if (sub_rule_passed) {
		// Sub-rule passed, but we need it to fail for NOT to succeed
		context.add_error(vformat("Value matched the negated schema when it should not have: %s", sub_rule->get_description()), "not");
		return false;
	}

	// Sub-rule failed, which is what we want for NOT - success!
	return true;
}

String NotRule::get_description() const {
	if (!sub_rule) {
		return "not(null)";
	}

	return vformat("not(%s)", sub_rule->get_description());
}
