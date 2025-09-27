#pragma once

#include "../selector/selector.hpp"
#include "validation_rule.hpp"

#include <godot_cpp/variant/variant.hpp>

#include <memory>
#include <vector>

namespace godot {

// Forward declarations
class ValidationContext;

/**
 * @class SelectorRule
 * @brief Combines a selector with a rule - applies the rule to all selected targets
 */
class SelectorRule : public ValidationRule {
private:
	std::unique_ptr<Selector> selector;
	std::shared_ptr<ValidationRule> rule;

public:
	SelectorRule(std::unique_ptr<Selector> sel, std::shared_ptr<ValidationRule> r);

	bool validate(const Variant &target, ValidationContext &context) const override;
	String get_rule_type() const override { return "selector"; }
	String get_description() const override;
};

} // namespace godot
