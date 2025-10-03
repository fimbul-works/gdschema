#include "pattern_properties_selector.hpp"

#include <godot_cpp/classes/reg_ex_match.hpp>

using namespace godot;

PatternPropertiesSelector::PatternPropertiesSelector(const String &pattern) : pattern_string(pattern) {
	pattern_regex = RegEx::create_from_string(pattern);
}

std::vector<SelectionTarget> PatternPropertiesSelector::select_targets(
		const Variant &instance, const ValidationContext &context) const {
	std::vector<SelectionTarget> targets;

	if (!pattern_regex.is_valid()) {
		UtilityFunctions::push_warning("PatternPropertiesSelector: invalid regex pattern");
		return targets;
	}

	if (instance.get_type() == Variant::DICTIONARY) {
		Dictionary dict = instance.operator Dictionary();
		Array keys = dict.keys();

		for (int i = 0; i < keys.size(); i++) {
			String key_str = String(keys[i]);
			Ref<RegExMatch> match = pattern_regex->search(key_str);

			if (match.is_valid()) {
				targets.emplace_back(dict[keys[i]], key_str);
			}
		}
	}

	return targets;
}
