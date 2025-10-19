# GDSchema

A full [JSON Schema Draft-7](https://json-schema.org/) validation plugin for Godot 4.3. This extension brings industry-standard JSON validation directly into Godot, enabling configuration validation, and API contract enforcement.

Schemas are compiled into navigable tree structures, supporting `$ref` references, schema registration, and detailed error reporting.

This extension is designed to integrate seamlessly with other Godot workflows, and more advanced tooling is in development.

**New to JSON Schema in Godot?** Check out the [`examples/`](examples/) directory for practical usage examples.

## Version History

- **1.2.1** (Current) - Added support for Android (arm64, x86_64) platforms
- **1.2.0** - Added `Schema.load_json` and `Schema.load_json_file` methods, and fixed schema meta validation
- **1.1.0** - Enabled storing custom data in `ValidationContext`, and support for external implementations of the `"default"` keyword
- **1.0.0** - First release with full JSON Schema Draft-7 support, schema registry, validation results, and editor-friendly API

## Features

- 📏 **Full JSON Schema Draft-7**: All validation keywords supported, including `type`, `minimum`, `maxLength`, `pattern`, `required`, `enum`, and logical composition (`allOf`, `oneOf`, `anyOf`, `not`)
- 🗂️ **Schema Registry**: Schemas with `$id` are auto-registered and can be referenced across documents with `$ref`
- 🔍 **Rich Validation Errors**: Detailed error messages with JSON Pointer paths, violated constraints, and invalid values
- 🧩 **Editor Integration**: Navigate schema trees, retrieve child nodes, and inspect definitions for editor auto-complete and tooling (coming in future versions)
- 🛡️ **Thread-Safe and Lazy**: Schemas compile lazily with caching for performance and thread safety
- ✅ **Comprehensive Results**: Validation returns a `SchemaValidationResult` with all errors collected at once

## Compatibility

- Requires **Godot 4.3** or higher
- Supported platforms:
  - Windows (x86 64-bit)
  - Linux (x86 64-bit)
  - macOS: (Universal)
    - **Note**: Some macOS configurations (particularly newer versions with stricter Gatekeeper policies) may prevent loading of GDExtensions generally, not just this plugin. If the extension fails to load, try building from source or test with other GDExtensions to determine if this is a system-wide issue.
  - Android (arm64, x86_64)

## Basic Usage

### Building a Schema

```gdscript
# Simple string schema
var schema = Schema.build_schema({
    "type": "string",
    "minLength": 3,
    "maxLength": 50
})

# Object schema with required fields
var user_schema = Schema.build_schema({
    "type": "object",
    "properties": {
        "name": {"type": "string"},
        "age": {"type": "integer", "minimum": 0},
        "email": {"type": "string", "format": "email"}
    },
    "required": ["name", "email"]
})
```

### Referencing Schemas

```gdscript
# Schema with $id (auto-registered globally)
var address_schema = Schema.build_schema({
    "$id": "http://example.com/address.json",
    "type": "object",
    "properties": {
        "street": {"type": "string"},
        "city": {"type": "string"}
    }
})

# Reference a registered schema
var person_schema = Schema.build_schema({
    "type": "object",
    "properties": {
        "name": {"type": "string"},
        "address": {"$ref": "http://example.com/address.json"}
    }
})
```

### Validating Data

```gdscript
var schema = Schema.build_schema({
    "type": "object",
    "properties": {
        "username": {"type": "string", "minLength": 3},
        "age": {"type": "integer", "minimum": 0}
    },
    "required": ["username"]
})

# Valid data
var result = schema.validate({
    "username": "alice",
    "age": 25
})
print(result.is_valid())  # true

# Invalid data
result = schema.validate({
    "username": "ab",
    "age": -5
})

if result.has_errors():
    print(result.get_summary())
    # Schema validation failed with 2 error(s):
    #   [1] At '/username': String length 2 is less than minimum 3 (minLength)
    #   [2] At '/age': Value -5 is less than minimum 0 (minimum)
```

### Navigating Schemas

```gdscript
var schema = Schema.build_schema({
    "type": "object",
    "properties": {
        "user": {
            "type": "object",
            "properties": {
                "name": {"type": "string"},
                "age": {"type": "integer"}
            }
        }
    }
})

# Navigate with JSON Pointer
var name_schema = schema.get_at_path("/properties/user/properties/name")
print(name_schema.get_schema_definition())  # {"type": "string"}
```

### Working with Validation Results

```gdscript
var schema = Schema.build_schema({
    "type": "integer",
    "minimum": 0,
    "maximum": 100
})

var result = schema.validate(150)
if result.has_errors():
    print(result.get_error_message(0))  # "Value 150 exceeds maximum 100"
    print(result.get_error_path(0))     # "/"
    print(result.get_error_constraint(0))  # "maximum"
```

## Installation

1. Download the plugin from the Godot Asset Library or from GitHub
2. Extract the contents into your project’s `addons/` directory
3. Enable the plugin in Project Settings → Plugins

## License

MIT License (see [LICENSE](LICENSE) file for details).

---

**Built with ✅ by [FimbulWorks](https://github.com/fimbul-works)**
