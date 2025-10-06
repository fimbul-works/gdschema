#include "array_item_selector.hpp"

using namespace godot;

std::vector<SelectionTarget> ArrayItemSelector::select_targets(const Variant &instance, const ValidationContext &context) const {
	std::vector<SelectionTarget> targets;

	if (instance.get_type() == Variant::ARRAY) {
		Array arr = instance.operator Array();
		if (index >= 0 && index < arr.size()) {
			targets.emplace_back(arr[index], String::num(index));
		}
	} else if (instance.get_type() == Variant::PACKED_BYTE_ARRAY) {
		PackedByteArray arr = instance.operator PackedByteArray();
		if (index >= 0 && index < arr.size()) {
			targets.emplace_back(arr[index], String::num(index));
		}
	} else if (instance.get_type() == Variant::PACKED_COLOR_ARRAY) {
		PackedColorArray arr = instance.operator PackedColorArray();
		if (index >= 0 && index < arr.size()) {
			targets.emplace_back(arr[index], String::num(index));
		}
	} else if (instance.get_type() == Variant::PACKED_FLOAT32_ARRAY) {
		PackedFloat32Array arr = instance.operator PackedFloat32Array();
		if (index >= 0 && index < arr.size()) {
			targets.emplace_back(arr[index], String::num(index));
		}
	} else if (instance.get_type() == Variant::PACKED_FLOAT64_ARRAY) {
		PackedFloat64Array arr = instance.operator PackedFloat64Array();
		if (index >= 0 && index < arr.size()) {
			targets.emplace_back(arr[index], String::num(index));
		}
	} else if (instance.get_type() == Variant::PACKED_INT32_ARRAY) {
		PackedInt32Array arr = instance.operator PackedInt32Array();
		if (index >= 0 && index < arr.size()) {
			targets.emplace_back(arr[index], String::num(index));
		}
	} else if (instance.get_type() == Variant::PACKED_INT64_ARRAY) {
		PackedInt64Array arr = instance.operator PackedInt64Array();
		if (index >= 0 && index < arr.size()) {
			targets.emplace_back(arr[index], String::num(index));
		}
	} else if (instance.get_type() == Variant::PACKED_STRING_ARRAY) {
		PackedStringArray arr = instance.operator PackedStringArray();
		if (index >= 0 && index < arr.size()) {
			targets.emplace_back(arr[index], String::num(index));
		}
	} else if (instance.get_type() == Variant::PACKED_VECTOR2_ARRAY) {
		PackedVector2Array arr = instance.operator PackedVector2Array();
		if (index >= 0 && index < arr.size()) {
			targets.emplace_back(arr[index], String::num(index));
		}
	} else if (instance.get_type() == Variant::PACKED_VECTOR3_ARRAY) {
		PackedVector3Array arr = instance.operator PackedVector3Array();
		if (index >= 0 && index < arr.size()) {
			targets.emplace_back(arr[index], String::num(index));
		}
	} else if (instance.get_type() == Variant::PACKED_VECTOR4_ARRAY) {
		PackedVector4Array arr = instance.operator PackedVector4Array();
		if (index >= 0 && index < arr.size()) {
			targets.emplace_back(arr[index], String::num(index));
		}
	}

	return targets;
}
