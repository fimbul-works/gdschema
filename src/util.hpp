#pragma once
#include "selector/selector.hpp"

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

	/**
	 * @brief Helper to get variant type name for error messages
	 * @param value The variant to analyze
	 * @return Type name string
	 */
	static String get_variant_json_type(const Variant &value) {
		switch (value.get_type()) {
			case Variant::NIL:
				return "null";
			case Variant::BOOL:
				return "boolean";
			case Variant::INT:
				return "integer";
			case Variant::FLOAT:
				if (value.operator double() == static_cast<double>(static_cast<int64_t>(value.operator double()))) {
					return "integer"; // Float with no fractional part is considered integer
				}
				return "number";
			case Variant::STRING:
			case Variant::STRING_NAME:
				return "string";
			case Variant::ARRAY:
			case Variant::PACKED_COLOR_ARRAY:
			case Variant::PACKED_FLOAT32_ARRAY:
			case Variant::PACKED_FLOAT64_ARRAY:
			case Variant::PACKED_INT32_ARRAY:
			case Variant::PACKED_INT64_ARRAY:
			case Variant::PACKED_STRING_ARRAY:
			case Variant::PACKED_VECTOR2_ARRAY:
			case Variant::PACKED_VECTOR3_ARRAY:
			case Variant::PACKED_VECTOR4_ARRAY:
				return "array";
			case Variant::DICTIONARY:
			case Variant::OBJECT:
				return "object";
			default:
				return "unknown";
		}
	}

	/**
	 * @brief Helper to convert numeric variant to double safely
	 * @param value The variant to convert
	 * @param out_value Output double value
	 * @return True if conversion successful
	 */
	static bool try_get_numeric_value(const Variant &value, double &out_value) {
		if (value.get_type() == Variant::INT) {
			out_value = static_cast<double>(value.operator int64_t());
			return true;
		} else if (value.get_type() == Variant::FLOAT) {
			out_value = value.operator double();
			return true;
		}
		return false;
	}

	/**
	 * @brief Helper to convert variant to non-negative integer safely
	 * @param value The variant to convert
	 * @param out_value Output integer value
	 * @return True if conversion successful and value is non-negative
	 */
	static bool try_get_non_negative_int(const Variant &value, int64_t &out_value) {
		if (value.get_type() == Variant::INT) {
			int64_t int_val = value.operator int64_t();
			if (int_val >= 0) {
				out_value = int_val;
				return true;
			}
		} else if (value.get_type() == Variant::FLOAT) {
			double float_val = value.operator double();
			if (float_val >= 0 && float_val == static_cast<double>(static_cast<int64_t>(float_val))) {
				out_value = static_cast<int64_t>(float_val);
				return true;
			}
		}
		return false;
	}
};

extern const int MAX_VALIDATION_DEPTH;

} //namespace godot
