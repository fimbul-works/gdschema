#pragma once

#include "../validation_context.hpp"
#include "validation_rule.hpp"

#include <godot_cpp/variant/variant.hpp>

namespace godot {

class ValidationContext;

class MultipleOfRule : public ValidationRule {
private:
	double multiple_of;

public:
	explicit MultipleOfRule(double multiplier) :
			multiple_of(multiplier) {}

	bool validate(const Variant &target, ValidationContext &context) const override;

	String get_rule_type() const override { return "multipleOf"; }
	String get_description() const override {
		return vformat("multipleOf(%f)", multiple_of);
	}
};

} //namespace godot
