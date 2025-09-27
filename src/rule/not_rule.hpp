#pragma once

#include "validation_rule.hpp"

#include <godot_cpp/variant/variant.hpp>

#include <memory>

namespace godot {

// Forward declarations
class ValidationContext;

/**
 * @class NotRule
 * @brief Validates that the sub-rule does NOT pass
 */
class NotRule : public ValidationRule {
private:
	std::shared_ptr<ValidationRule> sub_rule;

public:
	explicit NotRule(std::shared_ptr<ValidationRule> rule);

	bool validate(const Variant &target, ValidationContext &context) const override;
	String get_rule_type() const override { return "not"; }
	String get_description() const override;
};

} // namespace godot
