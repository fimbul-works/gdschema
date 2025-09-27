#include "meta_schema_definitions.hpp"
#include "schema.hpp"

using namespace godot;

Ref<Schema> MetaSchemaDefinitions::draft7_meta_schema = nullptr;
bool MetaSchemaDefinitions::meta_schema_initialized = false;

Ref<Schema> MetaSchemaDefinitions::get_draft7_meta_schema_instance() {
	if (!meta_schema_initialized) {
		Dictionary meta_schema_dict = get_draft7_meta_schema();
		// Build WITHOUT validation to avoid infinite recursion
		Ref<Schema> temp = memnew(Schema(meta_schema_dict));
		temp->compile();
		draft7_meta_schema = temp;
		meta_schema_initialized = true;
	}
	return draft7_meta_schema;
}

Dictionary MetaSchemaDefinitions::get_string_constraints_schema() {
	Dictionary schema;

	Dictionary properties;

	// minLength constraint
	Dictionary min_length_schema;
	min_length_schema["type"] = "integer";
	min_length_schema["minimum"] = 0;
	properties["minLength"] = min_length_schema;

	// maxLength constraint
	Dictionary max_length_schema;
	max_length_schema["type"] = "integer";
	max_length_schema["minimum"] = 0;
	properties["maxLength"] = max_length_schema;

	// pattern constraint
	Dictionary pattern_schema;
	pattern_schema["type"] = "string";
	// TODO: Could validate that it's a valid regex, but that's complex
	properties["pattern"] = pattern_schema;

	schema["type"] = "object";
	schema["properties"] = properties;
	schema["additionalProperties"] = false; // Only allow defined string constraints

	return schema;
}

