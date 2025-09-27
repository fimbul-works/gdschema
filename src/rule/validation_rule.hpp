#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/variant.hpp>

namespace godot {

// Forward declarations
class ValidationContext;

/**
 * @class ValidationRule
 * @brief Base interface for all validation rules
 */
class ValidationRule {
public:
	virtual ~ValidationRule() = default;

	/**
	 * @brief Validates a target value
	 * @param target The value to validate
	 * @param context Validation context for error tracking
	 * @return True if validation passes
	 */
	virtual bool validate(const Variant &target, ValidationContext &context) const = 0;

	/**
	 * @brief Gets the rule type name for debugging
	 * @return Rule type identifier
	 */
	virtual String get_rule_type() const = 0;

	/**
	 * @brief Gets a description of this rule for debugging
	 * @return Description string
	 */
	virtual String get_description() const = 0;
};

} // namespace godot
