#pragma once

#include "validation_rule.hpp"

#include <godot_cpp/variant/variant.hpp>

#include <memory>

namespace godot {

// Forward declarations
class ValidationContext;

/**
 * @class ConditionalRule
 * @brief Validates using if/then/else conditional logic
 */
class ConditionalRule : public ValidationRule {
private:
	std::shared_ptr<ValidationRule> if_rule;
	std::shared_ptr<ValidationRule> then_rule;
	std::shared_ptr<ValidationRule> else_rule;

public:
	ConditionalRule(
			std::shared_ptr<ValidationRule> if_schema,
			std::shared_ptr<ValidationRule> then_schema = nullptr,
			std::shared_ptr<ValidationRule> else_schema = nullptr) :
			if_rule(if_schema), then_rule(then_schema), else_rule(else_schema) {}

	bool validate(const Variant &target, ValidationContext &context) const override;
	String get_rule_type() const override { return "conditional"; }
	String get_description() const override;
};

} // namespace godot
