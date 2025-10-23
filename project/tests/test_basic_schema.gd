extends TestSuite

func _init() -> void:
	icon = "🏗️"

func test_schema_creation() -> void:
	var schema_dict = {"type": "string"}
	var schema = Schema.build_schema(schema_dict)

	expect(schema != null, "Schema should be created successfully")
	expect(schema.is_valid(), "Simple Schema should compile without errors")

func test_invalid_schema_compilation() -> void:
	# Test with invalid Schema structure
	var invalid_schema = Schema.build_schema({"type": "invalid_type"})

	expect(invalid_schema != null, "Schema object should be created even with errors")
	# Note: Depending on meta-validation implementation, this might be valid or invalid

func test_validation_result_structure() -> void:
	var schema = Schema.build_schema({"type": "string"})
	var result = schema.validate("test")

	expect(result != null, "Validation result should not be null")
	expect(result.is_valid(), "String should validate against string schema")
	expect_equal(result.error_count(), 0, "Valid data should have no errors")

func test_load_from_json_string() -> void:
	# Test valid JSON schema
	var json_string = """
	{
		"type": "object",
		"properties": {
			"username": {
				"type": "string",
				"minLength": 3,
				"maxLength": 20
			},
			"age": {
				"type": "integer",
				"minimum": 0
			}
		},
		"required": ["username"]
	}
	"""

	var schema = Schema.load_from_json(json_string)

	expect(schema != null, "Schema should load from valid JSON string")
	expect(schema.is_valid(), "Loaded Schema should compile successfully")

	# Test that the loaded Schema works correctly
	var valid_data = {"username": "alice", "age": 25}
	var result = schema.validate(valid_data)
	expect(result.is_valid(), "Valid data should pass validation")

func test_load_from_json_invalid() -> void:
	# Test with malformed JSON
	var bad_json = '{"type": "string", invalid json here}'
	var schema = Schema.load_from_json(bad_json)

	expect(schema == null, "Malformed JSON should return null")

	# Test with JSON that's not an object
	var array_json = '["not", "a", "schema"]'
	schema = Schema.load_from_json(array_json)

	expect(schema == null, "JSON array should return null (schemas must be objects)")

func test_load_from_json_with_id() -> void:
	# Test that schemas with $id are auto-registered
	var json_with_id = """
	{
		"$id": "http://example.com/test-schema-from-json",
		"type": "string",
		"minLength": 5
	}
	"""

	var schema = Schema.load_from_json(json_with_id)

	expect(schema != null, "Schema with $id should load")
	expect(Schema.is_schema_registered("http://example.com/test-schema-from-json"),
		"Schema with $id should be auto-registered")

	# Clean up
	Schema.unregister_schema("http://example.com/test-schema-from-json")

func test_load_from_json_with_meta_validation() -> void:
	# Test loading with meta-schema validation enabled
	var valid_schema_json = '{"type": "string", "minLength": 1}'
	var schema = Schema.load_from_json(valid_schema_json, true)

	expect(schema != null, "Schema object should be created")
	expect(schema.is_valid(), "Valid Schema should pass meta-validation")

	# Test with Schema that fails meta-validation
	var invalid_schema_json = '{"type": "string", "minLength": "not a number"}'
	schema = Schema.load_from_json(invalid_schema_json, true)

	expect(schema != null, "Invalid Schema should still return Schema object")
	expect(!schema.is_valid(), "Schema with meta-validation errors should be invalid")
	expect(schema.get_compile_errors().size() > 0, "Should have compilation errors from meta-validation")
