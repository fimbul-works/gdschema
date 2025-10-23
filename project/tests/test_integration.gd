extends TestSuite

func _init() -> void:
	icon = "🎯"

func test_complex_schema_validation() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"properties": {
			"id": {
				"type": "string",
				"pattern": "^[A-Z]{3}-\\d{4}$"
			},
			"items": {
				"type": "array",
				"items": {
					"type": "object",
					"properties": {
						"name": {"type": "string"},
						"price": {"type": "number", "minimum": 0}
					},
					"required": ["name", "price"]
				},
				"minItems": 1,
				"uniqueItems": true
			}
		},
		"required": ["id", "items"]
	})

	expect_equal(schema.get_id(), "", "Schema ID should be an empty string")
	expect_equal(schema.get_schema_url(), "", "Schema URL shouldbe an empty string")
	expect_equal(schema.get_title(), "", "Schema title should be an empty string")
	expect_equal(schema.get_comment(), "", "Schema comment should be an empty string")

	var valid_data = {
		"id": "ABC-1234",
		"items": [
			{"name": "Item 1", "price": 10.50},
			{"name": "Item 2", "price": 25.00}
		]
	}

	expect(schema.validate(valid_data).is_valid(), "Valid complex data should validate")

	var invalid_data = {
		"id": "invalid-id",
		"items": [
			{"name": "Item 1"},  # Missing price
			{"name": "Item 2", "price": -5}  # Invalid price
		]
	}

	var result = schema.validate(invalid_data)
	expect(!result.is_valid(), "Invalid complex data should not validate")
	expect(result.error_count() > 0, "Should have validation errors")

func test_schema_with_all_constraint_types() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"properties": {
			"str_prop": {
				"type": "string",
				"minLength": 3,
				"maxLength": 10,
				"pattern": "^[a-zA-Z]+$"
			},
			"num_prop": {
				"type": "number",
				"minimum": 0,
				"maximum": 100,
				"multipleOf": 5
			},
			"arr_prop": {
				"type": "array",
				"items": {"type": "string"},
				"minItems": 1,
				"maxItems": 3,
				"uniqueItems": true
			},
			"const_prop": {"const": "fixed_value"},
			"enum_prop": {"enum": ["option1", "option2", "option3"]}
		},
		"required": ["str_prop", "num_prop"],
		"minProperties": 2,
		"maxProperties": 10
	})

	var valid_data = {
		"str_prop": "hello",
		"num_prop": 25,
		"arr_prop": ["item1", "item2"],
		"const_prop": "fixed_value",
		"enum_prop": "option1"
	}

	expect(schema.is_valid(), "Complex Schema should compile successfully")
	expect(schema.validate(valid_data).is_valid(), "Valid data should validate against complex schema")

func test_meta_schema_validation() -> void:
	var schema_url := "http://json-schema.org/draft-07/schema#"

	var is_registered := Schema.is_schema_registered(schema_url)
	expect(is_registered, "Meta-schema should be registered")
	if !is_registered:
		return

	var meta_schema := Schema.get_schema_from_registry(schema_url)
	var is_valid := meta_schema.is_valid()
	expect(is_valid, "Meta-schema should be valid")
	if !is_valid:
		return

	var definitions := meta_schema.get_schema_definition()
	var validation_result := meta_schema.validate(definitions)
	expect(!validation_result.has_errors(), "Meta-schema validation against itself should have no errors")

	expect_equal(meta_schema.get_id(), schema_url, "Meta-schema ID should match")
	expect_equal(meta_schema.get_schema_url(), schema_url, "Meta-schema ID should match")
	expect_equal(meta_schema.get_title(), "Core schema meta-schema", "Meta-Schama title should match")
