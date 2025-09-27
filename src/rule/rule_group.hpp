#pragma once

#include "validation_rule.hpp"

#include <godot_cpp/variant/variant.hpp>

#include <memory>
#include <vector>

namespace godot {

// Forward declarations
class ValidationContext;

/**
 * @class RuleGroup
 * @brief Groups multiple rules that all must pass (AND logic)
 */
class RuleGroup : public ValidationRule {
private:
	std::vector<std::shared_ptr<ValidationRule>> rules;

public:
	void add_rule(std::shared_ptr<ValidationRule> rule);
	bool is_empty() const { return rules.empty(); }
	size_t rule_count() const { return rules.size(); }

	bool validate(const Variant &target, ValidationContext &context) const override;
	String get_rule_type() const override { return "group"; }
	String get_description() const override;
};

} // namespace godot
