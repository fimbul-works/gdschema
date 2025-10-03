#pragma once

#include "../validation_context.hpp"
#include "validation_rule.hpp"

#include <godot_cpp/variant/variant.hpp>

namespace godot {

class ValidationContext;

class UniqueItemsRule : public ValidationRule {
private:
	bool should_hash_variant(const Variant &value) const;

public:
	bool validate(const Variant &target, ValidationContext &context) const override;

	String get_rule_type() const override { return "uniqueItems"; }
	String get_description() const override { return "uniqueItems"; }
};

} //namespace godot
