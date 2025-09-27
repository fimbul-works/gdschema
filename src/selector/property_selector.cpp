#include "property_selector.hpp"

using namespace godot;

std::vector<SelectionTarget> PropertySelector::select_targets(
		const Variant &instance,
		const ValidationContext &context) const {
	std::vector<SelectionTarget> targets;

	if (instance.get_type() == Variant::DICTIONARY) {
		Dictionary dict = instance.operator Dictionary();
		if (dict.has(property_name)) {
			targets.emplace_back(dict[property_name], String(property_name));
		} else if (is_required) {
			// For required properties, we still need to select "nothing" to trigger validation
			// This allows RequiredPropertiesRule to detect missing properties
			targets.emplace_back(Variant(), String(property_name));
		}
	}

	return targets;
}
