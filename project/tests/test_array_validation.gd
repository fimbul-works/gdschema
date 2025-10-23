extends TestSuite

func _init() -> void:
	icon = "📊"

func test_array_type_validation() -> void:
	var schema = Schema.build_schema({"type": "array"})

	expect(schema.validate([]).is_valid(), "Empty array should validate")
	expect(schema.validate([1, 2, 3]).is_valid(), "Non-empty array should validate")
	expect(!schema.validate("not array").is_valid(), "String should not validate as array")
	expect(!schema.validate(123).is_valid(), "Number should not validate as array")

func test_array_min_items() -> void:
	var schema = Schema.build_schema({
		"type": "array",
		"minItems": 2
	})

	expect(schema.validate([1, 2]).is_valid(), "Array with exact min items should validate")
	expect(schema.validate([1, 2, 3]).is_valid(), "Array with more than min items should validate")
	expect(!schema.validate([]).is_valid(), "Empty array should not validate")
	expect(!schema.validate([1]).is_valid(), "Array with fewer items should not validate")

func test_array_max_items() -> void:
	var schema = Schema.build_schema({
		"type": "array",
		"maxItems": 3
	})

	expect(schema.validate([]).is_valid(), "Empty array should validate")
	expect(schema.validate([1, 2, 3]).is_valid(), "Array with exact max items should validate")
	expect(!schema.validate([1, 2, 3, 4]).is_valid(), "Array with more than max items should not validate")

func test_array_unique_items() -> void:
	var schema = Schema.build_schema({
		"type": "array",
		"uniqueItems": true
	})

	expect(schema.validate([1, 2, 3]).is_valid(), "Array with unique items should validate")
	expect(schema.validate([{ "type": "string" }, { "type": "number" }]).is_valid(), "Unique dictionaries should validate")
	expect(schema.validate([]).is_valid(), "Empty array should validate")
	expect(!schema.validate([1, 2, 2]).is_valid(), "Array with duplicates should not validate")
	expect(!schema.validate(["a", "b", "a"]).is_valid(), "String array with duplicates should not validate")
	expect(!schema.validate([{ "type": "string" }, { "type": "string" }]).is_valid(), "Duplicate dictionaries should not validate")

func test_array_items_validation() -> void:
	var schema = Schema.build_schema({
		"type": "array",
		"items": {
			"type": "string",
			"minLength": 2
		}
	})

	expect(schema.validate(["ab", "cd"]).is_valid(), "Array with valid items should validate")
	expect(schema.validate([]).is_valid(), "Empty array should validate")
	expect(!schema.validate(["a", "b"]).is_valid(), "Array with invalid items should not validate")
	expect(!schema.validate([1, 2]).is_valid(), "Array with wrong type items should not validate")

func test_array_combined_constraints() -> void:
	var schema = Schema.build_schema({
		"type": "array",
		"items": {"type": "string"},
		"minItems": 1,
		"maxItems": 3,
		"uniqueItems": true
	})

	expect(schema.validate(["a"]).is_valid(), "Valid array should validate")
	expect(schema.validate(["a", "b", "c"]).is_valid(), "Array at max size should validate")
	expect(!schema.validate([]).is_valid(), "Empty array should not validate (minItems)")
	expect(!schema.validate(["a", "b", "c", "d"]).is_valid(), "Too large array should not validate")
	expect(!schema.validate(["a", "a"]).is_valid(), "Array with duplicates should not validate")

# ========== BASIC CONTAINS TESTS ==========

func test_basic_contains_string() -> void:
	var schema = Schema.build_schema({
		"type": "array",
		"contains": {
			"type": "string",
			"minLength": 5
		}
	})

	expect(schema.validate(["short", "longer_string"]).is_valid(), "Array with one matching item should validate")
	expect(schema.validate(["hello", "world"]).is_valid(), "Array with multiple matching items should validate")
	expect(schema.validate(["very_long_string"]).is_valid(), "Array with single matching item should validate")
	expect(!schema.validate(["a", "hi", "bye"]).is_valid(), "Array with no matching items should not validate")
	expect(!schema.validate([]).is_valid(), "Empty array should not validate")

