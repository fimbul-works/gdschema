#include "all_of_rule.hpp"
#include "../validation_context.hpp"

using namespace godot;

void AllOfRule::add_sub_rule(std::shared_ptr<ValidationRule> rule) {
	if (rule) {
		sub_rules.push_back(rule);
	}
}

bool AllOfRule::validate(const Variant &target, ValidationContext &context) const {
	bool all_valid = true;
	int64_t failed_count = 0;
	std::vector<ValidationContext> sub_contexts;
	sub_contexts.reserve(sub_rules.size());

	// All sub-rules must pass, evaluated against the initial context
	for (int64_t i = 0; i < sub_rules.size(); i++) {
		ValidationContext sub_context = context.create_child_schema(vformat("allOf/%d", i));

		if (!sub_rules[i]->validate(target, sub_context)) {
			all_valid = false;
			failed_count++;
			// Continue to validate other sub-rules and collect all errors
		}

		// Merge sub-context errors into main context
		context.merge_errors(sub_context);
		sub_contexts.push_back(std::move(sub_context));
	}

	if (!all_valid) {
		context.add_error(vformat("Value failed %d out of %d allOf schemas", failed_count, static_cast<int64_t>(sub_rules.size())), "allOf");
		return false;
	}

	// If all sub-rules passed, merge evaluation data from all sub-contexts
	for (const auto &sub_ctx : sub_contexts) {
		context.merge_evaluation_data(sub_ctx);
	}

	return true;
}

String AllOfRule::get_description() const {
	if (sub_rules.empty()) {
		return "allOf(empty)";
	}

	String desc = "allOf(";
	for (size_t i = 0; i < sub_rules.size(); i++) {
		if (i > 0) {
			desc += " AND ";
		}
		desc += sub_rules[i]->get_description();
	}
	desc += ")";
	return desc;
}
