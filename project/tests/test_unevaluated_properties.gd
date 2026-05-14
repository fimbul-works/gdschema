extends TestSuite

func _init() -> void:
	icon = "🕵️"

func test_unevaluated_properties_basic() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"properties": {
			"foo": { "type": "string" }
		},
		"unevaluatedProperties": false
	})

	expect(schema.validate({"foo": "bar"}).is_valid(), "Object with only evaluated property should validate")
	expect(!schema.validate({"foo": "bar", "extra": "baz"}).is_valid(), "Object with unevaluated property should not validate")

func test_unevaluated_properties_with_allof() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"allOf": [
			{
				"properties": {
					"foo": { "type": "string" }
				}
			},
			{
				"properties": {
					"bar": { "type": "number" }
				}
			}
		],
		"unevaluatedProperties": false
	})

	expect(schema.validate({"foo": "test", "bar": 123}).is_valid(), "Object with properties evaluated in allOf should validate")
	expect(!schema.validate({"foo": "test", "bar": 123, "baz": true}).is_valid(), "Object with property not evaluated in allOf should not validate")

func test_unevaluated_properties_with_anyof() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"anyOf": [
			{
				"properties": {
					"foo": { "type": "string" }
				}
			},
			{
				"properties": {
					"bar": { "type": "number" }
				}
			}
		],
		"unevaluatedProperties": false
	})

	expect(schema.validate({"foo": "test"}).is_valid(), "Object with properties evaluated in matching anyOf should validate")
	expect(!schema.validate({"foo": "test", "baz": true}).is_valid(), "Object with property not evaluated in matching anyOf should not validate")
	expect(schema.validate({"foo": "test", "bar": 123}).is_valid(), "Object with properties evaluated in multiple matching anyOfs should validate")

func test_unevaluated_properties_with_ref() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"$defs": {
			"base": {
				"properties": {
					"foo": { "type": "string" }
				}
			}
		},
		"$ref": "#/$defs/base",
		"unevaluatedProperties": false
	})

	expect(schema.validate({"foo": "test"}).is_valid(), "Object with property evaluated in $ref should validate")
	expect(!schema.validate({"foo": "test", "extra": 123}).is_valid(), "Object with unevaluated extra property should not validate")

func test_unevaluated_properties_schema() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"properties": {
			"foo": { "type": "string" }
		},
		"unevaluatedProperties": { "type": "number" }
	})

	expect(schema.validate({"foo": "test", "extra1": 123, "extra2": 456}).is_valid(), "Object with unevaluated properties matching schema should validate")
	expect(!schema.validate({"foo": "test", "extra": "string"}).is_valid(), "Object with unevaluated property failing schema should not validate")

func test_unevaluated_properties_with_pattern_properties() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"patternProperties": {
			"^prefix_": { "type": "string" }
		},
		"unevaluatedProperties": false
	})

	expect(schema.validate({"prefix_test": "hello", "prefix_foo": "bar"}).is_valid(), "Properties evaluated by patternProperties should validate")
	expect(!schema.validate({"prefix_test": "hello", "other": "value"}).is_valid(), "Properties not matching patternProperties should not validate")

func test_unevaluated_properties_with_additional_properties() -> void:
	var schema = Schema.build_schema({
		"type": "object",
		"properties": {
			"foo": { "type": "string" }
		},
		"additionalProperties": { "type": "number" },
		"unevaluatedProperties": false
	})

	expect(schema.validate({"foo": "test", "extra": 123}).is_valid(), "Properties evaluated by additionalProperties should validate")
	expect(schema.validate({"foo": "test"}).is_valid(), "No extra properties should validate")
	expect(!schema.validate({"foo": "test", "extra": "string"}).is_valid(), "Fails additionalProperties, so fails validation entirely")
