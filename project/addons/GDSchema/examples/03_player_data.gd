extends Node

## Player Data Validation
## This example demonstrates validating complex nested game data structures

func _ready() -> void:
	print("=== Player Data Validation ===\n")

	validate_player_profile()
	validate_inventory()
	validate_quest_log()

## Example 1: Player Profile with Nested Data
func validate_player_profile() -> void:
	print("--- Player Profile ---")

	var player_schema = Schema.build_schema({
		"type": "object",
		"properties": {
			"id": {
				"type": "string",
				"pattern": "^player_[0-9]+$"
			},
			"name": {
				"type": "string",
				"minLength": 3,
				"maxLength": 20
			},
			"level": {
				"type": "integer",
				"minimum": 1,
				"maximum": 100
			},
			"experience": {
				"type": "integer",
				"minimum": 0
			},
			"stats": {
				"type": "object",
				"properties": {
					"health": {"type": "integer", "minimum": 0},
					"mana": {"type": "integer", "minimum": 0},
					"strength": {"type": "integer", "minimum": 1, "maximum": 100},
					"agility": {"type": "integer", "minimum": 1, "maximum": 100},
					"intelligence": {"type": "integer", "minimum": 1, "maximum": 100}
				},
				"required": ["health", "mana", "strength", "agility", "intelligence"]
			},
			"position": {
				"type": "object",
				"properties": {
					"x": {"type": "number"},
					"y": {"type": "number"},
					"z": {"type": "number"}
				},
				"required": ["x", "y", "z"]
			}
		},
		"required": ["id", "name", "level", "stats"]
	})

	# Valid player data
	var player_data = {
		"id": "player_12345",
		"name": "DragonSlayer",
		"level": 42,
		"experience": 125000,
		"stats": {
			"health": 450,
			"mana": 200,
			"strength": 75,
			"agility": 60,
			"intelligence": 55
		},
		"position": {
			"x": 100.5,
			"y": 50.0,
			"z": -25.3
		}
	}

	var result = player_schema.validate(player_data)
	print("✓ Valid player: ", result.is_valid())

	# Invalid player (stat out of range)
	var invalid_player = {
		"id": "player_99999",
		"name": "Hacker",
		"level": 42,
		"stats": {
			"health": 450,
			"mana": 200,
			"strength": 150,  # Over maximum!
			"agility": 60,
			"intelligence": 55
		}
	}

	result = player_schema.validate(invalid_player)
	print("✗ Invalid stats: ", result.is_valid())
	if result.has_errors():
		print("  Error: ", result.get_error_message(0))
	print()

## Example 2: Inventory System
func validate_inventory() -> void:
	print("--- Inventory System ---")

	var inventory_schema = Schema.build_schema({
		"type": "object",
		"properties": {
			"capacity": {
				"type": "integer",
				"minimum": 1,
				"maximum": 100
			},
			"items": {
				"type": "array",
				"items": {
					"type": "object",
					"properties": {
						"id": {"type": "string"},
						"name": {"type": "string", "minLength": 1},
						"type": {
							"enum": ["weapon", "armor", "consumable", "quest", "material"]
						},
						"quantity": {
							"type": "integer",
							"minimum": 1
						},
						"rarity": {
							"enum": ["common", "uncommon", "rare", "epic", "legendary"]
						},
						"stats": {
							"type": "object",
							"properties": {
								"damage": {"type": "integer", "minimum": 0},
								"defense": {"type": "integer", "minimum": 0},
								"value": {"type": "integer", "minimum": 0}
							}
						}
					},
					"required": ["id", "name", "type", "quantity"]
				},
				"uniqueItems": false  # Can have duplicate items
			},
			"equipped": {
				"type": "object",
				"properties": {
					"weapon": {"type": ["string", "null"]},
					"armor": {"type": ["string", "null"]},
					"accessory": {"type": ["string", "null"]}
				}
			}
		},
		"required": ["capacity", "items"]
	})

	# Valid inventory
	var inventory = {
		"capacity": 50,
		"items": [
			{
				"id": "sword_001",
				"name": "Iron Sword",
				"type": "weapon",
				"quantity": 1,
				"rarity": "common",
				"stats": {
					"damage": 25,
					"value": 100
				}
			},
			{
				"id": "potion_hp",
				"name": "Health Potion",
				"type": "consumable",
				"quantity": 5,
				"rarity": "common"
			}
		],
		"equipped": {
			"weapon": "sword_001",
			"armor": null,
			"accessory": null
		}
	}

	print("✓ Valid inventory: ", inventory_schema.validate(inventory).is_valid())

	# Invalid inventory (bad item type)
	var invalid_inventory = {
		"capacity": 50,
		"items": [
			{
				"id": "invalid_001",
				"name": "Mystery Item",
				"type": "unknown_type",  # Not in enum
				"quantity": 1
			}
		]
	}

	var result = inventory_schema.validate(invalid_inventory)
	print("✗ Invalid item type: ", result.is_valid())
	print()

## Example 3: Quest Log with Conditional Requirements
func validate_quest_log() -> void:
	print("--- Quest Log ---")

	var quest_schema = Schema.build_schema({
		"type": "object",
		"properties": {
			"id": {"type": "string"},
			"title": {"type": "string", "minLength": 1},
			"description": {"type": "string"},
			"status": {
				"enum": ["available", "in_progress", "completed", "failed"]
			},
			"level_requirement": {
				"type": "integer",
				"minimum": 1
			},
			"objectives": {
				"type": "array",
				"items": {
					"type": "object",
					"properties": {
						"description": {"type": "string"},
						"current": {"type": "integer", "minimum": 0},
						"target": {"type": "integer", "minimum": 1},
						"completed": {"type": "boolean"}
					},
					"required": ["description", "current", "target", "completed"]
				},
				"minItems": 1
			},
			"rewards": {
				"type": "object",
				"properties": {
					"experience": {"type": "integer", "minimum": 0},
					"gold": {"type": "integer", "minimum": 0},
					"items": {
						"type": "array",
						"items": {"type": "string"}
					}
				}
			}
		},
		"required": ["id", "title", "status", "objectives"]
	})

	# Valid quest
	var quest = {
		"id": "quest_dragon_001",
		"title": "Slay the Dragon",
		"description": "Defeat the ancient dragon terrorizing the village",
		"status": "in_progress",
		"level_requirement": 25,
		"objectives": [
			{
				"description": "Find dragon's lair",
				"current": 1,
				"target": 1,
				"completed": true
			},
			{
				"description": "Defeat dragon",
				"current": 0,
				"target": 1,
				"completed": false
			}
		],
		"rewards": {
			"experience": 5000,
			"gold": 1000,
			"items": ["legendary_sword", "dragon_scale"]
		}
	}

	print("✓ Valid quest: ", quest_schema.validate(quest).is_valid())

	# Invalid quest (missing required objectives)
	var invalid_quest = {
		"id": "quest_invalid",
		"title": "Broken Quest",
		"status": "available",
		"objectives": []  # Must have at least 1 objective
	}

	var result = quest_schema.validate(invalid_quest)
	print("✗ No objectives: ", result.is_valid())
	if result.has_errors():
		print("  Error: ", result.get_error_message(0))
	print()
