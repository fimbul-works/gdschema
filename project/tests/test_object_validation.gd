extends TestSuite

func _init() -> void:
	icon = "🏢"

func test_object_type_validation() -> void:
	var schema = Schema.build_schema({"type": "object"})

	expect(schema.validate({}).is_valid(), "Empty object should validate")
	expect(schema.validate({"key": "value"}).is_valid(), "Non-empty object should validate")
	expect(!schema.validate([]).is_valid(), "Array should not validate as object")
	expect(!schema.validate("string").is_valid(), "String should not validate as object")

func test_object_required_properties() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"properties": {
			"name": {"type": "string"},
			"age": {"type": "number"}
		},
		"required": ["name"]
	})

	expect(schema.validate({"name": "John"}).is_valid(), "Object with required property should validate")
	expect(schema.validate({"name": "John", "age": 30}).is_valid(), "Object with all properties should validate")
	expect(!schema.validate({}).is_valid(), "Object missing required property should not validate")
	expect(!schema.validate({"age": 30}).is_valid(), "Object with only optional property should not validate")

func test_object_properties_validation() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"properties": {
			"name": {"type": "string"},
			"age": {"type": "number", "minimum": 0}
		}
	})

	expect(schema.validate({"name": "John", "age": 30}).is_valid(), "Object with valid properties should validate")
	expect(schema.validate({"name": "John"}).is_valid(), "Object with some valid properties should validate")
	expect(!schema.validate({"name": 123}).is_valid(), "Object with invalid property type should not validate")
	expect(!schema.validate({"age": -5}).is_valid(), "Object with property violating constraints should not validate")

func test_object_min_properties() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"minProperties": 2
	})

	expect(schema.validate({"a": 1, "b": 2}).is_valid(), "Object with exact min properties should validate")
	expect(schema.validate({"a": 1, "b": 2, "c": 3}).is_valid(), "Object with more properties should validate")
	expect(!schema.validate({}).is_valid(), "Empty object should not validate")
	expect(!schema.validate({"a": 1}).is_valid(), "Object with fewer properties should not validate")

func test_object_max_properties() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"maxProperties": 2
	})

	expect(schema.validate({}).is_valid(), "Empty object should validate")
	expect(schema.validate({"a": 1, "b": 2}).is_valid(), "Object with exact max properties should validate")
	expect(!schema.validate({"a": 1, "b": 2, "c": 3}).is_valid(), "Object with more properties should not validate")

func test_object_additional_properties_false() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"properties": {
			"name": {"type": "string"}
		},
		"additionalProperties": false
	})

	expect(schema.validate({"name": "John"}).is_valid(), "Object with only defined properties should validate")
	expect(schema.validate({}).is_valid(), "Empty object should validate")
	expect(!schema.validate({"name": "John", "extra": "value"}).is_valid(), "Object with additional properties should not validate")

func test_object_pattern_properties() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"patternProperties": {
			"^str_": {"type": "string"},
			"^num_": {"type": "number"}
		}
	})

	expect(schema.validate({"str_name": "test", "num_age": 25}).is_valid(), "Object with valid pattern properties should validate")
	expect(!schema.validate({"str_name": 123}).is_valid(), "Object with invalid pattern property type should not validate")
	expect(!schema.validate({"num_age": "invalid"}).is_valid(), "Object with string where number expected should not validate")

func test_basic_property_dependency() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"dependencies": {
			"credit_card": ["billing_address"]
		}
	})

	expect(schema.validate({}).is_valid(), "Object without dependent property should validate")
	expect(schema.validate({"credit_card": "1234-5678", "billing_address": "123 Main St"}).is_valid(), "Object with both properties should validate")
	expect(schema.validate({"billing_address": "123 Main St"}).is_valid(), "Object with only dependent property should validate")
	expect(!schema.validate({"credit_card": "1234-5678"}).is_valid(), "Object with trigger property but missing dependency should not validate")

func test_multiple_property_dependencies() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"dependencies": {
			"credit_card": ["billing_address", "cardholder_name"]
		}
	})

	expect(schema.validate({"credit_card": "1234", "billing_address": "123 Main", "cardholder_name": "John Doe"}).is_valid(), "Object with all required properties should validate")
	expect(schema.validate({}).is_valid(), "Object without trigger property should validate")
	expect(!schema.validate({"credit_card": "1234", "billing_address": "123 Main"}).is_valid(), "Object missing one dependency should not validate")
	expect(!schema.validate({"credit_card": "1234"}).is_valid(), "Object missing all dependencies should not validate")

