#pragma once

#include "validation_rule.hpp"

#include <godot_cpp/variant/variant.hpp>

namespace godot {

// Forward declarations
class ValidationContext;

/**
 * @class MaximumRule
 * @brief Validates numeric maximum value (inclusive)
 */
class MaximumRule : public ValidationRule {
private:
	double maximum;

public:
	explicit MaximumRule(double max) :
			maximum(max) {}

	bool validate(const Variant &target, ValidationContext &context) const override;
	String get_rule_type() const override { return "maximum"; }
	String get_description() const override { return vformat("maximum(%f)", maximum); }
};

} // namespace godot
