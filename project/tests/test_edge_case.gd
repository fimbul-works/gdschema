extends TestSuite

func _init() -> void:
	icon = "🪨"

func test_string_edge_case_failures() -> void:
	var schema = Schema.build_schema({
		"type": "string",
		"minLength": 5,
		"maxLength": 10,
		"pattern": "^[A-Z][a-z]{4,9}$"
	})

	expect(!schema.validate("").is_valid(), "Empty string should fail minLength")
	expect(!schema.validate("a").is_valid(), "Single char should fail minLength")
	expect(!schema.validate("toolongstring").is_valid(), "Too long string should fail maxLength")
	expect(!schema.validate("hello").is_valid(), "Lowercase first letter should fail pattern")
	expect(!schema.validate("HELLO").is_valid(), "All uppercase should fail pattern")
	expect(!schema.validate("Hello123").is_valid(), "Numbers should fail pattern")
	expect(!schema.validate("Hello!").is_valid(), "Special chars should fail pattern")

func test_numeric_edge_case_failures() -> void:
	var schema = Schema.build_schema({
		"type": "number",
		"minimum": 10,
		"maximum": 100,
		"exclusiveMinimum": 0,
		"exclusiveMaximum": 1000,
		"multipleOf": 0.1
	})

	expect(!schema.validate(0).is_valid(), "Value equal to exclusiveMinimum should fail")
	expect(!schema.validate(1000).is_valid(), "Value equal to exclusiveMaximum should fail")
	expect(!schema.validate(5).is_valid(), "Value below minimum should fail")
	expect(!schema.validate(150).is_valid(), "Value above maximum should fail")
	expect(!schema.validate(10.15).is_valid(), "Non-multiple should fail")
	expect(!schema.validate(-5).is_valid(), "Negative value should fail exclusiveMinimum")

func test_array_edge_case_failures() -> void:
	var schema = Schema.build_schema({
		"type": "array",
		"items": {
			"type": "string",
			"minLength": 2
		},
		"minItems": 2,
		"maxItems": 4,
		"uniqueItems": true
	})

	expect(!schema.validate([]).is_valid(), "Empty array should fail minItems")
	expect(!schema.validate(["a"]).is_valid(), "Single item should fail minItems")
	expect(!schema.validate(["aa", "bb", "cc", "dd", "ee"]).is_valid(), "Too many items should fail maxItems")
	expect(!schema.validate(["aa", "aa"]).is_valid(), "Duplicate items should fail uniqueItems")
	expect(!schema.validate(["a", "bb"]).is_valid(), "Item failing constraint should fail")
	expect(!schema.validate([123, "bb"]).is_valid(), "Wrong item type should fail")

func test_object_edge_case_failures() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"properties": {
			"name": {"type": "string", "minLength": 3},
			"age": {"type": "number", "minimum": 0}
		},
		"required": ["name"],
		"minProperties": 1,
		"maxProperties": 3,
		"additionalProperties": false
	})

	expect(!schema.validate({}).is_valid(), "Missing required property should fail")
	expect(!schema.validate({"name": "ab"}).is_valid(), "Property failing constraint should fail")
	expect(!schema.validate({"name": 123}).is_valid(), "Property with wrong type should fail")
	expect(!schema.validate({"name": "abc", "age": -1}).is_valid(), "Property failing numeric constraint should fail")
	expect(!schema.validate({"name": "abc", "extra": "value"}).is_valid(), "Additional property should fail when forbidden")
	expect(!schema.validate({"name": "abc", "age": 25, "extra1": 1, "extra2": 2}).is_valid(), "Too many properties should fail maxProperties")

func test_logical_combination_failures() -> void:
	var all_of_schema = Schema.build_schema({
		"allOf": [
			{"type": "string"},
			{"minLength": 5},
			{"pattern": "^[A-Z]"}
		]
	})

	expect(!all_of_schema.validate("hi").is_valid(), "String failing one allOf schema should fail")
	expect(!all_of_schema.validate("hello").is_valid(), "String failing pattern in allOf should fail")
	expect(!all_of_schema.validate(123).is_valid(), "Non-string should fail allOf")

	var one_of_schema = Schema.build_schema({
		"oneOf": [
			{"type": "string"},
			{"type": "string", "minLength": 10}
		]
	})

	expect(!one_of_schema.validate("verylongstring").is_valid(), "Value matching multiple oneOf schemas should fail")
	expect(!one_of_schema.validate(123).is_valid(), "Value matching no oneOf schemas should fail")