func test_multiple_dependency_rules() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"dependencies": {
			"credit_card": ["billing_address"],
			"shipping": ["shipping_address"],
			"insurance": ["policy_number", "provider"]
		}
	})

	expect(schema.validate({
		"credit_card": "1234",
		"billing_address": "123 Main",
		"shipping": true,
		"shipping_address": "456 Oak"
	}).is_valid(), "Object satisfying multiple dependencies should validate")

	expect(!schema.validate({
		"credit_card": "1234",
		"shipping": true,
		"shipping_address": "456 Oak"
	}).is_valid(), "Object missing billing_address dependency should not validate")

	expect(!schema.validate({
		"credit_card": "1234",
		"billing_address": "123 Main",
		"insurance": "yes"
	}).is_valid(), "Object missing insurance dependencies should not validate")

func test_dependency_with_existing_required() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"required": ["name", "email"],
		"dependencies": {
			"phone": ["phone_verified"]
		}
	})

	expect(schema.validate({"name": "John", "email": "john@example.com"}).is_valid(), "Object with only required fields should validate")
	expect(schema.validate({"name": "John", "email": "john@example.com", "phone": "123-456", "phone_verified": true}).is_valid(), "Object with required fields and dependencies should validate")
	expect(!schema.validate({"name": "John"}).is_valid(), "Object missing required field should not validate")
	expect(!schema.validate({"name": "John", "email": "john@example.com", "phone": "123-456"}).is_valid(), "Object with phone but no verification should not validate")

func test_dependency_with_empty_array() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"dependencies": {
			"prop": []  # Empty array means no dependencies
		}
	})

	expect(schema.validate({"prop": "value"}).is_valid(), "Empty dependency array should always satisfy")

# ========== SCHEMA DEPENDENCIES TESTS ==========

func test_basic_schema_dependency() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"dependencies": {
			"credit_card": {
				"properties": {
					"billing_address": {"type": "string", "minLength": 1}
				},
				"required": ["billing_address"]
			}
		}
	})

	expect(schema.validate({}).is_valid(), "Object without trigger property should validate")
	expect(schema.validate({"credit_card": "1234", "billing_address": "123 Main St"}).is_valid(), "Object with valid dependency Schema should validate")
	expect(!schema.validate({"credit_card": "1234"}).is_valid(), "Object missing required property from dependency should not validate")
	expect(!schema.validate({"credit_card": "1234", "billing_address": ""}).is_valid(), "Object with invalid property from dependency should not validate")

func test_complex_schema_dependency() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"dependencies": {
			"payment_method": {
				"if": {
					"properties": {"payment_method": {"const": "credit_card"}}
				},
				"then": {
					"required": ["card_number", "expiry_date"],
					"properties": {
						"card_number": {"type": "string", "pattern": "^\\d{4}-\\d{4}-\\d{4}-\\d{4}$"},
						"expiry_date": {"type": "string", "pattern": "^\\d{2}/\\d{2}$"}
					}
				},
				"else": {
					"if": {
						"properties": {"payment_method": {"const": "bank_transfer"}}
					},
					"then": {
						"required": ["account_number", "routing_number"]
					}
				}
			}
		}
	})

	expect(schema.validate({"payment_method": "credit_card", "card_number": "1234-5678-9012-3456", "expiry_date": "12/25"}).is_valid(), "Credit card payment with valid details should validate")
	expect(schema.validate({"payment_method": "bank_transfer", "account_number": "123456789", "routing_number": "987654321"}).is_valid(), "Bank transfer with valid details should validate")
	expect(schema.validate({"payment_method": "cash"}).is_valid(), "Cash payment should validate without extra requirements")
	expect(!schema.validate({"payment_method": "credit_card", "card_number": "invalid"}).is_valid(), "Credit card with invalid number should not validate")

func test_schema_dependency_with_allof() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"dependencies": {
			"advanced_user": {
				"allOf": [
					{"required": ["api_key"]},
					{
						"required": ["rate_limit"],  # Add this!
						"properties": {
							"rate_limit": {"type": "integer", "minimum": 1000}
						}
					}
				]
			}
		}
	})

	expect(schema.validate({"advanced_user": true, "api_key": "abc123", "rate_limit": 5000}).is_valid(), "Advanced user with all requirements should validate")
	expect(schema.validate({}).is_valid(), "Object without trigger should validate")
	expect(!schema.validate({"advanced_user": true, "api_key": "abc123"}).is_valid(), "Advanced user missing rate_limit should not validate")
	expect(!schema.validate({"advanced_user": true, "rate_limit": 500}).is_valid(), "Advanced user with low rate_limit should not validate")

# ========== MIXED PROPERTY AND SCHEMA DEPENDENCIES ==========

