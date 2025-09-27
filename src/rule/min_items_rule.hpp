#pragma once

#include "../validation_context.hpp"
#include "validation_rule.hpp"

#include <godot_cpp/variant/variant.hpp>

namespace godot {

// Forward declarations
class ValidationContext;

class MinItemsRule : public ValidationRule {
private:
	int64_t min_items;

public:
	explicit MinItemsRule(int64_t min) :
			min_items(min) {}

	bool validate(const Variant &target, ValidationContext &context) const override;

	String get_rule_type() const override { return "minItems"; }
	String get_description() const override {
		return vformat("minItems(%d)", min_items);
	}

private:
	int64_t get_array_size(const Variant &value) const;
};

} //namespace godot
