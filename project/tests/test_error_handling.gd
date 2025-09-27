extends TestSuite

func _init() -> void:
	icon = "⚠️"

func test_validation_error_details() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"properties": {
			"name": {"type": "string", "minLength": 5}
		},
		"required": ["name"]
	})

	var result = schema.validate({"name": "hi"})
	expect(!result.is_valid(), "Invalid data should not validate")
	expect(result.error_count() > 0, "Should have at least one error")

	var error_message = result.get_error_message(0)
	expect(error_message.length() > 0, "Error message should not be empty")

func test_multiple_validation_errors() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"properties": {
			"name": {"type": "string", "minLength": 5},
			"age": {"type": "number", "minimum": 0}
		},
		"required": ["name", "age"]
	})

	var result = schema.validate({"name": "hi", "age": -5})
	expect(!result.is_valid(), "Invalid data should not validate")
	expect(result.error_count() >= 2, "Should have multiple errors")

func test_validation_error_paths() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"properties": {
			"user": {
				"type": "object",
				"properties": {
					"name": {"type": "string", "minLength": 3}
				}
			}
		}
	})

	var result = schema.validate({"user": {"name": "hi"}})
	expect(!result.is_valid(), "Invalid nested data should not validate")

	if result.error_count() > 0:
		var error_path = result.get_error_path(0)
		expect(error_path.contains("user"), "Error path should reference nested property")
