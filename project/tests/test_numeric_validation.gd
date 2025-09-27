extends TestSuite

func _init() -> void:
	icon = "🔢"

func test_numeric_type_validation() -> void:
	var number_schema = Schema.build_schema({"type": "number"})
	var integer_schema = Schema.build_schema({"type": "integer"})

	expect(number_schema.validate(42).is_valid(), "Integer should validate as number")
	expect(number_schema.validate(3.14).is_valid(), "Float should validate as number")
	expect(integer_schema.validate(42).is_valid(), "Integer should validate as integer")
	expect(!integer_schema.validate(3.14).is_valid(), "Float should not validate as integer")
	expect(!number_schema.validate("123").is_valid(), "String should not validate as number")

func test_numeric_minimum() -> void:
	var schema = Schema.build_schema({
		"type": "number",
		"minimum": 0
	})

	expect(schema.validate(0).is_valid(), "Value equal to minimum should validate")
	expect(schema.validate(10).is_valid(), "Value greater than minimum should validate")
	expect(!schema.validate(-5).is_valid(), "Value less than minimum should not validate")

func test_numeric_maximum() -> void:
	var schema = Schema.build_schema({
		"type": "number",
		"maximum": 100
	})

	expect(schema.validate(100).is_valid(), "Value equal to maximum should validate")
	expect(schema.validate(50).is_valid(), "Value less than maximum should validate")
	expect(!schema.validate(150).is_valid(), "Value greater than maximum should not validate")

func test_numeric_exclusive_minimum() -> void:
	var schema = Schema.build_schema({
		"type": "number",
		"exclusiveMinimum": 0
	})

	expect(schema.validate(0.1).is_valid(), "Value greater than exclusive minimum should validate")
	expect(schema.validate(10).is_valid(), "Value much greater should validate")
	expect(!schema.validate(0).is_valid(), "Value equal to exclusive minimum should not validate")
	expect(!schema.validate(-5).is_valid(), "Value less than exclusive minimum should not validate")

func test_numeric_exclusive_maximum() -> void:
	var schema = Schema.build_schema({
		"type": "number",
		"exclusiveMaximum": 100
	})

	expect(schema.validate(99.9).is_valid(), "Value less than exclusive maximum should validate")
	expect(schema.validate(50).is_valid(), "Value much less should validate")
	expect(!schema.validate(100).is_valid(), "Value equal to exclusive maximum should not validate")
	expect(!schema.validate(150).is_valid(), "Value greater than exclusive maximum should not validate")

func test_numeric_multiple_of() -> void:
	var schema = Schema.build_schema({
		"type": "number",
		"multipleOf": 0.5
	})

	expect(schema.validate(1.0).is_valid(), "Multiple should validate")
	expect(schema.validate(2.5).is_valid(), "Another multiple should validate")
	expect(schema.validate(0).is_valid(), "Zero should validate")
	expect(!schema.validate(1.3).is_valid(), "Non-multiple should not validate")

func test_numeric_combined_constraints() -> void:
	var schema = Schema.build_schema({
		"type": "number",
		"minimum": 0,
		"maximum": 100,
		"multipleOf": 5
	})

	expect(schema.validate(0).is_valid(), "Value meeting all constraints should validate")
	expect(schema.validate(50).is_valid(), "Middle value should validate")
	expect(schema.validate(100).is_valid(), "Maximum value should validate")
	expect(!schema.validate(-5).is_valid(), "Value below minimum should not validate")
	expect(!schema.validate(150).is_valid(), "Value above maximum should not validate")
	expect(!schema.validate(13).is_valid(), "Value not multiple of 5 should not validate")
