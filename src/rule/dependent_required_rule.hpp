#pragma once

#include "validation_rule.hpp"

#include <godot_cpp/variant/string_name.hpp>
#include <unordered_map>
#include <vector>

namespace godot {

/**
 * @class DependentRequiredRule
 * @brief Implements Draft 2020-12 'dependentRequired' keyword.
 * If a property is present, then other properties must also be present.
 */
class DependentRequiredRule : public ValidationRule {
private:
	std::unordered_map<StringName, std::vector<StringName>> dependencies;

public:
	void add_dependency(const StringName &trigger, const std::vector<StringName> &required_props) {
		dependencies[trigger] = required_props;
	}

	bool validate(const Variant &target, ValidationContext &context) const override;
	String get_rule_type() const override { return "dependentRequired"; }
	String get_description() const override;
};

} // namespace godot
