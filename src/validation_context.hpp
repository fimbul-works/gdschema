#pragma once

#include "hashers.hpp"
#include "validation_error.hpp"

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/variant.hpp>

#include <godot_cpp/variant/utility_functions.hpp>
#include <unordered_set>
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

	std::unordered_set<StringName, StringNameHasher, StringNameEqual> evaluated_properties;
	std::unordered_set<int64_t> evaluated_items;

	std::vector<Ref<Schema>> dynamic_scope;
	int validation_depth;

public:
	/**
	 * @brief Constructor
	 * @param schema Source Schema (can be null)
	 * @param inst_parts Current instance path parts
	 * @param sch_parts Current Schema path parts
	 * @param depth Current recursion depth
	 */
	ValidationContext(const Schema *schema = nullptr,
			const PackedStringArray &inst_parts = PackedStringArray(),
			const PackedStringArray &sch_parts = PackedStringArray(),
			int depth = 0);

	/**
	 * @brief Copy Constructor
	 */
	ValidationContext(const ValidationContext &other);

	/**
	 * @brief Assignment Operator
	 */
	ValidationContext &operator=(const ValidationContext &other);

	/**
	 * @brief Destructor
	 */
	~ValidationContext();

	/**
	 * @brief Creates a child context for validating a sub-instance
	 * @param segment Path segment to append to instance path
	 * @return New validation context with updated instance path
	 */
	ValidationContext create_child_instance(const String &segment) const;

	/**
	 * @brief Creates a child context for a sub-schema
	 * @param segment Path segment to append to Schema path
	 * @return New validation context with updated Schema path
	 */
	ValidationContext create_child_schema(const String &segment) const;

	/**
	 * @brief Creates a child context with both paths updated
	 * @param instance_segment Instance path segment
	 * @param schema_segment Schema path segment
	 * @return New validation context
	 */
	ValidationContext create_child_context(const String &instance_segment, const String &schema_segment = "") const;

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
	 * @brief Marks a property as evaluated at the current instance path
	 * @param property The property name
	 */
	void mark_property_evaluated(const StringName &property) {
		evaluated_properties.insert(property);
	}

	/**
	 * @brief Marks an array item as evaluated at the current instance path
	 * @param index The item index
	 */
	void mark_item_evaluated(int64_t index) {
		evaluated_items.insert(index);
	}

	/**
	 * @brief Checks if a property has been evaluated
	 * @param property The property name
	 * @return True if evaluated
	 */
	bool is_property_evaluated(const StringName &property) const {
		return evaluated_properties.find(property) != evaluated_properties.end();
	}

	/**
	 * @brief Checks if an array item has been evaluated
	 * @param index The item index
	 * @return True if evaluated
	 */
	bool is_item_evaluated(int64_t index) const {
		return evaluated_items.find(index) != evaluated_items.end();
	}

	/**
	 * @brief Merges evaluation data from another context (for applicators)
	 * @param other The other context
	 */
	void merge_evaluation_data(const ValidationContext &other) {
		evaluated_properties.insert(other.evaluated_properties.begin(), other.evaluated_properties.end());
		evaluated_items.insert(other.evaluated_items.begin(), other.evaluated_items.end());
	}

	/**
	 * @brief Adds a schema to the dynamic scope
	 * @param schema The schema being entered
	 */
	void push_dynamic_scope(const Ref<Schema> &schema);

	/**
	 * @brief Gets the current dynamic scope
	 * @return List of schemas in the dynamic scope
	 */
	const std::vector<Ref<Schema>> &get_dynamic_scope() const;

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
	 * @brief Gets the current Schema path
	 * @return Schema path string
	 */
	String get_schema_path() const {
		if (schema_path_parts.is_empty()) {
			return String();
		}
		return "/" + String("/").join(schema_path_parts);
	}

	/**
	 * @brief Gets the source Schema reference
	 * @return Source Schema (can be null)
	 */
	const Schema *get_source_schema() const { return source_schema; }

	/**
	 * @brief Gets the current validation depth
	 */
	int get_validation_depth() const { return validation_depth; }

	/**
	 * @brief Creates a context with incremented depth for reference resolution
	 */
	ValidationContext with_incremented_depth() const;

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
