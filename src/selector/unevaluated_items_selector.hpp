#pragma once

#include "../util.hpp"
#include "selector.hpp"

namespace godot {

/**
 * @class UnevaluatedItemsSelector
 * @brief Selects all array items that have not been evaluated by other keywords.
 * Used for JSON Schema Draft 2020-12 'unevaluatedItems'.
 */
class UnevaluatedItemsSelector : public Selector {
public:
	std::vector<SelectionTarget> select_targets(const Variant &instance, const ValidationContext &context) const override {
		std::vector<SelectionTarget> targets;
		int64_t size = SchemaUtil::get_array_size(instance);
		if (size < 0) {
			return targets;
		}

		for (int64_t i = 0; i < size; i++) {
			if (!context.is_item_evaluated(i)) {
				targets.emplace_back(SchemaUtil::get_array_item(instance, i), String::num_int64(i));
			}
		}

		return targets;
	}

	String get_description() const override { return "unevaluatedItems"; }
};

} // namespace godot