func test_nested_validation_failures() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"properties": {
			"user": {
				"type": "object",
				"properties": {
					"profile": {
						"type": "object",
						"properties": {
							"name": {"type": "string", "minLength": 3}
						},
						"required": ["name"]
					}
				},
				"required": ["profile"]
			}
		}
	})

	expect(!schema.validate({"user": {}}).is_valid(), "Missing nested required property should fail")
	expect(!schema.validate({"user": {"profile": {}}}).is_valid(), "Missing deeply nested required property should fail")
	expect(!schema.validate({"user": {"profile": {"name": "ab"}}}).is_valid(), "Deeply nested constraint failure should fail")

func test_type_coercion_failures() -> void:
	# Test that values aren't coerced to expected types
	var string_schema = Schema.build_schema({"type": "string"})
	expect(!string_schema.validate(0).is_valid(), "Number zero should not validate as string")
	expect(!string_schema.validate(false).is_valid(), "Boolean false should not validate as string")

	var number_schema = Schema.build_schema({"type": "number"})
	expect(!number_schema.validate("123").is_valid(), "String number should not validate as number")
	expect(!number_schema.validate(true).is_valid(), "Boolean true should not validate as number")

	var boolean_schema = Schema.build_schema({"type": "boolean"})
	expect(!boolean_schema.validate(0).is_valid(), "Number 0 should not validate as boolean")
	expect(!boolean_schema.validate(1).is_valid(), "Number 1 should not validate as boolean")
	expect(!boolean_schema.validate("true").is_valid(), "String 'true' should not validate as boolean")
	expect(!boolean_schema.validate("false").is_valid(), "String 'false' should not validate as boolean")

func test_pattern_properties_failures() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"patternProperties": {
			"^str_": {"type": "string", "minLength": 3},
			"^num_": {"type": "number", "minimum": 0}
		},
		"additionalProperties": false
	})

	expect(!schema.validate({"str_x": "ab"}).is_valid(), "Pattern property failing constraint should fail")
	expect(!schema.validate({"num_y": -5}).is_valid(), "Pattern property failing numeric constraint should fail")
	expect(!schema.validate({"str_x": 123}).is_valid(), "Pattern property with wrong type should fail")
	expect(!schema.validate({"invalid_prop": "value"}).is_valid(), "Property not matching pattern should fail when additionalProperties is false")

func test_multiple_constraint_violations() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"properties": {
			"name": {"type": "string", "minLength": 5, "maxLength": 10},
			"age": {"type": "number", "minimum": 0, "maximum": 120}
		},
		"required": ["name", "age"]
	})

	# Object with multiple violations should accumulate all errors
	var result = schema.validate({
		"name": "ab",  # Too short
		"age": -5      # Below minimum
	})

	expect(!result.is_valid(), "Object with multiple violations should fail")
	expect(result.error_count() >= 2, "Should have at least 2 errors for multiple violations")

func test_array_items_complex_failures() -> void:
	var schema = Schema.build_schema({
		"type": "array",
		"items": {
			"type": "object",
			"properties": {
				"id": {"type": "string", "pattern": "^[A-Z]{2}\\d{3}$"},
				"value": {"type": "number", "multipleOf": 5}
			},
			"required": ["id", "value"]
		},
		"uniqueItems": true
	})

	expect(!schema.validate([
		{"id": "AB123", "value": 10},
		{"id": "AB123", "value": 10}  # Duplicate
	]).is_valid(), "Array with duplicate complex objects should fail")

	expect(!schema.validate([
		{"id": "ab123", "value": 10}  # Wrong pattern
	]).is_valid(), "Array item failing pattern should fail")

	expect(!schema.validate([
		{"id": "AB123", "value": 13}  # Not multiple of 5
	]).is_valid(), "Array item failing multipleOf should fail")

func test_const_enum_edge_cases() -> void:
	var const_schema = Schema.build_schema({"const": null})
	expect(!const_schema.validate(0).is_valid(), "Zero should not equal null")
	expect(!const_schema.validate("").is_valid(), "Empty string should not equal null")
	expect(!const_schema.validate(false).is_valid(), "False should not equal null")

	var enum_schema = Schema.build_schema({"enum": [null, 0, "", false]})
	expect(!enum_schema.validate([]).is_valid(), "Empty array not in enum should fail")
	expect(!enum_schema.validate({}).is_valid(), "Empty object not in enum should fail")

func test_exclusive_boundary_edge_cases() -> void:
	var schema = Schema.build_schema({
		"type": "number",
		"exclusiveMinimum": 10.0,
		"exclusiveMaximum": 20.0
	})

	# Test exact boundary values
	expect(!schema.validate(10.0).is_valid(), "Exact exclusiveMinimum value should fail")
	expect(!schema.validate(20.0).is_valid(), "Exact exclusiveMaximum value should fail")
	expect(schema.validate(10.1).is_valid(), "Value just above exclusiveMinimum should pass")
	expect(schema.validate(19.9).is_valid(), "Value just below exclusiveMaximum should pass")
