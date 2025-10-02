extends TestSuite

func _init() -> void:
	icon = "🏷️"

func test_const_validation() -> void:
	var schema = Schema.build_schema({
		"const": "specific_value"
	})

	expect(schema.validate("specific_value").is_valid(), "Exact const value should validate")
	expect(!schema.validate("other_value").is_valid(), "Different value should not validate")
	expect(!schema.validate(123).is_valid(), "Different type should not validate")

func test_enum_validation() -> void:
	var schema = Schema.build_schema({
		"enum": ["red", "green", "blue", 42]
	})

	expect(schema.validate("red").is_valid(), "Enum value should validate")
	expect(schema.validate("green").is_valid(), "Another enum value should validate")
	expect(schema.validate(42).is_valid(), "Number enum value should validate")
	expect(!schema.validate("yellow").is_valid(), "Non-enum value should not validate")
	expect(!schema.validate(123).is_valid(), "Non-enum number should not validate")

func test_multiple_types() -> void:
	var schema = Schema.build_schema({
		"type": ["string", "number"]
	})

	expect(schema.validate("hello").is_valid(), "String should validate")
	expect(schema.validate(123).is_valid(), "Number should validate")
	expect(schema.validate(3.14).is_valid(), "Float should validate")
	expect(!schema.validate(true).is_valid(), "Boolean should not validate")
	expect(!schema.validate([]).is_valid(), "Array should not validate")

func test_null_type() -> void:
	var schema = Schema.build_schema({
		"type": "null"
	})

	expect(schema.validate(null).is_valid(), "Null should validate")
	expect(!schema.validate("").is_valid(), "Empty string should not validate as null")
	expect(!schema.validate(0).is_valid(), "Zero should not validate as null")

func test_boolean_type() -> void:
	var schema = Schema.build_schema({
		"type": "boolean"
	})

	expect(schema.validate(true).is_valid(), "True should validate")
	expect(schema.validate(false).is_valid(), "False should validate")
	expect(!schema.validate(1).is_valid(), "Number 1 should not validate as boolean")
	expect(!schema.validate("true").is_valid(), "String 'true' should not validate as boolean")

func test_boolean_schema_true() -> void:
	var schema = Schema.build_schema({})  # Validates everything

	expect(schema.validate("anything").is_valid(), "True schema should validate any value")
	expect(schema.validate(null).is_valid(), "True schema should validate null")
	expect(schema.validate({"complex": "object"}).is_valid(), "True schema should validate complex data")

func test_boolean_schema_false() -> void:
	var schema = Schema.build_schema({ "not": {} })  # Validates nothing

	expect(!schema.validate("anything").is_valid(), "False schema should not validate any value")
	expect(!schema.validate(null).is_valid(), "False schema should not validate null")
