extends TestSuite

func _init() -> void:
	icon = "📦"

func test_unevaluated_items_basic() -> void:
	var schema = Schema.build_schema({
		"type": "array",
		"prefixItems": [
			{ "type": "string" },
			{ "type": "number" }
		],
		"unevaluatedItems": false
	})

	expect(schema.validate(["test", 123]).is_valid(), "Array with evaluated items should validate")
	expect(schema.validate(["test"]).is_valid(), "Array with partial prefixItems should validate")
	expect(!schema.validate(["test", 123, true]).is_valid(), "Array with unevaluated item should not validate")

func test_unevaluated_items_with_items() -> void:
	var schema = Schema.build_schema({
		"type": "array",
		"prefixItems": [
			{ "type": "string" }
		],
		"items": { "type": "number" },
		"unevaluatedItems": false
	})

	expect(schema.validate(["test", 123, 456]).is_valid(), "Items evaluated by `items` should validate")
	# unevaluatedItems is false, but everything is evaluated by prefixItems + items
	expect(schema.validate(["test", 123]).is_valid(), "Array should validate")

func test_unevaluated_items_schema() -> void:
	var schema = Schema.build_schema({
		"type": "array",
		"prefixItems": [
			{ "type": "string" }
		],
		"unevaluatedItems": { "type": "number" }
	})

	expect(schema.validate(["test", 123, 456]).is_valid(), "Unevaluated items matching schema should validate")
	expect(!schema.validate(["test", 123, "fail"]).is_valid(), "Unevaluated items failing schema should not validate")

func test_unevaluated_items_with_allOf() -> void:
	var schema = Schema.build_schema({
		"type": "array",
		"allOf": [
			{
				"prefixItems": [
					{ "type": "string" }
				]
			},
			{
				"prefixItems": [
					true,
					{ "type": "number" }
				]
			}
		],
		"unevaluatedItems": false
	})

	# Index 0 is string (from allOf[0]). Index 1 is number (from allOf[1]).
	expect(schema.validate(["test", 123]).is_valid(), "Items evaluated across allOf should validate")
	expect(!schema.validate(["test", 123, true]).is_valid(), "Unevaluated item beyond allOf evaluated range should not validate")

func test_unevaluated_items_with_contains() -> void:
	var schema = Schema.build_schema({
		"type": "array",
		"contains": { "type": "number" },
		"unevaluatedItems": false
	})

	# Draft 2020-12 states that contains evaluates items that match its schema.
	expect(schema.validate([123]).is_valid(), "Item evaluated by contains should validate")
	# In some strict interpretations, items NOT matching contains are unevaluated.
	# But actually contains evaluates ALL items to find a match, and records the ones that pass.
	expect(!schema.validate(["string", 123]).is_valid(), "Item NOT evaluated by contains should not validate against unevaluatedItems: false")
