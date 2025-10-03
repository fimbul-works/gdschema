#include "schema.hpp"
#include "meta_schema_definitions.hpp"
#include "rule_factory.hpp"
#include "schema_registry.hpp"

#include <godot_cpp/core/memory.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void Schema::_bind_methods() {
	// Bind enum
	BIND_ENUM_CONSTANT(SCHEMA_SCALAR);
	BIND_ENUM_CONSTANT(SCHEMA_OBJECT);
	BIND_ENUM_CONSTANT(SCHEMA_ARRAY);

	// Tree navigation methods
	ClassDB::bind_method(D_METHOD("is_root"), &Schema::is_root);
	ClassDB::bind_method(D_METHOD("get_root"), &Schema::get_root);

	ClassDB::bind_method(D_METHOD("get_schema_type"), &Schema::get_schema_type);
	ClassDB::bind_method(D_METHOD("get_schema_type_name"), &Schema::get_schema_type_name);
	ClassDB::bind_method(D_METHOD("is_object"), &Schema::is_object);
	ClassDB::bind_method(D_METHOD("is_array"), &Schema::is_array);
	ClassDB::bind_method(D_METHOD("is_scalar"), &Schema::is_scalar);

	ClassDB::bind_method(D_METHOD("has_children"), &Schema::has_children);
	ClassDB::bind_method(D_METHOD("get_child_count"), &Schema::get_child_count);
	ClassDB::bind_method(D_METHOD("get_child_keys"), &Schema::get_child_keys);
	ClassDB::bind_method(D_METHOD("has_child", "key"), &Schema::has_child);
	ClassDB::bind_method(D_METHOD("get_child", "key"), &Schema::get_child);

	ClassDB::bind_method(D_METHOD("get_item_schema", "index"), &Schema::get_item_schema);
	ClassDB::bind_method(D_METHOD("get_item_schemas"), &Schema::get_item_schemas);
	ClassDB::bind_method(D_METHOD("get_item_count"), &Schema::get_item_count);
	ClassDB::bind_method(D_METHOD("get_schema_definition"), &Schema::get_schema_definition);
	ClassDB::bind_method(D_METHOD("get_at_path", "path"), &Schema::get_at_path);

	ClassDB::bind_method(D_METHOD("is_valid"), &Schema::is_valid);
	ClassDB::bind_method(D_METHOD("validate", "data"), &Schema::validate);
	ClassDB::bind_method(D_METHOD("get_compile_errors"), &Schema::get_compile_errors);
	ClassDB::bind_method(D_METHOD("get_compile_error_summary"), &Schema::get_compile_error_summary);

	ClassDB::bind_static_method("Schema", D_METHOD("build_schema", "schema_dict", "validate_against_meta"),
			&Schema::build_schema, DEFVAL(false));
	ClassDB::bind_static_method(
			"Schema", D_METHOD("register_schema", "schema", "id"), &Schema::register_schema, DEFVAL(""));
	ClassDB::bind_static_method("Schema", D_METHOD("is_schema_registered", "id"), &Schema::is_schema_registered);
	ClassDB::bind_static_method("Schema", D_METHOD("unregister_schema", "id"), &Schema::unregister_schema);

	BIND_VIRTUAL_METHOD(Schema, _to_string);
}

Schema::Schema() {
	schema_type = SchemaType::SCHEMA_OBJECT; // Default for empty root
	schema_path = ""; // Root has empty path
	is_compiled = false;
	compilation_mutex = Ref<Mutex>(memnew(Mutex));
}

Schema::Schema(const Dictionary &schema_dict, const Ref<Schema> &p_root_schema, const StringName &p_schema_path,
		const bool validate_against_meta) {
	schema_type = SchemaType::SCHEMA_OBJECT;
	schema_path = "";
	is_compiled = false;
	compilation_mutex = Ref<Mutex>(memnew(Mutex));

	if (validate_against_meta) {
		Ref<SchemaValidationResult> validation_result = MetaSchemaDefinitions::validate_schema_definition(schema_dict);

		if (validation_result->has_errors()) {
			UtilityFunctions::push_warning(
					vformat("Schema(%s) failed validation:\n%s\n", p_schema_path, validation_result->get_errors()),
					schema_dict);
		}
	}

	schema_definition = schema_dict;
	schema_type = detect_schema_type(schema_dict);
	schema_path = p_schema_path;

	if (schema_dict.has("$schema") && schema_dict["$schema"].get_type() == Variant::STRING) {
		schema_url = schema_dict["$schema"];
	}

	if (schema_dict.has("$id") && schema_dict["$id"].get_type() == Variant::STRING) {
		schema_id = schema_dict["$id"];
	}

	if (p_root_schema.is_valid()) {
		root_schema = p_root_schema;
	}

	construct_children(schema_dict);
}

