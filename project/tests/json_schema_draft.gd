class_name JSONSchemaDraft extends TestSuite

## TestSuite that dynamically populates child JSONSchemaFileSuite nodes for all JSON files in a directory

@export_dir var tests_dir: String = "res://tests/draft2020-12"
@export var exclude_files: PackedStringArray = []

func _ready() -> void:
	icon = "📁"
	_load_draft_suites()

func _load_draft_suites() -> void:
	if tests_dir.is_empty() or not DirAccess.dir_exists_absolute(tests_dir):
		return

	var dir := DirAccess.open(tests_dir)
	if not dir:
		push_error("Failed to open draft directory: %s" % tests_dir)
		return

	dir.list_dir_begin()
	var file_name := dir.get_next()
	var json_files: PackedStringArray = []

	while file_name != "":
		if not dir.current_is_dir() and file_name.ends_with(".json"):
			if not exclude_files.has(file_name):
				json_files.append(file_name)
		file_name = dir.get_next()

	dir.list_dir_end()

	# Sort file names for deterministic execution order
	json_files.sort()

	for json_file in json_files:
		var full_path := tests_dir.path_join(json_file)
		var file_suite = load("res://tests/json_schema_file_suite.gd").new()
		file_suite.name = json_file.get_basename()
		file_suite.json_path = full_path
		add_child(file_suite)
		file_suite.load_json_tests(full_path)
