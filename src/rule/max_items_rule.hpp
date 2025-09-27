
#pragma once

#include "../validation_context.hpp"
#include "validation_rule.hpp"

#include <godot_cpp/variant/variant.hpp>

namespace godot {

class ValidationContext;

class MaxItemsRule : public ValidationRule {
private:
	int64_t max_items;

public:
	explicit MaxItemsRule(int64_t max) :
			max_items(max) {}

	bool validate(const Variant &target, ValidationContext &context) const override;

	String get_rule_type() const override { return "maxItems"; }
	String get_description() const override {
		return vformat("maxItems(%d)", max_items);
	}

private:
	int64_t get_array_size(const Variant &value) const;
};

} //namespace godot
