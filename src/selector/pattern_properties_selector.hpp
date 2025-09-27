#pragma once

#include "selector.hpp"

#include <godot_cpp/classes/reg_ex.hpp>
#include <godot_cpp/variant/variant.hpp>

#include <vector>

namespace godot {

/**
 * @class PatternPropertiesSelector
 * @brief Selects object properties that match a regex pattern
 */
class PatternPropertiesSelector : public Selector {
private:
	String pattern_string;
	Ref<RegEx> pattern_regex;

public:
	explicit PatternPropertiesSelector(const String &pattern);

	std::vector<SelectionTarget> select_targets(
			const Variant &instance,
			const ValidationContext &context) const override;

	String get_description() const override {
		return vformat("properties[/%s/]", pattern_string);
	}
};

} // namespace godot
