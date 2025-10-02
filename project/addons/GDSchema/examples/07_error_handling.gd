extends Node

## Error Handling Examples
## This example demonstrates working with validation errors

func _ready() -> void:
	print("=== Error Handling Examples ===\n")

	basic_error_reporting()
	detailed_error_inspection()
	multiple_errors()
	nested_error_paths()
	user_friendly_errors()

## Example 1: Basic Error Reporting
func basic_error_reporting() -> void:
	print("--- Basic Error Reporting ---")

	var schema = Schema.build_schema({
		"type": "integer",
		"minimum": 0,
		"maximum": 100
	})

	var result = schema.validate(150)

	if result.has_errors():
		print("Validation failed!")
		print("Number of errors: ", result.error_count())
		print("Error message: ", result.get_error_message(0))
		print("Full summary:\n", result.get_summary())
	print()

## Example 2: Detailed Error Inspection
func detailed_error_inspection() -> void:
	print("--- Detailed Error Inspection ---")

	var schema = Schema.build_schema({
		"type": "object",
		"properties": {
			"username": {"type": "string", "minLength": 3},
			"age": {"type": "integer", "minimum": 0}
		},
		"required": ["username", "age"]
	})

	var invalid_data = {
		"username": "ab",  # Too short
		"age": -5  # Negative
	}

	var result = schema.validate(invalid_data)

	if result.has_errors():
		print("Found %d error(s):" % result.error_count())
		for i in range(result.error_count()):
			print("\nError #%d:" % (i + 1))
			print("  Path: ", result.get_error_path(i))
			print("  Constraint: ", result.get_error_constraint(i))
			print("  Message: ", result.get_error_message(i))
			print("  Value: ", result.get_error_value(i))
	print()

## Example 3: Multiple Validation Errors
func multiple_errors() -> void:
	print("--- Multiple Errors ---")

	var schema = Schema.build_schema({
		"type": "object",
		"properties": {
			"email": {
				"type": "string",
				"format": "email",
				"minLength": 5
			},
			"password": {
				"type": "string",
				"minLength": 8,
				"pattern": "^(?=.*[a-z])(?=.*[A-Z])(?=.*\\d).+$"
			},
			"age": {
				"type": "integer",
				"minimum": 13
			}
		},
		"required": ["email", "password", "age"]
	})

	var bad_data = {
		"email": "bad",  # Invalid format AND too short
		"password": "weak",  # Too short AND no uppercase/numbers
		"age": 10  # Below minimum
	}

	var result = schema.validate(bad_data)
	print("Total errors: ", result.error_count())
	print("\n" + result.get_summary())

## Example 4: Nested Error Paths
func nested_error_paths() -> void:
	print("--- Nested Error Paths ---")

	var schema = Schema.build_schema({
		"type": "object",
		"properties": {
			"player": {
				"type": "object",
				"properties": {
					"stats": {
						"type": "object",
						"properties": {
							"health": {"type": "integer", "minimum": 1}
						}
					}
				}
			}
		}
	})

	var data = {
		"player": {
			"stats": {
				"health": -10  # Invalid!
			}
		}
	}

	var result = schema.validate(data)
	if result.has_errors():
		print("Error at nested path:")
		print("  Path: ", result.get_error_path(0))
		print("  Message: ", result.get_error_message(0))
		print("\nYou can use the path to guide users to the exact problem location.")
	print()

## Example 5: User-Friendly Error Messages
func user_friendly_errors() -> void:
	print("--- User-Friendly Error Messages ---")

	var schema = Schema.build_schema({
		"type": "object",
		"properties": {
			"username": {"type": "string", "minLength": 3, "maxLength": 20},
			"email": {"type": "string", "format": "email"},
			"password": {"type": "string", "minLength": 8}
		},
		"required": ["username", "email", "password"]
	})

	var user_input = {
		"username": "ab",
		"email": "invalid-email",
		"password": "short"
	}

	var result = schema.validate(user_input)

	if result.has_errors():
		print("Please fix the following issues:\n")

		# Convert technical errors to user-friendly messages
		for i in range(result.error_count()):
			var path = result.get_error_path(i)
			var constraint = result.get_error_constraint(i)
			var message = result.get_error_message(i)

			# Extract field name from path
			var field = path.trim_prefix("/")

			# Create friendly message based on constraint type
			var friendly_msg = ""
			match constraint:
				"minLength":
					friendly_msg = "%s is too short. Minimum length is required." % field
				"format":
					friendly_msg = "%s format is invalid. Please check your input." % field
				"required":
					friendly_msg = "%s is required." % field
				_:
					friendly_msg = "%s: %s" % [field, message]

			print("  • ", friendly_msg)
	print()

## Bonus: Error Recovery Strategy
func error_recovery_example() -> void:
	print("--- Error Recovery Strategy ---")

	var schema = Schema.build_schema({
		"type": "object",
		"properties": {
			"config_version": {"type": "integer"},
			"settings": {"type": "object"}
		},
		"required": ["config_version"]
	})

	# Try to validate user config
	var user_config = {
		"settings": {"volume": 0.8}
		# Missing config_version!
	}

	var result = schema.validate(user_config)

	if !result.is_valid():
		print("Config validation failed. Attempting recovery...")

		# Check what's wrong
		if result.has_errors():
			var error_msg = result.get_error_message(0)
			if "config_version" in error_msg and "required" in error_msg:
				# Add missing version with default value
				user_config["config_version"] = 1
				print("Added default config_version = 1")

				# Retry validation
				result = schema.validate(user_config)
				if result.is_valid():
					print("✓ Config recovered successfully!")
				else:
					print("✗ Config still invalid after recovery")
	else:
		print("✓ Config is valid")

## Bonus: Collecting and Logging Errors
func collect_and_log_errors() -> void:
	print("--- Error Collection and Logging ---")

	var schema = Schema.build_schema({
		"type": "array",
		"items": {
			"type": "object",
			"properties": {
				"id": {"type": "integer"},
				"name": {"type": "string"}
			},
			"required": ["id", "name"]
		}
	})

	var items = [
		{"id": 1, "name": "Valid Item"},
		{"id": "not a number", "name": "Invalid Item"},  # Bad ID
		{"id": 3}  # Missing name
	]

	var result = schema.validate(items)

	if result.has_errors():
		# Collect all errors for logging
		var error_log = []
		for i in range(result.error_count()):
			error_log.append({
				"timestamp": Time.get_datetime_string_from_system(),
				"path": result.get_error_path(i),
				"constraint": result.get_error_constraint(i),
				"message": result.get_error_message(i),
				"value": result.get_error_value(i)
			})

		print("Collected %d errors for logging:" % error_log.size())
		for error in error_log:
			print("  [%s] %s: %s" % [error.timestamp, error.path, error.message])
