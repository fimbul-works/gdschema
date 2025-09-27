extends TestSuite

func _init() -> void:
	icon = "🔗"

func test_simple_local_ref() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"properties": {
			"name": "string",
			"another": {
				"$ref": "#/properties/name"
			}
		}
	})
	expect(schema.validate({ "name": "test", "another": "should work" }).is_valid(), "Should validate same type")
	expect(!schema.validate({ "name": "test", "another": 123 }).is_valid(), "Should not validate another type")

func test_root_reference() -> void:
	# Test "#" - reference to root schema
	var schema = Schema.build_schema({
		"type": "object",
		"properties": {
			"name": { "type": "string" },
			"nested": { "$ref": "#" }  # Points to root
		}
	})

	# Should allow recursive nesting
	var valid_data = {
		"name": "root",
		"nested": {
			"name": "child",
			"nested": {
				"name": "grandchild"
			}
		}
	}

	var invalid_data = {
		"name": "root",
		"nested": {
			"name": 123  # Wrong type
		}
	}

	expect(schema.validate(valid_data).is_valid(), "Root reference should allow recursive nesting")
	expect(!schema.validate(invalid_data).is_valid(), "Root reference should validate nested structure")

func test_property_references() -> void:
	# Test references to specific properties
	var schema = Schema.build_schema({
		"type": "object",
		"properties": {
			"email": {
				"type": "string",
				"pattern": "^[^@]+@[^@]+\\.[^@]+$"
			},
			"primary_email": { "$ref": "#/properties/email" },
			"backup_email": { "$ref": "#/properties/email" },
			"contact": {
				"type": "object",
				"properties": {
					"work_email": { "$ref": "#/properties/email" }
				}
			}
		}
	})

	var valid_data = {
		"email": "user@example.com",
		"primary_email": "primary@example.com",
		"backup_email": "backup@example.com",
		"contact": {
			"work_email": "work@company.com"
		}
	}

	var invalid_data = {
		"email": "user@example.com",
		"primary_email": "not-an-email",  # Invalid format
		"backup_email": "backup@example.com"
	}

	expect(schema.validate(valid_data).is_valid(), "Property references should validate correctly")
	expect(!schema.validate(invalid_data).is_valid(), "Property references should catch format violations")

func test_deep_json_pointer() -> void:
	# Test deep JSON pointer paths
	var schema = Schema.build_schema({
		"type": "object",
		"properties": {
			"config": {
				"type": "object",
				"properties": {
					"database": {
						"type": "object",
						"properties": {
							"connection": {
								"type": "object",
								"properties": {
									"host": { "type": "string" },
									"port": { "type": "integer", "minimum": 1, "maximum": 65535 }
								}
							}
						}
					}
				}
			},
			"primary_db": { "$ref": "#/properties/config/properties/database/properties/connection" },
			"backup_db": { "$ref": "#/properties/config/properties/database/properties/connection" }
		}
	})

	var valid_data = {
		"config": {
			"database": {
				"connection": {
					"host": "localhost",
					"port": 5432
				}
			}
		},
		"primary_db": {
			"host": "primary.db.com",
			"port": 3306
		},
		"backup_db": {
			"host": "backup.db.com",
			"port": 5432
		}
	}

	var invalid_data = {
		"primary_db": {
			"host": "primary.db.com",
			"port": 99999  # Port out of range
		}
	}

	expect(schema.validate(valid_data).is_valid(), "Deep JSON pointer should resolve correctly")
	expect(!schema.validate(invalid_data).is_valid(), "Deep JSON pointer should validate constraints")

func test_array_item_references() -> void:
	# Test references in array contexts
	var schema = Schema.build_schema({
		"type": "object",
		"properties": {
			"user_template": {
				"type": "object",
				"properties": {
					"name": { "type": "string", "minLength": 1 },
					"age": { "type": "integer", "minimum": 0 }
				},
				"required": ["name"]
			},
			"users": {
				"type": "array",
				"items": { "$ref": "#/properties/user_template" }
			},
			"admins": {
				"type": "array",
				"items": { "$ref": "#/properties/user_template" }
			}
		}
	})

	var valid_data = {
		"users": [
			{ "name": "Alice", "age": 25 },
			{ "name": "Bob" }
		],
		"admins": [
			{ "name": "Admin", "age": 35 }
		]
	}

	var invalid_data = {
		"users": [
			{ "name": "Alice", "age": 25 },
			{ "age": 30 }  # Missing required name
		]
	}

	expect(schema.validate(valid_data).is_valid(), "Array item references should work")
	expect(!schema.validate(invalid_data).is_valid(), "Array item references should validate items")

