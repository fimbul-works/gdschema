#pragma once

#include "validation_rule.hpp"

#include <godot_cpp/variant/variant.hpp>

namespace godot {

class Schema;

/**
 * @class DynamicRefRule
 * @brief Implements JSON Schema Draft 2020-12 '$dynamicRef'.
 * Resolves the reference at runtime based on the dynamic scope.
 */
class DynamicRefRule : public ValidationRule {
private:
	String reference_uri;
	const Schema *source_schema;

public:
	DynamicRefRule(const String &ref_uri, const Schema *schema) :
			reference_uri(ref_uri), source_schema(schema) {}

	bool validate(const Variant &target, ValidationContext &context) const override;
	String get_rule_type() const override { return "dynamicRef"; }
	String get_description() const override;
};

} // namespace godot
