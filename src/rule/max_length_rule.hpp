#pragma once

#include "validation_rule.hpp"

#include <godot_cpp/variant/variant.hpp>

namespace godot {

// Forward declarations
class ValidationContext;

/**
 * @class MaxLengthRule
 * @brief Validates maximum string length
 */
class MaxLengthRule : public ValidationRule {
private:
	int64_t max_length;

public:
	explicit MaxLengthRule(int64_t max) : max_length(max) {}

	bool validate(const Variant &target, ValidationContext &context) const override;
	String get_rule_type() const override { return "maxLength"; }
	String get_description() const override { return vformat("maxLength(%d)", max_length); }
};

} // namespace godot