Schema::~Schema() {
	// Children and items will be automatically freed by Ref<> destructors
	root_schema.unref();
}

Ref<Schema> Schema::build_schema(const Dictionary &schema_dict, bool validate_against_meta) {
	Ref<Schema> schema = memnew(Schema(schema_dict));
	schema->compile();
	if (schema->compile_errors.size() > 0) {
		UtilityFunctions::push_error("Building schema failed failed:\n", schema->get_compile_error_summary());
	}

	// Auto-register if $id is present
	if (schema.is_valid() && !schema->schema_id.is_empty()) {
		register_schema(schema);
	}

	return schema;
}

bool Schema::register_schema(const Ref<Schema> &schema, const StringName &id) {
	if (!schema.is_valid()) {
		UtilityFunctions::push_error("Cannot register null schema");
		return false;
	}

	// Determine the registration ID
	StringName registration_id = id;

	if (registration_id.is_empty()) {
		// Try to extract from schema's $id
		registration_id = schema->schema_id;

		if (registration_id.is_empty()) {
			UtilityFunctions::push_error("Schema has no $id and no explicit ID provided for registration");
			return false;
		}
	} else if (!schema->schema_id.is_empty() && schema->schema_id != registration_id) {
		// Warn if explicit ID conflicts with $id
		UtilityFunctions::push_warning(vformat(
				"Registering schema with ID '%s' but schema has $id '%s' - this may cause reference resolution issues",
				registration_id, schema->schema_id));
	}

	return SchemaRegistry::get_singleton().register_schema(registration_id, schema);
}

bool Schema::is_schema_registered(const StringName &id) {
	return SchemaRegistry::get_singleton().has_schema(id);
}

bool Schema::unregister_schema(const StringName &id) {
	return SchemaRegistry::get_singleton().unregister_schema(id);
}

void Schema::compile() {
	// Check if compilation is needed
	compilation_mutex->lock();
	if (is_compiled) {
		compilation_mutex->unlock();
		return; // Already compiled
	}
	compilation_mutex->unlock();

	// Let RuleFactory handle the compilation
	Ref<Schema> ref = (const_cast<Schema *>(this));
	RuleFactory::get_singleton().create_rules(ref);
}

Schema::SchemaType Schema::detect_schema_type(const Dictionary &dict) const {
	// Check explicit type declaration
	Variant type_var = dict.get("type", Variant());
	if (type_var.get_type() == Variant::STRING) {
		String type_str = type_var.operator String();
		if (type_str == "array") {
			return SchemaType::SCHEMA_ARRAY;
		} else if (type_str == "object") {
			return SchemaType::SCHEMA_OBJECT;
		}
	} else if (type_var.get_type() == Variant::ARRAY) {
		Array type_array = type_var.operator Array();
		for (int i = 0; i < type_array.size(); i++) {
			if (type_array[i].get_type() == Variant::STRING) {
				String type_str = type_array[i].operator String();
				if (type_str == "array") {
					return SchemaType::SCHEMA_ARRAY;
				} else if (type_str == "object") {
					return SchemaType::SCHEMA_OBJECT;
				}
			}
		}
	}

	// Check for array-specific keywords
	if (dict.has("items") || dict.has("minItems") || dict.has("maxItems") || dict.has("uniqueItems") ||
			dict.has("additionalItems") || dict.has("contains")) {
		return SchemaType::SCHEMA_ARRAY;
	}

	// Check for object-specific keywords
	if (dict.has("properties") || dict.has("required") || dict.has("additionalProperties") ||
			dict.has("patternProperties") || dict.has("minProperties") || dict.has("maxProperties") ||
			dict.has("dependencies") || dict.has("propertyNames")) {
		return SchemaType::SCHEMA_OBJECT;
	}

	// Logical schemas
	if (dict.has("allOf") || dict.has("anyOf") || dict.has("oneOf") || dict.has("not") || dict.has("if") ||
			dict.has("then") || dict.has("else")) {
		return SchemaType::SCHEMA_LOGICAL;
	}

	// Default to scalar for string, number, boolean constraints
	return SchemaType::SCHEMA_SCALAR;
}

