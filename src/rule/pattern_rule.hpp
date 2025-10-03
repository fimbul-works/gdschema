#pragma once

#include "validation_rule.hpp"

#include <godot_cpp/classes/reg_ex.hpp>
#include <godot_cpp/variant/variant.hpp>

namespace godot {

// Forward declarations
class ValidationContext;

/**
 * @class PatternRule
 * @brief Validates string against regex pattern
 */
class PatternRule : public ValidationRule {
private:
	String pattern_string;
	Ref<RegEx> pattern_regex;

public:
	explicit PatternRule(const String &pattern);

	bool validate(const Variant &target, ValidationContext &context) const override;
	String get_rule_type() const override { return "pattern"; }
	String get_description() const override { return vformat("pattern(/%s/)", pattern_string); }
};

} // namespace godot
