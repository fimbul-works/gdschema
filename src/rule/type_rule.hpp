#pragma once

#include "validation_rule.hpp"

#include <godot_cpp/classes/reg_ex.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/variant.hpp>

#include <vector>

namespace godot {

// Forward declarations
class ValidationContext;

/**
 * @class TypeRule
 * @brief Validates that a value matches expected JSON Schema types
 */
class TypeRule : public ValidationRule {
private:
	std::vector<String> allowed_types;

	bool is_type_compatible(const Variant &instance, const String &schema_type) const;
	String get_variant_json_type(const Variant &instance) const;

public:
	explicit TypeRule(const String &type);
	explicit TypeRule(const std::vector<String> &types);

	bool validate(const Variant &target, ValidationContext &context) const override;
	String get_rule_type() const override { return "type"; }
	String get_description() const override;
};

} // namespace godot
