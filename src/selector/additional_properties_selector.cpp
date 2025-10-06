#include "additional_properties_selector.hpp"

#include <godot_cpp/classes/reg_ex.hpp>
#include <godot_cpp/classes/reg_ex_match.hpp>

using namespace godot;

std::vector<SelectionTarget> AdditionalPropertiesSelector::select_targets(const Variant &instance, const ValidationContext &context) const {
	std::vector<SelectionTarget> targets;

	if (instance.get_type() != Variant::DICTIONARY) {
		return targets;
	}

	Dictionary dict = instance.operator Dictionary();
	Array keys = dict.keys();

	for (int i = 0; i < keys.size(); i++) {
		String key_str = String(keys[i]);
		StringName key_name = StringName(key_str);

		// Check if this property is covered by defined properties
		bool covered_by_properties = false;
		for (const StringName &defined_prop : defined_properties) {
			if (defined_prop == key_name) {
				covered_by_properties = true;
				break;
			}
		}

		if (covered_by_properties) {
			continue; // Skip this property
		}

		// Check if this property is covered by pattern properties
		bool covered_by_patterns = false;
		for (const String &pattern : pattern_properties) {
			Ref<RegEx> regex = RegEx::create_from_string(pattern);
			if (regex->is_valid()) {
				Ref<RegExMatch> match = regex->search(key_str);
				if (match.is_valid()) {
					covered_by_patterns = true;
					break;
				}
			}
		}

		if (!covered_by_patterns) {
			// This is an additional property
			targets.emplace_back(dict[keys[i]], key_str);
		}
	}

	return targets;
}
