#pragma once

#include "selector.hpp"

#include <godot_cpp/classes/reg_ex.hpp>
#include <godot_cpp/classes/reg_ex_match.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/string_name.hpp>
#include <vector>

namespace godot {

/**
 * @class AdditionalPropertiesSelector
 * @brief Selects object properties that are NOT covered by schema properties or pattern properties
 */
class AdditionalPropertiesSelector : public Selector {
private:
	std::vector<StringName> defined_properties;
	std::vector<Ref<RegEx>> pattern_regexes;

public:
	AdditionalPropertiesSelector(const std::vector<StringName> &defined_props, const std::vector<String> &pattern_props) :
			defined_properties(defined_props) {
		for (const String &pattern : pattern_props) {
			Ref<RegEx> regex = RegEx::create_from_string(pattern);
			if (regex.is_valid()) {
				pattern_regexes.push_back(regex);
			}
		}
	}

	std::vector<SelectionTarget> select_targets(const Variant &instance, const ValidationContext &context) const override;

	String get_description() const override { return "additionalProperties"; }
};

} // namespace godot