Variant Schema::variant_to_schema_dict(const Variant &value) const {
	Dictionary child_dict;

	switch (value.get_type()) {
		case Variant::DICTIONARY:
			// Full schema object - use as-is
			return value;

		case Variant::STRING: {
			// Handle shorthand like "string" -> {"type": "string"}
			String type_str = value.operator String();
			child_dict["type"] = type_str;
			return child_dict;
		}

		case Variant::BOOL: {
			// Handle boolean schemas
			bool bool_val = value.operator bool();
			if (bool_val) {
				// true schema - allow anything (empty schema)
				return Dictionary();
			} else {
				// false schema - allow nothing
				child_dict["not"] = Dictionary();
				return child_dict;
			}
		}

		case Variant::ARRAY: {
			// Array could be a type array like ["string", "number"]
			Array arr = value.operator Array();
			child_dict["type"] = arr;
			return child_dict;
		}

		case Variant::NIL:
			// null -> allow null type
			child_dict["type"] = "null";
			return child_dict;

		case Variant::INT:
		case Variant::FLOAT: {
			// Numeric value -> const constraint
			child_dict["const"] = value;
			return child_dict;
		}

		default:
			// Unsupported type - return null to indicate failure
			return Variant();
	}
}

void Schema::add_compile_error(const String error_message) {
	compile_errors.push_back(SchemaCompileError(error_message, PackedStringArray(schema_path.split("/"))));
}

void Schema::construct_children(const Dictionary &dict) {
	// ========== OBJECT SCHEMAS ==========
	if (schema_type == SchemaType::SCHEMA_OBJECT) {
		// Handle object properties - meta-schema already validated structure
		if (dict.has("properties")) {
			Dictionary properties = dict["properties"].operator Dictionary();
			Array keys = properties.keys();

			for (int i = 0; i < keys.size(); i++) {
				StringName key = keys[i];
				Variant value = properties[key];

				// Convert to schema dict - meta-schema already validated this will work
				Dictionary child_dict = variant_to_schema_dict(value);
				StringName child_key = vformat("properties/%s", key);
				StringName child_path = vformat("%s/%s", schema_path, child_key);
				Ref<Schema> child_node = memnew(Schema(child_dict, get_root(), child_path));
				children[child_key] = child_node;
			}
		}

		create_schema_child_if_exists(dict, "propertyNames");

		if (dict.has("patternProperties")) {
			Dictionary pattern_properties = dict["patternProperties"].operator Dictionary();
			Array patterns = pattern_properties.keys();

			for (int i = 0; i < patterns.size(); i++) {
				String pattern = patterns[i].operator String();
				Dictionary pattern_schema = pattern_properties[patterns[i]].operator Dictionary();
				StringName child_key = vformat("patternProperties/%s", pattern);
				create_schema_child(pattern_schema, child_key);
			}
		}

		create_schema_child_if_exists(dict, "additionalProperties");

		// Dependencies - only handle schema dependencies
		if (dict.has("dependencies")) {
			Dictionary dependencies = dict["dependencies"].operator Dictionary();
			Array dep_keys = dependencies.keys();

			for (int i = 0; i < dep_keys.size(); i++) {
				String dep_name = dep_keys[i].operator String();
				Variant dep_value = dependencies[dep_keys[i]];

				if (dep_value.get_type() == Variant::DICTIONARY) {
					Dictionary dep_schema = dep_value.operator Dictionary();
					StringName child_key = vformat("dependencies/%s", dep_name);
					create_schema_child(dep_schema, child_key);
				}
			}
		}
	}

	// ========== ARRAY SCHEMAS ==========
	else if (schema_type == SchemaType::SCHEMA_ARRAY) {
		if (dict.has("items")) {
			Variant items_var = dict["items"];

			if (items_var.get_type() == Variant::ARRAY) {
				// Array of schemas (tuple validation)
				Array schemas_array = items_var.operator Array();
				for (int64_t i = 0; i < schemas_array.size(); i++) {
					Variant schema_item = schemas_array[i];
					Variant schema_dict_var = variant_to_schema_dict(schema_item);
					if (schema_dict_var.get_type() == Variant::DICTIONARY) {
						Dictionary schema_dict = schema_dict_var.operator Dictionary();
						StringName child_key = vformat("items/%d", i);
						StringName child_path = vformat("%s/%s", schema_path, child_key);
						Ref<Schema> schema_node = memnew(Schema(schema_dict, get_root(), child_path));
						item_schemas.push_back(schema_node);
						children[child_key] = schema_node;
					}
				}
			} else {
				Variant item_schema_dict = variant_to_schema_dict(items_var);
				if (item_schema_dict.get_type() == Variant::DICTIONARY) {
					Dictionary item_schema = item_schema_dict.operator Dictionary();
					StringName child_key = "items";
					StringName child_path = vformat("%s/%s", schema_path, child_key);
					Ref<Schema> item_node = memnew(Schema(item_schema, get_root(), child_path));
					item_schemas.push_back(item_node);
					children[child_key] = item_node;
				}
			}
		}

		create_schema_child_if_exists(dict, "additionalItems");
		create_schema_child_if_exists(dict, "contains");
	}

	// ========== LOGICAL COMPOSITION ==========
	create_logical_children(dict, "allOf");
	create_logical_children(dict, "anyOf");
	create_logical_children(dict, "oneOf");
	create_schema_child_if_exists(dict, "not");

	// ========== CONDITIONAL SCHEMAS ==========
	create_schema_child_if_exists(dict, "if");
	create_schema_child_if_exists(dict, "then");
	create_schema_child_if_exists(dict, "else");

	// ========== SCHEMA DEFINITIONS ==========
	create_definitions_children(dict, "definitions");
	create_definitions_children(dict, "$defs");
}

