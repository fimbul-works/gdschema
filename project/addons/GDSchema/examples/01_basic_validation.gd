extends Node

## Basic Schema Validation Examples
## This example demonstrates simple type validation and constraints

func _ready() -> void:
	print("=== Basic Validation Examples ===\n")

	example_string_validation()
	example_number_validation()
	example_array_validation()
	example_combined_constraints()

## Example 1: String Validation
func example_string_validation() -> void:
	print("--- String Validation ---")

	var schema = Schema.build_schema({
		"type": "string",
		"minLength": 3,
		"maxLength": 20,
		"pattern": "^[a-zA-Z0-9_]+$"
	})

	# Valid username
	var result1 = schema.validate("player123")
	print("✓ 'player123': ", result1.is_valid())  # true

	# Too short
	var result2 = schema.validate("ab")
	print("✗ 'ab': ", result2.is_valid())  # false
	if result2.has_errors():
		print("  Error: ", result2.get_error_message(0))

	# Invalid characters
	var result3 = schema.validate("player@123")
	print("✗ 'player@123': ", result3.is_valid())  # false
	if result3.has_errors():
		print("  Error: ", result3.get_error_message(0))
	print()

## Example 2: Number Validation
func example_number_validation() -> void:
	print("--- Number Validation ---")

	var schema = Schema.build_schema({
		"type": "integer",
		"minimum": 1,
		"maximum": 100,
		"multipleOf": 5
	})

	# Valid values
	print("✓ 25: ", schema.validate(25).is_valid())  # true
	print("✓ 100: ", schema.validate(100).is_valid())  # true

	# Invalid values
	print("✗ 0: ", schema.validate(0).is_valid())  # false (below minimum)
	print("✗ 23: ", schema.validate(23).is_valid())  # false (not multiple of 5)
	print("✗ 150: ", schema.validate(150).is_valid())  # false (above maximum)
	print()

## Example 3: Array Validation
func example_array_validation() -> void:
	print("--- Array Validation ---")

	var schema = Schema.build_schema({
		"type": "array",
		"items": {"type": "string"},
		"minItems": 1,
		"maxItems": 5,
		"uniqueItems": true
	})

	# Valid array
	var result1 = schema.validate(["apple", "banana", "cherry"])
	print("✓ Unique fruits: ", result1.is_valid())  # true

	# Duplicate items
	var result2 = schema.validate(["apple", "apple"])
	print("✗ Duplicate fruits: ", result2.is_valid())  # false

	# Too many items
	var result3 = schema.validate(["a", "b", "c", "d", "e", "f"])
	print("✗ Too many items: ", result3.is_valid())  # false
	print()

## Example 4: Combined Constraints
func example_combined_constraints() -> void:
	print("--- Combined Constraints ---")

	var schema = Schema.build_schema({
		"type": "object",
		"properties": {
			"username": {
				"type": "string",
				"minLength": 3,
				"maxLength": 20
			},
			"level": {
				"type": "integer",
				"minimum": 1,
				"maximum": 100
			},
			"score": {
				"type": "number",
				"minimum": 0
			}
		},
		"required": ["username", "level"]
	})

	# Valid player data
	var valid_data = {
		"username": "hero123",
		"level": 42,
		"score": 1250.5
	}
	print("✓ Valid player: ", schema.validate(valid_data).is_valid())

	# Missing required field
	var invalid_data = {
		"username": "hero123"
		# Missing 'level'
	}
	var result = schema.validate(invalid_data)
	print("✗ Missing level: ", result.is_valid())
	if result.has_errors():
		print("  Error: ", result.get_error_message(0))
	print()