func test_basic_contains_number() -> void:
	var schema = Schema.build_schema({
		"type": "array",
		"contains": {
			"type": "number",
			"minimum": 10
		}
	})

	expect(schema.validate([5, 15, 8]).is_valid(), "Array with one number >= 10 should validate")
	expect(schema.validate([20, 25, 30]).is_valid(), "Array with all numbers >= 10 should validate")
	expect(schema.validate([10]).is_valid(), "Array with exactly 10 should validate")
	expect(!schema.validate([1, 2, 3]).is_valid(), "Array with all numbers < 10 should not validate")
	expect(!schema.validate([]).is_valid(), "Empty array should not validate")

func test_contains_object() -> void:
	var schema = Schema.build_schema({
		"type": "array",
		"contains": {
			"type": "object",
			"properties": {
				"status": {"const": "active"}
			},
			"required": ["status"]
		}
	})

	expect(schema.validate([{"status": "inactive"}, {"status": "active"}]).is_valid(), "Array with one active object should validate")
	expect(schema.validate([{"status": "active", "name": "test"}]).is_valid(), "Array with active object and extra properties should validate")
	expect(!schema.validate([{"status": "inactive"}, {"status": "pending"}]).is_valid(), "Array with no active objects should not validate")
	expect(!schema.validate([{"name": "test"}]).is_valid(), "Array with objects missing status should not validate")

# ========== CONTAINS WITH DIFFERENT TYPES ==========

func test_contains_multiple_types() -> void:
	var schema = Schema.build_schema({
		"type": "array",
		"contains": {
			"type": ["string", "number"]
		}
	})

	expect(schema.validate([true, "hello", false]).is_valid(), "Array with string should validate")
	expect(schema.validate([true, 42, false]).is_valid(), "Array with number should validate")
	expect(schema.validate(["text", 123]).is_valid(), "Array with both string and number should validate")
	expect(!schema.validate([true, false, []]).is_valid(), "Array with no strings or numbers should not validate")

func test_contains_const_value() -> void:
	var schema = Schema.build_schema({
		"type": "array",
		"contains": {
			"const": "special"
		}
	})

	expect(schema.validate(["normal", "special", "other"]).is_valid(), "Array containing exact value should validate")
	expect(schema.validate(["special"]).is_valid(), "Array with only the special value should validate")
	expect(!schema.validate(["normal", "other"]).is_valid(), "Array without special value should not validate")
	expect(!schema.validate(["SPECIAL"]).is_valid(), "Array with case-different value should not validate")

func test_contains_enum_value() -> void:
	var schema = Schema.build_schema({
		"type": "array",
		"contains": {
			"enum": ["red", "green", "blue"]
		}
	})

	expect(schema.validate(["yellow", "red", "purple"]).is_valid(), "Array with one enum value should validate")
	expect(schema.validate(["red", "green", "blue"]).is_valid(), "Array with all enum values should validate")
	expect(schema.validate(["blue"]).is_valid(), "Array with single enum value should validate")
	expect(!schema.validate(["yellow", "purple", "orange"]).is_valid(), "Array with no enum values should not validate")

# ========== CONTAINS WITH COMPLEX CONSTRAINTS ==========

func test_contains_complex_string() -> void:
	var schema = Schema.build_schema({
		"type": "array",
		"contains": {
			"type": "string",
			"minLength": 3,
			"maxLength": 10,
			"pattern": "^[A-Z][a-z]+$"
		}
	})

	expect(schema.validate(["test", "Hello", "123"]).is_valid(), "Array with properly formatted string should validate")
	expect(schema.validate(["World", "Universe"]).is_valid(), "Array with multiple valid strings should validate")
	expect(!schema.validate(["test", "hello", "123"]).is_valid(), "Array with no capitalized strings should not validate")
	expect(!schema.validate(["Hi", "HELLO", "123"]).is_valid(), "Array with too short or all-caps strings should not validate")

func test_contains_complex_number() -> void:
	var schema = Schema.build_schema({
		"type": "array",
		"contains": {
			"type": "number",
			"minimum": 0,
			"maximum": 100,
			"multipleOf": 5
		}
	})

	expect(schema.validate([3, 25, 101]).is_valid(), "Array with valid multiple of 5 should validate")
	expect(schema.validate([50, 75, 100]).is_valid(), "Array with multiple valid numbers should validate")
	expect(!schema.validate([3, 7, 101]).is_valid(), "Array with no valid multiples should not validate")
	expect(!schema.validate([105, 110, 115]).is_valid(), "Array with out-of-range multiples should not validate")

