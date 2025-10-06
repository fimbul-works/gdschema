#pragma once

#include "../validation_context.hpp"
#include "validation_rule.hpp"

#include <godot_cpp/variant/variant.hpp>

#include <memory>

namespace godot {

// Forward declarations
class ValidationContext;

/**
 * @class ContainsRule
 * @brief Validates that at least one array item matches the given schema
 */
class ContainsRule : public ValidationRule {
private:
	std::shared_ptr<ValidationRule> item_rule;

public:
	explicit ContainsRule(std::shared_ptr<ValidationRule> rule) :
			item_rule(rule) {}

	bool validate(const Variant &target, ValidationContext &context) const override;
	String get_rule_type() const override { return "contains"; }
	String get_description() const override;
};

} // namespace godot
