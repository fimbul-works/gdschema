#include "array_items_selector.hpp"

using namespace godot;

std::vector<SelectionTarget> ArrayItemsSelector::select_targets(const Variant &instance, const ValidationContext &context) const {
	std::vector<SelectionTarget> targets;

	if (instance.get_type() == Variant::ARRAY) {
		Array arr = instance.operator Array();
		for (int i = 0; i < arr.size(); i++) {
			targets.emplace_back(arr[i], String::num(i));
		}
	} else if (instance.get_type() == Variant::PACKED_BYTE_ARRAY) {
		PackedByteArray arr = instance.operator PackedByteArray();
		for (int i = 0; i < arr.size(); i++) {
			targets.emplace_back(Variant(arr[i]), String::num(i));
		}
	} else if (instance.get_type() == Variant::PACKED_COLOR_ARRAY) {
		PackedColorArray arr = instance.operator PackedColorArray();
		for (int i = 0; i < arr.size(); i++) {
			targets.emplace_back(Variant(arr[i]), String::num(i));
		}
	} else if (instance.get_type() == Variant::PACKED_FLOAT32_ARRAY) {
		PackedFloat32Array arr = instance.operator PackedFloat32Array();
		for (int i = 0; i < arr.size(); i++) {
			targets.emplace_back(Variant(arr[i]), String::num(i));
		}
	} else if (instance.get_type() == Variant::PACKED_FLOAT64_ARRAY) {
		PackedFloat64Array arr = instance.operator PackedFloat64Array();
		for (int i = 0; i < arr.size(); i++) {
			targets.emplace_back(Variant(arr[i]), String::num(i));
		}
	} else if (instance.get_type() == Variant::PACKED_INT32_ARRAY) {
		PackedInt32Array arr = instance.operator PackedInt32Array();
		for (int i = 0; i < arr.size(); i++) {
			targets.emplace_back(Variant(arr[i]), String::num(i));
		}
	} else if (instance.get_type() == Variant::PACKED_INT64_ARRAY) {
		PackedInt64Array arr = instance.operator PackedInt64Array();
		for (int i = 0; i < arr.size(); i++) {
			targets.emplace_back(Variant(arr[i]), String::num(i));
		}
	} else if (instance.get_type() == Variant::PACKED_STRING_ARRAY) {
		PackedStringArray arr = instance.operator PackedStringArray();
		for (int i = 0; i < arr.size(); i++) {
			targets.emplace_back(Variant(arr[i]), String::num(i));
		}
	} else if (instance.get_type() == Variant::PACKED_VECTOR2_ARRAY) {
		PackedVector2Array arr = instance.operator PackedVector2Array();
		for (int i = 0; i < arr.size(); i++) {
			targets.emplace_back(Variant(arr[i]), String::num(i));
		}
	} else if (instance.get_type() == Variant::PACKED_VECTOR3_ARRAY) {
		PackedVector3Array arr = instance.operator PackedVector3Array();
		for (int i = 0; i < arr.size(); i++) {
			targets.emplace_back(Variant(arr[i]), String::num(i));
		}
	} else if (instance.get_type() == Variant::PACKED_VECTOR4_ARRAY) {
		PackedVector4Array arr = instance.operator PackedVector4Array();
		for (int i = 0; i < arr.size(); i++) {
			targets.emplace_back(Variant(arr[i]), String::num(i));
		}
	}

	return targets;
}