func test_contains_complex_object() -> void:
	var schema = Schema.build_schema({
		"type": "array",
		"contains": {
			"type": "object",
			"properties": {
				"name": {"type": "string", "minLength": 1},
				"age": {"type": "integer", "minimum": 0},
				"active": {"const": true}
			},
			"required": ["name", "age", "active"],
			"additionalProperties": false
		}
	})

	var valid_array = [
		{"name": "John", "age": 25, "active": false},
		{"name": "Jane", "age": 30, "active": true},
		{"invalid": "object"}
	]

	var invalid_array = [
		{"name": "John", "age": 25, "active": false},
		{"name": "", "age": 30, "active": true},  # Empty name
		{"name": "Bob", "age": -5, "active": true}  # Negative age
	]

	expect(schema.validate(valid_array).is_valid(), "Array with one fully valid object should validate")
	expect(!schema.validate(invalid_array).is_valid(), "Array with no fully valid objects should not validate")

# ========== CONTAINS WITH ARRAY ITEMS ==========

func test_contains_nested_array() -> void:
	var schema = Schema.build_schema({
		"type": "array",
		"contains": {
			"type": "array",
			"minItems": 2,
			"items": {"type": "number"}
		}
	})

	expect(schema.validate([[1], [2, 3], ["a", "b"]]).is_valid(), "Array with one valid nested array should validate")
	expect(schema.validate([[1, 2, 3], [4, 5]]).is_valid(), "Array with multiple valid nested arrays should validate")
	expect(!schema.validate([[1], ["a"], [true]]).is_valid(), "Array with no valid nested arrays should not validate")

func test_contains_array_with_specific_items() -> void:
	var schema = Schema.build_schema({
		"type": "array",
		"contains": {
			"type": "array",
			"items": [
				{"type": "string"},
				{"type": "number"}
			],
			"minItems": 2
		}
	})

	expect(schema.validate([["hello", 42], [1, 2, 3]]).is_valid(), "Array with properly structured nested array should validate")
	expect(schema.validate([[1, 2], ["hello", 42, "extra"]]).is_valid(), "Array with valid structure and extra items should validate")
	expect(!schema.validate([["hello"], [1, 2]]).is_valid(), "Array with no properly structured nested arrays should not validate")

# ========== CONTAINS WITH LOGICAL OPERATORS ==========

func test_contains_with_allof() -> void:
	var schema = Schema.build_schema({
		"type": "array",
		"contains": {
			"allOf": [
				{"type": "number"},
				{"minimum": 10},
				{"multipleOf": 3}
			]
		}
	})

	expect(schema.validate([5, 12, 7]).is_valid(), "Array with number that satisfies all conditions should validate")
	expect(schema.validate([15, 18, 21]).is_valid(), "Array with multiple valid numbers should validate")
	expect(!schema.validate([5, 8, 11]).is_valid(), "Array with no numbers satisfying all conditions should not validate")
	expect(!schema.validate([3, 6, 9]).is_valid(), "Array with multiples of 3 but < 10 should not validate")

func test_contains_with_anyof() -> void:
	var schema = Schema.build_schema({
		"type": "array",
		"contains": {
			"anyOf": [
				{"type": "string", "minLength": 10},
				{"type": "number", "minimum": 100}
			]
		}
	})

	expect(schema.validate(["short", "very_long_string", 50]).is_valid(), "Array with long string should validate")
	expect(schema.validate(["short", "medium", 150]).is_valid(), "Array with large number should validate")
	expect(schema.validate(["very_long_string", 150]).is_valid(), "Array with both conditions should validate")
	expect(!schema.validate(["short", "medium", 50]).is_valid(), "Array satisfying neither condition should not validate")

func test_contains_with_oneof() -> void:
	var schema = Schema.build_schema({
		"type": "array",
		"contains": {
			"oneOf": [
				{"type": "string", "maxLength": 5},
				{"type": "string", "minLength": 10}
			]
		}
	})

	expect(schema.validate(["hello", "medium_string", "very_long_string"]).is_valid(), "Array with short string should validate")
	expect(schema.validate(["medium", "very_long_string"]).is_valid(), "Array with long string should validate")
	expect(!schema.validate(["medium", "average"]).is_valid(), "Array with medium strings (satisfying neither or both) should not validate")

