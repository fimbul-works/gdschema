#include "dependency_rule.hpp"

using namespace godot;

bool DependencyRule::validate(const Variant &target, ValidationContext &context) const {
	if (target.get_type() != Variant::DICTIONARY) {
		return true; // Dependencies only apply to objects
	}

	Dictionary dict = target.operator Dictionary();

	// Check if trigger property exists
	if (!dict.has(trigger_property)) {
		return true; // Dependency not triggered
	}

	if (is_schema_dependency) {
		// Schema dependency: validate entire object against dependency schema
		if (!dependency_schema) {
			context.add_error(vformat("Schema dependency for '%s' is not defined", trigger_property), "dependencies");
			return false;
		}

		ValidationContext dep_context = context.create_child_schema(vformat("dependencies/%s", trigger_property));

		bool result = dependency_schema->validate(target, dep_context);
		context.merge_errors(dep_context);
		return result;

	} else {
		// Property dependency: check required properties exist
		bool all_present = true;

		for (const String &required_prop : required_properties) {
			StringName prop_name = StringName(required_prop);
			if (!dict.has(prop_name)) {
				context.add_error(vformat("Property '%s' is required when '%s' is present", required_prop, trigger_property), "dependencies");
				all_present = false;
			}
		}

		return all_present;
	}
}

String DependencyRule::get_description() const {
	if (is_schema_dependency) {
		return vformat("dependency(%s -> schema)", trigger_property);
	} else {
		String deps;
		for (size_t i = 0; i < required_properties.size(); i++) {
			if (i > 0) {
				deps += ", ";
			}
			deps += required_properties[i];
		}
		return vformat("dependency(%s -> [%s])", trigger_property, deps);
	}
}
