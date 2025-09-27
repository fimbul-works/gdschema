#pragma once

#include "../rule/validation_rule.hpp"
#include "selector.hpp"

#include <godot_cpp/classes/reg_ex.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/variant.hpp>

#include <memory>
#include <vector>

namespace godot {

/**
 * @class ValueSelector
 * @brief Selects the value itself (most common case)
 */
class ValueSelector : public Selector {
public:
	std::vector<SelectionTarget> select_targets(
			const Variant &instance,
			const ValidationContext &context) const override {
		return { SelectionTarget(instance) };
	}

	String get_description() const override {
		return "value";
	}
};

} // namespace godot
