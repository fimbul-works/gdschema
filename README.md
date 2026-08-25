# GDSchema

This is the **C++ GDExtension implementation** of the GDSchema plugin. It provides a full [JSON Schema Draft 2020-12](https://json-schema.org/) validation plugin for Godot 4.3 and later.

## Version History

- **2.1.0** (Current) - Full conformance with the official JSON Schema Test Suite for draft-07 and draft-2020-12, and various memory leaks fixed
- **2.0.1** - Fixed `multipleOf`-rule on platforms with 32-bit floating point precision, and various memory leaks
- **2.0.0** - Upgraded to JSON Schema Draft 2020-12, and fixed various bugs
- **1.3.0** - Improved meta-schema handling, added `get_id`, `get_title`, `get_description`, and `get_comment` methods to the `Schema class`, and the `Schema.get_schema_from_registry` static method
- **1.2.2** - Added support for Android (arm64, x86_64) platforms
- **1.2.1** - Updated documentation
- **1.2.0** - Added `Schema.load_json` and `Schema.load_json_file` methods, and fixed schema meta validation
- **1.1.0** - Enabled storing custom data in `ValidationContext`, and support for external implementations of the `"default"` keyword
- **1.0.0** - Initial public release

## Quick Start

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

📌 **For full usage details and API documentation, see the plugin README:**
📂 [`project/addons/GDSchema/README.md`](project/addons/GDSchema/README.md)

## Features

- 📏 **Full JSON Schema Draft 2020-12**: All validation keywords supported, including `type`, `minimum`, `maxLength`, `pattern`, `required`, `enum`, and logical composition (`allOf`, `oneOf`, `anyOf`, `not`), along with Draft 2020-12 keywords like `unevaluatedItems` and `unevaluatedProperties`, and dynamic references
- 🗂️ **Schema Registry**: Schemas with `$id` are auto-registered and can be referenced across documents with `$ref`
- 🔍 **Rich Validation Errors**: Detailed error messages with JSON Pointer paths, violated constraints, and invalid values
- 🧩 **Editor Integration**: Navigate schema trees, retrieve child nodes, and inspect definitions for editor auto-complete and tooling
- 🛡️ **Thread-Safe and Lazy**: Schemas compile lazily with caching for performance and thread safety
- ✅ **Comprehensive Results**: Validation returns a `SchemaValidationResult` with all errors collected at once

## Installation & Setup

GDSchema is available on the [Godot Asset Library](https://godotengine.org/asset-library/asset/4383). Be sure to enable the plugin in your project settings after installing.

### Building From Source

Alternatively you can build the binaries yourself. For the [latest versions of macOS](#supported-platforms) this may be necessary.

#### Prerequisites
- **Git** (for cloning and submodules)
- **Python 3.x** (for SCons build system)
- **C++ compiler** with C++17 support:
  - **Windows**: Visual Studio 2022 with C++ workload
  - **Linux/macOS**: GCC 9+ or Clang 10+
- **SCons** build system (`pip install scons`)
- **Android**: JDK version 20 and NDK version 23.2.8568313

#### Step 1: Clone the Repository
```bash
# Clone with submodules
git clone --recursive https://github.com/fimbul-works/GDSchema

# Or if already cloned, initialize submodules
git submodule update --init --recursive
```

#### Step 2: Build the Extension
```bash
# Debug build
scons target=template_debug

# Release build
scons target=template_release

# Specify platform (default is platform-dependent)
scons platform=windows target=template_release
scons platform=linux target=template_release
```

#### Build Options
- `platform`: Target (`windows`, `linux`, `macos`, `android`, etc.)
- `target`: Build type (`template_debug`, `template_release`)
- `arch`: CPU architecture (`x86_32`, `x86_64`, `arm64`, etc.)
- `dev_build`: Enable extra debugging (`yes`/`no`)
- `use_llvm`: Use Clang/LLVM compiler (`yes`/`no`)
- `verbose`: Verbose build output (`yes`/`no`)

---

## Supported Platforms

- **Windows**: ✅ Prebuilt binaries available for **x86 64-bit** architecture.
- **Linux**: ✅ Prebuilt binaries available for **x86 64-bit** architecture.
- **macOS**: ⚠️ Prebuilt binaries available for **universal** architecture.
  - **Note**: Some macOS configurations (particularly newer versions with stricter Gatekeeper policies) may prevent loading of GDExtensions generally, not just this plugin. If the extension fails to load, try building from source or test with other GDExtensions to determine if this is a system-wide issue.
- **Android**: ✅ Prebuilt binaries available for **ARM 64-bit** and **x86 64-bit** architectures.

---

## Contributing

### Development Guidelines

1. **Follow Godot's API design patterns.**
4. **Thread safety**: No global state; ensure **safe multithreading**.
5. **Write tests**: Every new feature should have **test coverage**. See [the test suite](project/tests/) in the project folder.
6. **Document your changes**: All public APIs **must be documented**.

---

## **License**

MIT License (see [LICENSE](LICENSE) file for details).

---

**Built with ✅ by [FimbulWorks](https://github.com/fimbul-works)**
