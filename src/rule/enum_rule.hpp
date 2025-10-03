#pragma once

#include "validation_rule.hpp"

#include <godot_cpp/variant/variant.hpp>

namespace godot {

// Forward declarations
class ValidationContext;

/**
 * @class EnumRule
 * @brief Validates that value is one of allowed enum values
 */
class EnumRule : public ValidationRule {
private:
	Array allowed_values;

public:
	explicit EnumRule(const Array &values) : allowed_values(values) {}

	bool validate(const Variant &target, ValidationContext &context) const override;
	String get_rule_type() const override { return "enum"; }
	String get_description() const override;
};

} // namespace godot
