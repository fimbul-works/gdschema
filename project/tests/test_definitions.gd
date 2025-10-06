extends TestSuite

func _init() -> void:
	icon = "📚"

func test_defs_basic() -> void:
	var schema = Schema.build_schema({
		"$defs": {
			"name": {
				"type": "string",
				"minLength": 1
			},
			"age": {
				"type": "integer",
				"minimum": 0
			}
		},
		"type": "object",
		"properties": {
			"user_name": { "$ref": "#/$defs/name" },
			"user_age": { "$ref": "#/$defs/age" }
		}
	})

	# Test valid data
	expect(schema.validate({ "user_name": "John", "user_age": 25 }).is_valid(), "Should validate with $defs references")

	# Test invalid data
	expect(!schema.validate({ "user_name": "", "user_age": 25 }).is_valid(), "Should fail with empty name")
	expect(!schema.validate({ "user_name": "John", "user_age": -5 }).is_valid(), "Should fail with negative age")

func test_defs_vs_definitions() -> void:
	# Test both $defs (modern) and definitions (legacy) work
	var modern_schema = Schema.build_schema({
		"$defs": {
			"person": { "type": "string" }
		},
		"type": "object",
		"properties": {
			"name": { "$ref": "#/$defs/person" }
		}
	})

	var legacy_schema = Schema.build_schema({
		"definitions": {
			"person": { "type": "string" }
		},
		"type": "object",
		"properties": {
			"name": { "$ref": "#/definitions/person" }
		}
	})

	var test_data = { "name": "Alice" }
	expect(modern_schema.validate(test_data).is_valid(), "$defs should work")
	expect(legacy_schema.validate(test_data).is_valid(), "definitions should work")

func test_nested_defs() -> void:
	# Test definitions at non-root levels (allowed by spec)
	var schema = Schema.build_schema({
		"type": "object",
		"properties": {
			"billing": {
				"type": "object",
				"$defs": {
					"address": {
						"type": "object",
						"properties": {
							"street": { "type": "string" },
							"city": { "type": "string" }
						},
						"required": ["street", "city"]
					}
				},
				"properties": {
					"home": { "$ref": "#/properties/billing/$defs/address" },
					"work": { "$ref": "#/properties/billing/$defs/address" }
				}
			}
		}
	})

	var valid_data = {
		"billing": {
			"home": { "street": "123 Main St", "city": "Anytown" },
			"work": { "street": "456 Oak Ave", "city": "Somewhere" }
		}
	}

	var invalid_data = {
		"billing": {
			"home": { "street": "123 Main St" }, # Missing city
			"work": { "street": "456 Oak Ave", "city": "Somewhere" }
		}
	}

	expect(schema.validate(valid_data).is_valid(), "Should validate nested $defs")
	expect(!schema.validate(invalid_data).is_valid(), "Should fail with missing required field")

func test_complex_defs_composition() -> void:
	# Test $defs with complex compositions (allOf, anyOf, etc.)
	var schema = Schema.build_schema({
		"$defs": {
			"base_user": {
				"type": "object",
				"properties": {
					"name": { "type": "string", "minLength": 1 },
					"id": { "type": "integer", "minimum": 1 }
				},
				"required": ["name", "id"]
			},
			"admin_user": {
				"allOf": [
					{ "$ref": "#/$defs/base_user" },
					{
						"type": "object",
						"properties": {
							"admin_level": { "type": "integer", "minimum": 1, "maximum": 10 }
						},
						"required": ["admin_level"]
					}
				]
			},
			"guest_user": {
				"allOf": [
					{ "$ref": "#/$defs/base_user" },
					{
						"type": "object",
						"properties": {
							"guest_expires": { "type": "string" }
						},
						"required": ["guest_expires"]
					}
				]
			}
		},
		"type": "object",
		"properties": {
			"users": {
				"type": "array",
				"items": {
					"anyOf": [
						{ "$ref": "#/$defs/admin_user" },
						{ "$ref": "#/$defs/guest_user" }
					]
				}
			}
		}
	})

	var valid_data = {
		"users": [
			{ "name": "Admin", "id": 1, "admin_level": 5 },
			{ "name": "Guest", "id": 2, "guest_expires": "2024-12-31" }
		]
	}

	var invalid_data = {
		"users": [
			{ "name": "Admin", "id": 1 }, # Missing admin_level
			{ "name": "Guest", "id": 2, "guest_expires": "2024-12-31" }
		]
	}

	expect(schema.validate(valid_data).is_valid(), "Should validate complex $defs composition")
	expect(!schema.validate(invalid_data).is_valid(), "Should fail with missing admin_level")

