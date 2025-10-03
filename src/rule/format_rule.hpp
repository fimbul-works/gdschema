#pragma once

#include "validation_rule.hpp"

#include <godot_cpp/variant/variant.hpp>

namespace godot {

// Forward declarations
class ValidationContext;

/**
 * @class FormatRule
 * @brief Validates that value follows a format
 */
class FormatRule : public ValidationRule {
private:
	String format;

public:
	explicit FormatRule(const String &value) : format(value) {}

	bool validate(const Variant &target, ValidationContext &context) const override;
	String get_rule_type() const override { return "format"; }
	String get_description() const override;

private:
	bool range_check(const int &value, const int &min, const int &max) const {
		if (!((value >= min) && (value <= max))) {
			return false;
		}
		return true;
	}

	bool validate_regex(const String &regex_pattern, const String &str, ValidationContext &context) const;

	bool validate_email(const String &str, ValidationContext &context) const;

	bool validate_date(const String &str, ValidationContext &context) const;

	bool validate_time(const String &str, ValidationContext &context) const;

	bool validate_date_time(const String &str, ValidationContext &context) const;

	bool validate_ipv4(const String &str, ValidationContext &context) const;

	bool validate_ipv6(const String &str, ValidationContext &context) const;

	bool validate_ipv6_with_ipv4(const String &str, ValidationContext &context) const;

	bool validate_uri(const String &str, ValidationContext &context) const;

	bool validate_uri_reference(const String &str, ValidationContext &context) const;

	bool validate_json_pointer(const String &str, ValidationContext &context) const;

	bool validate_relative_json_pointer(const String &str, ValidationContext &context) const;

	bool validate_base64(const String &str, ValidationContext &context) const;

	bool validate_base64url(const String &str, ValidationContext &context) const;

	bool validate_regex_format(const String &str, ValidationContext &context) const;
};

} // namespace godot
