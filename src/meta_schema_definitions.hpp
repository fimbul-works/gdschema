#pragma once

#include "schema.hpp"
#include "validation_result.hpp"

#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/variant.hpp>

namespace godot {

/**
 * @class MetaSchemaDefinitions
 * @brief Contains JSON Schema definitions for validating JSON Schema documents
 *
 * This implements the brilliant idea of using our validation system to validate schemas themselves!
 */
class MetaSchemaDefinitions {
private:
	static Ref<Schema> draft7_meta_schema;
	static bool meta_schema_initialized;

public:
	/**
	 * Check if the meta schema is initialized
	 */
	static bool is_meta_initialized() { return meta_schema_initialized; }

	/**
	 * @brief Gets the compiled draft-7 meta-schema (initialized without validation)
	 */
	static Ref<Schema> get_draft7_meta_schema_instance();

	/**
	 * @brief Gets the meta-schema for JSON Schema Draft-7
	 * @return Dictionary containing the meta-schema definition
	 */
	static Dictionary get_draft7_meta_schema();

	/**
	 * @brief Gets the meta-schema for string constraints
	 * @return Dictionary for validating string constraint objects
	 */
	static Dictionary get_string_constraints_schema();

	/**
	 * @brief Gets the meta-schema for numeric constraints
	 * @return Dictionary for validating numeric constraint objects
	 */
	static Dictionary get_numeric_constraints_schema();

	/**
	 * @brief Gets the meta-schema for array constraints
	 * @return Dictionary for validating array constraint objects
	 */
	static Dictionary get_array_constraints_schema();

	/**
	 * @brief Gets the meta-schema for object constraints
	 * @return Dictionary for validating object constraint objects
	 */
	static Dictionary get_object_constraints_schema();

	/**
	 * @brief Validates a schema definition using meta-validation
	 * @param schema_def The schema definition to validate
	 * @return Validation context with results
	 */
	static Ref<SchemaValidationResult> validate_schema_definition(const Dictionary &schema_def);
};

} // namespace godot
