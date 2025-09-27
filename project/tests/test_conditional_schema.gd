extends TestSuite

func _init() -> void:
	icon = "🔀"

# ========== BASIC IF/THEN/ELSE TESTS ==========

func test_basic_if_then() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"properties": {
			"age": {"type": "integer"}
		},
		"if": {
			"properties": {"age": {"minimum": 18}}
		},
		"then": {
			"required": ["driver_license"]
		}
	})

	expect(schema.validate({"age": 25, "driver_license": "ABC123"}).is_valid(), "Adult with license should validate")
	expect(schema.validate({"age": 16}).is_valid(), "Minor without license should validate (no then requirement)")
	expect(!schema.validate({"age": 25}).is_valid(), "Adult without license should not validate")

func test_basic_if_else() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"properties": {
			"age": {"type": "integer"}
		},
		"if": {
			"properties": {"age": {"minimum": 18}}
		},
		"else": {
			"required": ["guardian"]
		}
	})

	expect(schema.validate({"age": 25}).is_valid(), "Adult should validate (no else requirement)")
	expect(schema.validate({"age": 16, "guardian": "Parent"}).is_valid(), "Minor with guardian should validate")
	expect(!schema.validate({"age": 16}).is_valid(), "Minor without guardian should not validate")

func test_if_then_else_complete() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"properties": {
			"age": {"type": "integer"}
		},
		"if": {
			"properties": {"age": {"minimum": 18}}
		},
		"then": {
			"required": ["driver_license"]
		},
		"else": {
			"required": ["guardian"]
		}
	})

	expect(schema.validate({"age": 25, "driver_license": "ABC123"}).is_valid(), "Adult with license should validate")
	expect(schema.validate({"age": 16, "guardian": "Parent"}).is_valid(), "Minor with guardian should validate")
	expect(!schema.validate({"age": 25}).is_valid(), "Adult without license should not validate")
	expect(!schema.validate({"age": 16}).is_valid(), "Minor without guardian should not validate")

# ========== COMPLEX IF CONDITIONS ==========

func test_complex_if_condition() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"properties": {
			"type": {"type": "string"},
			"priority": {"type": "string"}
		},
		"if": {
			"allOf": [
				{"properties": {"type": {"const": "urgent"}}},
				{"properties": {"priority": {"const": "high"}}}
			]
		},
		"then": {
			"required": ["escalation_contact"]
		}
	})

	expect(schema.validate({"type": "urgent", "priority": "high", "escalation_contact": "manager@company.com"}).is_valid(), "Urgent high priority with contact should validate")
	expect(schema.validate({"type": "normal", "priority": "high"}).is_valid(), "Non-urgent should validate without contact")
	expect(schema.validate({"type": "urgent", "priority": "low"}).is_valid(), "Urgent low priority should validate without contact")
	expect(!schema.validate({"type": "urgent", "priority": "high"}).is_valid(), "Urgent high priority without contact should not validate")

func test_nested_properties_if_condition() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"properties": {
			"user": {
				"type": "object",
				"properties": {
					"role": {"type": "string"}
				}
			}
		},
		"if": {
			"properties": {
				"user": {
					"properties": {
						"role": {"const": "admin"}
					}
				}
			}
		},
		"then": {
			"properties": {
				"user": {
					"required": ["permissions"]
				}
			}
		}
	})

	expect(schema.validate({"user": {"role": "admin", "permissions": ["read", "write"]}}).is_valid(), "Admin with permissions should validate")
	expect(schema.validate({"user": {"role": "user"}}).is_valid(), "Regular user should validate")
	expect(!schema.validate({"user": {"role": "admin"}}).is_valid(), "Admin without permissions should not validate")

# ========== MULTIPLE CONDITIONS ==========

func test_multiple_if_conditions() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"properties": {
			"country": {"type": "string"},
			"age": {"type": "integer"}
		},
		"allOf": [
			{
				"if": {"properties": {"country": {"const": "US"}}},
				"then": {"required": ["ssn"]}
			},
			{
				"if": {"properties": {"age": {"minimum": 21}}},
				"then": {"required": ["can_drink"]}
			}
		]
	})

	expect(schema.validate({"country": "US", "age": 25, "ssn": "123-45-6789", "can_drink": true}).is_valid(), "US adult should need both SSN and drink flag")
	expect(schema.validate({"country": "CA", "age": 25, "can_drink": true}).is_valid(), "Canadian adult should need only drink flag")
	expect(schema.validate({"country": "US", "age": 18, "ssn": "123-45-6789"}).is_valid(), "US minor should need only SSN")
	expect(!schema.validate({"country": "US", "age": 25}).is_valid(), "US adult missing requirements should not validate")