func test_mixed_dependencies() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"dependencies": {
			"has_car": ["driver_license"],  # Property dependency
			"employment_status": {  # Schema dependency
				"if": {
					"properties": {"employment_status": {"const": "employed"}}
				},
				"then": {
					"required": ["employer", "salary"],
					"properties": {
						"salary": {"type": "number", "minimum": 0}
					}
				}
			}
		}
	})

	expect(schema.validate({
		"has_car": true,
		"driver_license": "DL123456",
		"employment_status": "employed",
		"employer": "Tech Corp",
		"salary": 50000
	}).is_valid(), "Object satisfying both types of dependencies should validate")

	expect(!schema.validate({
		"has_car": true,
		"employment_status": "employed",
		"employer": "Tech Corp",
		"salary": 50000
	}).is_valid(), "Object missing property dependency should not validate")

	expect(!schema.validate({
		"has_car": true,
		"driver_license": "DL123456",
		"employment_status": "employed"
	}).is_valid(), "Object missing Schema dependency requirements should not validate")

# ========== DEPENDENCY CHAINS ==========

func test_dependency_chains() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"dependencies": {
			"level_1": ["level_2"],
			"level_2": ["level_3"],
			"level_3": ["level_4"]
		}
	})

	expect(schema.validate({}).is_valid(), "Empty object should validate")
	expect(schema.validate({"level_4": "value"}).is_valid(), "Object with only deepest level should validate")
	expect(schema.validate({"level_1": "a", "level_2": "b", "level_3": "c", "level_4": "d"}).is_valid(), "Object with complete chain should validate")
	expect(!schema.validate({"level_1": "a"}).is_valid(), "Object breaking chain at level 1 should not validate")
	expect(!schema.validate({"level_1": "a", "level_2": "b"}).is_valid(), "Object breaking chain at level 2 should not validate")

func test_circular_dependency_prevention() -> void:
	# Test that circular dependencies don't cause infinite loops
	var schema = Schema.build_schema({
		"type": "object",
		"dependencies": {
			"prop_a": ["prop_b"],
			"prop_b": ["prop_a"]
		}
	})

	expect(schema.validate({}).is_valid(), "Empty object should validate")
	expect(schema.validate({"prop_a": "value", "prop_b": "value"}).is_valid(), "Object with both circular dependencies should validate")
	expect(!schema.validate({"prop_a": "value"}).is_valid(), "Object with only prop_a should not validate")
	expect(!schema.validate({"prop_b": "value"}).is_valid(), "Object with only prop_b should not validate")

# ========== DEPENDENCIES WITH DIFFERENT PROPERTY TYPES ==========

func test_dependency_with_boolean_trigger() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"dependencies": {
			"is_premium": {
				"if": {
					"properties": {"is_premium": {"const": true}}
				},
				"then": {
					"required": ["premium_features"]
				}
			}
		}
	})

	expect(schema.validate({"is_premium": true, "premium_features": ["feature1"]}).is_valid(), "Premium user with features should validate")
	expect(schema.validate({"is_premium": false}).is_valid(), "Non-premium user should validate")
	expect(schema.validate({}).is_valid(), "Object without premium flag should validate")
	expect(!schema.validate({"is_premium": true}).is_valid(), "Premium user without features should not validate")

func test_dependency_with_number_trigger() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"dependencies": {
			"quantity": {
				"if": {
					"properties": {"quantity": {"minimum": 10}}
				},
				"then": {
					"required": ["bulk_discount"]
				}
			}
		}
	})

	expect(schema.validate({"quantity": 15, "bulk_discount": 0.1}).is_valid(), "Large quantity with discount should validate")
	expect(schema.validate({"quantity": 5}).is_valid(), "Small quantity should validate")
	expect(!schema.validate({"quantity": 15}).is_valid(), "Large quantity without discount should not validate")

func test_dependency_with_array_trigger() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"dependencies": {
			"tags": {
				"if": {
					"properties": {
						"tags": {
							"contains": {"const": "sensitive"}
						}
					}
				},
				"then": {
					"required": ["security_clearance"]
				}
			}
		}
	})

	expect(schema.validate({"tags": ["public", "sensitive"], "security_clearance": "high"}).is_valid(), "Sensitive item with clearance should validate")
	expect(schema.validate({"tags": ["public"]}).is_valid(), "Non-sensitive item should validate")
	expect(!schema.validate({"tags": ["public", "sensitive"]}).is_valid(), "Sensitive item without clearance should not validate")

# ========== DEPENDENCIES WITH NESTED OBJECTS ==========

