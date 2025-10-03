#include "unique_items_rule.hpp"
#include "../validation_context.hpp"

using namespace godot;

bool UniqueItemsRule::validate(const Variant &target, ValidationContext &context) const {
	if (target.get_type() != Variant::ARRAY) {
		return true; // Rule only applies to arrays
	}

	Array arr = target.operator Array();
	std::vector<Variant> seen_items;

	for (int64_t i = 0; i < arr.size(); i++) {
		Variant current_item = arr[i];

		// Check if we've seen this item before
		for (const Variant &seen_item : seen_items) {
			if (current_item.get_type() != seen_item.get_type()) {
				continue;
			}

			if (current_item == seen_item ||
					(should_hash_variant(current_item) && current_item.hash_compare(seen_item))) {
				context.add_error(vformat("Array contains duplicate item at index %d: %s", i, current_item.stringify()),
						"uniqueItems", target);
				return false;
			}
		}

		seen_items.push_back(current_item);
	}

	return true;
}

bool UniqueItemsRule::should_hash_variant(const Variant &value) const {
	switch (value.get_type()) {
		case Variant::ARRAY:
		case Variant::PACKED_BYTE_ARRAY:
		case Variant::PACKED_COLOR_ARRAY:
		case Variant::PACKED_FLOAT32_ARRAY:
		case Variant::PACKED_FLOAT64_ARRAY:
		case Variant::PACKED_INT32_ARRAY:
		case Variant::PACKED_INT64_ARRAY:
		case Variant::PACKED_STRING_ARRAY:
		case Variant::PACKED_VECTOR2_ARRAY:
		case Variant::PACKED_VECTOR3_ARRAY:
		case Variant::PACKED_VECTOR4_ARRAY:
		case Variant::DICTIONARY:
			return true;
		default:
			return false;
	}
}
