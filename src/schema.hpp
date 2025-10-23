#pragma once

#include "hashers.hpp"
#include "rule/rule_group.hpp"
#include "rule_factory.hpp"
#include "schema_compile_error.hpp"
#include "validation_result.hpp"

#include <godot_cpp/classes/mutex.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/string_name.hpp>
#include <godot_cpp/variant/variant.hpp>

#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

namespace godot {

// Forward reference
class RuleFactory;

/**
 * @class Schema
 * @brief Tree-based representation of JSON Schema nodes (refactored)
 *
 * This refactored version focuses purely on tree structure and metadata.
 * Validation logic is handled by the rule system with lazy compilation.
 *
 * @extends RefCounted
 */
class Schema : public RefCounted {
	GDCLASS(Schema, RefCounted)

public:
	/**
	 * @brief Schema node types
	 */
	enum SchemaType {
		SCHEMA_SCALAR, // String, number, boolean, null constraints
		SCHEMA_OBJECT, // Has properties, required, etc.
		SCHEMA_ARRAY, // Has items, minItems, etc.
		SCHEMA_LOGICAL, // anyOf, not, if, etc.
	};

private:
	// Tree structure
	Ref<Schema> root_schema; // null for root node
	StringName schema_path; // Path from root like "/properties/user"
	std::unordered_map<StringName, Ref<Schema>, StringNameHasher, StringNameEqual> children; // For object properties
	std::vector<Ref<Schema>> item_schemas; // For array items

	// Schema metadata (immutable after construction)
	Dictionary schema_definition;
	SchemaType schema_type;
	StringName schema_url;
	StringName schema_id;
	StringName title;
	StringName description;
	StringName comment;

	// Validation rules
	mutable std::shared_ptr<RuleGroup> rules;
	mutable std::vector<SchemaCompileError> compile_errors;
	mutable bool is_compiled;
	mutable Ref<Mutex> compilation_mutex;

	/**
	 * @brief Determines Schema type from definition
	 * @param dict The Schema definition dictionary
	 * @return Detected Schema type
	 */
	SchemaType detect_schema_type(const Dictionary &dict) const;

	/**
	 * @brief Recursively constructs child nodes from dictionary
	 * @param dict The dictionary to process
	 */
	void construct_children(const Dictionary &dict);

	/**
	 * @brief Creates a child Schema node from a dictionary
	 * @param child_schema The child Schema definition
	 * @param child_key The key or index identifying the child
	 * @return New child Schema instance
	 */
	Ref<Schema> create_schema_child(const Dictionary &child_schema, const StringName &child_key);

	/**
	 * @brief Creates a child Schema node if the key exists in the dictionary
	 * @param dict The parent dictionary
	 * @param key The key to check and create child for
	 */
	void create_schema_child_if_exists(const Dictionary &dict, const StringName &key);

	/**
	 * @brief Creates child schemas for logical keywords (anyOf, allOf, oneOf, not, if, then, else)
	 * @param dict The parent dictionary
	 * @param key The logical keyword
	 */
	void create_logical_children(const Dictionary &dict, const StringName &key);

	/**
	 * @brief Creates definitions child schemas from "definitions" or "$defs"
	 * @param dict The parent dictionary
	 * @param key The definitions keyword ("definitions" or "$defs")
	 */
	void create_definitions_children(const Dictionary &dict, const StringName &key);

	/**
	 * @brief Converts a Variant value to a Schema-compatible dictionary
	 * @param value The Variant value to convert
	 * @return Dictionary representation suitable for Schema validation
	 */
	Variant variant_to_schema_dict(const Variant &value) const;

	/**
	 * @brief Adds a compilation error to the list
	 * @param error_message The error message
	 */
	void add_compile_error(const String error_message);

	/**
	 * @brief Normalizes a JSON Pointer by removing fragments and cleaning up
	 * @param pointer The raw JSON pointer string
	 * @return Normalized pointer starting with "/"
	 */
	static String normalize_json_pointer(const String &pointer);

	/**
	 * @brief Parses a JSON Pointer into path segments
	 * @param pointer Normalized JSON pointer (starts with "/")
	 * @return Array of path segments
	 */
	static PackedStringArray parse_json_pointer(const String &pointer);

	/**
	 * @brief Unescapes JSON Pointer special characters
	 * @param segment A single path segment that may contain escapes
	 * @return Unescaped segment
	 */
	static String unescape_json_pointer_segment(const String &segment);

	/**
	 * @brief Compiles rules from a Schema
	 */
	void compile();

	/**
	 * @brief Validates an uncompiled Schema against this Schema
	 * @param data The Schema to validate
	 * @return Validation context with results
	 */
	Ref<SchemaValidationResult> validate_uncompiled(const Dictionary &schema_dict);

protected:
	static void _bind_methods();

public:
	/**
	 * @brief Default constructor (creates root node)
	 */
	Schema();

