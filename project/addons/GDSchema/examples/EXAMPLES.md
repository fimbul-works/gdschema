# GDSchema Examples

This directory contains practical examples demonstrating how to use GDSchema for JSON Schema validation in Godot projects.

## Running the Examples

### Script Examples

Each numbered script can be run independently:

1. Add the script to a Node in your scene
2. Run the scene
3. Check the output console for validation results

Or simply run them from the editor:
- Open the script
- Click "Run" or press `Ctrl+Shift+X` (may vary by platform)

### Interactive Demo

The `validation_demo` scene provides an interactive UI for testing validation:

1. Open `validation_demo.tscn`
2. Run the scene (F5)
3. Choose an example or enter your own JSON schema and data
4. Click "Validate" to see results

## Example Files

### 01_basic_validation.gd
**Covers**: Simple type validation, constraints, and basic schemas

Learn how to:
- Validate strings with length and pattern constraints
- Validate numbers with min/max and multipleOf
- Validate arrays with uniqueness and size limits
- Combine multiple constraints in object schemas

**Best for**: Getting started with JSON Schema

---

### 02_game_config.gd
**Covers**: Game configuration validation, JSON file loading

Learn how to:
- Create schemas for game settings (graphics, audio)
- Validate configuration files
- Use enums for fixed choices
- Parse and validate JSON from strings/files
- Implement config recovery strategies

**Best for**: Validating player settings and game configuration

---

### 03_player_data.gd
**Covers**: Complex nested structures, game data models

Learn how to:
- Validate player profiles with nested stats
- Create inventory system schemas
- Validate quest logs with objectives
- Handle deeply nested object validation
- Work with arrays of complex objects

**Best for**: Save games, player data, and game state validation

---

### 04_schema_registry.gd
**Covers**: Schema references, reusability, and organization

Learn how to:
- Use `$ref` for internal references
- Register schemas with `$id` for global access
- Reference external schemas across documents
- Create recursive schemas (trees, graphs)
- Use `definitions` and `$defs` for reusable components

**Best for**: Large projects with shared schemas

---

### 05_conditional_schemas.gd
**Covers**: Conditional validation with if/then/else

Learn how to:
- Validate based on age requirements (adults vs minors)
- Apply different rules based on item types
- Validate payment methods with specific requirements
- Use difficulty levels to determine required fields
- Combine multiple conditional requirements

**Best for**: Dynamic validation rules based on data content

---

### 06_format_validation.gd
**Covers**: Built-in format validators

Learn how to:
- Validate email addresses
- Validate URIs and URLs
- Validate dates, times, and date-times (ISO 8601)
- Validate IPv4 and IPv6 addresses
- Validate UUIDs
- Combine formats with other constraints

**Best for**: User input validation, API data verification

---

### 07_error_handling.gd
**Covers**: Working with validation errors and results

Learn how to:
- Inspect validation results
- Access error messages, paths, and values
- Handle multiple errors
- Navigate nested error paths
- Create user-friendly error messages
- Implement error recovery strategies
- Collect and log validation errors

**Best for**: Building robust validation feedback systems

---

## Common Use Cases

### Validating Save Game Data
See: `03_player_data.gd`
```gdscript
var save_schema = Schema.build_schema({ ... })
var result = save_schema.validate(save_data)
if result.is_valid():
	save_to_file(save_data)
else:
	show_error(result.get_summary())
```

### Validating Configuration Files
See: `02_game_config.gd`
```gdscript
var config_text = FileAccess.get_file_as_string("user://config.json")
var json = JSON.new()
if json.parse(config_text) == OK:
	var result = config_schema.validate(json.data)
	if result.is_valid():
		apply_config(json.data)
```

### Validating Network Messages
See: `04_schema_registry.gd`, `06_format_validation.gd`
```gdscript
# Register message schemas
var player_action_schema = Schema.build_schema({
	"$id": "game://schemas/player_action.json",
	...
})

# Validate incoming messages
func on_message_received(message: Dictionary):
	var schema = Schema.get_registered_schema("game://schemas/player_action.json")
	if schema and schema.validate(message).is_valid():
		process_action(message)
```

### Form Validation
See: `06_format_validation.gd`, `07_error_handling.gd`
```gdscript
func validate_form(form_data: Dictionary) -> Array:
	var result = form_schema.validate(form_data)
	if result.is_valid():
		return []

	var errors = []
	for i in range(result.error_count()):
		errors.append({
			"field": result.get_error_path(i).trim_prefix("/"),
			"message": result.get_error_message(i)
		})
	return errors
```

## Tips and Best Practices

### 1. Schema Organization
- Use `definitions` or `$defs` for reusable components
- Register commonly used schemas with `$id`
- Keep schemas in separate JSON files for large projects

### 2. Error Handling
- Always check `is_valid()` before using data
- Use `get_summary()` for debugging
- Create user-friendly messages from error details
- Log validation errors for analytics

### 3. Performance
- Schemas are compiled once and cached
- Reuse Schema objects when validating multiple items
- Consider pre-registering schemas at startup

### 4. Common Patterns
```gdscript
# Pattern: Validate and extract
var result = schema.validate(data)
if result.is_valid():
	var player_name = data["name"]
	var player_level = data["level"]
	# ... use validated data safely

# Pattern: Validate with fallback
var result = schema.validate(config)
if !result.is_valid():
	config = get_default_config()

# Pattern: Partial validation for nested data
var stats_schema = schema.get_at_path("/properties/stats")
var stats_result = stats_schema.validate(stats_data)
```

## Additional Resources

- [JSON Schema Documentation](https://json-schema.org/)
- [JSON Schema Draft-7 Specification](https://json-schema.org/draft-07/json-schema-release-notes.html)
- [Understanding JSON Schema](https://json-schema.org/understanding-json-schema/)
- [GDSchema Main README](../README.md)

## Interactive Testing

The `validation_demo` scene provides a visual way to:
- Test schemas against data in real-time
- See error messages and paths
- Experiment with examples
- Learn by modifying existing schemas

Perfect for learning and prototyping validation logic!

## Questions or Issues?

If you encounter any problems or have questions about these examples:
1. Check the main [README](../README.md) for API documentation
2. Review the for comprehensive Schema examples in the [GDSchema GitHub repository](https://github.com/fimbul-works/gdschema)
3. Open an issue on the project repository

---

**Happy Validating! 🎮✅**
