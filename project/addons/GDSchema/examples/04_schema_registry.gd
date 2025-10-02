extends Node

## Schema Registry and References
## This example shows how to create reusable schemas with $ref

func _ready() -> void:
	print("=== Schema Registry Examples ===\n")

	basic_references()
	external_schema_references()
	recursive_schemas()

## Example 1: Basic $ref Usage
func basic_references() -> void:
	print("--- Basic References ---")

	var schema = Schema.build_schema({
		"definitions": {
			"email": {
				"type": "string",
				"pattern": "^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$"
			},
			"positive_integer": {
				"type": "integer",
				"minimum": 1
			}
		},
		"type": "object",
		"properties": {
			"user_email": {"$ref": "#/definitions/email"},
			"backup_email": {"$ref": "#/definitions/email"},
			"user_id": {"$ref": "#/definitions/positive_integer"},
			"friend_id": {"$ref": "#/definitions/positive_integer"}
		},
		"required": ["user_email", "user_id"]
	})

	# Valid data reusing schema definitions
	var valid_data = {
		"user_email": "player@example.com",
		"backup_email": "backup@example.com",
		"user_id": 12345,
		"friend_id": 67890
	}
	print("✓ Valid with refs: ", schema.validate(valid_data).is_valid())

	# Invalid email format
	var invalid_data = {
		"user_email": "not-an-email",
		"user_id": 12345
	}
	var result = schema.validate(invalid_data)
	print("✗ Invalid email: ", result.is_valid())
	print()

## Example 2: External Schema References
func external_schema_references() -> void:
	print("--- External Schema References ---")

	# Register a common address schema
	var address_schema = Schema.build_schema({
		"$id": "http://example.com/schemas/address.json",
		"type": "object",
		"properties": {
			"street": {"type": "string", "minLength": 1},
			"city": {"type": "string", "minLength": 1},
			"state": {"type": "string", "minLength": 2, "maxLength": 2},
			"zip": {"type": "string", "pattern": "^\\d{5}(-\\d{4})?$"}
		},
		"required": ["street", "city", "state", "zip"]
	})

	print("Address schema registered: ",
		Schema.is_schema_registered("http://example.com/schemas/address.json"))

	# Register a contact schema that references address
	var contact_schema = Schema.build_schema({
		"$id": "http://example.com/schemas/contact.json",
		"type": "object",
		"properties": {
			"name": {"type": "string", "minLength": 1},
			"phone": {"type": "string", "pattern": "^\\d{3}-\\d{3}-\\d{4}$"},
			"email": {"type": "string"}
		},
		"required": ["name"]
	})

	# Now use both schemas in a person schema
	var person_schema = Schema.build_schema({
		"type": "object",
		"properties": {
			"contact": {"$ref": "http://example.com/schemas/contact.json"},
			"home_address": {"$ref": "http://example.com/schemas/address.json"},
			"work_address": {"$ref": "http://example.com/schemas/address.json"}
		},
		"required": ["contact"]
	})

	# Valid person with referenced schemas
	var person = {
		"contact": {
			"name": "John Doe",
			"phone": "555-123-4567",
			"email": "john@example.com"
		},
		"home_address": {
			"street": "123 Main St",
			"city": "Springfield",
			"state": "IL",
			"zip": "62701"
		}
	}

	print("✓ Valid person: ", person_schema.validate(person).is_valid())

	# Invalid zip code
	var invalid_person = {
		"contact": {
			"name": "Jane Doe"
		},
		"home_address": {
			"street": "456 Oak Ave",
			"city": "Portland",
			"state": "OR",
			"zip": "INVALID"  # Bad format
		}
	}

	var result = person_schema.validate(invalid_person)
	print("✗ Invalid zip: ", result.is_valid())
	print()

## Example 3: Recursive Schemas (Tree Structures)
func recursive_schemas() -> void:
	print("--- Recursive Schemas ---")

	# Tree node that can contain other tree nodes
	var tree_schema = Schema.build_schema({
		"$id": "http://example.com/schemas/tree-node.json",
		"type": "object",
		"properties": {
			"value": {"type": ["string", "number"]},
			"children": {
				"type": "array",
				"items": {"$ref": "http://example.com/schemas/tree-node.json"}
			}
		},
		"required": ["value"]
	})

	# Valid tree structure
	var tree_data = {
		"value": "root",
		"children": [
			{
				"value": "child1",
				"children": [
					{"value": "grandchild1"},
					{"value": "grandchild2"}
				]
			},
			{
				"value": "child2",
				"children": []
			}
		]
	}

	print("✓ Valid tree: ", tree_schema.validate(tree_data).is_valid())

	# Invalid tree (missing required value in nested node)
	var invalid_tree = {
		"value": "root",
		"children": [
			{
				"children": []  # Missing 'value'!
			}
		]
	}

	var result = tree_schema.validate(invalid_tree)
	print("✗ Invalid nested node: ", result.is_valid())
	if result.has_errors():
		print("  Error: ", result.get_error_message(0))
	print()

## Bonus: Working with Definitions ($defs)
func modern_definitions_example() -> void:
	print("--- Modern $defs (Draft-7+) ---")

	# Using $defs instead of definitions (both work)
	var schema = Schema.build_schema({
		"$defs": {
			"item": {
				"type": "object",
				"properties": {
					"id": {"type": "string"},
					"name": {"type": "string"},
					"price": {"type": "number", "minimum": 0}
				},
				"required": ["id", "name", "price"]
			}
		},
		"type": "object",
		"properties": {
			"cart_items": {
				"type": "array",
				"items": {"$ref": "#/$defs/item"}
			},
			"wishlist_items": {
				"type": "array",
				"items": {"$ref": "#/$defs/item"}
			}
		}
	})

	var shopping_data = {
		"cart_items": [
			{"id": "item_001", "name": "Sword", "price": 99.99},
			{"id": "item_002", "name": "Shield", "price": 49.99}
		],
		"wishlist_items": [
			{"id": "item_003", "name": "Armor", "price": 199.99}
		]
	}

	print("✓ Valid shopping data: ", schema.validate(shopping_data).is_valid())
