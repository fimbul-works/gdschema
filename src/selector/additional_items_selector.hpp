#pragma once

#include "selector.hpp"

#include <godot_cpp/variant/variant.hpp>

#include <vector>

namespace godot {

/**
 * @class AdditionalItemsSelector
 * @brief Selects array items beyond the tuple validation length
 */
class AdditionalItemsSelector : public Selector {
private:
	int64_t tuple_length; // Number of items in the tuple schema

public:
	explicit AdditionalItemsSelector(int tuple_len) : tuple_length(tuple_len) {}

	std::vector<SelectionTarget> select_targets(
			const Variant &instance, const ValidationContext &context) const override;

	String get_description() const override { return vformat("array[%d:]", tuple_length); }
};

} // namespace godot
