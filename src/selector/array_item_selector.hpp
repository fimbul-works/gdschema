#pragma once

#include "selector.hpp"

#include <godot_cpp/variant/variant.hpp>

#include <vector>

namespace godot {

/**
 * @class ArrayItemSelector
 * @brief Selects a specific array item by index
 */
class ArrayItemSelector : public Selector {
private:
	int64_t index;

public:
	explicit ArrayItemSelector(int idx) : index(idx) {}

	std::vector<SelectionTarget> select_targets(
			const Variant &instance, const ValidationContext &context) const override;

	String get_description() const override { return vformat("array[%d]", index); }
};

} // namespace godot
