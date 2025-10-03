#pragma once

#include "validation_rule.hpp"

#include <godot_cpp/variant/variant.hpp>

namespace godot {

// Forward declarations
class ValidationContext;

/**
 * @class MinLengthRule
 * @brief Validates minimum string length
 */
class MinLengthRule : public ValidationRule {
private:
	int64_t min_length;

public:
	explicit MinLengthRule(int64_t min) : min_length(min) {}

	bool validate(const Variant &target, ValidationContext &context) const override;
	String get_rule_type() const override { return "minLength"; }
	String get_description() const override { return vformat("minLength(%d)", min_length); }
};

} // namespace godot
