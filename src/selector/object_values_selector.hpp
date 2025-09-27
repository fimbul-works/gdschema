#pragma once

#include "selector.hpp"

#include <godot_cpp/variant/variant.hpp>

#include <vector>

namespace godot {

/**
 * @class ObjectValuesSelector
 * @brief Selects all values in an object
 */
class ObjectValuesSelector : public Selector {
public:
	std::vector<SelectionTarget> select_targets(
			const Variant &instance,
			const ValidationContext &context) const override;

	String get_description() const override {
		return "object.values()";
	}
};

} // namespace godot
