# GDSchema

This is the **C++ GDExtension implementation** of the GDSchema plugin. It provides a full [JSON Schema Draft-7](https://json-schema.org/) validation plugin for Godot 4.3 and later.

## Version History

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

* 📏 **Full JSON Schema Draft-7**: All validation keywords supported, including `type`, `minimum`, `maxLength`, `pattern`, `required`, `enum`, and logical composition (`allOf`, `oneOf`, `anyOf`, `not`)
* 🗂️ **Schema Registry**: Schemas with `$id` are auto-registered and can be referenced across documents with `$ref`
* 🔍 **Rich Validation Errors**: Detailed error messages with JSON Pointer paths, violated constraints, and invalid values
* 🧩 **Editor Integration**: Navigate schema trees, retrieve child nodes, and inspect definitions for editor auto-complete and tooling
* 🛡️ **Thread-Safe and Lazy**: Schemas compile lazily with caching for performance and thread safety
* ✅ **Comprehensive Results**: Validation returns a `SchemaValidationResult` with all errors collected at once

## Installation & Setup

### **Building From Source**

#### **Prerequisites**
- **Git** (for cloning and submodules)
- **Python 3.x** (for SCons build system)
- **C++ compiler** with C++17 support:
  - **Windows**: Visual Studio 2022 with C++ workload
  - **Linux/macOS**: GCC 9+ or Clang 10+
- **SCons** build system (`pip install scons`)

#### **Step 1: Clone the Repository**
```bash
# Clone with submodules
git clone --recursive https://github.com/fimbul-works/GDSchema

# Or if already cloned, initialize submodules
git submodule update --init --recursive
```

#### **Step 2: Build the Extension**
```bash
# Debug build
scons target=template_debug

# Release build
scons target=template_release

# Specify platform (default is platform-dependent)
scons platform=windows target=template_release
scons platform=linux target=template_release
```

#### **Build Options**
- `platform`: Target (`windows`, `linux`, `macos`, etc.)
- `target`: Build type (`template_debug`, `template_release`)
- `arch`: CPU architecture (`x86_32`, `x86_64`, `arm64`, etc.)
- `dev_build`: Enable extra debugging (`yes`/`no`)
- `use_llvm`: Use Clang/LLVM compiler (`yes`/`no`)
- `verbose`: Verbose build output (`yes`/`no`)

---

## Supported Platforms

- **Windows**: ✅ Prebuilt binaries available for **x86 64-bit** architecture.
- **Linux**: ✅ Prebuilt binaries available for **x86 64-bit** architecture.
- **macOS**: ✅ Prebuilt binaries available for **universal** architecture.

📌 **Contributions welcome!** If you can help with testing macOS, open a PR.

---

## **Contributing**

### **Development Guidelines**
1. **Follow Godot's API design patterns.**
4. **Thread safety**: No global state; ensure **safe multithreading**.
5. **Write tests**: Every new feature should have **test coverage**. See [the test suite](project/tests/) in the project folder.
6. **Document your changes**: All public APIs **must be documented**.

### **Code Formatting**
```bash
# Format code (requires clang-format)
clang-format -i src/*.cpp src/*.hpp src/rule/*.cpp src/rule/*.hpp src/selector/*.cpp src/selector/*.hpp
```

---

## **License**

MIT License (see [LICENSE](LICENSE) file for details).

---

**Built with ✅ by [FimbulWorks](https://github.com/fimbul-works)**
