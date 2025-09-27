#pragma once

#include "../validation_context.hpp"
#include "validation_rule.hpp"

#include <godot_cpp/variant/variant.hpp>

namespace godot {

class ValidationContext;

class ExclusiveMaximumRule : public ValidationRule {
private:
	double exclusive_maximum;

public:
	explicit ExclusiveMaximumRule(double max) :
			exclusive_maximum(max) {}

	bool validate(const Variant &target, ValidationContext &context) const override;
	String get_rule_type() const override { return "exclusiveMaximum"; }
	String get_description() const override {
		return vformat("exclusiveMaximum(%d)", exclusive_maximum);
	}
};

} //namespace godot
