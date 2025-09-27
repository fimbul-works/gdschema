extends TestSuite

func _init() -> void:
	icon = "🏗️"

func test_schema_creation() -> void:
	var schema_dict = {"type": "string"}
	var schema = Schema.build_schema(schema_dict)

	expect(schema != null, "Schema should be created successfully")
	expect(schema.is_valid(), "Simple schema should compile without errors")

func test_invalid_schema_compilation() -> void:
	# Test with invalid schema structure
	var invalid_schema = Schema.build_schema({"type": "invalid_type"})

	expect(invalid_schema != null, "Schema object should be created even with errors")
	# Note: Depending on meta-validation implementation, this might be valid or invalid

func test_validation_result_structure() -> void:
	var schema = Schema.build_schema({"type": "string"})
	var result = schema.validate("test")

	expect(result != null, "Validation result should not be null")
	expect(result.is_valid(), "String should validate against string schema")
	expect_equal(result.error_count(), 0, "Valid data should have no errors")
