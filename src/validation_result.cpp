#include "validation_result.hpp"
#include "validation_context.hpp"

using namespace godot;

void SchemaValidationResult::_bind_methods() {
	// Status checking
	ClassDB::bind_method(D_METHOD("is_valid"), &SchemaValidationResult::is_valid);
	ClassDB::bind_method(D_METHOD("has_errors"), &SchemaValidationResult::has_errors);
	ClassDB::bind_method(D_METHOD("error_count"), &SchemaValidationResult::error_count);

	// Error access
	ClassDB::bind_method(D_METHOD("get_errors"), &SchemaValidationResult::get_errors);
	ClassDB::bind_method(D_METHOD("get_error", "index"), &SchemaValidationResult::get_error);
	ClassDB::bind_method(D_METHOD("get_error_message", "index"), &SchemaValidationResult::get_error_message);
	ClassDB::bind_method(D_METHOD("get_error_path", "index"), &SchemaValidationResult::get_error_path);
	ClassDB::bind_method(D_METHOD("get_error_path_array", "index"), &SchemaValidationResult::get_error_path_array);
	ClassDB::bind_method(D_METHOD("get_error_constraint", "index"), &SchemaValidationResult::get_error_constraint);
	ClassDB::bind_method(D_METHOD("get_error_value", "index"), &SchemaValidationResult::get_error_value);

	// Summary and formatting
	ClassDB::bind_method(D_METHOD("get_summary"), &SchemaValidationResult::get_summary);
	ClassDB::bind_method(D_METHOD("get_brief_summary"), &SchemaValidationResult::get_brief_summary);
	ClassDB::bind_method(D_METHOD("get_all_error_paths"), &SchemaValidationResult::get_all_error_paths);
	ClassDB::bind_method(D_METHOD("get_violated_constraints"), &SchemaValidationResult::get_violated_constraints);

	// Static methods
	ClassDB::bind_static_method("SchemaValidationResult", D_METHOD("success"), &SchemaValidationResult::success);

	BIND_VIRTUAL_METHOD(SchemaValidationResult, _to_string);
}

Ref<SchemaValidationResult> SchemaValidationResult::from_context(const ValidationContext &context) {
	Ref<SchemaValidationResult> result = memnew(SchemaValidationResult);

	// Convert ValidationContext errors to YAMLValidationError format
	const std::vector<ValidationError> &context_errors = context.get_errors();

	for (const ValidationError &ctx_error : context_errors) {
		// Convert instance path string to array of path segments
		Array path_segments;
		String path_str = ctx_error.instance_path;

		if (!path_str.is_empty() && path_str != "/") {
			// Remove leading slash and split by slash
			if (path_str.begins_with("/")) {
				path_str = path_str.substr(1);
			}
			PackedStringArray parts = path_str.split("/");
			path_segments.resize(parts.size());
			for (int i = 0; i < parts.size(); i++) {
				path_segments[i] = parts[i];
			}
		}

		YAMLValidationError error(ctx_error.message, ctx_error.instance_path, path_segments, ctx_error.keyword,
				ctx_error.schema_path, ctx_error.invalid_value);

		result->add_error(error);
	}

	return result;
}

Ref<SchemaValidationResult> SchemaValidationResult::success() {
	return memnew(SchemaValidationResult);
}

Array SchemaValidationResult::get_errors() const {
	Array result;
	result.resize(errors.size());
	for (size_t i = 0; i < errors.size(); i++) {
		result[i] = errors[i].to_dict();
	}
	return result;
}

Dictionary SchemaValidationResult::get_error(int index) const {
	if (index >= 0 && index < static_cast<int>(errors.size())) {
		return errors[index].to_dict();
	}
	return Dictionary();
}

String SchemaValidationResult::get_error_message(int index) const {
	if (index >= 0 && index < static_cast<int>(errors.size())) {
		return errors[index].message;
	}
	return "";
}

String SchemaValidationResult::get_error_path(int index) const {
	if (index >= 0 && index < static_cast<int>(errors.size())) {
		return errors[index].path;
	}
	return "";
}

Array SchemaValidationResult::get_error_path_array(int index) const {
	if (index >= 0 && index < static_cast<int>(errors.size())) {
		return errors[index].path_array;
	}
	return Array();
}

String SchemaValidationResult::get_error_constraint(int index) const {
	if (index >= 0 && index < static_cast<int>(errors.size())) {
		return errors[index].constraint;
	}
	return "";
}

Variant SchemaValidationResult::get_error_value(int index) const {
	if (index >= 0 && index < static_cast<int>(errors.size())) {
		return errors[index].invalid_value;
	}
	return Variant();
}

String SchemaValidationResult::get_summary() const {
	if (is_valid()) {
		return "Validation successful - no errors";
	}

	String summary = vformat("Schema validation failed with %d error(s):\n", (int64_t)errors.size());

	for (int64_t i = 0; i < errors.size(); i++) {
		const auto &error = errors[i];

		summary += vformat("  [%d] ", i + 1);

		if (!error.path.is_empty()) {
			summary += vformat("At '%s': ", error.path);
		}

		summary += error.message;

		if (!error.constraint.is_empty()) {
			summary += vformat(" (%s)", error.constraint);
		}

		if (i < errors.size() - 1) {
			summary += "\n";
		}
	}

	return summary;
}

String SchemaValidationResult::get_brief_summary() const {
	if (is_valid()) {
		return "Validation successful";
	}
	return vformat("Validation failed: %d error(s)", (int64_t)errors.size());
}

Array SchemaValidationResult::get_all_error_paths() const {
	Array paths;
	paths.resize(errors.size());
	for (size_t i = 0; i < errors.size(); i++) {
		paths[i] = errors[i].path;
	}
	return paths;
}

Array SchemaValidationResult::get_violated_constraints() const {
	Array constraints;
	std::vector<String> unique_constraints;

	for (const auto &error : errors) {
		if (!error.constraint.is_empty()) {
			// Check if already added
			bool found = false;
			for (const String &existing : unique_constraints) {
				if (existing == error.constraint) {
					found = true;
					break;
				}
			}
			if (!found) {
				unique_constraints.push_back(error.constraint);
			}
		}
	}

	constraints.resize(unique_constraints.size());
	for (size_t i = 0; i < unique_constraints.size(); i++) {
		constraints[i] = unique_constraints[i];
	}

	return constraints;
}

String SchemaValidationResult::_to_string() const {
	return get_brief_summary();
}

void SchemaValidationResult::add_error(const YAMLValidationError &error) {
	errors.push_back(error);
	validation_succeeded = false;
}
