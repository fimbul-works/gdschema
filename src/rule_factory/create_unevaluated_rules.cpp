#include "../rule/false_rule.hpp"
#include "../rule/selector_rule.hpp"
#include "../selector/unevaluated_items_selector.hpp"
#include "../selector/unevaluated_properties_selector.hpp"
#include "rule_factory.hpp"

using namespace godot;

void RuleFactory::create_unevaluated_rules(const Dictionary &schema_def, const Ref<Schema> &schema, RuleCompileResult &result) {
	// unevaluatedProperties - JSON Schema 2020-12
	if (schema_def.has("unevaluatedProperties")) {
		Variant uneval_props_var = schema_def["unevaluatedProperties"];
		if (uneval_props_var.get_type() == Variant::BOOL && !uneval_props_var.operator bool()) {
			// unevaluatedProperties: false
			auto selector = std::make_unique<UnevaluatedPropertiesSelector>();
			auto rule = std::make_shared<FalseRule>();
			result.rules->add_rule(std::make_unique<SelectorRule>(std::move(selector), std::move(rule)));
		} else if (uneval_props_var.get_type() == Variant::DICTIONARY) {
			// unevaluatedProperties: {...}
			Ref<Schema> child_schema = schema->get_child("unevaluatedProperties");
			if (child_schema.is_valid()) {
				auto uneval_result = create_rules(child_schema);
				result.errors.insert(result.errors.end(), uneval_result.errors.begin(), uneval_result.errors.end());

				if (uneval_result.is_valid() && !uneval_result.rules->is_empty()) {
					auto selector = std::make_unique<UnevaluatedPropertiesSelector>();
					result.rules->add_rule(std::make_unique<SelectorRule>(std::move(selector), std::move(uneval_result.rules)));
				}
			}
		}
	}

	// unevaluatedItems - JSON Schema 2020-12
	if (schema_def.has("unevaluatedItems")) {
		Variant uneval_items_var = schema_def["unevaluatedItems"];
		if (uneval_items_var.get_type() == Variant::BOOL && !uneval_items_var.operator bool()) {
			// unevaluatedItems: false
			auto selector = std::make_unique<UnevaluatedItemsSelector>();
			auto rule = std::make_shared<FalseRule>();
			result.rules->add_rule(std::make_unique<SelectorRule>(std::move(selector), std::move(rule)));
		} else if (uneval_items_var.get_type() == Variant::DICTIONARY) {
			// unevaluatedItems: {...}
			Ref<Schema> child_schema = schema->get_child("unevaluatedItems");
			if (child_schema.is_valid()) {
				auto uneval_result = create_rules(child_schema);
				result.errors.insert(result.errors.end(), uneval_result.errors.begin(), uneval_result.errors.end());

				if (uneval_result.is_valid() && !uneval_result.rules->is_empty()) {
					auto selector = std::make_unique<UnevaluatedItemsSelector>();
					result.rules->add_rule(std::make_unique<SelectorRule>(std::move(selector), std::move(uneval_result.rules)));
				}
			}
		}
	}
}
