#pragma once

#include "validation_rule.hpp"
#include <memory>

namespace godot {

class Schema;

/**
 * @class DynamicScopeRule
 * @brief Wraps rules and pushes the associated schema to the dynamic scope during validation.
 */
class DynamicScopeRule : public ValidationRule {
private:
	const Schema *schema;
	std::shared_ptr<ValidationRule> wrapped_rule;

public:
	DynamicScopeRule(const Schema *p_schema, std::shared_ptr<ValidationRule> p_rule) :
			schema(p_schema), wrapped_rule(p_rule) {}

	bool validate(const Variant &target, ValidationContext &context) const override;
	String get_rule_type() const override { return "dynamicScope"; }
	String get_description() const override;
};

} // namespace godot
