#include "max_items_rule.hpp"

using namespace godot;

bool MaxItemsRule::validate(const Variant &target, ValidationContext &context) const {
	int64_t actual_size = get_array_size(target);
	if (actual_size < 0) {
		return true; // Rule doesn't apply to non-arrays
	}

	if (actual_size > max_items) {
		context.add_error(
				vformat("Array has %d items but maximum is %d", actual_size, max_items),
				"maxItems",
				target);
		return false;
	}

	return true;
}

int64_t MaxItemsRule::get_array_size(const Variant &value) const {
	switch (value.get_type()) {
		case Variant::ARRAY:
			return value.operator Array().size();
		case Variant::PACKED_BYTE_ARRAY:
			return value.operator PackedByteArray().size();
		case Variant::PACKED_COLOR_ARRAY:
			return value.operator PackedColorArray().size();
		case Variant::PACKED_FLOAT32_ARRAY:
			return value.operator PackedFloat32Array().size();
		case Variant::PACKED_FLOAT64_ARRAY:
			return value.operator PackedFloat64Array().size();
		case Variant::PACKED_INT32_ARRAY:
			return value.operator PackedInt32Array().size();
		case Variant::PACKED_INT64_ARRAY:
			return value.operator PackedInt64Array().size();
		case Variant::PACKED_STRING_ARRAY:
			return value.operator PackedStringArray().size();
		case Variant::PACKED_VECTOR2_ARRAY:
			return value.operator PackedVector2Array().size();
		case Variant::PACKED_VECTOR3_ARRAY:
			return value.operator PackedVector3Array().size();
		case Variant::PACKED_VECTOR4_ARRAY:
			return value.operator PackedVector4Array().size();
		default:
			return -1; // Not an array
	}
}
