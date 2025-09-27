#include "any_of_rule.hpp"
#include "../validation_context.hpp"

using namespace godot;

void AnyOfRule::add_sub_rule(std::shared_ptr<ValidationRule> rule) {
	if (rule) {
		sub_rules.push_back(rule);
	}
}

bool AnyOfRule::validate(const Variant &target, ValidationContext &context) const {
	if (sub_rules.empty()) {
		context.add_error("anyOf requires at least one sub-schema", "anyOf");
		return false;
	}

	// At least one sub-rule must pass
	bool any_passed = false;
	std::vector<ValidationContext> failed_contexts;

	for (size_t i = 0; i < sub_rules.size(); i++) {
		ValidationContext sub_context = context.create_child_schema(vformat("anyOf/%d", i));

		if (sub_rules[i]->validate(target, sub_context)) {
			any_passed = true;
			break; // Success - we can stop here
		} else {
			failed_contexts.push_back(sub_context);
		}
	}

	if (!any_passed) {
		context.add_error(
				vformat("Value failed all %d anyOf schemas", sub_rules.size()),
				"anyOf");

		// Optionally merge some failed context errors for debugging
		// (but don't overwhelm with all of them)
		if (!failed_contexts.empty()) {
			context.merge_errors(failed_contexts[0]); // Show errors from first failed schema
		}

		return false;
	}

	return true;
}

String AnyOfRule::get_description() const {
	if (sub_rules.empty()) {
		return "anyOf(empty)";
	}

	String desc = "anyOf(";
	for (size_t i = 0; i < sub_rules.size(); i++) {
		if (i > 0) {
			desc += " OR ";
		}
		desc += sub_rules[i]->get_description();
	}
	desc += ")";
	return desc;
}
