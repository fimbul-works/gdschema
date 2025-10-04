#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/classes/weak_ref.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/string_name.hpp>
#include <godot_cpp/variant/variant.hpp>

namespace godot {

/**
 * @struct SchemaCompileError
 * @brief Represents a single compilation error encountered during validator construction
 */
struct SchemaCompileError {
	PackedStringArray schema_path_parts; // Path components as array (e.g., ["properties", "user", "minimum"])
	String message; // Human-readable error message

	SchemaCompileError(const String &msg, const PackedStringArray &path_parts = PackedStringArray()) :
			schema_path_parts(path_parts), message(msg) {}

	/**
	 * @brief Convenience constructor for single path part
	 * @param msg Error message
	 * @param single_path_part Single path part (e.g., "minimum")
	 */
	SchemaCompileError(const String &msg, const String &single_path_part) : message(msg) {
		if (!single_path_part.is_empty()) {
			schema_path_parts.push_back(single_path_part);
		}
	}

	/**
	 * @brief Converts path parts to JSON Pointer style string
	 * @return String like "/properties/user/minimum"
	 */
	String get_path_string() const {
		if (schema_path_parts.is_empty()) {
			return String();
		}
		return "/" + String("/").join(schema_path_parts);
	}

	/**
	 * @brief Convert to Dictionary for debugging/GDScript access
	 * @return Dictionary with keys: schema_path_parts, schema_path_string, message
	 */
	Dictionary to_dict() const {
		Dictionary result;
		result["schema_path_parts"] = schema_path_parts;
		result["schema_path_string"] = get_path_string();
		result["message"] = message;
		return result;
	}
};

} //namespace godot
