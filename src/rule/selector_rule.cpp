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
		if (rule->validate(selection.value, child_context)) {
			// If validation succeeded and we have a path segment, mark it as evaluated in the parent context
			if (!selection.path_segment.is_empty()) {
				if (selection.path_segment.is_valid_int()) {
					context.mark_item_evaluated(selection.path_segment.to_int());
				} else {
					context.mark_property_evaluated(selection.path_segment);
				}
			}
		} else {
			all_valid = false;
			// Continue to validate other targets and collect all errors
		}
		context.merge_errors(child_context);
		// Also merge any evaluation data from child (in case of nested evaluators)
		context.merge_evaluation_data(child_context);
	}

	return all_valid;
}

String SelectorRule::get_description() const {
	if (!selector || !rule) {
		return "invalid_selector_rule";
	}

	return selector->get_description() + " -> " + rule->get_description();
}
