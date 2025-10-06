#pragma once

#include "selector.hpp"

#include <godot_cpp/variant/variant.hpp>

#include <vector>

namespace godot {

/**
 * @class ArrayItemsSelector
 * @brief Selects all items in an array
 */
class ArrayItemsSelector : public Selector {
public:
	std::vector<SelectionTarget> select_targets(const Variant &instance, const ValidationContext &context) const override;

	String get_description() const override { return "array[*]"; }
};

} // namespace godot