	/**
	 * @brief Constructor that builds tree from dictionary
	 * @param schema_dict The JSON Schema definition
	 * @param root_schema Reference to root Schema (for child nodes)
	 * @param schema_path Path from root (for debugging)
	 * @param validate_against_meta If true, validate against meta-Schema
	 */
	Schema(const Dictionary &schema_dict, const Ref<Schema> &root_schema = nullptr, const StringName &schema_path = "", const bool validate_against_meta = false);

	/**
	 * @brief Destructor
	 */
	virtual ~Schema();

	/**
	 * @brief Sets the compilation result (rules and errors)
	 * @param compiled_rules The compiled rule group
	 * @param errors List of compilation errors
	 */
	void set_compilation_result(std::shared_ptr<RuleGroup> compiled_rules, std::vector<SchemaCompileError> errors);

	// ========== Factory Methods ==========

	/**
	 * @brief Creates a Schema from a dictionary
	 * @param schema_dict The JSON Schema definition
	 * @return New Schema instance
	 */
	static Ref<Schema> build_schema(const Dictionary &schema_dict, bool validate_against_meta = false);

	/**
	 * @brief Registers a Schema with a ID for reference resolution
	 * @param id The Schema ID (e.g., "http://example.com/schema.json")
	 * @param schema The Schema instance to register
	 * @return True if registration succeeded, false if ID already registered
	 */
	static bool register_schema(const Ref<Schema> &schema, const StringName &id = "");

	/**
	 * @brief Checks if a Schema is registered for a given ID
	 * @param id The Schema ID to check
	 * @return True if a Schema is registered with the ID, false otherwise
	 */
	static bool is_schema_registered(const StringName &id);

	/**
	 * @brief Get a Schema from the registry by a given ID
	 * @param id The Schema ID to load
	 * @return The Schema instance to load
	 */
	static Ref<Schema> get_schema_from_registry(const StringName &id);

	/**
	 * @brief Unregisters a Schema by its ID
	 * @param id The Schema ID to unregister
	 * @return True if unregistration succeeded, false if ID was not found
	 */
	static bool unregister_schema(const StringName &id);

	/**
	 * @brief Loads a Schema from a JSON file
	 * @param path Path to the JSON Schema file
	 * @param validate_against_meta If true, validate against meta-Schema
	 * @return New Schema instance or null on error
	 */
	static Ref<Schema> load_from_json_file(const String &path, bool validate_against_meta = false);

	/**
	 * @brief Loads a Schema from a JSON string
	 * @param json_string JSON Schema as string
	 * @param validate_against_meta If true, validate against meta-Schema
	 * @return New Schema instance or null on error
	 */
	static Ref<Schema> load_from_json(const String &json_string, bool validate_against_meta = false);

	// ========== Tree Navigation ==========

	/**
	 * @brief Checks if this is a root node
	 * @return True if no parent exists
	 */
	bool is_root() const { return root_schema.is_null(); }

	/**
	 * @brief Gets the root node of this tree
	 * @return Root node
	 */
	Ref<Schema> get_root() const {
		if (is_root()) {
			return Ref<Schema>(const_cast<Schema *>(this));
		}

		return root_schema;
	}

	/**
	 * @brief Resolves a JSON Schema reference URI
	 * @param reference_uri The reference URI (e.g., "#", "#/properties/user", "external#/def")
	 * @return Referenced Schema or null if not found
	 */
	Ref<Schema> resolve_reference(const String &reference_uri) const;
	/**
	 * @brief Gets the Schema path from root
	 * @return Schema path string like "/properties/user/items"
	 */
	StringName get_schema_path() const { return schema_path; }

	/**
	 * @brief Gets the Schema type
	 * @return Schema type enum
	 */
	SchemaType get_schema_type() const { return schema_type; }

	/**
	 * @brief Gets the Schema type name as string
	 * @return Schema type as string
	 */
	String get_schema_type_name() const {
		switch (schema_type) {
			case SCHEMA_SCALAR:
				return "Scalar";
			case SCHEMA_OBJECT:
				return "Object";
			case SCHEMA_ARRAY:
				return "Array";
			case SCHEMA_LOGICAL:
				return "Logical";
			default:
				return "Unknown";
		}
	}

	// ========== Schema Information ==========

	/**
	 * @brief Gets the Schema ID
	 * @return Schema ID
	 */
	StringName get_id() const { return schema_id; }

	/**
	 * @brief Gets the Schema URL
	 * @return Schema URL
	 */
	StringName get_schema_url() const { return schema_url; }

	/**
	 * @brief Gets the Schema title
	 * @return Schema title
	 */
	StringName get_title() const { return title; }

