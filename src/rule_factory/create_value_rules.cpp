#include "../rule/const_rule.hpp"
#include "../rule/enum_rule.hpp"
#include "../rule/selector_rule.hpp"
#include "../schema.hpp"
#include "../selector/value_selector.hpp"
#include "../util.hpp"
#include "rule_factory.hpp"

using namespace godot;

void RuleFactory::create_value_rules(const Dictionary &schema_def, RuleCompileResult &result) {
	// const
	if (schema_def.has("const")) {
		Variant const_value = schema_def["const"];
		auto selector = std::make_unique<ValueSelector>();
		auto rule = std::make_unique<ConstRule>(const_value);
		result.rules->add_rule(std::make_unique<SelectorRule>(std::move(selector), std::move(rule)));
	}

	// enum
	if (schema_def.has("enum")) {
		Variant enum_var = schema_def["enum"];
		if (enum_var.get_type() == Variant::ARRAY) {
			Array enum_values = enum_var.operator Array();
			auto selector = std::make_unique<ValueSelector>();
			auto rule = std::make_unique<EnumRule>(enum_values);
			result.rules->add_rule(std::make_unique<SelectorRule>(std::move(selector), std::move(rule)));
		}
	}

	// default
	if (schema_def.has("default")) {
		// This is documentation/metadata, not a validation constraint
		// Default value is stored in Schema definition, not as validation rule
	}
}
