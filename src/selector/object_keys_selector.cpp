#include "object_keys_selector.hpp"

using namespace godot;

std::vector<SelectionTarget> ObjectKeysSelector::select_targets(
		const Variant &instance, const ValidationContext &context) const {
	std::vector<SelectionTarget> targets;

	if (instance.get_type() == Variant::DICTIONARY) {
		Dictionary dict = instance.operator Dictionary();
		Array keys = dict.keys();
		for (int i = 0; i < keys.size(); i++) {
			String key_str = String(keys[i]);
			targets.emplace_back(keys[i], "propertyName:" + key_str);
		}
	}

	return targets;
}