Ref<Schema> Schema::create_schema_child(const Dictionary &child_schema, const StringName &child_key) {
	StringName child_path = vformat("%s/%s", schema_path, child_key);
	Ref<Schema> child_node = memnew(Schema(child_schema, get_root(), child_path));
	children[child_key] = child_node;
	return child_node;
}

void Schema::create_schema_child_if_exists(const Dictionary &dict, const StringName &key) {
	if (dict.has(key)) {
		Variant value = dict[key];
		Variant schema_dict_var = variant_to_schema_dict(value);
		if (schema_dict_var.get_type() == Variant::DICTIONARY) {
			Dictionary child_schema = schema_dict_var.operator Dictionary();
			create_schema_child(child_schema, key);
		}
	}
}

void Schema::create_logical_children(const Dictionary &dict, const StringName &key) {
	if (dict.has(key)) {
		Array array = dict[key].operator Array();
		for (int64_t i = 0; i < array.size(); i++) {
			Variant sub_schema_dict_var = variant_to_schema_dict(array[i]);
			if (sub_schema_dict_var.get_type() == Variant::DICTIONARY) {
				Dictionary sub_schema_dict = sub_schema_dict_var.operator Dictionary();
				StringName child_key = vformat("%s/%d", key, i);
				create_schema_child(sub_schema_dict, child_key);
			}
		}
	}
}

void Schema::create_definitions_children(const Dictionary &dict, const StringName &key) {
	if (dict.has(key)) {
		Dictionary definitions = dict[key].operator Dictionary();
		Array def_keys = definitions.keys();

		for (int i = 0; i < def_keys.size(); i++) {
			String def_name = def_keys[i].operator String();
			Variant def_value = definitions[def_keys[i]];

			if (def_value.get_type() == Variant::DICTIONARY) {
				Dictionary def_schema = def_value.operator Dictionary();
				StringName child_key = vformat("%s/%s", key, def_name);
				create_schema_child(def_schema, child_key);
			}
		}
	}
}

