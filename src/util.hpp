#include <godot_cpp/variant/variant.hpp>

namespace godot {

class SchemaUtil {
public:
	static int64_t get_array_size(const Variant &value) {
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

	static Variant get_array_item(const Variant &array, int index) {
		switch (array.get_type()) {
			case Variant::ARRAY:
				return array.operator Array()[index];
			case Variant::PACKED_BYTE_ARRAY:
				return Variant(array.operator PackedByteArray()[index]);
			case Variant::PACKED_COLOR_ARRAY:
				return Variant(array.operator PackedColorArray()[index]);
			case Variant::PACKED_FLOAT32_ARRAY:
				return Variant(array.operator PackedFloat32Array()[index]);
			case Variant::PACKED_FLOAT64_ARRAY:
				return Variant(array.operator PackedFloat64Array()[index]);
			case Variant::PACKED_INT32_ARRAY:
				return Variant(array.operator PackedInt32Array()[index]);
			case Variant::PACKED_INT64_ARRAY:
				return Variant(array.operator PackedInt64Array()[index]);
			case Variant::PACKED_STRING_ARRAY:
				return Variant(array.operator PackedStringArray()[index]);
			case Variant::PACKED_VECTOR2_ARRAY:
				return Variant(array.operator PackedVector2Array()[index]);
			case Variant::PACKED_VECTOR3_ARRAY:
				return Variant(array.operator PackedVector3Array()[index]);
			case Variant::PACKED_VECTOR4_ARRAY:
				return Variant(array.operator PackedVector4Array()[index]);
			default:
				return Variant(); // Invalid array type
		}
	}
};

} //namespace godot