func test_contains_with_not() -> void:
	var schema = Schema.build_schema({
		"type": "array",
		"contains": {
			"not": {
				"type": "string"
			}
		}
	})

	expect(schema.validate(["hello", 42, "world"]).is_valid(), "Array with non-string should validate")
	expect(schema.validate([true, false, []]).is_valid(), "Array with only non-strings should validate")
	expect(!schema.validate(["hello", "world", "test"]).is_valid(), "Array with only strings should not validate")

# ========== CONTAINS WITH OTHER ARRAY CONSTRAINTS ==========

func test_contains_with_minitems() -> void:
	var schema = Schema.build_schema({
		"type": "array",
		"minItems": 3,
		"contains": {
			"type": "string",
			"minLength": 5
		}
	})

	expect(schema.validate(["a", "b", "hello"]).is_valid(), "Array meeting both minItems and contains should validate")
	expect(!schema.validate(["hello"]).is_valid(), "Array with contains match but too few items should not validate")
	expect(!schema.validate(["a", "b", "c"]).is_valid(), "Array with enough items but no contains match should not validate")

func test_contains_with_maxitems() -> void:
	var schema = Schema.build_schema({
		"type": "array",
		"maxItems": 3,
		"contains": {
			"type": "number",
			"minimum": 10
		}
	})

	expect(schema.validate([5, 15]).is_valid(), "Array meeting both maxItems and contains should validate")
	expect(!schema.validate([5, 15, 8, 12]).is_valid(), "Array with contains match but too many items should not validate")
	expect(!schema.validate([5, 8, 9]).is_valid(), "Array within size limit but no contains match should not validate")

func test_contains_with_uniqueitems() -> void:
	var schema = Schema.build_schema({
		"type": "array",
		"uniqueItems": true,
		"contains": {
			"type": "string",
			"const": "unique"
		}
	})

	expect(schema.validate(["a", "unique", "b"]).is_valid(), "Array with unique items and contains match should validate")
	expect(!schema.validate(["a", "unique", "a"]).is_valid(), "Array with duplicate items should not validate")
	expect(!schema.validate(["a", "b", "c"]).is_valid(), "Array with unique items but no contains match should not validate")

func test_contains_with_items_schema() -> void:
	var schema = Schema.build_schema({
		"type": "array",
		"items": {
			"type": ["string", "number"]
		},
		"contains": {
			"type": "string",
			"minLength": 3
		}
	})

	expect(schema.validate(["ab", 123, "hello"]).is_valid(), "Array where all items match items Schema and one matches contains should validate")
	expect(!schema.validate([true, 123, "hello"]).is_valid(), "Array with item not matching items Schema should not validate")
	expect(!schema.validate(["ab", 123, "hi"]).is_valid(), "Array with no items matching contains should not validate")

# ========== MULTIPLE CONTAINS REQUIREMENTS ==========

func test_multiple_contains_via_allof() -> void:
	var schema = Schema.build_schema({
		"type": "array",
		"allOf": [
			{
				"contains": {
					"type": "string",
					"minLength": 5
				}
			},
			{
				"contains": {
					"type": "number",
					"minimum": 10
				}
			}
		]
	})

	expect(schema.validate(["hello", 15, "a"]).is_valid(), "Array satisfying both contains requirements should validate")
	expect(schema.validate(["very_long_string", 100]).is_valid(), "Array with items satisfying both requirements should validate")
	expect(!schema.validate(["hello", 5, "a"]).is_valid(), "Array missing number >= 10 should not validate")
	expect(!schema.validate(["hi", 15, "a"]).is_valid(), "Array missing string >= 5 chars should not validate")

# ========== CONTAINS WITH REFERENCES ==========

