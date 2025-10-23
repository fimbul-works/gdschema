#include "prefix_items_selector.hpp"
#include "../util.hpp"

using namespace godot;

std::vector<SelectionTarget> PrefixItemsSelector::select_targets(const Variant &instance, const ValidationContext &context) const {
	std::vector<SelectionTarget> targets;

	int64_t size = SchemaUtil::get_array_size(instance);
	if (size == -1 || index < 0 || index >= size) {
		return targets;
	}

	targets.emplace_back(SchemaUtil::get_array_item(instance, index), String::num(index));

	return targets;
}