func test_dependency_with_nested_properties() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"properties": {
			"user": {
				"type": "object",
				"properties": {
					"type": {"type": "string"}
				}
			}
		},
		"dependencies": {
			"user": {
				"if": {
					"properties": {
						"user": {
							"properties": {
								"type": {"const": "admin"}
							}
						}
					}
				},
				"then": {
					"properties": {
						"user": {
							"required": ["admin_level"],
							"properties": {
								"admin_level": {"type": "integer", "minimum": 1}
							}
						}
					}
				}
			}
		}
	})

	expect(schema.validate({"user": {"type": "admin", "admin_level": 5}}).is_valid(), "Admin user with level should validate")
	expect(schema.validate({"user": {"type": "regular"}}).is_valid(), "Regular user should validate")
	expect(schema.validate({}).is_valid(), "Object without user should validate")
	expect(!schema.validate({"user": {"type": "admin"}}).is_valid(), "Admin user without level should not validate")

# ========== DEPENDENCIES WITH PATTERN PROPERTIES ==========

func test_dependency_with_pattern_properties() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"patternProperties": {
			"^config_": {"type": "string"}
		},
		"dependencies": {
			"config_database": ["config_host", "config_port"]
		}
	})

	expect(schema.validate({"config_database": "postgres", "config_host": "localhost", "config_port": "5432"}).is_valid(), "Complete database config should validate")
	expect(schema.validate({"config_other": "value"}).is_valid(), "Other config without dependencies should validate")
	expect(!schema.validate({"config_database": "postgres", "config_host": "localhost"}).is_valid(), "Incomplete database config should not validate")

# ========== DEPENDENCIES WITH ADDITIONAL PROPERTIES ==========

func test_dependency_with_additional_properties() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"properties": {
			"base_prop": {"type": "string"}
		},
		"additionalProperties": {"type": "string"},
		"dependencies": {
			"needs_validation": ["validation_code"]
		}
	})

	expect(schema.validate({"base_prop": "value", "needs_validation": "yes", "validation_code": "123456"}).is_valid(), "Object with valid additional properties and dependencies should validate")
	expect(schema.validate({"base_prop": "value", "other_prop": "value"}).is_valid(), "Object without dependencies should validate")
	expect(!schema.validate({"base_prop": "value", "needs_validation": "yes"}).is_valid(), "Object missing dependency should not validate")

func test_pattern_and_additional_properties_interaction() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"properties": {
			"builtin": {"type": "string"}
		},
		"patternProperties": {
			"^i_": {"type": "integer"}
		},
		"additionalProperties": {"type": "string"}
	})

	# Properties matched by patternProperties should not be considered "additional"
	expect(schema.validate({"builtin": "a", "i_num": 1, "other": "b"}).is_valid(), "Should handle all three categories")

# ========== ERROR HANDLING ==========

func test_dependency_error_messages() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"dependencies": {
			"credit_card": ["billing_address", "cardholder_name"]
		}
	})

	var result = schema.validate({"credit_card": "1234-5678"})
	expect(!result.is_valid(), "Object should not validate")
	expect(result.error_count() > 0, "Should have error messages")

	# Error should mention missing dependencies
	if result.error_count() > 0:
		var error_msg = result.get_error_message(0).to_lower()
		expect(error_msg.contains("billing_address") or error_msg.contains("cardholder_name") or error_msg.contains("required"), "Error should mention missing dependencies")

func test_schema_dependency_error_messages() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"dependencies": {
			"user_type": {
				"if": {
					"properties": {"user_type": {"const": "premium"}}
				},
				"then": {
					"required": ["subscription_id"],
					"properties": {
						"subscription_id": {"type": "string", "minLength": 5}
					}
				}
			}
		}
	})

	var result = schema.validate({"user_type": "premium", "subscription_id": "abc"})
	expect(!result.is_valid(), "Object should not validate")
	expect(result.error_count() > 0, "Should have error messages")

# ========== EDGE CASES ==========

func test_dependency_on_nonexistent_property() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"dependencies": {
			"nonexistent": ["required_prop"]
		}
	})

	# Since nonexistent property is never present, dependency never triggers
	expect(schema.validate({}).is_valid(), "Empty object should validate")
	expect(schema.validate({"other_prop": "value"}).is_valid(), "Object with other properties should validate")
	expect(schema.validate({"required_prop": "value"}).is_valid(), "Object with dependent property should validate")

func test_self_dependency() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"dependencies": {
			"self_ref": ["self_ref"]
		}
	})

	# Self-dependency should always be satisfied if property exists
	expect(schema.validate({}).is_valid(), "Empty object should validate")
	expect(schema.validate({"self_ref": "value"}).is_valid(), "Object with self-referencing property should validate")

func test_empty_dependency_list() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"dependencies": {
			"trigger": []
		}
	})

	# Empty dependency list means no additional requirements
	expect(schema.validate({}).is_valid(), "Empty object should validate")
	expect(schema.validate({"trigger": "value"}).is_valid(), "Object with trigger property should validate")

