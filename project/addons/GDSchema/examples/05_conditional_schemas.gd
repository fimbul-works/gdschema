extends Node

## Conditional Schema Validation
## This example demonstrates if/then/else conditional validation

func _ready() -> void:
	print("=== Conditional Validation Examples ===\n")

	age_based_requirements()
	item_type_requirements()
	payment_method_validation()
	difficulty_based_stats()

## Example 1: Age-Based Requirements
func age_based_requirements() -> void:
	print("--- Age-Based Requirements ---")

	var player_schema = Schema.build_schema({
		"type": "object",
		"properties": {
			"name": {"type": "string"},
			"age": {"type": "integer", "minimum": 1}
		},
		"required": ["name", "age"],
		"if": {
			"properties": {"age": {"minimum": 18}}
		},
		"then": {
			"required": ["email", "terms_accepted"],
			"properties": {
				"email": {"type": "string"},
				"terms_accepted": {"const": true}
			}
		},
		"else": {
			"required": ["parent_email"],
			"properties": {
				"parent_email": {"type": "string"}
			}
		}
	})

	# Adult player (needs email and terms)
	var adult = {
		"name": "Adult Player",
		"age": 25,
		"email": "adult@example.com",
		"terms_accepted": true
	}
	print("✓ Adult player: ", player_schema.validate(adult).is_valid())

	# Minor player (needs parent email)
	var minor = {
		"name": "Young Player",
		"age": 12,
		"parent_email": "parent@example.com"
	}
	print("✓ Minor player: ", player_schema.validate(minor).is_valid())

	# Adult without terms (invalid)
	var invalid_adult = {
		"name": "Invalid Adult",
		"age": 25,
		"email": "adult@example.com"
		# Missing terms_accepted
	}
	var result = player_schema.validate(invalid_adult)
	print("✗ Adult without terms: ", result.is_valid())
	print()

## Example 2: Item Type Requirements
func item_type_requirements() -> void:
	print("--- Item Type Requirements ---")

	var item_schema = Schema.build_schema({
		"type": "object",
		"properties": {
			"id": {"type": "string"},
			"name": {"type": "string"},
			"type": {"enum": ["weapon", "consumable", "quest"]}
		},
		"required": ["id", "name", "type"],
		"if": {
			"properties": {"type": {"const": "weapon"}}
		},
		"then": {
			"required": ["damage", "durability"],
			"properties": {
				"damage": {"type": "integer", "minimum": 1},
				"durability": {"type": "integer", "minimum": 1, "maximum": 100}
			}
		},
		"else": {
			"if": {
				"properties": {"type": {"const": "consumable"}}
			},
			"then": {
				"required": ["effect", "quantity"],
				"properties": {
					"effect": {"type": "string"},
					"quantity": {"type": "integer", "minimum": 1}
				}
			}
		}
	})

	# Valid weapon
	var weapon = {
		"id": "sword_001",
		"name": "Iron Sword",
		"type": "weapon",
		"damage": 25,
		"durability": 100
	}
	print("✓ Weapon: ", item_schema.validate(weapon).is_valid())

	# Valid consumable
	var consumable = {
		"id": "potion_001",
		"name": "Health Potion",
		"type": "consumable",
		"effect": "restore_hp",
		"quantity": 5
	}
	print("✓ Consumable: ", item_schema.validate(consumable).is_valid())

	# Quest item (no special requirements)
	var quest_item = {
		"id": "key_001",
		"name": "Ancient Key",
		"type": "quest"
	}
	print("✓ Quest item: ", item_schema.validate(quest_item).is_valid())
	print()

