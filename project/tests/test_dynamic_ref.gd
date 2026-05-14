extends TestSuite

func _init() -> void:
	icon = "⚓"

func test_dynamic_ref_basic() -> void:
	var schema = Schema.build_schema({
		"$id": "https://example.com/tree",
		"$dynamicAnchor": "node",
		"type": "object",
		"properties": {
			"data": true,
			"children": {
				"type": "array",
				"items": {
					"$dynamicRef": "#node"
				}
			}
		}
	})

	expect(schema.validate({
		"data": "anything",
		"children": [
			{
				"data": "child",
				"children": []
			}
		]
	}).is_valid(), "Basic dynamic ref resolving to itself should validate")

	expect(!schema.validate({
		"data": "anything",
		"children": [
			{
				"data": "child",
				"children": [
					"invalid"
				]
			}
		]
	}).is_valid(), "Basic dynamic ref failing inner validation should not validate")

func test_dynamic_ref_with_override() -> void:
	var schema_base = Schema.build_schema({
		"$id": "https://example.com/base",
		"$dynamicAnchor": "node",
		"type": "object",
		"properties": {
			"data": true,
			"children": {
				"type": "array",
				"items": {
					"$dynamicRef": "#node"
				}
			}
		}
	})

	# Register the base schema so we can $ref it
	Schema.register_schema(schema_base, "https://example.com/base")

	var schema_extended = Schema.build_schema({
		"$id": "https://example.com/extended",
		"$dynamicAnchor": "node",
		"type": "object",
		"properties": {
			"data": {"type": "number"},
			"children": {
				"type": "array",
				"items": {
					"$dynamicRef": "https://example.com/base#node"
				}
			}
		}
	})

	# Using the extended schema, the dynamic ref should resolve to the extended schema's #node
	expect(schema_extended.validate({
		"data": 123,
		"children": [
			{
				"data": 456,
				"children": []
			}
		]
	}).is_valid(), "Dynamic ref overridden by extended schema should validate")

	expect(!schema_extended.validate({
		"data": "string is invalid",
		"children": []
	}).is_valid(), "Data failing extended schema should not validate")

	expect(!schema_extended.validate({
		"data": 123,
		"children": [
			{
				"data": "string is invalid",
				"children": []
			}
		]
	}).is_valid(), "Child failing extended schema should not validate, because $dynamicRef evaluated at extended schema")

	# Clean up
	Schema.unregister_schema("https://example.com/base")

func test_dynamic_ref_falls_back_to_static() -> void:
	var schema = Schema.build_schema({
		"$id": "https://example.com/static-fallback",
		"$defs": {
			"target": {
				"$dynamicAnchor": "myAnchor",
				"type": "string"
			}
		},
		"type": "object",
		"properties": {
			"prop": {
				"$dynamicRef": "#myAnchor"
			}
		}
	})

	# It should behave like a normal $ref if there's no dynamic override
	expect(schema.validate({"prop": "valid string"}).is_valid(), "Dynamic ref falling back to lexical scope should validate")
	expect(!schema.validate({"prop": 123}).is_valid(), "Dynamic ref failure should not validate")
