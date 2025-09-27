#include "content_media_type_rule.hpp"
#include "../validation_context.hpp"

using namespace godot;

bool ContentMediaTypeRule::validate(const Variant &target, ValidationContext &context) const {
	// Content media type only applies to strings
	if (target.get_type() != Variant::STRING && target.get_type() != Variant::STRING_NAME) {
		return true;
	}

	const String str = target.operator String();

	if (media_type == "application/json") {
		return validate_json(str, context);
	} else if (media_type == "application/xml" || media_type == "text/xml") {
		return validate_xml(str, context);
	} else if (media_type.begins_with("text/")) {
		// Text media types are generally valid for any string
		return true;
	} else {
		// For other media types, we assume the content is valid
		// In a real implementation, you might want to add more specific validation
		return true;
	}
}

String ContentMediaTypeRule::get_description() const {
	return vformat("contentMediaType(%s)", media_type);
}

bool ContentMediaTypeRule::validate_json(const String &str, ValidationContext &context) const {
	if (str.is_empty()) {
		context.add_error("JSON content cannot be empty", "contentMediaType", str);
		return false;
	}

	Variant result = JSON::parse_string(str);
	if (result.get_type() == Variant::NIL) {
		context.add_error("Invalid JSON content", "contentMediaType", str);
		return false;
	}

	return true;
}

bool ContentMediaTypeRule::validate_xml(const String &str, ValidationContext &context) const {
	if (str.is_empty()) {
		context.add_error("XML content cannot be empty", "contentMediaType", str);
		return false;
	}

	// Basic XML validation - check for balanced tags
	// This is a simplified validation; a full XML parser would be better
	if (!str.strip_edges().begins_with("<")) {
		context.add_error("XML content must start with '<'", "contentMediaType", str);
		return false;
	}

	if (!str.strip_edges().ends_with(">")) {
		context.add_error("XML content must end with '>'", "contentMediaType", str);
		return false;
	}

	// Could add more sophisticated XML validation here
	return true;
}
