#pragma once

#include "../validation_context.hpp"
#include "validation_rule.hpp"

#include <godot_cpp/variant/variant.hpp>
#include <memory>
#include <vector>

namespace godot {

class ValidationContext;

/**
 * @class DependencyRule
 * @brief Validates dependencies (property or schema) when a trigger property exists
 */
class DependencyRule : public ValidationRule {
private:
	StringName trigger_property;
	std::vector<String> required_properties; // For property dependencies
	std::shared_ptr<ValidationRule> dependency_schema; // For schema dependencies
	bool is_schema_dependency;

public:
	// Constructor for property dependencies
	DependencyRule(const StringName &trigger, const std::vector<String> &required_props) :
			trigger_property(trigger), required_properties(required_props), is_schema_dependency(false) {}

	// Constructor for schema dependencies
	DependencyRule(const StringName &trigger, std::shared_ptr<ValidationRule> schema) :
			trigger_property(trigger), dependency_schema(schema), is_schema_dependency(true) {}

	bool validate(const Variant &target, ValidationContext &context) const override;
	String get_rule_type() const override { return "dependency"; }
	String get_description() const override;
};

} // namespace godot