func test_contains_with_ref() -> void:
	var schema = Schema.build_schema({
		"definitions": {
			"valid_item": {
				"type": "object",
				"properties": {
					"id": {"type": "integer", "minimum": 1},
					"name": {"type": "string", "minLength": 1}
				},
				"required": ["id", "name"]
			}
		},
		"type": "array",
		"contains": {
			"$ref": "#/definitions/valid_item"
		}
	})

	expect(schema.validate([{"invalid": true}, {"id": 1, "name": "valid"}]).is_valid(), "Array with one item matching reference should validate")
	expect(!schema.validate([{"invalid": true}, {"id": 0, "name": ""}]).is_valid(), "Array with no items matching reference should not validate")

# ========== TUPLE VALIDATION ==========

func test_tuple_validation_with_additional_items() -> void:
	var schema = Schema.build_schema({
		"type": "array",
		"items": [
			{"type": "string"},
			{"type": "number"}
		],
		"additionalItems": false
	})

	expect(schema.validate(["hello", 42]).is_valid(), "Tuple matching Schema should validate")
	expect(!schema.validate(["hello", 42, "extra"]).is_valid(), "Extra items should not validate when additionalItems is false")

func test_tuple_with_additional_items_schema() -> void:
	var schema = Schema.build_schema({
		"type": "array",
		"items": [
			{"type": "string"},
			{"type": "number"}
		],
		"additionalItems": {"type": "boolean"}
	})

	expect(schema.validate(["hello", 42, true, false]).is_valid(), "Additional booleans should validate")
	expect(!schema.validate(["hello", 42, "invalid"]).is_valid(), "Additional non-boolean should not validate")

# ========== ERROR HANDLING ==========

func test_contains_error_messages() -> void:
	var schema = Schema.build_schema({
		"type": "array",
		"contains": {
			"type": "string",
			"minLength": 5,
			"pattern": "^[A-Z]"
		}
	})

	var result = schema.validate(["hello", "world"])
	expect(!result.is_valid(), "Array should not validate")
	expect(result.error_count() > 0, "Should have error messages")

	# Error should indicate that no items matched the contains schema
	if result.error_count() > 0:
		var error_msg = result.get_summary().to_lower()
		expect(error_msg.contains("contains") or error_msg.contains("no items"), "Error should mention contains requirement")

# ========== EDGE CASES ==========

func test_contains_empty_schema() -> void:
	var schema = Schema.build_schema({
		"type": "array",
		"contains": {}  # Empty Schema matches everything
	})

	expect(schema.validate(["anything"]).is_valid(), "Array with any item should validate with empty contains")
	expect(schema.validate([123, true, null]).is_valid(), "Array with any types should validate")
	expect(!schema.validate([]).is_valid(), "Empty array should not validate even with empty contains")

func test_contains_false_schema() -> void:
	var schema = Schema.build_schema({
		"type": "array",
		"contains": false  # Never matches
	})

	expect(!schema.validate(["anything"]).is_valid(), "No array should validate with false contains")
	expect(!schema.validate([]).is_valid(), "Empty array should not validate with false contains")

func test_contains_true_schema() -> void:
	var schema = Schema.build_schema({
		"type": "array",
		"contains": true  # Always matches if item exists
	})

	expect(schema.validate(["anything"]).is_valid(), "Array with any item should validate with true contains")
	expect(schema.validate([null, false, 0]).is_valid(), "Array with falsy values should validate")
	expect(!schema.validate([]).is_valid(), "Empty array should not validate even with true contains")

func test_contains_multiple_matches() -> void:
	var schema = Schema.build_schema({
		"type": "array",
		"contains": {
			"type": "string",
			"minLength": 3
		}
	})

	# Multiple items can match - contains only requires at least one
	expect(schema.validate(["hello", "world", "test"]).is_valid(), "Array with multiple matching items should validate")
	expect(schema.validate(["hi", "hello", "world", "a"]).is_valid(), "Array with some non-matching and some matching should validate")

func test_contains_with_nested_contains() -> void:
	var schema = Schema.build_schema({
		"type": "array",
		"contains": {
			"type": "array",
			"contains": {
				"type": "string",
				"const": "target"
			}
		}
	})

	expect(schema.validate([["a", "b"], ["c", "target"], ["d"]]).is_valid(), "Nested array containing target string should validate")
	expect(schema.validate([[["target"]], ["c", "target"]]).is_valid(), "Multiple nested arrays with target should validate")
	expect(!schema.validate([["a", "b"], ["c", "d"], ["e"]]).is_valid(), "No nested arrays containing target should not validate")
