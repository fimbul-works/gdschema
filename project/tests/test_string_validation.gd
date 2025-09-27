extends TestSuite

func _init() -> void:
	icon = "📝"

func test_string_type_validation() -> void:
	var schema = Schema.build_schema({"type": "string"})

	expect(schema.validate("hello").is_valid(), "String should validate")
	expect(!schema.validate(123).is_valid(), "Number should not validate as string")
	expect(!schema.validate(true).is_valid(), "Boolean should not validate as string")
	expect(!schema.validate([]).is_valid(), "Array should not validate as string")

func test_string_min_length() -> void:
	var schema = Schema.build_schema({
		"type": "string",
		"minLength": 5
	})

	expect(schema.validate("hello").is_valid(), "String with exact min length should validate")
	expect(schema.validate("hello world").is_valid(), "String longer than min should validate")
	expect(!schema.validate("hi").is_valid(), "String shorter than min should not validate")

func test_string_max_length() -> void:
	var schema = Schema.build_schema({
		"type": "string",
		"maxLength": 10
	})

	expect(schema.validate("hello").is_valid(), "String shorter than max should validate")
	expect(schema.validate("1234567890").is_valid(), "String with exact max length should validate")
	expect(!schema.validate("this is too long").is_valid(), "String longer than max should not validate")

func test_string_pattern() -> void:
	var schema = Schema.build_schema({
		"type": "string",
		"pattern": "^[A-Z][a-z]+$"
	})

	expect(schema.validate("Hello").is_valid(), "String matching pattern should validate")
	expect(schema.validate("World").is_valid(), "Another matching string should validate")
	expect(!schema.validate("hello").is_valid(), "String not matching pattern should not validate")
	expect(!schema.validate("HELLO").is_valid(), "All caps should not validate")
	expect(!schema.validate("Hello123").is_valid(), "String with numbers should not validate")

func test_string_combined_constraints() -> void:
	var schema = Schema.build_schema({
		"type": "string",
		"minLength": 3,
		"maxLength": 8,
		"pattern": "^[a-zA-Z]+$"
	})

	expect(schema.validate("abc").is_valid(), "String meeting all constraints should validate")
	expect(schema.validate("Hello").is_valid(), "Mixed case string should validate")
	expect(!schema.validate("ab").is_valid(), "Too short string should not validate")
	expect(!schema.validate("toolongstring").is_valid(), "Too long string should not validate")
	expect(!schema.validate("abc123").is_valid(), "String with numbers should not validate")