func test_definitions_references() -> void:
	# Test references to definitions section
	var schema = Schema.build_schema({
		"definitions": {
			"address": {
				"type": "object",
				"properties": {
					"street": { "type": "string" },
					"city": { "type": "string" },
					"zipcode": { "type": "string", "pattern": "^[0-9]{5}$" }
				},
				"required": ["street", "city", "zipcode"]
			},
			"person": {
				"type": "object",
				"properties": {
					"name": { "type": "string" },
					"home_address": { "$ref": "#/definitions/address" },
					"work_address": { "$ref": "#/definitions/address" }
				}
			}
		},
		"type": "object",
		"properties": {
			"employee": { "$ref": "#/definitions/person" },
			"company_address": { "$ref": "#/definitions/address" }
		}
	})

	var valid_data = {
		"employee": {
			"name": "John",
			"home_address": {
				"street": "123 Home St",
				"city": "Hometown",
				"zipcode": "12345"
			}
		},
		"company_address": {
			"street": "456 Business Ave",
			"city": "Business City",
			"zipcode": "54321"
		}
	}

	var invalid_data = {
		"company_address": {
			"street": "456 Business Ave",
			"city": "Business City",
			"zipcode": "invalid"  # Wrong pattern
		}
	}

	expect(schema.validate(valid_data).is_valid(), "References to definitions should work")
	expect(!schema.validate(invalid_data).is_valid(), "References to definitions should validate")

func test_nested_references() -> void:
	# Test references that point to schemas containing other references
	var schema = Schema.build_schema({
		"definitions": {
			"coordinate": { "type": "number" },
			"point": {
				"type": "array",
				"items": { "$ref": "#/definitions/coordinate" },
				"minItems": 2,
				"maxItems": 2
			},
			"line": {
				"type": "object",
				"properties": {
					"start": { "$ref": "#/definitions/point" },
					"end": { "$ref": "#/definitions/point" }
				},
				"required": ["start", "end"]
			}
		},
		"type": "object",
		"properties": {
			"primary_line": { "$ref": "#/definitions/line" },
			"backup_line": { "$ref": "#/definitions/line" }
		}
	})

	var valid_data = {
		"primary_line": {
			"start": [0, 0],
			"end": [10, 5]
		},
		"backup_line": {
			"start": [1, 1],
			"end": [2, 2]
		}
	}

	var invalid_data = {
		"primary_line": {
			"start": [0, 0],
			"end": ["not", "numbers"]  # Wrong types in nested reference
		}
	}

	expect(schema.validate(valid_data).is_valid(), "Nested references should resolve correctly")
	expect(!schema.validate(invalid_data).is_valid(), "Nested references should validate all levels")
	print("Unexpected!")

func test_circular_references() -> void:
	# Test circular reference detection and handling
	var schema = Schema.build_schema({
		"type": "object",
		"properties": {
			"value": { "type": "string" },
			"child": { "$ref": "#" },  # Self-reference
			"children": {
				"type": "array",
				"items": { "$ref": "#" }
			}
		}
	})

	# This should work - circular references are allowed in data
	var valid_circular = {
		"value": "root",
		"child": {
			"value": "child1",
			"child": {
				"value": "grandchild"
			}
		},
		"children": [
			{ "value": "child2" },
			{ "value": "child3", "child": { "value": "nested" } }
		]
	}

	var invalid_circular = {
		"value": "root",
		"child": {
			"value": 123  # Wrong type in circular reference
		}
	}

	expect(schema.validate(valid_circular).is_valid(), "Circular references should allow valid recursive data")
	expect(!schema.validate(invalid_circular).is_valid(), "Circular references should validate recursive data")

func test_logical_composition_references() -> void:
	# Test references within allOf, anyOf, oneOf, not
	var schema = Schema.build_schema({
		"definitions": {
			"base_user": {
				"type": "object",
				"properties": {
					"name": { "type": "string" },
					"id": { "type": "integer" }
				},
				"required": ["name", "id"]
			},
			"admin_props": {
				"type": "object",
				"properties": {
					"admin_level": { "type": "integer", "minimum": 1, "maximum": 10 }
				},
				"required": ["admin_level"]
			}
		},
		"type": "object",
		"properties": {
			"admin_user": {
				"allOf": [
					{ "$ref": "#/definitions/base_user" },
					{ "$ref": "#/definitions/admin_props" }
				]
			},
			"any_user": {
				"anyOf": [
					{ "$ref": "#/definitions/base_user" },
					{ "type": "string" }  # Could also be just a name string
				]
			},
			"not_admin": {
				"allOf": [
					{ "$ref": "#/definitions/base_user" },
					{ "not": { "$ref": "#/definitions/admin_props" } }
				]
			}
		}
	})

	var valid_data = {
		"admin_user": {
			"name": "Admin",
			"id": 1,
			"admin_level": 5
		},
		"any_user": "Just a name string",
		"not_admin": {
			"name": "Regular User",
			"id": 2
		}
	}

	var invalid_data = {
		"admin_user": {
			"name": "Admin",
			"id": 1
			# Missing admin_level
		}
	}

	expect(schema.validate(valid_data).is_valid(), "References in logical composition should work")
	expect(!schema.validate(invalid_data).is_valid(), "References in logical composition should validate")