Ref<Schema> Schema::resolve_reference(const String &reference_uri) const {
	String uri = reference_uri.strip_edges();

	if (uri.is_empty()) {
		return Ref<Schema>();
	}

	// Handle different reference formats
	if (uri == "#") {
		// Root reference - return the root schema
		return get_root();
	}

	if (uri.begins_with("#/")) {
		// JSON Pointer within current document
		String pointer = uri.substr(1); // Remove the '#'
		String normalized = normalize_json_pointer(pointer);
		return get_root()->get_at_path(normalized);
	}

	if (uri.begins_with("#")) {
		// Fragment identifier (legacy anchor, not JSON Pointer)
		String fragment = uri.substr(1);
		UtilityFunctions::push_warning(vformat("Anchor references not implemented: %s", uri));
		return Ref<Schema>();
	}

	// External reference - check if it contains fragment
	int fragment_pos = uri.find("#");
	if (fragment_pos != -1) {
		// External reference with fragment: "schema-id#/path"
		String schema_id = uri.substr(0, fragment_pos);
		String fragment = uri.substr(fragment_pos + 1);

		// Get external schema from registry
		Ref<Schema> external_schema = SchemaRegistry::get_singleton().get_schema(schema_id);
		if (!external_schema.is_valid()) {
			UtilityFunctions::push_error(vformat("External schema not found: %s", schema_id));
			return Ref<Schema>();
		}

		if (fragment.is_empty() || fragment == "") {
			return external_schema; // Root of external schema
		}

		if (fragment.begins_with("/")) {
			// JSON Pointer in external schema
			String normalized = normalize_json_pointer(fragment);
			return external_schema->get_at_path(normalized);
		} else {
			// Anchor reference in external schema
			UtilityFunctions::push_warning(vformat("External anchor references not implemented: %s", uri));
			return Ref<Schema>();
		}
	} else {
		// Pure external reference (whole document)
		return SchemaRegistry::get_singleton().get_schema(uri);
	}
}

String Schema::normalize_json_pointer(const String &pointer) {
	String clean = pointer.strip_edges();

	// Ensure it starts with "/"
	if (!clean.begins_with("/")) {
		clean = "/" + clean;
	}

	// Remove duplicate slashes
	while (clean.contains("//")) {
		clean = clean.replace("//", "/");
	}

	// Remove trailing slash unless it's the root
	if (clean.length() > 1 && clean.ends_with("/")) {
		clean = clean.substr(0, clean.length() - 1);
	}

	return clean;
}

PackedStringArray Schema::parse_json_pointer(const String &pointer) {
	PackedStringArray segments;

	if (pointer == "/") {
		return segments; // Empty array for root
	}

	String clean = pointer;
	if (clean.begins_with("/")) {
		clean = clean.substr(1); // Remove leading slash
	}

	if (clean.is_empty()) {
		return segments;
	}

	PackedStringArray raw_segments = clean.split("/");

	// Unescape each segment
	for (int i = 0; i < raw_segments.size(); i++) {
		String unescaped = unescape_json_pointer_segment(raw_segments[i]);
		segments.append(unescaped);
	}

	return segments;
}

String Schema::unescape_json_pointer_segment(const String &segment) {
	// JSON Pointer escaping rules:
	// "~0" represents "~"
	// "~1" represents "/"
	String result = segment;
	result = result.replace("~1", "/");
	result = result.replace("~0", "~");
	return result;
}

Array Schema::get_child_keys() const {
	if (schema_type != SchemaType::SCHEMA_OBJECT) {
		return Array();
	}

	Array keys;
	keys.resize(children.size());

	int i = 0;
	for (const auto &pair : children) {
		keys[i++] = pair.first;
	}

	return keys;
}

Ref<Schema> Schema::get_child(const StringName &key) const {
	auto it = children.find(key);
	if (it != children.end()) {
		return it->second;
	}

	return Ref<Schema>();
}

Ref<Schema> Schema::get_at_path(const StringName &path) const {
	if (path.is_empty() || path == StringName("/")) {
		return get_root();
	}

	// Remove leading slash and split path
	String clean_path = path;
	if (clean_path.begins_with("/")) {
		clean_path = clean_path.substr(1);
	}

	PackedStringArray path_parts = clean_path.split("/");
	Ref<Schema> current = get_root(); // Always start from root for absolute paths

	// Navigate through each path segment
	for (int i = 0; i < path_parts.size(); i++) {
		String part = path_parts[i];
		if (part.is_empty()) {
			continue;
		}

		// Unescape JSON pointer segment
		part = unescape_json_pointer_segment(part);

		Ref<Schema> next_schema;

		// Try different child key formats based on the path part
		if (part == "properties" || part == "definitions" || part == "$defs") {
			// These are schema keywords - look for direct children
			StringName child_key = StringName(part);
			next_schema = current->get_child(child_key);

			if (!next_schema.is_valid()) {
				// Try with the next part combined (for flattened structure)
				if (i + 1 < path_parts.size()) {
					String next_part = unescape_json_pointer_segment(path_parts[i + 1]);
					StringName combined_key = StringName(vformat("%s/%s", part, next_part));
					next_schema = current->get_child(combined_key);
					if (next_schema.is_valid()) {
						i++; // Skip the next part since we consumed it
					}
				}
			}
		} else {
			// This is a property name or definition name
			// Look for it under the appropriate parent (properties/, definitions/, etc.)
			StringName direct_key = StringName(part);
			next_schema = current->get_child(direct_key);

			if (!next_schema.is_valid()) {
				// Try prefixed versions
				StringName properties_key = StringName(vformat("properties/%s", part));
				next_schema = current->get_child(properties_key);

				if (!next_schema.is_valid()) {
					StringName definitions_key = StringName(vformat("definitions/%s", part));
					next_schema = current->get_child(definitions_key);
				}

				if (!next_schema.is_valid()) {
					StringName defs_key = StringName(vformat("$defs/%s", part));
					next_schema = current->get_child(defs_key);
				}
			}
		}

		if (!next_schema.is_valid()) {
			return Ref<Schema>();
		}

		current = next_schema;
	}

	return current;
}