func test_dependency_with_null_values() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"dependencies": {
			"nullable_prop": ["required_when_null"]
		}
	})

	expect(schema.validate({"nullable_prop": null, "required_when_null": "value"}).is_valid(), "Object with null trigger and dependency should validate")
	expect(schema.validate({}).is_valid(), "Object without nullable prop should validate")
	expect(!schema.validate({"nullable_prop": null}).is_valid(), "Object with null trigger but missing dependency should not validate")

# ========== PERFORMANCE TESTS ==========

func test_many_dependencies_performance() -> void:
	var dependencies = {}
	for i in range(10):
		dependencies["prop_" + str(i)] = ["dep_" + str(i)]

	var schema = Schema.build_schema({
		"type": "object",
		"dependencies": dependencies
	})

	var valid_data = {}
	for i in range(10):
		valid_data["prop_" + str(i)] = "value"
		valid_data["dep_" + str(i)] = "dependency"

	expect(schema.validate(valid_data).is_valid(), "Object satisfying many dependencies should validate")
	expect(schema.validate({}).is_valid(), "Empty object should validate quickly")

func test_deeply_nested_schema_dependencies() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"dependencies": {
			"level1": {
				"properties": {
					"level2": {
						"type": "object",
						"properties": {
							"level3": {
								"type": "object",
								"required": ["deep_prop"]
							}
						}
					}
				}
			}
		}
	})

	expect(schema.validate({"level1": "trigger", "level2": {"level3": {"deep_prop": "value"}}}).is_valid(), "Deeply nested valid structure should validate")
	expect(schema.validate({}).is_valid(), "Empty object should validate")
	expect(!schema.validate({"level1": "trigger", "level2": {"level3": {}}}).is_valid(), "Missing deep property should not validate")


func test_property_names_pattern() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"propertyNames": {
			"pattern": "^[a-z_]+$"
		}
	})

	expect(schema.validate({"valid_name": 1, "another_name": 2}).is_valid(), "Object with valid property names should validate")
	expect(schema.validate({"lowercase": "value"}).is_valid(), "Single valid property should validate")
	expect(schema.validate({}).is_valid(), "Empty object should validate")
	expect(!schema.validate({"validName": 1, "invalid_name": 2}).is_valid(), "Object with camelCase property should not validate")
	expect(!schema.validate({"UPPERCASE": 1}).is_valid(), "Object with uppercase property should not validate")
	expect(!schema.validate({"invalid-name": 1}).is_valid(), "Object with hyphenated property should not validate")

func test_property_names_minlength() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"propertyNames": {
			"minLength": 3
		}
	})

	expect(schema.validate({"abc": 1, "defg": 2}).is_valid(), "Object with property names >= 3 chars should validate")
	expect(schema.validate({"property_name": "value"}).is_valid(), "Long property name should validate")
	expect(!schema.validate({"ab": 1, "cde": 2}).is_valid(), "Object with short property name should not validate")
	expect(!schema.validate({"a": 1}).is_valid(), "Single character property should not validate")

func test_property_names_maxlength() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"propertyNames": {
			"maxLength": 10
		}
	})

	expect(schema.validate({"short": 1, "mediumname": 2}).is_valid(), "Object with property names <= 10 chars should validate")
	expect(schema.validate({"a": 1}).is_valid(), "Short property name should validate")
	expect(!schema.validate({"this_is_too_long": 1}).is_valid(), "Object with long property name should not validate")
	expect(!schema.validate({"verylongpropertyname": 1}).is_valid(), "Very long property name should not validate")

func test_property_names_type() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"propertyNames": {
			"type": "string"
		}
	})

	# All property names in JSON are strings by definition, so this should always pass
	expect(schema.validate({"any_property": 1}).is_valid(), "String property names should validate")
	expect(schema.validate({"123": "value"}).is_valid(), "Numeric string property should validate")
	expect(schema.validate({"": "empty"}).is_valid(), "Empty string property should validate")

# ========== PROPERTY NAMES WITH COMPLEX CONSTRAINTS ==========

func test_property_names_combined_constraints() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"propertyNames": {
			"type": "string",
			"minLength": 2,
			"maxLength": 20,
			"pattern": "^[a-zA-Z][a-zA-Z0-9_]*$"
		}
	})

	expect(schema.validate({"validName": 1, "another_name2": 2}).is_valid(), "Properties matching all constraints should validate")
	expect(schema.validate({"a1": "value"}).is_valid(), "Minimal valid property should validate")
	expect(!schema.validate({"a": 1}).is_valid(), "Too short property should not validate")
	expect(!schema.validate({"this_is_a_very_long_property_name": 1}).is_valid(), "Too long property should not validate")
	expect(!schema.validate({"123invalid": 1}).is_valid(), "Property starting with number should not validate")
	expect(!schema.validate({"invalid-name": 1}).is_valid(), "Property with hyphen should not validate")

