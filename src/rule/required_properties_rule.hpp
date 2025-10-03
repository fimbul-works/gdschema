#pragma once

#include "validation_rule.hpp"

#include <godot_cpp/variant/variant.hpp>

#include <vector>

namespace godot {

// Forward declarations
class ValidationContext;

/**
 * @class RequiredPropertiesRule
 * @brief Validates that object has all required properties
 */
class RequiredPropertiesRule : public ValidationRule {
private:
	std::vector<String> required_props;

public:
	explicit RequiredPropertiesRule(const std::vector<String> &props) : required_props(props) {}

	bool validate(const Variant &target, ValidationContext &context) const override;
	String get_rule_type() const override { return "required"; }
	String get_description() const override;
};

} // namespace godot
