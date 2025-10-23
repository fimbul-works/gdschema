#pragma once

#include "selector.hpp"

#include <godot_cpp/variant/variant.hpp>

#include <vector>

namespace godot {

/**
 * @class PrefixItemsSelector
 * @brief Selects array items for tuple validation using prefixItems
 *
 * This selector is used for JSON Schema 2020-12's prefixItems keyword
 */
class PrefixItemsSelector : public Selector {
private:
	int64_t index;

public:
	explicit PrefixItemsSelector(int64_t idx) :
			index(idx) {}

	std::vector<SelectionTarget> select_targets(const Variant &instance, const ValidationContext &context) const override;

	String get_description() const override { return vformat("prefixItems[%d]", index); }
};

} // namespace godot