func test_property_names_enum() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"propertyNames": {
			"enum": ["name", "age", "email", "address"]
		}
	})

	expect(schema.validate({"name": "John", "age": 30}).is_valid(), "Object with allowed property names should validate")
	expect(schema.validate({"email": "john@example.com"}).is_valid(), "Single allowed property should validate")
	expect(schema.validate({}).is_valid(), "Empty object should validate")
	expect(!schema.validate({"name": "John", "phone": "123-456"}).is_valid(), "Object with disallowed property should not validate")
	expect(!schema.validate({"invalid": "value"}).is_valid(), "Object with only invalid properties should not validate")

func test_property_names_const() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"propertyNames": {
			"const": "data"
		}
	})

	expect(schema.validate({"data": "value"}).is_valid(), "Object with only allowed property should validate")
	expect(schema.validate({}).is_valid(), "Empty object should validate")
	expect(!schema.validate({"data": "value", "other": "invalid"}).is_valid(), "Object with additional properties should not validate")
	expect(!schema.validate({"other": "value"}).is_valid(), "Object with different property should not validate")

# ========== PROPERTY NAMES WITH LOGICAL OPERATORS ==========

func test_property_names_allof() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"propertyNames": {
			"allOf": [
				{"minLength": 3},
				{"pattern": "^[a-z]+$"},
				{"maxLength": 10}
			]
		}
	})

	expect(schema.validate({"abc": 1, "defghij": 2}).is_valid(), "Properties satisfying all constraints should validate")
	expect(schema.validate({"hello": "world"}).is_valid(), "Single valid property should validate")
	expect(!schema.validate({"ab": 1}).is_valid(), "Property failing minLength should not validate")
	expect(!schema.validate({"HELLO": 1}).is_valid(), "Property failing pattern should not validate")
	expect(!schema.validate({"verylongname": 1}).is_valid(), "Property failing maxLength should not validate")

func test_property_names_anyof() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"propertyNames": {
			"anyOf": [
				{"pattern": "^prefix_"},
				{"pattern": "_suffix$"},
				{"enum": ["special", "allowed"]}
			]
		}
	})

	expect(schema.validate({"prefix_name": 1, "name_suffix": 2, "special": 3}).is_valid(), "Properties matching any constraint should validate")
	expect(schema.validate({"prefix_test": "value"}).is_valid(), "Property with prefix should validate")
	expect(schema.validate({"test_suffix": "value"}).is_valid(), "Property with suffix should validate")
	expect(schema.validate({"allowed": "value"}).is_valid(), "Special allowed property should validate")
	expect(!schema.validate({"invalid": 1}).is_valid(), "Property matching no constraints should not validate")

func test_property_names_oneof() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"propertyNames": {
			"oneOf": [
				{"pattern": "^[a-z]+$"},
				{"pattern": "^[A-Z]+$"}
			]
		}
	})

	expect(schema.validate({"lowercase": 1, "UPPERCASE": 2}).is_valid(), "Properties matching exactly one constraint should validate")
	expect(schema.validate({"hello": "value"}).is_valid(), "Lowercase property should validate")
	expect(schema.validate({"WORLD": "value"}).is_valid(), "Uppercase property should validate")
	expect(!schema.validate({"MixedCase": 1}).is_valid(), "Property matching no patterns should not validate")
	expect(!schema.validate({"123": 1}).is_valid(), "Numeric property should not validate")

func test_property_names_not() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"propertyNames": {
			"not": {
				"pattern": "^_"
			}
		}
	})

	expect(schema.validate({"valid": 1, "property": 2}).is_valid(), "Properties not starting with underscore should validate")
	expect(schema.validate({"name": "value"}).is_valid(), "Regular property should validate")
	expect(!schema.validate({"_private": 1}).is_valid(), "Property starting with underscore should not validate")
	expect(!schema.validate({"valid": 1, "_invalid": 2}).is_valid(), "Mix with invalid property should not validate")

# ========== PROPERTY NAMES WITH OTHER OBJECT CONSTRAINTS ==========

func test_property_names_with_properties() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"properties": {
			"valid_prop": {"type": "string"},
			"another_prop": {"type": "number"}
		},
		"propertyNames": {
			"pattern": "^[a-z_]+$"
		}
	})

	expect(schema.validate({"valid_prop": "text", "another_prop": 123}).is_valid(), "Defined properties with valid names should validate")
	expect(schema.validate({"valid_prop": "text", "extra_prop": "value"}).is_valid(), "Defined and additional properties with valid names should validate")
	expect(!schema.validate({"validProp": "text"}).is_valid(), "Property with invalid name should not validate")
	expect(!schema.validate({"valid_prop": "text", "invalidProp": "value"}).is_valid(), "Mix of valid and invalid names should not validate")

