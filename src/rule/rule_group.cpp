#include "rule_group.hpp"
#include "../validation_context.hpp"

using namespace godot;

void RuleGroup::add_rule(std::shared_ptr<ValidationRule> rule) {
	if (rule) {
		rules.push_back(rule);
	}
}

bool RuleGroup::validate(const Variant &target, ValidationContext &context) const {
	bool all_valid = true;

	// Validate against all rules and collect all errors
	for (const auto &rule : rules) {
		if (!rule->validate(target, context)) {
			all_valid = false;
			// Continue validating other rules to collect all errors
		}
	}

	return all_valid;
}

String RuleGroup::get_description() const {
	if (rules.empty()) {
		return "empty_group";
	}

	if (rules.size() == 1) {
		return rules[0]->get_description();
	}

	String desc = "group(";
	for (size_t i = 0; i < rules.size(); i++) {
		if (i > 0) {
			desc += " AND ";
		}
		desc += rules[i]->get_description();
	}
	desc += ")";
	return desc;
}
