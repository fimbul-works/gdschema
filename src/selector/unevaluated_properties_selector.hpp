#pragma once

#include "selector.hpp"

#include <godot_cpp/variant/dictionary.hpp>

namespace godot {

/**
 * @class UnevaluatedPropertiesSelector
 * @brief Selects all object properties that have not been evaluated by other keywords.
 * Used for JSON Schema Draft 2020-12 'unevaluatedProperties'.
 */
class UnevaluatedPropertiesSelector : public Selector {
public:
	std::vector<SelectionTarget> select_targets(const Variant &instance, const ValidationContext &context) const override {
		std::vector<SelectionTarget> targets;
		if (instance.get_type() != Variant::DICTIONARY) {
			return targets;
		}

		Dictionary dict = instance.operator Dictionary();
		Array keys = dict.keys();

		for (int i = 0; i < keys.size(); i++) {
			StringName key = keys[i];
			if (!context.is_property_evaluated(key)) {
				targets.emplace_back(dict[key], key);
			}
		}

		return targets;
	}

	String get_description() const override { return "unevaluatedProperties"; }
};

} // namespace godot
