#pragma once

#include "../validation_context.hpp"
#include "validation_rule.hpp"

#include <godot_cpp/variant/variant.hpp>

namespace godot {

class ValidationContext;

class ExclusiveMinimumRule : public ValidationRule {
private:
	double exclusive_minimum;

public:
	explicit ExclusiveMinimumRule(double min) :
			exclusive_minimum(min) {}

	bool validate(const Variant &target, ValidationContext &context) const override;
	String get_rule_type() const override { return "exclusiveMinimum"; }
	String get_description() const override {
		return vformat("exclusiveMinimum(%d)", exclusive_minimum);
	}
};

} //namespace godot