func test_property_names_with_pattern_properties() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"patternProperties": {
			"^str_": {"type": "string"},
			"^num_": {"type": "number"}
		},
		"propertyNames": {
			"pattern": "^[a-z_]+$"
		}
	})

	expect(schema.validate({"str_name": "text", "num_value": 123}).is_valid(), "Pattern properties with valid names should validate")
	expect(!schema.validate({"str_Name": "text"}).is_valid(), "Pattern property with invalid name should not validate")
	expect(!schema.validate({"STR_name": "text"}).is_valid(), "Uppercase pattern property should not validate")

func test_property_names_with_additional_properties() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"properties": {
			"known": {"type": "string"}
		},
		"additionalProperties": {"type": "number"},
		"propertyNames": {
			"minLength": 3
		}
	})

	expect(schema.validate({"known": "text", "extra": 123}).is_valid(), "Properties with valid names and types should validate")
	expect(!schema.validate({"known": "text", "ex": 123}).is_valid(), "Additional property with short name should not validate")
	expect(!schema.validate({"xy": "text"}).is_valid(), "Known property with short name should not validate")

func test_property_names_with_required() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"required": ["mandatory_field"],
		"propertyNames": {
			"pattern": "^[a-z_]+$"
		}
	})

	expect(schema.validate({"mandatory_field": "value", "optional_field": "data"}).is_valid(), "Required and optional properties with valid names should validate")
	expect(!schema.validate({"mandatoryField": "value"}).is_valid(), "Required property with invalid name should not validate")
	expect(!schema.validate({"mandatory_field": "value", "optionalField": "data"}).is_valid(), "Optional property with invalid name should not validate")

# ========== PROPERTY NAMES WITH NESTED OBJECTS ==========

func test_property_names_nested_objects() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"properties": {
			"outer_nested": {
				"type": "object",
				"propertyNames": {
					"pattern": "^inner_"
				}
			}
		},
		"propertyNames": {
			"pattern": "^outer_"
		}
	})

	expect(schema.validate({
		"outer_nested": {  # Use the property name defined in schema
			"inner_prop": "value",
			"inner_data": 123
		}
	}).is_valid(), "Nested objects with valid property names should validate")

	expect(!schema.validate({
		"invalid_prop": {
			"inner_prop": "value"
		}
	}).is_valid(), "Outer object with invalid property name should not validate")

	expect(!schema.validate({
		"outer_nested": {
			"invalid_prop": "value"
		}
	}).is_valid(), "Inner object with invalid property name should not validate")

func test_property_names_deeply_nested() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"propertyNames": {"minLength": 2},
		"properties": {
			"level1": {
				"type": "object",
				"propertyNames": {"pattern": "^l1_"},
				"properties": {
					"l1_level2": {
						"type": "object",
						"propertyNames": {"pattern": "^l2_"}
					}
				}
			}
		}
	})

	expect(schema.validate({
		"level1": {
			"l1_prop": "value",
			"l1_level2": {
				"l2_deep": "nested"
			}
		}
	}).is_valid(), "Deeply nested objects with valid names should validate")

	expect(!schema.validate({
		"level1": {
			"l1_prop": "value",
			"l1_level2": {
				"invalid": "nested"
			}
		}
	}).is_valid(), "Deep object with invalid property name should not validate")

# ========== PROPERTY NAMES WITH REFERENCES ==========

func test_property_names_with_refs() -> void:
	var schema = Schema.build_schema({
		"definitions": {
			"name_pattern": {
				"pattern": "^[a-zA-Z][a-zA-Z0-9_]*$"
			}
		},
		"type": "object",
		"propertyNames": {
			"$ref": "#/definitions/name_pattern"
		}
	})

	expect(schema.validate({"validName": 1, "another_name2": 2}).is_valid(), "Properties matching referenced pattern should validate")
	expect(!schema.validate({"123invalid": 1}).is_valid(), "Property not matching referenced pattern should not validate")

func test_property_names_with_recursive_refs() -> void:
	var schema = Schema.build_schema({
		"definitions": {
			"valid_object": {
				"propertyNames": {"$ref": "#/definitions/valid_name"},
				"additionalProperties": {"$ref": "#/definitions/valid_object"}
			},
			"valid_name": {
				"pattern": "^[a-z_]+$"
			}
		},
		"$ref": "#/definitions/valid_object"
	})

	expect(schema.validate({
		"level_one": {
			"level_two": {
				"level_three": "value"
			}
		}
	}).is_valid(), "Recursive structure with valid names should validate")

	expect(!schema.validate({
		"level_one": {
			"InvalidName": {
				"level_three": "value"
			}
		}
	}).is_valid(), "Recursive structure with invalid name should not validate")