Dictionary MetaSchemaDefinitions::get_draft7_meta_schema() {
	Dictionary schema;
	Dictionary properties;

	// Basic type constraint
	Dictionary type_schema;
	Array type_options;

	// Single type option
	Dictionary single_type;
	single_type["type"] = "string";
	Array valid_types;
	valid_types.push_back("null");
	valid_types.push_back("boolean");
	valid_types.push_back("object");
	valid_types.push_back("array");
	valid_types.push_back("number");
	valid_types.push_back("integer");
	valid_types.push_back("string");
	single_type["enum"] = valid_types;
	type_options.push_back(single_type);

	// Array of types option
	Dictionary array_type;
	array_type["type"] = "array";
	Dictionary array_items;
	array_items["type"] = "string";
	array_items["enum"] = valid_types;
	array_type["items"] = array_items;
	array_type["minItems"] = 1;
	array_type["uniqueItems"] = true;
	type_options.push_back(array_type);

	Dictionary type_one_of;
	type_one_of["oneOf"] = type_options;
	properties["type"] = type_one_of;

	// Value constraints
	properties["const"] = Dictionary(); // Any value allowed

	Dictionary enum_schema;
	enum_schema["type"] = "array";
	enum_schema["minItems"] = 1;
	properties["enum"] = enum_schema;

	// String constraints
	Dictionary non_negative_int;
	non_negative_int["type"] = "integer";
	non_negative_int["minimum"] = 0;

	properties["minLength"] = non_negative_int;
	properties["maxLength"] = non_negative_int;

	Dictionary pattern_schema;
	pattern_schema["type"] = "string";
	properties["pattern"] = pattern_schema;

	// Numeric constraints
	Dictionary numeric_constraint;
	numeric_constraint["type"] = "number";
	properties["minimum"] = numeric_constraint;
	properties["maximum"] = numeric_constraint;
	properties["exclusiveMinimum"] = numeric_constraint;
	properties["exclusiveMaximum"] = numeric_constraint;

	Dictionary multiple_of_constraint;
	multiple_of_constraint["type"] = "number";
	multiple_of_constraint["exclusiveMinimum"] = 0;
	properties["multipleOf"] = multiple_of_constraint;

	// Array constraints
	properties["minItems"] = non_negative_int;
	properties["maxItems"] = non_negative_int;

	Dictionary unique_items_schema;
	unique_items_schema["type"] = "boolean";
	properties["uniqueItems"] = unique_items_schema;

	// items can be schema or array of schemas
	Dictionary items_schema;
	Array items_options;
	Dictionary items_single_schema;
	items_single_schema["$ref"] = "#"; // Reference to root schema (recursive)
	items_options.push_back(items_single_schema);

	Dictionary items_array_schema;
	items_array_schema["type"] = "array";
	Dictionary items_array_items;
	items_array_items["$ref"] = "#";
	items_array_schema["items"] = items_array_items;
	items_options.push_back(items_array_schema);

	items_schema["oneOf"] = items_options;
	properties["items"] = items_schema;

	Dictionary additional_items_schema;
	additional_items_schema["$ref"] = "#";
	properties["additionalItems"] = additional_items_schema;

	Dictionary contains_schema;
	contains_schema["$ref"] = "#";
	properties["contains"] = contains_schema;

	// Object constraints
	properties["minProperties"] = non_negative_int;
	properties["maxProperties"] = non_negative_int;

	Dictionary required_schema;
	required_schema["type"] = "array";
	Dictionary required_items;
	required_items["type"] = "string";
	required_schema["items"] = required_items;
	required_schema["uniqueItems"] = true;
	properties["required"] = required_schema;

	// properties is a map of property names to schemas
	Dictionary properties_schema;
	properties_schema["type"] = "object";
	Dictionary properties_additional;
	properties_additional["$ref"] = "#";
	properties_schema["additionalProperties"] = properties_additional;
	properties["properties"] = properties_schema;

	// patternProperties is a map of regex patterns to schemas
	Dictionary pattern_properties_schema;
	pattern_properties_schema["type"] = "object";
	pattern_properties_schema["additionalProperties"] = properties_additional;
	properties["patternProperties"] = pattern_properties_schema;

	// additionalProperties can be boolean or schema
	Dictionary additional_props_schema;
	Array additional_props_options;
	Dictionary additional_props_bool;
	additional_props_bool["type"] = "boolean";
	additional_props_options.push_back(additional_props_bool);
	additional_props_options.push_back(properties_additional);
	additional_props_schema["oneOf"] = additional_props_options;
	properties["additionalProperties"] = additional_props_schema;

	Dictionary property_names_schema;
	property_names_schema["$ref"] = "#";
	properties["propertyNames"] = property_names_schema;

	// dependencies can be property list or schema
	Dictionary dependencies_schema;
	dependencies_schema["type"] = "object";
	Dictionary dependencies_additional;
	Array dependencies_options;
	Dictionary dependencies_string_array;
	dependencies_string_array["type"] = "array";
	dependencies_string_array["items"] = required_items;
	dependencies_string_array["uniqueItems"] = true;
	dependencies_options.push_back(dependencies_string_array);
	dependencies_options.push_back(properties_additional);
	Dictionary dependencies_one_of;
	dependencies_one_of["oneOf"] = dependencies_options;
	dependencies_additional = dependencies_one_of;
	dependencies_schema["additionalProperties"] = dependencies_additional;
	properties["dependencies"] = dependencies_schema;

	// Logical composition constraints
	Dictionary schema_array;
	schema_array["type"] = "array";
	Dictionary schema_array_items;
	schema_array_items["$ref"] = "#";
	schema_array["items"] = schema_array_items;
	schema_array["minItems"] = 1;

	properties["allOf"] = schema_array;
	properties["anyOf"] = schema_array;
	properties["oneOf"] = schema_array;

	Dictionary not_schema;
	not_schema["$ref"] = "#";
	properties["not"] = not_schema;

	// Conditional schema (if/then/else)
	Dictionary if_schema;
	if_schema["$ref"] = "#";
	properties["if"] = if_schema;

	Dictionary then_schema;
	then_schema["$ref"] = "#";
	properties["then"] = then_schema;

	Dictionary else_schema;
	else_schema["$ref"] = "#";
	properties["else"] = else_schema;

	// Format constraint (for future extension)
	Dictionary format_schema;
	format_schema["type"] = "string";
	properties["format"] = format_schema;

	// Meta-schema properties
	Dictionary schema_uri;
	schema_uri["type"] = "string";
	schema_uri["format"] = "uri";
	properties["$schema"] = schema_uri;

	Dictionary id_schema;
	id_schema["type"] = "string";
	id_schema["format"] = "uri-reference";
	properties["$id"] = id_schema;

	Dictionary ref_schema;
	ref_schema["type"] = "string";
	ref_schema["format"] = "uri-reference";
	properties["$ref"] = ref_schema;

	Dictionary comment_schema;
	comment_schema["type"] = "string";
	properties["$comment"] = comment_schema;

	// Annotation keywords
	Dictionary string_schema;
	string_schema["type"] = "string";
	properties["title"] = string_schema;
	properties["description"] = string_schema;

	properties["default"] = Dictionary(); // Any value

	Dictionary examples_schema;
	examples_schema["type"] = "array";
	properties["examples"] = examples_schema;

	// Definitions/Defs
	Dictionary definitions_schema;
	definitions_schema["type"] = "object";
	definitions_schema["additionalProperties"] = properties_additional;
	properties["definitions"] = definitions_schema;
	properties["$defs"] = definitions_schema;

	// Custom YAML tag constraint (extension)
	Dictionary yaml_tag_schema;
	yaml_tag_schema["type"] = "string";
	properties["yamlTag"] = yaml_tag_schema;

	// Root schema structure
	schema["$schema"] = "http://json-schema.org/draft-07/schema#";
	schema["$id"] = "http://json-schema.org/draft-07/schema#";
	schema["type"] = Array({ "object", "boolean" });
	schema["properties"] = properties;

	return schema;
}

Ref<SchemaValidationResult> MetaSchemaDefinitions::validate_schema_definition(const Dictionary &schema_def) {
	static bool building_meta_schema = false;

	if (building_meta_schema) {
		// We're building the meta-schema itself, skip validation
		return SchemaValidationResult::success();
	}

	building_meta_schema = true;
	Ref<Schema> meta_validator = get_draft7_meta_schema_instance();
	Ref<SchemaValidationResult> result = meta_validator->validate_uncompiled(schema_def);
	building_meta_schema = false;

	return result;
}