func test_defs_with_arrays() -> void:
	# Test $defs with array item definitions
	var schema = Schema.build_schema({
		"$defs": {
			"coordinate": {
				"type": "number"
			},
			"point": {
				"type": "array",
				"items": { "$ref": "#/$defs/coordinate" },
				"minItems": 2,
				"maxItems": 3
			},
			"polygon": {
				"type": "array",
				"items": { "$ref": "#/$defs/point" },
				"minItems": 3
			}
		},
		"type": "object",
		"properties": {
			"shape": { "$ref": "#/$defs/polygon" }
		}
	})

	var valid_triangle = {
		"shape": [
			[0, 0],
			[1, 0],
			[0.5, 1]
		]
	}

	var invalid_shape = {
		"shape": [
			[0, 0],
			["not a number", 0], # Invalid coordinate
			[0.5, 1]
		]
	}

	expect(schema.validate(valid_triangle).is_valid(), "Should validate nested array $defs")
	expect(!schema.validate(invalid_shape).is_valid(), "Should fail with invalid coordinate type")

func test_defs_recursive() -> void:
	# Test recursive definitions (like JSON tree structures)
	var schema = Schema.build_schema({
		"$defs": {
			"node": {
				"type": "object",
				"properties": {
					"value": { "type": ["string", "number"] },
					"children": {
						"type": "array",
						"items": { "$ref": "#/$defs/node" }
					}
				},
				"required": ["value"]
			}
		},
		"$ref": "#/$defs/node"  # Root is a node
	})

	var valid_tree = {
		"value": "root",
		"children": [
			{ "value": "child1" },
			{
				"value": "child2",
				"children": [
					{ "value": "grandchild" }
				]
			}
		]
	}

	var invalid_tree = {
		"value": "root",
		"children": [
			{ "value": "child1" },
			{ "children": [] } # Missing required 'value'
		]
	}

	expect(schema.validate(valid_tree).is_valid(), "Should validate recursive $defs")
	expect(!schema.validate(invalid_tree).is_valid(), "Should fail with missing required field in recursion")

func test_defs_error_messages() -> void:
	# Test that error messages correctly reference $defs paths
	var schema = Schema.build_schema({
		"$defs": {
			"email": {
				"type": "string",
				"pattern": "^[^@]+@[^@]+\\.[^@]+$"
			}
		},
		"type": "object",
		"properties": {
			"contact": { "$ref": "#/$defs/email" }
		}
	})

	var result = schema.validate({ "contact": "not-an-email" })
	expect(!result.is_valid(), "Should fail validation")

	# Check that error mentions the path correctly
	var errors = result.get_errors()
	expect(errors.size() > 0, "Should have validation errors")

	# The error should reference the actual property path, not the $defs path
	if errors.size() > 0:
		var error_path = errors[0]["instance_path"]
		expect(error_path == "/contact", "Error path should be /contact, got: " + str(error_path))

func test_mixed_defs_and_definitions() -> void:
	# Test schema with both $defs and definitions (should work)
	var schema = Schema.build_schema({
		"definitions": {
			"old_style": { "type": "string" }
		},
		"$defs": {
			"new_style": { "type": "integer" }
		},
		"type": "object",
		"properties": {
			"legacy": { "$ref": "#/definitions/old_style" },
			"modern": { "$ref": "#/$defs/new_style" }
		}
	})

	expect(schema.validate({ "legacy": "text", "modern": 42 }).is_valid(), "Should handle mixed defs/definitions")
	expect(!schema.validate({ "legacy": 123, "modern": 42 }).is_valid(), "Should fail with wrong legacy type")
	expect(!schema.validate({ "legacy": "text", "modern": "text" }).is_valid(), "Should fail with wrong modern type")
