#pragma once

#include "selector.hpp"

#include <godot_cpp/variant/variant.hpp>

#include <vector>

namespace godot {

/**
 * @class ObjectKeysSelector
 * @brief Selects all keys in an object (for propertyNames validation)
 */
class ObjectKeysSelector : public Selector {
public:
	std::vector<SelectionTarget> select_targets(const Variant &instance, const ValidationContext &context) const override;

	String get_description() const override { return "object.keys()"; }
};

} // namespace godot
