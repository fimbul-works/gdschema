extends TestSuite

func _init() -> void:
	icon = "🧩"

func test_all_of_validation() -> void:
	var schema = Schema.build_schema({
		"allOf": [
			{"type": "string"},
			{"minLength": 5},
			{"maxLength": 10}
		]
	})

	expect(schema.validate("hello").is_valid(), "Value satisfying all schemas should validate")
	expect(schema.validate("testing").is_valid(), "Another valid value should validate")
	expect(!schema.validate("hi").is_valid(), "Value failing one Schema should not validate")
	expect(!schema.validate("toolongstring").is_valid(), "Value failing another Schema should not validate")
	expect(!schema.validate(123).is_valid(), "Value failing first Schema should not validate")

func test_any_of_validation() -> void:
	var schema = Schema.build_schema({
		"anyOf": [
			{"type": "string", "minLength": 5},
			{"type": "number", "minimum": 10}
		]
	})

	expect(schema.validate("hello").is_valid(), "Value matching first Schema should validate")
	expect(schema.validate(15).is_valid(), "Value matching second Schema should validate")
	expect(!schema.validate("test").is_valid(), "Short string should not validate (fails minLength)")
	expect(!schema.validate(5).is_valid(), "Small number should not validate (fails minimum)")
	expect(!schema.validate(true).is_valid(), "Value matching neither Schema should not validate")

func test_one_of_validation() -> void:
	var schema = Schema.build_schema({
		"oneOf": [
			{"type": "string", "maxLength": 5},
			{"type": "string", "minLength": 10}
		]
	})

	expect(schema.validate("hi").is_valid(), "Short string should validate (matches first only)")
	expect(schema.validate("verylongstring").is_valid(), "Long string should validate (matches second only)")
	expect(!schema.validate("medium").is_valid(), "Medium string should not validate (matches neither)")
	expect(!schema.validate(123).is_valid(), "Non-string should not validate")

func test_not_validation() -> void:
	var schema = Schema.build_schema({
		"not": {
			"type": "string"
		}
	})

	expect(schema.validate(123).is_valid(), "Non-string should validate")
	expect(schema.validate(true).is_valid(), "Boolean should validate")
	expect(schema.validate([]).is_valid(), "Array should validate")
	expect(!schema.validate("hello").is_valid(), "String should not validate")
