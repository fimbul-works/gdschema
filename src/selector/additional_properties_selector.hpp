#pragma once

#include "selector.hpp"

#include <godot_cpp/variant/variant.hpp>

#include <vector>

namespace godot {

/**
 * @class AdditionalPropertiesSelector
 * @brief Selects object properties that are NOT covered by schema properties or pattern properties
 */
class AdditionalPropertiesSelector : public Selector {
private:
	std::vector<StringName> defined_properties;
	std::vector<String> pattern_properties;

public:
	AdditionalPropertiesSelector(
			const std::vector<StringName> &defined_props, const std::vector<String> &pattern_props) :
			defined_properties(defined_props), pattern_properties(pattern_props) {}

	std::vector<SelectionTarget> select_targets(
			const Variant &instance, const ValidationContext &context) const override;

	String get_description() const override { return "additionalProperties"; }
};

} // namespace godot