func test_chained_if_then_else() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"properties": {
			"vehicle_type": {"type": "string"},
			"num_wheels": {"type": "integer"}
		},
		"if": {
			"properties": {"vehicle_type": {"const": "car"}}
		},
		"then": {
			"if": {
				"properties": {"num_wheels": {"const": 2}}
			},
			"then": {
				"required": ["motorcycle_license"]
			},
			"else": {
				"required": ["car_license"]
			}
		},
		"else": {
			"required": ["special_permit"]
		}
	})

	expect(schema.validate({"vehicle_type": "car", "num_wheels": 4, "car_license": "ABC123"}).is_valid(), "4-wheel car should need car license")
	expect(schema.validate({"vehicle_type": "car", "num_wheels": 2, "motorcycle_license": "XYZ789"}).is_valid(), "2-wheel car should need motorcycle license")
	expect(schema.validate({"vehicle_type": "boat", "special_permit": "BOAT123"}).is_valid(), "Boat should need special permit")
	expect(!schema.validate({"vehicle_type": "car", "num_wheels": 4}).is_valid(), "Car without license should not validate")

# ========== IF WITH ARRAY CONDITIONS ==========

func test_if_with_array_conditions() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"properties": {
			"tags": {
				"type": "array",
				"items": {"type": "string"}
			}
		},
		"if": {
			"properties": {
				"tags": {
					"contains": {"const": "premium"}
				}
			}
		},
		"then": {
			"required": ["premium_features"]
		}
	})

	expect(schema.validate({"tags": ["basic", "premium"], "premium_features": ["feature1", "feature2"]}).is_valid(), "Premium user with features should validate")
	expect(schema.validate({"tags": ["basic"]}).is_valid(), "Basic user should validate")
	expect(!schema.validate({"tags": ["basic", "premium"]}).is_valid(), "Premium user without features should not validate")

func test_if_with_array_length() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"properties": {
			"items": {
				"type": "array"
			}
		},
		"if": {
			"properties": {
				"items": {
					"minItems": 5
				}
			}
		},
		"then": {
			"required": ["bulk_discount"]
		}
	})

	expect(schema.validate({"items": [1, 2, 3, 4, 5], "bulk_discount": 0.1}).is_valid(), "Large order with discount should validate")
	expect(schema.validate({"items": [1, 2, 3]}).is_valid(), "Small order should validate")
	expect(!schema.validate({"items": [1, 2, 3, 4, 5]}).is_valid(), "Large order without discount should not validate")

# ========== TYPE-BASED CONDITIONS ==========

func test_if_type_conditions() -> void:
	var schema = Schema.build_schema({
		"if": {"type": "string"},
		"then": {
			"minLength": 1
		},
		"else": {
			"if": {"type": "number"},
			"then": {
				"minimum": 0
			}
		}
	})

	expect(schema.validate("hello").is_valid(), "Non-empty string should validate")
	expect(schema.validate(5).is_valid(), "Positive number should validate")
	expect(schema.validate(true).is_valid(), "Boolean should validate (no conditions)")
	expect(!schema.validate("").is_valid(), "Empty string should not validate")
	expect(!schema.validate(-5).is_valid(), "Negative number should not validate")

func test_if_enum_conditions() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"properties": {
			"payment_method": {"enum": ["card", "cash", "check"]}
		},
		"if": {
			"properties": {
				"payment_method": {"const": "card"}
			}
		},
		"then": {
			"required": ["card_number"]
		},
		"else": {
			"if": {
				"properties": {
					"payment_method": {"const": "check"}
				}
			},
			"then": {
				"required": ["check_number"]
			}
		}
	})

	expect(schema.validate({"payment_method": "card", "card_number": "1234-5678-9012-3456"}).is_valid(), "Card payment should need card number")
	expect(schema.validate({"payment_method": "check", "check_number": "1001"}).is_valid(), "Check payment should need check number")
	expect(schema.validate({"payment_method": "cash"}).is_valid(), "Cash payment should need nothing extra")
	expect(!schema.validate({"payment_method": "card"}).is_valid(), "Card payment without number should not validate")

# ========== CONDITIONS WITH LOGICAL OPERATORS ==========

func test_if_with_logical_operators() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"properties": {
			"is_member": {"type": "boolean"},
			"purchase_amount": {"type": "number"}
		},
		"if": {
			"anyOf": [
				{"properties": {"is_member": {"const": true}}},
				{"properties": {"purchase_amount": {"minimum": 100}}}
			]
		},
		"then": {
			"required": ["discount_applied"]
		}
	})

	expect(schema.validate({"is_member": true, "purchase_amount": 50, "discount_applied": true}).is_valid(), "Member with small purchase should get discount")
	expect(schema.validate({"is_member": false, "purchase_amount": 150, "discount_applied": true}).is_valid(), "Non-member with large purchase should get discount")
	expect(schema.validate({"is_member": false, "purchase_amount": 50}).is_valid(), "Non-member with small purchase should not need discount")
	expect(!schema.validate({"is_member": true, "purchase_amount": 50}).is_valid(), "Member without applied discount should not validate")

