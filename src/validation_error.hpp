#pragma once

#include <godot_cpp/variant/variant.hpp>

namespace godot {

/**
 * @struct ValidationError
 * @brief Represents a single validation error with detailed path information
 */
struct ValidationError {
	String message;
	PackedStringArray instance_path_parts;
	PackedStringArray schema_path_parts;
	String keyword;
	Variant invalid_value;

	ValidationError(const String &msg,
			const PackedStringArray &inst_parts = PackedStringArray(),
			const PackedStringArray &sch_parts = PackedStringArray(),
			const String &kw = "",
			const Variant &value = Variant()) :
			message(msg), instance_path_parts(inst_parts), schema_path_parts(sch_parts), keyword(kw), invalid_value(value) {}

	/**
	 * @brief Get the instance path as a string
	 * @return Instance path string (e.g. "/user/name")
	 */
	String get_instance_path() const {
		if (instance_path_parts.is_empty()) {
			return String();
		}
		return "/" + String("/").join(instance_path_parts);
	}

	/**
	 * @brief Get the Schema path as a string
	 * @return Schema path string (e.g. "/properties/user/properties/name/minLength")
	 */
	String get_schema_path() const {
		if (schema_path_parts.is_empty()) {
			return String();
		}
		return "/" + String("/").join(schema_path_parts);
	}

	/**
	 * @brief Convert to Dictionary for GDScript access
	 * @return Dictionary with all error information
	 */
	Dictionary to_dict() const {
		Dictionary result;
		result["message"] = message;
		result["instance_path"] = get_instance_path();
		result["instance_path_array"] = instance_path_parts;
		result["schema_path"] = get_schema_path();
		result["schema_path_array"] = schema_path_parts;
		result["keyword"] = keyword;
		result["invalid_value"] = invalid_value;
		return result;
	}
};

} //namespace godot
