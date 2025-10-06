#pragma once

#include "../validation_context.hpp"
#include "validation_rule.hpp"

#include <godot_cpp/variant/variant.hpp>

namespace godot {

// Forward declarations
class ValidationContext;

class MaxPropertiesRule : public ValidationRule {
private:
	int64_t max_properties;

public:
	explicit MaxPropertiesRule(int64_t max) :
			max_properties(max) {}

	bool validate(const Variant &target, ValidationContext &context) const override;
	String get_rule_type() const override { return "maxProperties"; }
	String get_description() const override { return vformat("maxProperties(%d)", max_properties); }
};

} // namespace godot
