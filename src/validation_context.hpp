#pragma once

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
 * @struct ValidationError
 * @brief Represents a single validation error
 */
struct ValidationError {
	String instance_path; // Path to invalid data: "/user/age"
	String schema_path; // Path to violated rule: "/properties/user/minimum"
	String message; // Human-readable error message
	String keyword; // Schema keyword that failed
	Variant invalid_value; // The actual invalid value

	ValidationError(
			const String &msg,
			const String &inst_path = "",
			const String &sch_path = "",
			const String &kw = "",
			const Variant &value = Variant()) :
			message(msg), instance_path(inst_path), schema_path(sch_path), keyword(kw), invalid_value(value) {}

	// Convert to Dictionary for GDScript access
	Dictionary to_dict() const {
		Dictionary result;
		result["instance_path"] = instance_path;
		result["schema_path"] = schema_path;
		result["message"] = message;
		result["keyword"] = keyword;
		result["invalid_value"] = invalid_value;
		return result;
	}
};

/**
 * @class ValidationContext
 * @brief Context for tracking validation state and collecting errors
 */
class ValidationContext {
private:
	String instance_path;
	String schema_path;
	const Schema *source_schema; // Weak reference to avoid cycles
	std::vector<ValidationError> errors;

public:
	/**
	 * @brief Constructor
	 * @param schema Source schema (can be null)
	 * @param inst_path Current instance path
	 * @param sch_path Current schema path
	 */
	ValidationContext(
			const Schema *schema = nullptr,
			const String &inst_path = "",
			const String &sch_path = "") :
			source_schema(schema), instance_path(inst_path), schema_path(sch_path) {}

	/**
	 * @brief Creates a child context for validating a sub-instance
	 * @param path_segment Path segment to append to instance path
	 * @return New validation context with updated instance path
	 */
	ValidationContext create_child_instance(const String &path_segment) const {
		String new_path = build_path(instance_path, path_segment);
		return ValidationContext(source_schema, new_path, schema_path);
	}

	/**
	 * @brief Creates a child context for a sub-schema
	 * @param path_segment Path segment to append to schema path
	 * @return New validation context with updated schema path
	 */
	ValidationContext create_child_schema(const String &path_segment) const {
		String new_path = build_path(schema_path, path_segment);
		return ValidationContext(source_schema, instance_path, new_path);
	}

	/**
	 * @brief Creates a child context with both paths updated
	 * @param instance_segment Instance path segment
	 * @param schema_segment Schema path segment
	 * @return New validation context
	 */
	ValidationContext create_child_context(
			const String &instance_segment,
			const String &schema_segment = "") const {
		String new_inst_path = build_path(instance_path, instance_segment);
		String new_sch_path = schema_segment.is_empty() ? schema_path : build_path(schema_path, schema_segment);
		return ValidationContext(source_schema, new_inst_path, new_sch_path);
	}

	/**
	 * @brief Adds a validation error
	 * @param message Error message
	 * @param keyword Schema keyword that failed (optional)
	 * @param invalid_value The value that failed validation (optional)
	 */
	void add_error(
			const String &message,
			const String &keyword = "",
			const Variant &invalid_value = Variant()) {
		errors.emplace_back(message, instance_path, schema_path, keyword, invalid_value);
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
	 * @brief Gets a formatted summary of all validation errors
	 * @return Multi-line string with all errors
	 */
	String get_error_summary() const {
		if (errors.empty()) {
			return "Validation successful - no errors";
		}

		String summary = vformat("Schema validation failed with %d error(s):\n", (int)errors.size());

		for (size_t i = 0; i < errors.size(); i++) {
			const auto &error = errors[i];

			summary += vformat("  [%d] ", (int)i + 1);

			if (!error.instance_path.is_empty()) {
				summary += vformat("At '%s': ", error.instance_path);
			}

			summary += error.message;

			if (!error.keyword.is_empty()) {
				summary += vformat(" (%s)", error.keyword);
			}

			if (i < errors.size() - 1) {
				summary += "\n";
			}
		}

		return summary;
	}

	/**
	 * @brief Gets the current instance path
	 * @return Instance path string
	 */
	String get_instance_path() const { return instance_path; }

	/**
	 * @brief Gets the current schema path
	 * @return Schema path string
	 */
	String get_schema_path() const { return schema_path; }

	/**
	 * @brief Gets the source schema reference
	 * @return Source schema (can be null)
	 */
	const Schema *get_source_schema() const { return source_schema; }

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