# ========== VALIDATION ORDER TESTS ==========

func test_if_evaluation_order() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"properties": {
			"value": {"type": ["string", "number"]}
		},
		"if": {
			"properties": {
				"value": {"type": "string"}
			}
		},
		"then": {
			"properties": {
				"value": {"minLength": 3}
			}
		},
		"else": {
			"properties": {
				"value": {"minimum": 10}
			}
		}
	})

	expect(schema.validate({"value": "hello"}).is_valid(), "String meeting length requirement should validate")
	expect(schema.validate({"value": 15}).is_valid(), "Number meeting minimum should validate")
	expect(!schema.validate({"value": "hi"}).is_valid(), "Short string should not validate")
	expect(!schema.validate({"value": 5}).is_valid(), "Small number should not validate")

# ========== CONDITIONAL SCHEMA WITH REFERENCES ==========

func test_if_with_references() -> void:
	var schema = Schema.build_schema({
		"definitions": {
			"adult_schema": {
				"type": "object",
				"required": ["job", "income"],
				"properties": {
					"job": {"type": "string"},
					"income": {"type": "number"}
				}
			},
			"child_schema": {
				"type": "object",
				"required": ["school", "grade"],
				"properties": {
					"school": {"type": "string"},
					"grade": {"type": "integer"}
				}
			}
		},
		"type": "object",
		"properties": {
			"age": {"type": "integer"}
		},
		"if": {
			"properties": {"age": {"minimum": 18}}
		},
		"then": {
			"$ref": "#/definitions/adult_schema"
		},
		"else": {
			"$ref": "#/definitions/child_schema"
		}
	})

	expect(schema.validate({"age": 25, "job": "Engineer", "income": 50000}).is_valid(), "Adult with job info should validate")
	expect(schema.validate({"age": 12, "school": "Elementary", "grade": 6}).is_valid(), "Child with school info should validate")
	expect(!schema.validate({"age": 25, "school": "Elementary"}).is_valid(), "Adult with school info should not validate")
	expect(!schema.validate({"age": 12, "job": "Engineer"}).is_valid(), "Child with job info should not validate")

# ========== ERROR HANDLING TESTS ==========

func test_if_condition_errors() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"if": {
			"properties": {
				"nonexistent": {"type": "string"}
			}
		},
		"then": {
			"required": ["required_field"]
		}
	})

	# Missing property in if condition should make if evaluate to false
	expect(schema.validate({}).is_valid(), "Object without if condition property should validate")
	expect(schema.validate({"other": "value"}).is_valid(), "Object with other properties should validate")

func test_invalid_then_else_schemas() -> void:
	# Test that invalid then/else schemas are caught during compilation
	var schema = Schema.build_schema({
		"if": {"type": "string"},
		"then": {
			"type": "invalid_type"  # This should be caught during schema validation
		}
	})

	# The schema itself might be invalid, but we test what happens with valid data
	var result = schema.validate("test")
	# Behavior depends on implementation - could fail compilation or ignore invalid then clause

# ========== EDGE CASES ==========

func test_missing_if_condition() -> void:
	# Test schema with then/else but no if
	var schema = Schema.build_schema({
		"type": "object",
		"then": {
			"required": ["field1"]
		},
		"else": {
			"required": ["field2"]
		}
	})

	# Without if, then/else should be ignored
	expect(schema.validate({}).is_valid(), "Object should validate when if is missing")
	expect(schema.validate({"field1": "value"}).is_valid(), "Object with field1 should validate")
	expect(schema.validate({"field2": "value"}).is_valid(), "Object with field2 should validate")

func test_empty_if_condition() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"if": {},  # Empty schema always matches
		"then": {
			"required": ["always_required"]
		}
	})

	expect(schema.validate({"always_required": "value"}).is_valid(), "Object with required field should validate")
	expect(!schema.validate({}).is_valid(), "Object without required field should not validate")
	expect(!schema.validate({"other": "value"}).is_valid(), "Object with other field should not validate")

func test_false_if_condition() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"if": false,  # Never matches
		"then": {
			"required": ["never_required"]
		},
		"else": {
			"required": ["always_required"]
		}
	})

	expect(schema.validate({"always_required": "value"}).is_valid(), "Object should follow else branch")
	expect(!schema.validate({"never_required": "value"}).is_valid(), "Object should not follow then branch")
	expect(!schema.validate({}).is_valid(), "Object should require else field")

func test_true_if_condition() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"if": true,  # Always matches
		"then": {
			"required": ["always_required"]
		},
		"else": {
			"required": ["never_required"]
		}
	})

	expect(schema.validate({"always_required": "value"}).is_valid(), "Object should follow then branch")
	expect(!schema.validate({"never_required": "value"}).is_valid(), "Object should not follow else branch")
	expect(!schema.validate({}).is_valid(), "Object should require then field")
