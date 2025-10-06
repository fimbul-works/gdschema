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

	const std::vector<ValidationError> &context_errors = context.get_errors();
	for (const ValidationError &error : context_errors) {
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
		return errors[index].get_instance_path();
	}
	return "";
}

PackedStringArray SchemaValidationResult::get_error_path_array(int index) const {
	if (index >= 0 && index < static_cast<int>(errors.size())) {
		return errors[index].instance_path_parts;
	}
	return Array();
}

String SchemaValidationResult::get_error_constraint(int index) const {
	if (index >= 0 && index < static_cast<int>(errors.size())) {
		return errors[index].keyword;
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
		return "Validation successful";
	}

	String summary = vformat("Schema validation failed with %d error(s):\n", (int64_t)errors.size());

	for (int64_t i = 0; i < errors.size(); i++) {
		const auto &error = errors[i];

		summary += vformat("  [%d] ", i + 1);

		String err_path = error.get_instance_path();
		if (!err_path.is_empty()) {
			summary += vformat("At '%s': ", err_path);
		}

		summary += error.message;

		if (!error.keyword.is_empty()) {
			summary += vformat(" (%s)", error.keyword);
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

PackedStringArray SchemaValidationResult::get_all_error_paths() const {
	PackedStringArray paths;
	paths.resize(errors.size());
	for (size_t i = 0; i < errors.size(); i++) {
		paths[i] = errors[i].get_instance_path();
	}
	return paths;
}

PackedStringArray SchemaValidationResult::get_violated_constraints() const {
	PackedStringArray constraints;
	std::vector<String> unique_constraints;

	for (const auto &error : errors) {
		if (!error.keyword.is_empty()) {
			// Check if already added
			bool found = false;
			for (const String &existing : unique_constraints) {
				if (existing == error.keyword) {
					found = true;
					break;
				}
			}
			if (!found) {
				unique_constraints.push_back(error.keyword);
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

void SchemaValidationResult::add_error(const ValidationError &error) {
	errors.push_back(error);
	validation_succeeded = false;
}
