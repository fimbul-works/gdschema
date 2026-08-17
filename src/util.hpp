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

	/**
	 * @brief Check if a value is a String or StringName.
	 * @param value The variant to check
	 * @return True if value is a String or StringName
	 */
	static inline bool is_string(const Variant &value) {
		return value.get_type() == Variant::STRING || value.get_type() == Variant::STRING_NAME;
	}

	/**
	 * @brief Resolves a relative URI against a base URI according to RFC 3986.
	 */
	static String resolve_uri(const String &base_uri, const String &relative_uri) {
		String rel = relative_uri.strip_edges();
		if (rel.is_empty()) {
			return base_uri;
		}

		// 1. If rel is an absolute URI with a scheme (e.g. "http:", "https:", "urn:", "file:"), return rel
		int colon_pos = rel.find(":");
		if (colon_pos > 0) {
			bool is_scheme = true;
			for (int i = 0; i < colon_pos; i++) {
				char32_t c = rel[i];
				if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (i > 0 && ((c >= '0' && c <= '9') || c == '+' || c == '-' || c == '.')))) {
					is_scheme = false;
					break;
				}
			}
			if (is_scheme) {
				return rel;
			}
		}

		String base = base_uri.strip_edges();
		if (base.is_empty()) {
			return rel;
		}

		// Remove fragment from base URI
		int base_frag = base.find("#");
		if (base_frag != -1) {
			base = base.substr(0, base_frag);
		}

		// 2. If rel is a fragment only ("#...")
		if (rel.begins_with("#")) {
			return base + rel;
		}

		// Extract scheme and authority from base
		String scheme_authority;
		String base_path;

		int scheme_pos = base.find("://");
		if (scheme_pos != -1) {
			int authority_end = base.find("/", scheme_pos + 3);
			if (authority_end != -1) {
				scheme_authority = base.substr(0, authority_end);
				base_path = base.substr(authority_end);
			} else {
				scheme_authority = base;
				base_path = "/";
			}
		} else {
			// URN or other scheme without authority (e.g., urn:uuid:...)
			int u_colon = base.find(":");
			if (u_colon != -1) {
				scheme_authority = base.substr(0, u_colon + 1);
				base_path = base.substr(u_colon + 1);
			} else {
				base_path = base;
			}
		}

		// 3. If rel starts with "/" (absolute path reference)
		if (rel.begins_with("/")) {
			return scheme_authority + rel;
		}

		// 4. Relative path: combine with base_path directory
		int last_slash = base_path.rfind("/");
		String dir = (last_slash != -1) ? base_path.substr(0, last_slash + 1) : "/";
		String combined_path = dir + rel;

		// Normalize dot segments in combined_path
		PackedStringArray segments = combined_path.split("/");
		std::vector<String> normalized_segments;

		for (int i = 0; i < segments.size(); i++) {
			String seg = segments[i];
			if (seg == "." || (seg.is_empty() && i > 0 && i < segments.size() - 1)) {
				continue;
			} else if (seg == "..") {
				if (!normalized_segments.empty() && normalized_segments.back() != "..") {
					normalized_segments.pop_back();
				}
			} else {
				normalized_segments.push_back(seg);
			}
		}

		String result_path = "";
		for (size_t i = 0; i < normalized_segments.size(); i++) {
			if (i > 0) {
				result_path += "/";
			}
			result_path += normalized_segments[i];
		}

		if (result_path.is_empty() || !result_path.begins_with("/")) {
			result_path = "/" + result_path;
		}

		return scheme_authority + result_path;
	}
};

extern const int MAX_VALIDATION_DEPTH;

} //namespace godot
