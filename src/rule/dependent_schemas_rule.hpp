#pragma once

#include "validation_rule.hpp"

#include <godot_cpp/variant/string_name.hpp>
#include <memory>
#include <unordered_map>

namespace godot {

/**
 * @class DependentSchemasRule
 * @brief Implements Draft 2020-12 'dependentSchemas' keyword.
 * If a property is present, then the instance must validate against the corresponding schema.
 */
class DependentSchemasRule : public ValidationRule {
private:
	std::unordered_map<StringName, std::shared_ptr<ValidationRule>> dependent_schemas;

public:
	void add_schema(const StringName &trigger, std::shared_ptr<ValidationRule> rule) {
		if (rule) {
			dependent_schemas[trigger] = rule;
		}
	}

	bool validate(const Variant &target, ValidationContext &context) const override;
	String get_rule_type() const override { return "dependentSchemas"; }
	String get_description() const override;
};

} // namespace godot
