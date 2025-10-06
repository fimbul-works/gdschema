#pragma once

#include "../rule/validation_rule.hpp"
#include "../validation_context.hpp"

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/variant.hpp>

#include <memory>
#include <vector>

namespace godot {

// Forward declarations
class ValidationContext;

/**
 * @struct SelectionTarget
 * @brief Represents a selected value with its context path
 */
struct SelectionTarget {
	Variant value;
	String path_segment; // For building instance paths

	SelectionTarget(const Variant &val, const String &segment = "") :
			value(val),
			path_segment(segment) {}
};

/**
 * @class Selector
 * @brief Base interface for selecting what values a rule should apply to
 */
class Selector {
public:
	virtual ~Selector() = default;

	/**
	 * @brief Selects target values from the instance
	 * @param instance The value to select from
	 * @param context Current validation context
	 * @return Array of SelectionTarget structs
	 */
	virtual std::vector<SelectionTarget> select_targets(const Variant &instance, const ValidationContext &context) const = 0;

	/**
	 * @brief Gets a description of this selector for debugging
	 * @return Description string
	 */
	virtual String get_description() const = 0;
};

} // namespace godot
