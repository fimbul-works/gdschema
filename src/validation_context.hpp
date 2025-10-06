#pragma once

#include "validation_error.hpp"

#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/variant.hpp>

#include <godot_cpp/variant/utility_functions.hpp>
#include <vector>

namespace godot {

// Forward declarations
class Schema;

/**
 * @class ValidationContext
 * @brief Context for tracking validation state and collecting errors
 */
class ValidationContext {
private:
	PackedStringArray instance_path_parts;
	PackedStringArray schema_path_parts;
	const Schema *source_schema; // Weak reference to avoid cycles
	std::vector<ValidationError> errors;
	Dictionary custom_data;

public:
	/**
	 * @brief Constructor
	 * @param schema Source schema (can be null)
	 * @param inst_path Current instance path
	 * @param sch_path Current schema path
	 */
	ValidationContext(const Schema *schema = nullptr,
			const PackedStringArray &inst_parts = PackedStringArray(),
			const PackedStringArray &sch_parts = PackedStringArray()) :
			source_schema(schema), instance_path_parts(inst_parts), schema_path_parts(sch_parts) {}

	/**
	 * @brief Creates a child context for validating a sub-instance
	 * @param segment Path segment to append to instance path
	 * @return New validation context with updated instance path
	 */
	ValidationContext create_child_instance(const String &segment) const {
		PackedStringArray new_parts = instance_path_parts;
		if (!segment.is_empty()) {
			new_parts.push_back(segment);
		}
		return ValidationContext(source_schema, new_parts, schema_path_parts);
	}

	/**
	 * @brief Creates a child context for a sub-schema
	 * @param segment Path segment to append to schema path
	 * @return New validation context with updated schema path
	 */
	ValidationContext create_child_schema(const String &segment) const {
		PackedStringArray new_parts = schema_path_parts;
		if (!segment.is_empty()) {
			new_parts.push_back(segment);
		}
		return ValidationContext(source_schema, instance_path_parts, new_parts);
	}

	/**
	 * @brief Creates a child context with both paths updated
	 * @param instance_segment Instance path segment
	 * @param schema_segment Schema path segment
	 * @return New validation context
	 */
	ValidationContext create_child_context(const String &instance_segment, const String &schema_segment = "") const {
		PackedStringArray new_instance_parts = instance_path_parts;
		if (!instance_segment.is_empty()) {
			new_instance_parts.push_back(instance_segment);
		}

		PackedStringArray new_schema_parts = schema_path_parts;
		if (!schema_segment.is_empty()) {
			new_schema_parts.push_back(schema_segment);
		}
		return ValidationContext(source_schema, new_instance_parts, new_schema_parts);
	}

	/**
	 * @brief Adds a validation error
	 * @param message Error message
	 * @param keyword Schema keyword that failed (optional)
	 * @param invalid_value The value that failed validation (optional)
	 */
	void add_error(const String &message, const String &keyword = "",
			const Variant &invalid_value = Variant()) {
		errors.emplace_back(message, instance_path_parts, schema_path_parts,
				keyword, invalid_value);
	}

	/**
	 * @brief Merges errors from another context into this one
	 * @param other The other context to merge from
	 */
	void merge_errors(const ValidationContext &other) {
		errors.insert(errors.end(), other.errors.begin(), other.errors.end());
	}

	/**
	 * @brief Checks if validation was successful
	 * @return True if no errors occurred
	 */
	bool is_valid() const { return errors.empty(); }

	/**
	 * @brief Checks if there are validation errors
	 * @return True if one or more errors occurred
	 */
	bool has_errors() const { return !errors.empty(); }

	/**
	 * @brief Gets the number of validation errors
	 * @return Number of errors
	 */
	size_t error_count() const { return errors.size(); }

	/**
	 * @brief Gets all validation errors
	 * @return Vector of all errors
	 */
	const std::vector<ValidationError> &get_errors() const { return errors; }

	/**
	 * @brief Gets all errors as an Array of Dictionaries
	 * @return Array containing error information
	 */
	Array get_errors_as_array() const {
		Array result;
		result.resize(errors.size());
		for (size_t i = 0; i < errors.size(); i++) {
			result[i] = errors[i].to_dict();
		}
		return result;
	}

	/**
	 * @brief Gets the current instance path
	 * @return Instance path string
	 */
	String get_instance_path() const {
		if (instance_path_parts.is_empty()) {
			return String();
		}
		return "/" + String("/").join(instance_path_parts);
	}
	/**
	 * @brief Gets the current schema path
	 * @return Schema path string
	 */
	String get_schema_path() const {
		if (schema_path_parts.is_empty()) {
			return String();
		}
		return "/" + String("/").join(schema_path_parts);
	}

	/**
	 * @brief Gets the source schema reference
	 * @return Source schema (can be null)
	 */
	const Schema *get_source_schema() const { return source_schema; }

	/**
	 * @brief Sets custom data associated with this context
	 * @param key The key for the custom data
	 * @param value The value to store
	 */
	void set_custom_data(const String &key, const Variant &value) { custom_data[key] = value; }

	/**
	 * @brief Gets custom data associated with this context
	 * @param key The key for the custom data
	 * @param default_value Value to return if key not found
	 * @return The stored value or default if not found
	 */
	Variant get_custom_data(const String &key, const Variant &default_value = Variant()) const {
		return custom_data.get(key, default_value);
	}

	/**
	 * @brief Checks if custom data exists for a given key
	 * @param key The key to check
	 * @return True if the key exists
	 */
	bool has_custom_data(const String &key) const { return custom_data.has(key); }

	/**
	 * @brief Gets the JSON Schema type name for a Variant
	 * @param instance The variant to analyze
	 * @return Type name string
	 */
	static String get_variant_json_type(const Variant &instance) {
		switch (instance.get_type()) {
			case Variant::NIL:
				return "null";
			case Variant::BOOL:
				return "boolean";
			case Variant::INT:
				return "integer";
			case Variant::FLOAT:
				return "number";
			case Variant::STRING:
			case Variant::STRING_NAME:
				return "string";
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
				return "array";
			case Variant::DICTIONARY:
			case Variant::OBJECT:
				return "object";
			default:
				return "unknown";
		}
	}

private:
	/**
	 * @brief Builds a JSON Pointer style path
	 * @param base_path The base path
	 * @param segment The segment to append
	 * @return Combined path
	 */
	static String build_path(const String &base_path, const String &segment) {
		if (segment.is_empty()) {
			return base_path;
		}

		if (base_path.is_empty()) {
			return "/" + segment;
		}

		return base_path + String("/") + segment;
	}
};

} // namespace godot
