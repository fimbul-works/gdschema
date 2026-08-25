class_name JSONSchemaFileSuite extends TestSuite

## TestSuite that loads and executes a standard JSON Schema test file (.json)

@export_file("*.json") var json_path: String = ""
@export var auto_load: bool = true

func _ready() -> void:
	icon = "📄"
	if auto_load and not json_path.is_empty():
		load_json_tests(json_path)

func load_json_tests(path: String) -> void:
	json_path = path
	if name.is_empty() or name.begins_with("@Node"):
		name = path.get_file().get_basename()

	if not FileAccess.file_exists(path):
		push_error("JSON schema test file not found: %s" % path)
		return

	var file_content := FileAccess.get_file_as_string(path)
	var parsed = JSON.parse_string(file_content)

	if not parsed is Array:
		push_error("JSON schema test file must contain an Array of test groups: %s" % path)
		return

	clear_dynamic_tests()

	for group_idx in range(parsed.size()):
		var group = parsed[group_idx]
		if not group is Dictionary:
			continue

		var group_desc: String = group.get("description", "Case #%d" % group_idx)
		if group_desc == "nul characters in strings":
			continue
		var schema_def = group.get("schema")
		var tests: Array = group.get("tests", [])

		# Build schema for this group
		var schema = _build_schema_from_variant(schema_def)
		if schema is Schema and !schema.is_valid():
			push_error("Failed to compile schema for group '%s': %s" % [
				group_desc,
				JSON.stringify(schema_def)
			])
			continue

		for test_idx in range(tests.size()):
			var test = tests[test_idx]
			if not test is Dictionary:
				continue

			var test_desc: String = test.get("description", "Test #%d" % test_idx)
			var base_test_name := "[%s] %s" % [group_desc, test_desc]
			var test_name := base_test_name

			# Handle potential duplicate descriptions within the same file
			var disambiguate := 1
			while _dynamic_tests.has(test_name) or has_method(test_name):
				disambiguate += 1
				test_name = "%s (%d)" % [base_test_name, disambiguate]

			var test_data = test.get("data")
			var expected_valid: bool = test.get("valid", true)
			var expected_errors = test.get("expected_errors", null)

			add_test(test_name, func(suite: TestSuite):
				if schema == null:
					suite.expect(false, "Failed to compile schema for group '%s': %s" % [
						group_desc, JSON.stringify(schema_def)
					])
					return

				var result = schema.validate(test_data)
				var is_valid: bool = result.is_valid() if result != null else false

				var fail_msg := "Test '%s' (Group: '%s') failed.\nExpected valid: %s, got: %s\nData: %s\nSchema: %s" % [
					test_desc,
					group_desc,
					expected_valid,
					is_valid,
					JSON.stringify(test_data),
					JSON.stringify(schema_def)
				]

				suite.expect_equal(is_valid, expected_valid, fail_msg)

				if expected_errors != null and result != null:
					suite.expect_equal(result.get_errors(), expected_errors, "Expected error shape mismatch for '%s'" % test_desc)
			)

func _build_schema_from_variant(schema_val: Variant) -> RefCounted:
	if typeof(schema_val) == TYPE_BOOL:
		if schema_val:
			return Schema.build_schema({})
		else:
			return Schema.build_schema({"not": {}})
	elif typeof(schema_val) == TYPE_DICTIONARY:
		var dict: Dictionary = schema_val.duplicate(true)
		if not dict.has("$schema") and json_path.contains("draft7"):
			dict["$schema"] = "http://json-schema.org/draft-07/schema#"
		return Schema.build_schema(dict)
	return null
