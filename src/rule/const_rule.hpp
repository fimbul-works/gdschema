#pragma once

#include "validation_rule.hpp"

#include <godot_cpp/variant/variant.hpp>

namespace godot {

// Forward declarations
class ValidationContext;

/**
 * @class ConstRule
 * @brief Validates that value equals a specific constant
 */
class ConstRule : public ValidationRule {
private:
	Variant constant_value;

public:
	explicit ConstRule(const Variant &value) :
			constant_value(value) {}

	bool validate(const Variant &target, ValidationContext &context) const override;
	String get_rule_type() const override { return "const"; }
	String get_description() const override {
		return vformat("const(%s)", constant_value.stringify());
	}
};

} // namespace godot