# ========== ERROR HANDLING ==========

func test_property_names_error_messages() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"propertyNames": {
			"pattern": "^[a-z]+$",
			"minLength": 3
		}
	})

	var result := schema.validate({"ab": 1, "InvalidName": 2})
	expect(!result.is_valid(), "Object should not validate")
	expect(result.error_count() > 0, "Should have error messages")

	# Check that errors mention property name validation
	if result.error_count() > 0:
		var has_property_name_error = false
		var summary := result.get_summary()
		if summary.contains("property") and (summary.contains("name") or summary.contains("pattern") or summary.contains("length")):
			has_property_name_error = true
		expect(has_property_name_error, "Should have property name related error")

# ========== EDGE CASES ==========

func test_property_names_empty_object() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"propertyNames": {
			"minLength": 5
		}
	})

	expect(schema.validate({}).is_valid(), "Empty object should validate regardless of property name constraints")

func test_property_names_false_schema() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"propertyNames": false
	})

	expect(schema.validate({}).is_valid(), "Empty object should validate")
	expect(!schema.validate({"any": "property"}).is_valid(), "Object with any property should not validate when propertyNames is false")

func test_property_names_true_schema() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"propertyNames": true
	})

	expect(schema.validate({}).is_valid(), "Empty object should validate")
	expect(schema.validate({"any": "property", "multiple": "properties"}).is_valid(), "Object with any properties should validate when propertyNames is true")

func test_property_names_empty_schema() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"propertyNames": {}
	})

	expect(schema.validate({}).is_valid(), "Empty object should validate")
	expect(schema.validate({"any": "property"}).is_valid(), "Object with any properties should validate with empty propertyNames schema")

func test_property_names_with_special_characters() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"propertyNames": {
			"pattern": "^[a-zA-Z0-9_$]+$"
		}
	})

	expect(schema.validate({"valid_name": 1, "name$with$dollar": 2, "name123": 3}).is_valid(), "Properties with allowed special characters should validate")
	expect(!schema.validate({"invalid-name": 1}).is_valid(), "Property with hyphen should not validate")
	expect(!schema.validate({"invalid.name": 1}).is_valid(), "Property with dot should not validate")
	expect(!schema.validate({"invalid name": 1}).is_valid(), "Property with space should not validate")

func test_property_names_unicode() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"propertyNames": {
			"minLength": 1,
			"maxLength": 20
		}
	})

	# Test with unicode characters (if supported by Godot)
	expect(schema.validate({"naïve": "value", "café": "coffee"}).is_valid(), "Unicode property names should validate")
	expect(schema.validate({"日本語": "japanese"}).is_valid(), "Non-Latin unicode should validate")

func test_property_names_numeric_strings() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"propertyNames": {
			"pattern": "^\\d+$"
		}
	})

	expect(schema.validate({"123": "value", "456": "data"}).is_valid(), "Numeric string properties should validate")
	expect(schema.validate({"0": "zero"}).is_valid(), "Single digit property should validate")
	expect(!schema.validate({"123a": "invalid"}).is_valid(), "Alphanumeric property should not validate")
	expect(!schema.validate({"abc": "invalid"}).is_valid(), "Alphabetic property should not validate")

# ========== PERFORMANCE TESTS ==========

func test_property_names_many_properties() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"propertyNames": {
			"pattern": "^prop_\\d+$"
		}
	})

	var large_object = {}
	for i in range(100):
		large_object["prop_" + str(i)] = i

	expect(schema.validate(large_object).is_valid(), "Object with many valid properties should validate")

	# Add one invalid property
	large_object["invalid_prop"] = "bad"
	expect(!schema.validate(large_object).is_valid(), "Object with one invalid property among many should not validate")

func test_property_names_complex_regex() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"propertyNames": {
			"pattern": "^[a-zA-Z][a-zA-Z0-9]*(_[a-zA-Z0-9]+)*$"
		}
	})

	expect(schema.validate({"validName": 1, "valid_name_123": 2, "a": 3}).is_valid(), "Properties matching complex regex should validate")
	expect(!schema.validate({"_invalid": 1}).is_valid(), "Property starting with underscore should not validate")
	expect(!schema.validate({"invalid__double": 1}).is_valid(), "Property with double underscore should not validate")
	expect(!schema.validate({"123invalid": 1}).is_valid(), "Property starting with number should not validate")