func test_conditional_references() -> void:
	# Test references in if/then/else contexts
	var schema = Schema.build_schema({
		"definitions": {
			"adult_info": {
				"type": "object",
				"properties": {
					"job": { "type": "string" },
					"salary": { "type": "number" }
				},
				"required": ["job", "salary"]
			},
			"child_info": {
				"type": "object",
				"properties": {
					"school": { "type": "string" },
					"grade": { "type": "integer" }
				},
				"required": ["school", "grade"]
			}
		},
		"type": "object",
		"properties": {
			"name": { "type": "string" },
			"age": { "type": "integer" }
		},
		"if": {
			"properties": { "age": { "minimum": 18 } }
		},
		"then": {
			"allOf": [
				{ "$ref": "#" },  # Keep base properties
				{ "$ref": "#/definitions/adult_info" }
			]
		},
		"else": {
			"allOf": [
				{ "$ref": "#" },  # Keep base properties
				{ "$ref": "#/definitions/child_info" }
			]
		}
	})

	var adult_data = {
		"name": "Adult",
		"age": 25,
		"job": "Engineer",
		"salary": 50000
	}

	var child_data = {
		"name": "Child",
		"age": 10,
		"school": "Elementary",
		"grade": 5
	}

	var invalid_adult = {
		"name": "Adult",
		"age": 25,
		"school": "Should not have school"  # Adult should not have child properties
	}

	expect(schema.validate(adult_data).is_valid(), "Conditional references should work for adults")
	expect(schema.validate(child_data).is_valid(), "Conditional references should work for children")
	expect(!schema.validate(invalid_adult).is_valid(), "Conditional references should enforce correct branch")

func test_json_pointer_escaping() -> void:
	# Test JSON pointer escaping rules (~0 for ~, ~1 for /)
	var schema = Schema.build_schema({
		"properties": {
			"tilde~field": { "type": "string" },
			"slash/field": { "type": "integer" },
			"both~/fields": { "type": "boolean" }
		},
		"type": "object",
		"additionalProperties": {
			"anyOf": [
				{ "$ref": "#/properties/tilde~0field" },      # ~0 = ~
				{ "$ref": "#/properties/slash~1field" },      # ~1 = /
				{ "$ref": "#/properties/both~0~1fields" }     # ~0~1 = ~/
			]
		}
	})

	var valid_data = {
		"tilde~field": "test",
		"slash/field": 42,
		"both~/fields": true,
		"extra1": "string",     # Should match tilde~field type
		"extra2": 123,          # Should match slash/field type
		"extra3": false         # Should match both~/fields type
	}

	expect(schema.validate(valid_data).is_valid(), "JSON pointer escaping should work correctly")

func test_reference_error_cases() -> void:
	# Test various error conditions

	# Non-existent reference
	var bad_ref_schema = Schema.build_schema({
		"type": "object",
		"properties": {
			"test": { "$ref": "#/nonexistent/path" }
		}
	})

	var result1 = bad_ref_schema.validate({"test": "value"})
	expect(!result1.is_valid(), "Non-existent reference should fail validation")
	expect(result1.get_errors().size() > 0, "Should have error messages for bad reference")

	# Invalid reference format
	var invalid_ref_schema = Schema.build_schema({
		"type": "object",
		"properties": {
			"test": { "$ref": "" }  # Empty reference
		}
	})

	var result2 = invalid_ref_schema.validate({"test": "value"})
	expect(!result2.is_valid(), "Empty reference should fail validation")

func test_multiple_reference_levels() -> void:
	# Test complex multi-level reference chains
	var schema = Schema.build_schema({
		"definitions": {
			"level1": {
				"type": "object",
				"properties": {
					"data": { "$ref": "#/definitions/level2" }
				}
			},
			"level2": {
				"type": "object",
				"properties": {
					"value": { "$ref": "#/definitions/level3" }
				}
			},
			"level3": {
				"type": "string",
				"minLength": 1
			}
		},
		"$ref": "#/definitions/level1"
	})

	var valid_data = {
		"data": {
			"value": "final value"
		}
	}

	var invalid_data = {
		"data": {
			"value": ""  # Too short
		}
	}

	expect(schema.validate(valid_data).is_valid(), "Multi-level references should resolve correctly")
	expect(!schema.validate(invalid_data).is_valid(), "Multi-level references should validate at all levels")

func test_reference_with_additional_properties() -> void:
	# Test references combined with additionalProperties
	var schema = Schema.build_schema({
		"definitions": {
			"base_schema": {
				"type": "string",
				"minLength": 3
			}
		},
		"type": "object",
		"properties": {
			"known_field": { "type": "integer" }
		},
		"additionalProperties": { "$ref": "#/definitions/base_schema" }
	})

	var valid_data = {
		"known_field": 42,
		"extra1": "valid string",
		"extra2": "another valid string"
	}

	var invalid_data = {
		"known_field": 42,
		"extra1": "ok",  # Too short for base_schema
		"extra2": "valid string"
	}

	expect(schema.validate(valid_data).is_valid(), "References in additionalProperties should work")
	expect(!schema.validate(invalid_data).is_valid(), "References in additionalProperties should validate")