	/**
	 * @brief Gets the Schema description
	 * @return Schema description
	 */
	StringName get_description() const { return description; }

	/**
	 * @brief Gets the Schema comment
	 * @return Schema comment
	 */
	StringName get_comment() const { return comment; }

	/**
	 * @brief Checks if this is an object Schema
	 * @return True if object type
	 */
	bool is_object() const { return schema_type == SCHEMA_OBJECT; }

	/**
	 * @brief Checks if this is an array Schema
	 * @return True if array type
	 */
	bool is_array() const { return schema_type == SCHEMA_ARRAY; }

	/**
	 * @brief Checks if this is a scalar Schema
	 * @return True if scalar type
	 */
	bool is_scalar() const { return schema_type == SCHEMA_SCALAR; }

	/**
	 * @brief Checks if this is a logical Schema
	 * @return True if logical type
	 */
	bool is_logical() const { return schema_type == SCHEMA_LOGICAL; }

	// ========== Object Schema Navigation ==========

	/**
	 * @brief Checks if this node has properties
	 * @return True if any properties exist
	 */
	bool has_children() const { return !children.empty(); }

	/**
	 * @brief Gets the number of properties
	 * @return Property count
	 */
	int get_child_count() const { return children.size(); }

	/**
	 * @brief Gets all property keys
	 * @return Array of StringName keys
	 */
	Array get_child_keys() const;

	/**
	 * @brief Checks if this node has a specific property
	 * @param key The property key to check
	 * @return True if property exists
	 */
	bool has_child(const StringName &key) const { return children.find(key) != children.end(); }

	/**
	 * @brief Gets a child property by key
	 * @param key The property key to look up
	 * @return Child node or null if not found
	 */
	Ref<Schema> get_child(const StringName &key) const;

	// ========== Array Schema Navigation ==========

	/**
	 * @brief Gets array item Schema at index (for array schemas)
	 * @param index The item index
	 * @return Schema if item is object Schema, Variant for scalar constraints, or null
	 */
	Ref<Schema> get_item_schema(int index) const {
		if (schema_type != SCHEMA_ARRAY || index < 0 || index >= item_schemas.size()) {
			return Ref<Schema>();
		}
		return item_schemas[index];
	}

	/**
	 * @brief Gets all array item schemas (for array schemas)
	 * @return Array containing Schema instances
	 */
	Array get_item_schemas() const {
		Array arr;
		for (const auto &schema : item_schemas) {
			arr.append(schema);
		}
		return arr;
	}

	/**
	 * @brief Gets the number of array items (for array schemas)
	 * @return Item count
	 */
	int get_item_count() const { return schema_type == SCHEMA_ARRAY ? item_schemas.size() : 0; }

	// ========== General Navigation ==========

	/**
	 * @brief Traverses to a Schema node by path
	 * @param path JSON pointer style path (e.g., "/properties/name" or "/items/0")
	 * @return Schema node at path or null if not found
	 */
	Ref<Schema> get_at_path(const StringName &path) const;

	// ========== Schema Metadata ==========

	/**
	 * @brief Gets the Schema definition for this node
	 * @return The Schema definition dictionary
	 */
	Dictionary get_schema_definition() const { return schema_definition; }

	/**
	 * @brief Gets the default value for this Schema node if defined
	 * @return The default value or null if not defined
	 */
	Variant get_default_value() const { return schema_definition.get("default", Variant()); }

	/**
	 * @brief Checks if this Schema has a default value
	 * @return True if default is defined
	 */
	bool has_default_value() const { return schema_definition.has("default"); }

	/**
	 * @brief Gets custom metadata if present
	 * @param key The custom key (e.g., "x-yaml-tag")
	 * @return The value or null if not found
	 */
	Variant get_custom_metadata(const String &key) const { return schema_definition.get(key, Variant()); }

	// ========== Validation ==========

	/**
	 * @brief Validates data against this Schema
	 * @param data The data to validate
	 * @return Validation context with results
	 */
	Ref<SchemaValidationResult> validate(const Variant &data);

	/**
	 * @brief Checks if the Schema is valid (no compilation errors)
	 * @return True if Schema compiled successfully
	 */
	bool is_valid() const;

	/**
	 * @brief Gets compilation errors
	 * @return Compilation errors as an Array of Dictionaries
	 */
	Array get_compile_errors();

	/**
	 * @brief Gets compilation error summary
	 * @return Error summary string or empty if valid
	 */
	String get_compile_error_summary();

	// ========== Debugging ==========

	/**
	 * @brief String representation for debugging
	 */
	String _to_string() const;

	friend class RuleFactory;
	friend class RefRule;
};

} // namespace godot

VARIANT_ENUM_CAST(Schema::SchemaType);
