extends Node

## Game Configuration Validation
## This example shows how to validate game settings and configuration files

func _ready() -> void:
	print("=== Game Configuration Validation ===\n")

	validate_game_settings()
	validate_audio_config()
	load_and_validate_json_file()

## Example 1: Game Settings Schema
func validate_game_settings() -> void:
	print("--- Game Settings ---")

	var settings_schema = Schema.build_schema({
		"type": "object",
		"properties": {
			"graphics": {
				"type": "object",
				"properties": {
					"resolution": {
						"type": "array",
						"items": {"type": "integer", "minimum": 1},
						"minItems": 2,
						"maxItems": 2
					},
					"fullscreen": {"type": "boolean"},
					"vsync": {"type": "boolean"},
					"quality": {
						"enum": ["low", "medium", "high", "ultra"]
					}
				},
				"required": ["resolution", "fullscreen"]
			},
			"audio": {
				"type": "object",
				"properties": {
					"master_volume": {"type": "number", "minimum": 0.0, "maximum": 1.0},
					"music_volume": {"type": "number", "minimum": 0.0, "maximum": 1.0},
					"sfx_volume": {"type": "number", "minimum": 0.0, "maximum": 1.0}
				}
			}
		},
		"required": ["graphics", "audio"]
	})

	# Valid configuration
	var valid_config = {
		"graphics": {
			"resolution": [1920, 1080],
			"fullscreen": true,
			"vsync": true,
			"quality": "high"
		},
		"audio": {
			"master_volume": 0.8,
			"music_volume": 0.6,
			"sfx_volume": 0.7
		}
	}

	var result = settings_schema.validate(valid_config)
	print("✓ Valid config: ", result.is_valid())

	# Invalid configuration (bad quality setting)
	var invalid_config = {
		"graphics": {
			"resolution": [1920, 1080],
			"fullscreen": true,
			"quality": "maximum"  # Not in enum
		},
		"audio": {
			"master_volume": 0.8
		}
	}

	result = settings_schema.validate(invalid_config)
	print("✗ Invalid quality: ", result.is_valid())
	if result.has_errors():
		for i in range(result.error_count()):
			print("  Error %d: %s" % [i + 1, result.get_error_message(i)])
	print()

## Example 2: Audio Configuration with Constraints
func validate_audio_config() -> void:
	print("--- Audio Configuration ---")

	var audio_schema = Schema.build_schema({
		"type": "object",
		"properties": {
			"master_volume": {
				"type": "number",
				"minimum": 0.0,
				"maximum": 1.0,
				"multipleOf": 0.01  # Volume in 1% increments
			},
			"audio_bus_count": {
				"type": "integer",
				"minimum": 1,
				"maximum": 32
			},
			"sample_rate": {
				"enum": [22050, 44100, 48000, 96000]
			}
		},
		"required": ["master_volume"]
	})

	# Valid audio config
	var valid_audio = {
		"master_volume": 0.75,
		"audio_bus_count": 8,
		"sample_rate": 44100
	}
	print("✓ Valid audio: ", audio_schema.validate(valid_audio).is_valid())

	# Invalid volume (too precise)
	var invalid_audio = {
		"master_volume": 0.755  # Not a multiple of 0.01
	}
	var result = audio_schema.validate(invalid_audio)
	print("✗ Invalid volume precision: ", result.is_valid())
	print()

## Example 3: Loading and Validating JSON Files
func load_and_validate_json_file() -> void:
	print("--- Loading JSON Configuration ---")

	var config_schema = Schema.build_schema({
		"type": "object",
		"properties": {
			"game_name": {"type": "string", "minLength": 1},
			"version": {"type": "string", "pattern": "^\\d+\\.\\d+\\.\\d+$"},
			"max_players": {"type": "integer", "minimum": 1, "maximum": 100}
		},
		"required": ["game_name", "version"]
	})

	# Simulate loading JSON (in practice, you'd use FileAccess)
	var json_string = """{
		"game_name": "My Awesome Game",
		"version": "1.2.3",
		"max_players": 16
	}"""

	var json = JSON.new()
	var parse_result = json.parse(json_string)

	if parse_result == OK:
		var config_data = json.data
		var validation_result = config_schema.validate(config_data)

		if validation_result.is_valid():
			print("✓ Configuration file is valid!")
			print("  Game: ", config_data["game_name"])
			print("  Version: ", config_data["version"])
		else:
			print("✗ Configuration file has errors:")
			print(validation_result.get_summary())
	else:
		print("✗ Failed to parse JSON")
	print()

## Bonus: Save validated configuration
func save_config_if_valid(config: Dictionary, schema: Schema) -> bool:
	var result = schema.validate(config)
	if result.is_valid():
		# In practice, save to file using FileAccess
		print("✓ Configuration saved successfully")
		return true
	else:
		print("✗ Configuration validation failed:")
		print(result.get_summary())
		return false
