#include "../rule/selector_rule.hpp"
#include "../rule/type_rule.hpp"
#include "../schema.hpp"
#include "../selector/value_selector.hpp"
#include "../util.hpp"
#include "rule_factory.hpp"

using namespace godot;

void RuleFactory::create_type_rules(const Variant &type_def, RuleCompileResult &result) {
	if (SchemaUtil::is_string(type_def)) {
		// Single type
		String type_str = type_def.operator String();
		auto selector = std::make_unique<ValueSelector>();
		auto rule = std::make_unique<TypeRule>(type_str);
		result.rules->add_rule(std::make_unique<SelectorRule>(std::move(selector), std::move(rule)));

	} else if (type_def.get_type() == Variant::ARRAY) {
		// Array of types
		Array type_array = type_def.operator Array();
		std::vector<String> types;

		for (int64_t i = 0; i < type_array.size(); i++) {
			if (SchemaUtil::is_string(type_array[i])) {
				types.push_back(type_array[i].operator String());
			} else {
				result.add_error(vformat("Type array element must be string, got %s", SchemaUtil::get_variant_json_type(type_array[i])), vformat("type/%d", i));
				return;
			}
		}

		if (!types.empty()) {
			auto selector = std::make_unique<ValueSelector>();
			auto rule = std::make_unique<TypeRule>(types);
			result.rules->add_rule(std::make_unique<SelectorRule>(std::move(selector), std::move(rule)));
		}
	}
	// Note: Meta-validation already ensures type is string or array, so no else case needed
}