## Example 3: Payment Method Validation
func payment_method_validation() -> void:
	print("--- Payment Method Validation ---")

	var payment_schema = Schema.build_schema({
		"type": "object",
		"properties": {
			"amount": {"type": "number", "minimum": 0},
			"method": {"enum": ["credit_card", "paypal", "crypto"]}
		},
		"required": ["amount", "method"],
		"allOf": [
			{
				"if": {"properties": {"method": {"const": "credit_card"}}},
				"then": {
					"required": ["card_number", "expiry", "cvv"],
					"properties": {
						"card_number": {"type": "string", "pattern": "^\\d{16}$"},
						"expiry": {"type": "string", "pattern": "^\\d{2}/\\d{2}$"},
						"cvv": {"type": "string", "pattern": "^\\d{3}$"}
					}
				}
			},
			{
				"if": {"properties": {"method": {"const": "paypal"}}},
				"then": {
					"required": ["paypal_email"],
					"properties": {
						"paypal_email": {"type": "string"}
					}
				}
			},
			{
				"if": {"properties": {"method": {"const": "crypto"}}},
				"then": {
					"required": ["wallet_address", "currency"],
					"properties": {
						"wallet_address": {"type": "string"},
						"currency": {"enum": ["BTC", "ETH", "USDT"]}
					}
				}
			}
		]
	})

	# Credit card payment
	var cc_payment = {
		"amount": 49.99,
		"method": "credit_card",
		"card_number": "1234567890123456",
		"expiry": "12/25",
		"cvv": "123"
	}
	print("✓ Credit card: ", payment_schema.validate(cc_payment).is_valid())

	# PayPal payment
	var paypal_payment = {
		"amount": 29.99,
		"method": "paypal",
		"paypal_email": "user@paypal.com"
	}
	print("✓ PayPal: ", payment_schema.validate(paypal_payment).is_valid())

	# Crypto payment
	var crypto_payment = {
		"amount": 99.99,
		"method": "crypto",
		"wallet_address": "0x1234...abcd",
		"currency": "ETH"
	}
	print("✓ Crypto: ", payment_schema.validate(crypto_payment).is_valid())
	print()

## Example 4: Difficulty-Based Stat Requirements
func difficulty_based_stats() -> void:
	print("--- Difficulty-Based Requirements ---")

	var enemy_schema = Schema.build_schema({
		"type": "object",
		"properties": {
			"name": {"type": "string"},
			"difficulty": {"enum": ["easy", "normal", "hard", "nightmare"]},
			"health": {"type": "integer", "minimum": 1}
		},
		"required": ["name", "difficulty", "health"],
		"if": {
			"properties": {"difficulty": {"enum": ["hard", "nightmare"]}}
		},
		"then": {
			"required": ["special_abilities", "loot_table"],
			"properties": {
				"special_abilities": {
					"type": "array",
					"items": {"type": "string"},
					"minItems": 1
				},
				"loot_table": {
					"type": "array",
					"items": {"type": "string"},
					"minItems": 1
				}
			}
		}
	})

	# Easy enemy (no special requirements)
	var easy_enemy = {
		"name": "Slime",
		"difficulty": "easy",
		"health": 50
	}
	print("✓ Easy enemy: ", enemy_schema.validate(easy_enemy).is_valid())

	# Hard enemy (needs abilities and loot)
	var hard_enemy = {
		"name": "Dragon",
		"difficulty": "hard",
		"health": 5000,
		"special_abilities": ["fire_breath", "tail_swipe"],
		"loot_table": ["dragon_scale", "gold_coin", "rare_gem"]
	}
	print("✓ Hard enemy: ", enemy_schema.validate(hard_enemy).is_valid())

	# Hard enemy without abilities (invalid)
	var invalid_enemy = {
		"name": "Broken Dragon",
		"difficulty": "hard",
		"health": 5000
		# Missing special_abilities and loot_table
	}
	var result = enemy_schema.validate(invalid_enemy)
	print("✗ Hard enemy without abilities: ", result.is_valid())
	if result.has_errors():
		for i in range(result.error_count()):
			print("  Error %d: %s" % [i + 1, result.get_error_message(i)])
	print()
