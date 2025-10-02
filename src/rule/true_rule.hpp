#pragma once

#include "validation_rule.hpp"
#include <godot_cpp/variant/variant.hpp>

namespace godot {

/**
 * @class TrueRule
 * @brief Always validates successfully (represents schema: true)
 */
class TrueRule : public ValidationRule {
public:
	bool validate(const Variant &target, ValidationContext &context) const override;

	String get_rule_type() const override { return "true"; }
	String get_description() const override { return "true"; }
	bool is_empty() const override { return true; }
};

} //namespace godot
