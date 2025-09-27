#pragma once

#include "validation_rule.hpp"

#include <godot_cpp/variant/variant.hpp>

namespace godot {

// Forward declarations
class ValidationContext;

/**
 * @class MinimumRule
 * @brief Validates numeric minimum value (inclusive)
 */
class MinimumRule : public ValidationRule {
private:
	double minimum;

public:
	explicit MinimumRule(double min) :
			minimum(min) {}

	bool validate(const Variant &target, ValidationContext &context) const override;
	String get_rule_type() const override { return "minimum"; }
	String get_description() const override {
		return vformat("minimum(%d)", minimum);
	}
};

} // namespace godot
