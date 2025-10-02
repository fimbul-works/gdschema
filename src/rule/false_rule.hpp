#pragma once

#include "validation_rule.hpp"
#include <godot_cpp/variant/variant.hpp>

namespace godot {

/**
 * @class TrueRule
 * @brief Always validates successfully (represents schema: true)
 */
class FalseRule : public ValidationRule {
public:
	bool validate(const Variant &target, ValidationContext &context) const override;

	String get_rule_type() const override { return "false"; }
	String get_description() const override { return "false"; }
	bool is_empty() const override { return false; } // Never matches ≠ empty
};

} //namespace godot
