extends Control

## Interactive Validation Demo
## A simple UI for testing JSON Schema validation

@onready var schema_input: TextEdit = %SchemaInput
@onready var data_input: TextEdit = %DataInput
@onready var result_label: RichTextLabel = %ResultLabel
@onready var validate_button: Button = %ValidateButton
@onready var example_selector: OptionButton = %ExampleSelectorButton

var current_schema: Schema = null

func _ready() -> void:
	# Connect signals
	validate_button.pressed.connect(_on_validate_pressed)
	example_selector.item_selected.connect(_on_example_selected)

	# Add example options
	example_selector.add_item("Custom", 0)
	example_selector.add_item("Player Data", 1)
	example_selector.add_item("Game Config", 2)
	example_selector.add_item("Item Inventory", 3)
	example_selector.add_item("Email Validation", 4)

	# Styles
	for label in [$VBoxContainer/MarginContainer/ButtonPanel/Label, $VBoxContainer/SchemaPanel/VBoxContainer/SchemaLabel, $VBoxContainer/SchemaPanel/VBoxContainer2/DataLabel]:
		label.add_theme_font_size_override("font_size", 16)
		label.add_theme_color_override("font_color", Color(0.8, 0.8, 0.8))

	for text_edit in [%SchemaInput, %DataInput]:
		text_edit.add_theme_font_size_override("font_size", 12)
		text_edit.add_theme_color_override("background_color", Color(0.15, 0.15, 0.15))
		text_edit.add_theme_color_override("font_color", Color(0.9, 0.9, 0.9))

	result_label.add_theme_font_size_override("font_size", 12)
	result_label.add_theme_color_override("default_color", Color(0.9, 0.9, 0.9))

	# Load default example
	_load_example(1)

func _on_validate_pressed() -> void:
	# Clear previous results
	result_label.clear()

	# Parse schema JSON
	var schema_json = JSON.new()
	var schema_parse_result = schema_json.parse(schema_input.text)

	if schema_parse_result != OK:
		_show_error("Schema JSON is invalid: " + schema_json.get_error_message())
		return

	# Build schema
	current_schema = Schema.build_schema(schema_json.data)
	if !current_schema.is_valid():
		_show_error("Schema compilation failed. Check your schema definition.")
		return

	# Parse data JSON
	var data_json = JSON.new()
	var data_parse_result = data_json.parse(data_input.text)

	if data_parse_result != OK:
		_show_error("Data JSON is invalid: " + data_json.get_error_message())
		return

	# Validate data against schema
	var validation_result = current_schema.validate(data_json.data)

	# Display results
	if validation_result.is_valid():
		_show_success("✓ Validation passed! Data is valid.")
	else:
		_show_validation_errors(validation_result)

func _on_example_selected(index: int) -> void:
	_load_example(index)

func _load_example(index: int) -> void:
	match index:
		0:  # Custom - clear fields
			schema_input.text = "{\n  \"type\": \"string\"\n}"
			data_input.text = "\"Hello World\""
		1:  # Player Data
			schema_input.text = """{
  "type": "object",
  "properties": {
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
	"health": {
	  "type": "integer",
	  "minimum": 0
	},
	"inventory": {
	  "type": "array",
	  "items": {"type": "string"},
	  "maxItems": 10
	}
  },
  "required": ["name", "level", "health"]
}"""
			data_input.text = """{
  "name": "DragonSlayer",
  "level": 42,
  "health": 450,
  "inventory": ["sword", "shield", "potion"]
}"""

		2:  # Game Config
			schema_input.text = """{
  "type": "object",
  "properties": {
	"resolution": {
	  "type": "array",
	  "items": {"type": "integer"},
	  "minItems": 2,
	  "maxItems": 2
	},
	"fullscreen": {"type": "boolean"},
	"quality": {
	  "enum": ["low", "medium", "high", "ultra"]
	},
	"volume": {
	  "type": "number",
	  "minimum": 0.0,
	  "maximum": 1.0
	}
  },
  "required": ["resolution", "quality"]
}"""
			data_input.text = """{
  "resolution": [1920, 1080],
  "fullscreen": true,
  "quality": "high",
  "volume": 0.75
}"""

		3:  # Item Inventory
			schema_input.text = """{
  "type": "object",
  "properties": {
	"item_id": {"type": "string"},
	"name": {"type": "string"},
	"type": {
	  "enum": ["weapon", "armor", "consumable"]
	},
	"rarity": {
	  "enum": ["common", "rare", "epic", "legendary"]
	},
	"stats": {
	  "type": "object",
	  "properties": {
		"damage": {"type": "integer", "minimum": 0},
		"defense": {"type": "integer", "minimum": 0}
	  }
	}
  },
  "required": ["item_id", "name", "type"]
}"""
			data_input.text = """{
  "item_id": "sword_001",
  "name": "Legendary Sword",
  "type": "weapon",
  "rarity": "legendary",
  "stats": {
	"damage": 100,
	"defense": 0
  }
}"""

		4:  # Email Validation
			schema_input.text = """{
  "type": "object",
  "properties": {
	"email": {
	  "type": "string",
	  "format": "email"
	},
	"backup_email": {
	  "type": "string",
	  "format": "email"
	},
	"website": {
	  "type": "string",
	  "format": "uri"
	}
  },
  "required": ["email"]
}"""
			data_input.text = """{
  "email": "player@example.com",
  "backup_email": "backup@example.com",
  "website": "https://player.example.com"
}"""

func _show_success(message: String) -> void:
	result_label.clear()
	result_label.push_color(Color.GREEN)
	result_label.add_text(message)
	result_label.pop()

func _show_error(message: String) -> void:
	result_label.clear()
	result_label.push_color(Color.RED)
	result_label.add_text("✗ Error: " + message)
	result_label.pop()

func _show_validation_errors(validation_result: SchemaValidationResult) -> void:
	result_label.clear()

	# Title
	result_label.push_color(Color.RED)
	result_label.add_text("✗ Validation Failed\n\n")
	result_label.pop()

	# Error count
	result_label.push_color(Color.YELLOW)
	result_label.add_text("Found %d error(s):\n\n" % validation_result.error_count())
	result_label.pop()

	# List each error
	for i in range(validation_result.error_count()):
		result_label.push_color(Color.WHITE)
		result_label.add_text("[%d] " % (i + 1))
		result_label.pop()

		# Path
		result_label.push_color(Color.CYAN)
		result_label.add_text("At '%s': " % validation_result.get_error_path(i))
		result_label.pop()

		# Message
		result_label.push_color(Color.LIGHT_GRAY)
		result_label.add_text(validation_result.get_error_message(i) + "\n")
		result_label.pop()

		# Constraint type
		result_label.push_color(Color.DARK_GRAY)
		result_label.add_text("  Constraint: %s\n" % validation_result.get_error_constraint(i))
		result_label.pop()

		# Invalid value
		var error_value = validation_result.get_error_value(i)
		if error_value != null:
			result_label.push_color(Color.DARK_GRAY)
			result_label.add_text("  Value: %s\n" % str(error_value))
			result_label.pop()

		result_label.add_text("\n")
