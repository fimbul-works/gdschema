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

	expect(schema.is_valid(), "Complex schema should compile successfully")
	expect(schema.validate(valid_data).is_valid(), "Valid data should validate against complex schema")
