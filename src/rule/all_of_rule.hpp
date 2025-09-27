#pragma once

#include "validation_rule.hpp"

#include <godot_cpp/variant/variant.hpp>

#include <memory>
#include <vector>

namespace godot {

// Forward declarations
class ValidationContext;

/**
 * @class AllOfRule
 * @brief Validates that all sub-rules pass
 */
class AllOfRule : public ValidationRule {
private:
	std::vector<std::shared_ptr<ValidationRule>> sub_rules;

public:
	void add_sub_rule(std::shared_ptr<ValidationRule> rule);

	bool validate(const Variant &target, ValidationContext &context) const override;
	String get_rule_type() const override { return "allOf"; }
	String get_description() const override;
};

} // namespace godot
