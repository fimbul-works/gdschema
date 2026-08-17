#include "contains_rule.hpp"
#include "../util.hpp"
#include "../validation_context.hpp"

using namespace godot;

bool ContainsRule::validate(const Variant &target, ValidationContext &context) const {
	int64_t array_size = SchemaUtil::get_array_size(target);
	if (array_size < 0) {
		return true; // Rule doesn't apply to non-arrays
	}

	// Handle case where there's no sub-rule (shouldn't happen in normal flow)
	if (!item_rule) {
		return true;
	}

	// Normal case: check how many items validate against the schema
	int64_t match_count = 0;
	std::vector<int64_t> matched_indices;
	std::vector<ValidationContext> matched_contexts;

	for (int64_t i = 0; i < array_size; i++) {
		Variant item = SchemaUtil::get_array_item(target, i);

		// Create a temporary context for this item
		ValidationContext item_context = context.create_child_instance(String::num_int64(i));

		if (item_rule->validate(item, item_context)) {
			match_count++;
			matched_indices.push_back(i);
			matched_contexts.push_back(std::move(item_context));
		}
	}

	bool valid = true;

	if (match_count < min_contains) {
		context.add_error(vformat("Array contains %d matching items, but at least %d are required", match_count, min_contains), "minContains", target);
		valid = false;
	}

	if (max_contains >= 0 && match_count > max_contains) {
		context.add_error(vformat("Array contains %d matching items, but at most %d are allowed", match_count, max_contains), "maxContains", target);
		valid = false;
	}

	if (valid) {
		for (size_t k = 0; k < matched_indices.size(); k++) {
			context.mark_item_evaluated(matched_indices[k]);
			context.merge_evaluation_data(matched_contexts[k]);
		}
	}

	return valid;
}

String ContainsRule::get_description() const {
	if (!item_rule) {
		return "contains(null)";
	}
	return vformat("contains(%s)", item_rule->get_description());
}
