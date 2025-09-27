#pragma once

#include "selector.hpp"

#include <godot_cpp/classes/reg_ex.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/variant.hpp>

#include <memory>

namespace godot {

/**
 * @class PropertySelector
 * @brief Selects a specific property from an object
 */
class PropertySelector : public Selector {
private:
	StringName property_name;
	bool is_required;

public:
	PropertySelector(const StringName &name, bool required = false) :
			property_name(name), is_required(required) {}

	std::vector<SelectionTarget> select_targets(
			const Variant &instance,
			const ValidationContext &context) const override;

	String get_description() const override {
		return vformat("property[%s%s]", property_name,
				is_required ? ", required" : "");
	}
};

} // namespace godot
