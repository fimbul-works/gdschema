#include "type_rule.hpp"
#include "../util.hpp"
#include "../validation_context.hpp"

using namespace godot;

TypeRule::TypeRule(const String &type) {
	allowed_types.push_back(type);
}

TypeRule::TypeRule(const std::vector<String> &types) :
		allowed_types(types) {}

bool TypeRule::validate(const Variant &target, ValidationContext &context) const {
	String actual_type = SchemaUtil::get_variant_json_type(target);

	// Check if actual type matches any allowed type
	for (const String &allowed_type : allowed_types) {
		if (is_type_compatible(target, allowed_type)) {
			return true;
		}
	}

	// No match found - build error message
	String allowed_str;
	for (size_t i = 0; i < allowed_types.size(); i++) {
		if (i > 0) {
			allowed_str += ", ";
		}
		allowed_str += allowed_types[i];
	}

	context.add_error(vformat("Value has type %s but expected one of: %s", actual_type, allowed_str), "type", target);

	return false;
}

String TypeRule::get_description() const {
	if (allowed_types.size() == 1) {
		return vformat("type(%s)", allowed_types[0]);
	} else {
		String desc = "type(";
		for (size_t i = 0; i < allowed_types.size(); i++) {
			if (i > 0) {
				desc += ", ";
			}
			desc += allowed_types[i];
		}
		desc += ")";
		return desc;
	}
}

bool TypeRule::is_type_compatible(const Variant &instance, const String &schema_type) const {
	String actual_type = SchemaUtil::get_variant_json_type(instance);

	// Direct match
	if (actual_type == schema_type) {
		return true;
	}

	// Integer is compatible with number type
	if (schema_type == "number" && actual_type == "integer") {
		return true;
	}

	return false;
}
