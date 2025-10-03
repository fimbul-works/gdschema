#pragma once

#include "../validation_context.hpp"
#include "validation_rule.hpp"

#include <godot_cpp/variant/variant.hpp>

namespace godot {

// Forward declarations
class ValidationContext;

class MinPropertiesRule : public ValidationRule {
private:
	int64_t min_properties;

public:
	explicit MinPropertiesRule(int64_t min) : min_properties(min) {}

	bool validate(const Variant &target, ValidationContext &context) const override;
	String get_rule_type() const override { return "minProperties"; }
	String get_description() const override { return vformat("minProperties(%d)", min_properties); }
};

} // namespace godot
