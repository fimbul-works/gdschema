#include "additional_items_selector.hpp"
#include "../util.hpp"

using namespace godot;

std::vector<SelectionTarget> AdditionalItemsSelector::select_targets(const Variant &instance, const ValidationContext &context) const {
	std::vector<SelectionTarget> targets;

	int64_t size = SchemaUtil::get_array_size(instance);
	if (size == -1) {
		return targets;
	}

	for (int64_t i = tuple_length; i < size; i++) {
		targets.emplace_back(SchemaUtil::get_array_item(instance, i), String::num(i));
	}

	return targets;
}
