#include "contains_rule.hpp"
#include "../util.hpp"
#include "../validation_context.hpp"

using namespace godot;

bool ContainsRule::validate(const Variant &target, ValidationContext &context) const {
	int64_t array_size = SchemaUtil::get_array_size(target);
	if (array_size < 0) {
		return true; // Rule doesn't apply to non-arrays
	}

	if (array_size == 0) {
		context.add_error("Empty array cannot contain any items", "contains", target);
		return false;
	}

	// Handle case where there's no sub-rule (shouldn't happen in normal flow)
	if (!item_rule) {
		context.add_error("Array does not contain any item matching the required schema", "contains", target);
		return false;
	}

	// Handle empty schema (contains: true or contains: {})
	if (item_rule->is_empty()) {
		// Empty schema matches everything - any non-empty array passes
		return true;
	}

	// Normal case: check if at least one item validates against the schema
	bool found_match = false;

	for (int64_t i = 0; i < array_size; i++) {
		Variant item = SchemaUtil::get_array_item(target, i);

		// Create a temporary context for this item
		// We don't want to propagate errors from failed attempts to the main context
		ValidationContext item_context = context.create_child_instance(String::num(i));

		if (item_rule->validate(item, item_context)) {
			found_match = true;
			break; // Success - we found at least one matching item
		}
	}

	if (!found_match) {
		context.add_error(
				vformat("Array with %d items does not contain any item matching the required schema", array_size),
				"contains",
				target);
		return false;
	}

	return true;
}

String ContainsRule::get_description() const {
	if (!item_rule) {
		return "contains(null)";
	}
	return vformat("contains(%s)", item_rule->get_description());
}
