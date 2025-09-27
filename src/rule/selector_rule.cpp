#include "selector_rule.hpp"
#include "../validation_context.hpp"

using namespace godot;

SelectorRule::SelectorRule(std::unique_ptr<Selector> sel, std::shared_ptr<ValidationRule> r) :
		selector(std::move(sel)), rule(r) {}

bool SelectorRule::validate(const Variant &target, ValidationContext &context) const {
	if (!selector || !rule) {
		context.add_error("Internal error: invalid selector rule");
		return false;
	}

	std::vector<SelectionTarget> targets = selector->select_targets(target, context);
	bool all_valid = true;

	for (const auto &selection : targets) {
		ValidationContext child_context = context.create_child_instance(selection.path_segment);
		if (!rule->validate(selection.value, child_context)) {
			all_valid = false;
			// Continue to validate other targets and collect all errors
		}
		context.merge_errors(child_context);
	}

	return all_valid;
}

String SelectorRule::get_description() const {
	if (!selector || !rule) {
		return "invalid_selector_rule";
	}

	return selector->get_description() + " -> " + rule->get_description();
}
