#pragma once

#include "validation_rule.hpp"

namespace godot {
// Forward declarations
class ValidationContext;

class ContentEncodingRule : public ValidationRule {
private:
	String encoding;

public:
	ContentEncodingRule(const String &encoding) :
			encoding(encoding) {}

	bool validate(const Variant &target, ValidationContext &context) const override;
	String get_rule_type() const override { return "contentEncoding"; }
	String get_description() const override;

private:
	bool validate_base64(const String &str, ValidationContext &context) const;
	bool validate_base64url(const String &str, ValidationContext &context) const;
};

} //namespace godot
