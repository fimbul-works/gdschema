#include "one_of_rule.hpp"
#include "../validation_context.hpp"

using namespace godot;

void OneOfRule::add_sub_rule(std::shared_ptr<ValidationRule> rule) {
	if (rule) {
		sub_rules.push_back(rule);
	}
}

bool OneOfRule::validate(const Variant &target, ValidationContext &context) const {
	if (sub_rules.empty()) {
		context.add_error("oneOf requires at least one sub-schema", "oneOf");
		return false;
	}

	// Exactly one sub-rule must pass
	int64_t passed_count = 0;
	std::vector<size_t> passed_indices;
	std::vector<ValidationContext> failed_contexts;

	for (int64_t i = 0; i < sub_rules.size(); i++) {
		ValidationContext sub_context = context.create_child_schema(vformat("oneOf/%d", i));

		if (sub_rules[i]->validate(target, sub_context)) {
			passed_count++;
			passed_indices.push_back(i);

			// Continue checking all schemas to count total passes
		} else {
			failed_contexts.push_back(sub_context);
		}
	}

	if (passed_count == 0) {
		context.add_error(vformat("Value failed all %d oneOf schemas", (int64_t)sub_rules.size()), "oneOf");

		// Show errors from first failed schema for debugging
		if (!failed_contexts.empty()) {
			context.merge_errors(failed_contexts[0]);
		}

		return false;

	} else if (passed_count > 1) {
		String passed_list;
		for (size_t i = 0; i < passed_indices.size(); i++) {
			if (i > 0) {
				passed_list += ", ";
			}
			passed_list += String::num(passed_indices[i]);
		}

		context.add_error(vformat("Value matched %d oneOf schemas (indices: %s) but exactly 1 is required", passed_count, passed_list), "oneOf");
		return false;
	}

	// Exactly one passed - success!
	return true;
}

String OneOfRule::get_description() const {
	if (sub_rules.empty()) {
		return "oneOf(empty)";
	}

	String desc = "oneOf(";
	for (size_t i = 0; i < sub_rules.size(); i++) {
		if (i > 0) {
			desc += " XOR ";
		}
		desc += sub_rules[i]->get_description();
	}
	desc += ")";
	return desc;
}
