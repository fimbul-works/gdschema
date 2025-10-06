#pragma once

#include "validation_rule.hpp"

#include <godot_cpp/classes/json.hpp>

namespace godot {
// Forward declarations
class ValidationContext;

class ContentMediaTypeRule : public ValidationRule {
private:
	String media_type;

public:
	ContentMediaTypeRule(const String &media_type) :
			media_type(media_type) {}

	bool validate(const Variant &target, ValidationContext &context) const override;
	String get_rule_type() const override { return "contentMediaType"; }
	String get_description() const override;

private:
	bool validate_json(const String &str, ValidationContext &context) const;
	bool validate_xml(const String &str, ValidationContext &context) const;
};

} //namespace godot
