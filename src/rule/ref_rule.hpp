#pragma once

#include "../schema.hpp"
#include "validation_rule.hpp"

#include <godot_cpp/variant/variant.hpp>
#include <memory>
#include <unordered_set>

namespace godot {

// Forward declarations
class ValidationContext;
class Schema;

/**
 * @class RefRule
 * @brief Validates by delegating to a referenced schema
 *
 * Handles JSON Schema $ref by resolving the reference and delegating
 * validation to the target schema's rules.
 */
class RefRule : public ValidationRule {
private:
	String reference_uri; // The $ref URI
	const Schema *source_schema; // Schema containing this $ref
	mutable Ref<Schema> cached_schema; // Cache the resolved schema
	mutable bool resolution_attempted;

public:
	/**
	 * @brief Constructor
	 * @param ref_uri The reference URI (e.g., "#", "#/properties/user", "external#/def")
	 * @param schema The schema containing this reference
	 */
	RefRule(const String &ref_uri, const Schema *schema);

	/**
	 * @brief Validates target by delegating to referenced schema
	 * @param target The value to validate
	 * @param context Validation context for error tracking
	 * @return True if validation passes
	 */
	bool validate(const Variant &target, ValidationContext &context) const override;

	String get_rule_type() const override { return "ref"; }

	String get_description() const override { return vformat("$ref(%s)", reference_uri); }
};

} // namespace godot