Ref<SchemaValidationResult> Schema::validate(const Variant &data) {
	// Check compilation status safely
	compilation_mutex->lock();
	bool needs_compilation = !is_compiled;
	compilation_mutex->unlock();

	// Ensure compilation is complete
	if (needs_compilation) {
		compile();
	}

	// Lock for reading compilation state
	compilation_mutex->lock();

	ValidationContext context(this);

	if (!is_valid()) {
		for (auto error : compile_errors) {
			context.add_error(error.message, error.get_path_string());
		}
		compilation_mutex->unlock();
		return SchemaValidationResult::from_context(context);
	}

	if (!rules) {
		context.add_error("Schema not compiled");
		compilation_mutex->unlock();
		return SchemaValidationResult::from_context(context);
	}

	auto validation_rules = rules;
	compilation_mutex->unlock();

	validation_rules->validate(data, context);
	return SchemaValidationResult::from_context(context);
}

Ref<SchemaValidationResult> Schema::validate_uncompiled(const Dictionary &schema_dict) {
	ValidationContext context(this);
	rules->validate(schema_dict, context);
	return SchemaValidationResult::from_context(context);
}

bool Schema::is_valid() const {
	compilation_mutex->lock();
	bool valid = is_compiled && compile_errors.size() == 0;
	compilation_mutex->unlock();
	return valid;
}

void Schema::set_compilation_result(std::shared_ptr<RuleGroup> compiled_rules, std::vector<SchemaCompileError> errors) {
	compilation_mutex->lock();
	rules = compiled_rules;
	compile_errors = std::move(errors);
	is_compiled = true;
	compilation_mutex->unlock();
}

Array Schema::get_compile_errors() {
	if (compile_errors.empty()) {
		return Array();
	}

	Array errors;
	for (size_t i = 0; i < compile_errors.size(); i++) {
		const auto &error = compile_errors[i];

		Dictionary error_dict;
		error_dict["path"] = error.get_path_string();
		error_dict["message"] = error.message;
		errors.append(error_dict);
	}

	return errors;
}

String Schema::get_compile_error_summary() {
	if (!is_compiled) {
		return "Schema is not compiled";
	}

	if (compile_errors.empty()) {
		return "";
	}

	String summary = vformat("Schema compilation failed with %d error(s):\n", (int64_t)compile_errors.size());

	for (int64_t i = 0; i < compile_errors.size(); i++) {
		const auto &error = compile_errors[i];

		summary += vformat("  [%d] ", i + 1);

		if (!error.schema_path_parts.is_empty()) {
			summary += vformat("At '%s': ", error.get_path_string());
		}

		summary += error.message;

		if (i < compile_errors.size() - 1) {
			summary += "\n";
		}
	}

	return summary;
}

String Schema::_to_string() const {
	String type_str;
	switch (schema_type) {
		case SchemaType::SCHEMA_OBJECT:
			type_str = vformat("object, %d properties", (int64_t)get_child_count());
			break;
		case SchemaType::SCHEMA_ARRAY:
			type_str = vformat("array, %d items", (int64_t)get_item_count());
			break;
		case SchemaType::SCHEMA_SCALAR:
			type_str = "scalar";
			break;
		case SchemaType::SCHEMA_LOGICAL:
			type_str = "logical";
			break;
	}

	String path_info = is_root() ? "root" : vformat("path: %s", schema_path);
	return vformat("Schema(%s, %s)", path_info, type_str);
}
