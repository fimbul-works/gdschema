#include "meta_schema_definitions.hpp"
#include "schema.hpp"

#include <godot_cpp/classes/json.hpp>

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

Dictionary MetaSchemaDefinitions::get_draft7_meta_schema() {
	Dictionary draft7_meta_schema = JSON::parse_string(
			"{\"$schema\":\"http://json-schema.org/draft-07/schema#\",\"$id\":\"http://json-schema.org/draft-07/"
			"schema#\",\"title\":\"Core schema "
			"meta-schema\",\"definitions\":{\"schemaArray\":{\"type\":\"array\",\"minItems\":1,\"items\":{\"$ref\":\"#"
			"\"}},\"nonNegativeInteger\":{\"type\":\"integer\",\"minimum\":0},\"nonNegativeIntegerDefault0\":{"
			"\"allOf\":[{\"$ref\":\"#/definitions/"
			"nonNegativeInteger\"},{\"default\":0}]},\"simpleTypes\":{\"enum\":[\"array\",\"boolean\",\"integer\","
			"\"null\",\"number\",\"object\",\"string\"]},\"stringArray\":{\"type\":\"array\",\"items\":{\"type\":"
			"\"string\"},\"uniqueItems\":true,\"default\":[]}},\"type\":[\"object\",\"boolean\"],\"properties\":{\"$"
			"id\":{\"type\":\"string\",\"format\":\"uri-reference\"},\"$schema\":{\"type\":\"string\",\"format\":"
			"\"uri\"},\"$ref\":{\"type\":\"string\",\"format\":\"uri-reference\"},\"$comment\":{\"type\":\"string\"},"
			"\"title\":{\"type\":\"string\"},\"description\":{\"type\":\"string\"},\"default\":true,\"readOnly\":{"
			"\"type\":\"boolean\",\"default\":false},\"writeOnly\":{\"type\":\"boolean\",\"default\":false},"
			"\"examples\":{\"type\":\"array\",\"items\":true},\"multipleOf\":{\"type\":\"number\",\"exclusiveMinimum\":"
			"0},\"maximum\":{\"type\":\"number\"},\"exclusiveMaximum\":{\"type\":\"number\"},\"minimum\":{\"type\":"
			"\"number\"},\"exclusiveMinimum\":{\"type\":\"number\"},\"maxLength\":{\"$ref\":\"#/definitions/"
			"nonNegativeInteger\"},\"minLength\":{\"$ref\":\"#/definitions/"
			"nonNegativeIntegerDefault0\"},\"pattern\":{\"type\":\"string\",\"format\":\"regex\"},\"additionalItems\":{"
			"\"$ref\":\"#\"},\"items\":{\"anyOf\":[{\"$ref\":\"#\"},{\"$ref\":\"#/definitions/"
			"schemaArray\"}],\"default\":true},\"maxItems\":{\"$ref\":\"#/definitions/"
			"nonNegativeInteger\"},\"minItems\":{\"$ref\":\"#/definitions/"
			"nonNegativeIntegerDefault0\"},\"uniqueItems\":{\"type\":\"boolean\",\"default\":false},\"contains\":{\"$"
			"ref\":\"#\"},\"maxProperties\":{\"$ref\":\"#/definitions/"
			"nonNegativeInteger\"},\"minProperties\":{\"$ref\":\"#/definitions/"
			"nonNegativeIntegerDefault0\"},\"required\":{\"$ref\":\"#/definitions/"
			"stringArray\"},\"additionalProperties\":{\"$ref\":\"#\"},\"definitions\":{\"type\":\"object\","
			"\"additionalProperties\":{\"$ref\":\"#\"},\"default\":{}},\"properties\":{\"type\":\"object\","
			"\"additionalProperties\":{\"$ref\":\"#\"},\"default\":{}},\"patternProperties\":{\"type\":\"object\","
			"\"additionalProperties\":{\"$ref\":\"#\"},\"propertyNames\":{\"format\":\"regex\"},\"default\":{}},"
			"\"dependencies\":{\"type\":\"object\",\"additionalProperties\":{\"anyOf\":[{\"$ref\":\"#\"},{\"$ref\":\"#/"
			"definitions/"
			"stringArray\"}]}},\"propertyNames\":{\"$ref\":\"#\"},\"const\":true,\"enum\":{\"type\":\"array\","
			"\"items\":true,\"minItems\":1,\"uniqueItems\":true},\"type\":{\"anyOf\":[{\"$ref\":\"#/definitions/"
			"simpleTypes\"},{\"type\":\"array\",\"items\":{\"$ref\":\"#/definitions/"
			"simpleTypes\"},\"minItems\":1,\"uniqueItems\":true}]},\"format\":{\"type\":\"string\"},"
			"\"contentMediaType\":{\"type\":\"string\"},\"contentEncoding\":{\"type\":\"string\"},\"if\":{\"$ref\":\"#"
			"\"},\"then\":{\"$ref\":\"#\"},\"else\":{\"$ref\":\"#\"},\"allOf\":{\"$ref\":\"#/definitions/"
			"schemaArray\"},\"anyOf\":{\"$ref\":\"#/definitions/schemaArray\"},\"oneOf\":{\"$ref\":\"#/definitions/"
			"schemaArray\"},\"not\":{\"$ref\":\"#\"}},\"default\":true}");
	return draft7_meta_schema;
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
