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
	int64_t min_contains;
	int64_t max_contains;

public:
	explicit ContainsRule(std::shared_ptr<ValidationRule> rule, int64_t min = 1, int64_t max = -1) :
			item_rule(rule), min_contains(min), max_contains(max) {}

	bool validate(const Variant &target, ValidationContext &context) const override;
	String get_rule_type() const override { return "contains"; }
	String get_description() const override;
};

} // namespace godot
