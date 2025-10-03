#include "max_items_rule.hpp"
#include "../util.hpp"

using namespace godot;

bool MaxItemsRule::validate(const Variant &target, ValidationContext &context) const {
	int64_t actual_size = SchemaUtil::get_array_size(target);
	if (actual_size < 0) {
		return true; // Rule doesn't apply to non-arrays
	}

	if (actual_size > max_items) {
		context.add_error(vformat("Array has %d items but maximum is %d", actual_size, max_items), "maxItems", target);
		return false;
	}

	return true;
}
