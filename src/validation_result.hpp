#pragma once

#include "validation_error.hpp"

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/variant.hpp>

#include <vector>

namespace godot {

// Forward declaration
class ValidationContext;

/**
 * @class SchemaValidationResult
 * @brief Clean, user-facing validation result with flat error list
 *
 * This provides a simple interface for validation results without exposing
 * the internal ValidationContext tree structure complexity.
 */
class SchemaValidationResult : public RefCounted {
	GDCLASS(SchemaValidationResult, RefCounted)

private:
	std::vector<ValidationError> errors;
	bool validation_succeeded;

protected:
	static void _bind_methods();

public:
	/**
	 * @brief Default constructor - creates successful result
	 */
	SchemaValidationResult() :
			validation_succeeded(true) {}

	/**
	 * @brief Creates result from internal ValidationContext
	 * @param context The internal validation context with tree structure
	 * @return New result with flattened error list
	 */
	static Ref<SchemaValidationResult> from_context(const ValidationContext &context);

	/**
	 * @brief Creates successful validation result
	 * @return Result indicating validation passed
	 */
	static Ref<SchemaValidationResult> success();

	// ========== Status Checking ==========

	/**
	 * @brief Checks if validation succeeded
	 * @return True if no errors occurred
	 */
	bool is_valid() const { return validation_succeeded && errors.empty(); }

	/**
	 * @brief Checks if there are validation errors
	 * @return True if one or more errors occurred
	 */
	bool has_errors() const { return !errors.empty(); }

	/**
	 * @brief Gets the number of validation errors
	 * @return Error count
	 */
	int error_count() const { return errors.size(); }

	// ========== Error Access ==========

	/**
	 * @brief Gets all errors as Array of Dictionaries for GDScript
	 * @return Array where each element is an error Dictionary
	 */
	Array get_errors() const;

	/**
	 * @brief Gets error at specific index
	 * @param index Error index (0-based)
	 * @return Error Dictionary or empty Dictionary if index out of range
	 */
	Dictionary get_error(int index) const;

	/**
	 * @brief Gets error message at specific index
	 * @param index Error index
	 * @return Error message or empty string if index out of range
	 */
	String get_error_message(int index) const;

	/**
	 * @brief Gets error path as string at specific index
	 * @param index Error index
	 * @return Path string like "/user/name" or empty if index out of range
	 */
	String get_error_path(int index) const;

	/**
	 * @brief Gets error path as array at specific index
	 * @param index Error index
	 * @return Path array like ["user", "name"] or empty array if index out of range
	 */
	PackedStringArray get_error_path_array(int index) const;

	/**
	 * @brief Gets the constraint that was violated at specific index
	 * @param index Error index
	 * @return Constraint name like "minLength" or empty if index out of range
	 */
	String get_error_constraint(int index) const;

	/**
	 * @brief Gets the invalid value at specific index
	 * @param index Error index
	 * @return The value that failed validation or null Variant if index out of range
	 */
	Variant get_error_value(int index) const;

	// ========== Summary and Formatting ==========

	/**
	 * @brief Gets formatted summary of all validation errors
	 * @return Multi-line string with all errors or success message
	 */
	String get_summary() const;

	/**
	 * @brief Gets brief summary for logging/debugging
	 * @return Single line summary
	 */
	String get_brief_summary() const;

	/**
	 * @brief Gets all error paths as array of strings
	 * @return Array of path strings for all errors
	 */
	PackedStringArray get_all_error_paths() const;

	/**
	 * @brief Gets all unique constraints that were violated
	 * @return Array of unique constraint names
	 */
	PackedStringArray get_violated_constraints() const;

	// ========== Debugging ==========

	/**
	 * @brief String representation for debugging
	 */
	String _to_string() const;

	/**
	 * @brief Add error to list
	 * @param error The error to add
	 */
	void add_error(const ValidationError &error);
};

} // namespace godot
