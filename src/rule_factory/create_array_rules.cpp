#include "../rule/contains_rule.hpp"
#include "../rule/false_rule.hpp"
#include "../rule/max_items_rule.hpp"
#include "../rule/min_items_rule.hpp"
#include "../rule/selector_rule.hpp"
#include "../rule/true_rule.hpp"
#include "../rule/unique_items_rule.hpp"
#include "../schema.hpp"
#include "../selector/additional_items_selector.hpp"
#include "../selector/array_item_selector.hpp"
#include "../selector/array_items_selector.hpp"
#include "../selector/prefix_items_selector.hpp"
#include "../selector/unevaluated_items_selector.hpp"
#include "../selector/value_selector.hpp"
#include "../util.hpp"
#include "rule_factory.hpp"

using namespace godot;

void RuleFactory::create_array_rules(const Dictionary &schema_def, const Ref<Schema> &schema, RuleCompileResult &result) {
	// minItems
	if (schema_def.has("minItems")) {
		Variant min_items_var = schema_def["minItems"];
		int64_t min_items;
		if (SchemaUtil::try_get_non_negative_int(min_items_var, min_items)) {
			auto selector = std::make_unique<ValueSelector>();
			auto rule = std::make_unique<MinItemsRule>(min_items);
			result.rules->add_rule(std::make_unique<SelectorRule>(std::move(selector), std::move(rule)));
		}
	}

	// maxItems
	if (schema_def.has("maxItems")) {
		Variant max_items_var = schema_def["maxItems"];
		int64_t max_items;
		if (SchemaUtil::try_get_non_negative_int(max_items_var, max_items)) {
			auto selector = std::make_unique<ValueSelector>();
			auto rule = std::make_unique<MaxItemsRule>(max_items);
			result.rules->add_rule(std::make_unique<SelectorRule>(std::move(selector), std::move(rule)));
		}
	}

	// uniqueItems
	if (schema_def.has("uniqueItems")) {
		Variant unique_items_var = schema_def["uniqueItems"];
		if (unique_items_var.get_type() == Variant::BOOL && unique_items_var.operator bool()) {
			auto selector = std::make_unique<ValueSelector>();
			auto rule = std::make_unique<UniqueItemsRule>();
			result.rules->add_rule(std::make_unique<SelectorRule>(std::move(selector), std::move(rule)));
		}
	}

	// Track prefix/tuple length for subsequent validation
	int64_t prefix_length = -1;
	bool has_prefix_items = schema_def.has("prefixItems");

	// prefixItems - JSON Schema 2020-12
	if (has_prefix_items) {
		Variant prefix_items_var = schema_def["prefixItems"];
		if (prefix_items_var.get_type() == Variant::ARRAY) {
			Array prefix_items_array = prefix_items_var.operator Array();
			prefix_length = prefix_items_array.size();

			for (int64_t i = 0; i < prefix_items_array.size(); i++) {
				StringName child_key = vformat("prefixItems/%d", i);
				Ref<Schema> child_schema = schema->get_child(child_key);

				if (child_schema.is_valid()) {
					auto item_result = create_rules(child_schema);
					result.errors.insert(result.errors.end(), item_result.errors.begin(), item_result.errors.end());

					if (item_result.is_valid()) {
						auto selector = std::make_unique<PrefixItemsSelector>(i);
						result.rules->add_rule(std::make_unique<SelectorRule>(std::move(selector), std::move(item_result.rules)));
					}
				}
			}
		}
	}

	// items handling - behavior depends on whether we're in Draft-07 or 2020-12 mode
	if (schema_def.has("items")) {
		Variant items_var = schema_def["items"];

		if (has_prefix_items) {
			// JSON Schema 2020-12: items applies to elements AFTER prefixItems
			if (items_var.get_type() == Variant::BOOL) {
				if (!items_var.operator bool()) {
					// items: false - no additional items allowed beyond prefixItems
					auto selector = std::make_unique<AdditionalItemsSelector>(prefix_length);
					auto rule = std::make_shared<FalseRule>();
					result.rules->add_rule(std::make_unique<SelectorRule>(std::move(selector), std::move(rule)));
				}
				// items: true - additional items allowed (default, no rule needed)
			} else if (items_var.get_type() == Variant::DICTIONARY) {
				// items: {...} - schema for items after prefixItems
				Ref<Schema> child_schema = schema->get_child("items");

				if (child_schema.is_valid()) {
					auto items_result = create_rules(child_schema);
					result.errors.insert(result.errors.end(), items_result.errors.begin(), items_result.errors.end());

					if (items_result.is_valid()) {
						auto selector = std::make_unique<AdditionalItemsSelector>(prefix_length);
						result.rules->add_rule(std::make_unique<SelectorRule>(std::move(selector), std::move(items_result.rules)));
					}
				}
			}
		} else {
			// JSON Schema Draft-07: items can be schema (all items) or array (tuple)
			if (items_var.get_type() == Variant::DICTIONARY) {
				// Single Schema applies to all items
				Ref<Schema> child_schema = schema->get_child("items");

				if (child_schema.is_valid()) {
					auto items_result = create_rules(child_schema);
					result.errors.insert(result.errors.end(), items_result.errors.begin(), items_result.errors.end());

					if (items_result.is_valid()) {
						auto selector = std::make_unique<ArrayItemsSelector>();
						result.rules->add_rule(std::make_unique<SelectorRule>(std::move(selector), std::move(items_result.rules)));
					}
				}
			} else if (items_var.get_type() == Variant::ARRAY) {
				// Draft-07 tuple validation - each position has its own Schema
				Array items_array = items_var.operator Array();
				prefix_length = items_array.size(); // Store tuple length for additionalItems

				for (int64_t i = 0; i < items_array.size(); i++) {
					StringName child_key = vformat("items/%d", i);
					Ref<Schema> child_schema = schema->get_child(child_key);

					if (child_schema.is_valid()) {
						auto item_result = create_rules(child_schema);
						result.errors.insert(result.errors.end(), item_result.errors.begin(), item_result.errors.end());

						if (item_result.is_valid()) {
							// Create selector for this specific array position
							auto selector = std::make_unique<ArrayItemSelector>(i);
							result.rules->add_rule(std::make_unique<SelectorRule>(std::move(selector), std::move(item_result.rules)));
						}
					}
				}
			}
		}
	}

	// additionalItems - only applies in Draft-07 when items is an array (tuple validation)
	// In 2020-12, items keyword itself handles this
	if (!has_prefix_items && prefix_length >= 0 && schema_def.has("additionalItems")) {
		Variant additional_items_var = schema_def["additionalItems"];

		if (additional_items_var.get_type() == Variant::BOOL && !additional_items_var.operator bool()) {
			// additionalItems: false - no additional items allowed beyond tuple
			auto selector = std::make_unique<AdditionalItemsSelector>(prefix_length);
			auto rule = std::make_shared<FalseRule>();
			result.rules->add_rule(std::make_unique<SelectorRule>(std::move(selector), std::move(rule)));
		} else if (additional_items_var.get_type() == Variant::DICTIONARY) {
			// additionalItems: {...} - additional items must match this Schema
			Ref<Schema> child_schema = schema->get_child("additionalItems");
			if (child_schema.is_valid()) {
				auto additional_result = create_rules(child_schema);
				result.errors.insert(result.errors.end(), additional_result.errors.begin(), additional_result.errors.end());

				if (additional_result.is_valid()) {
					auto selector = std::make_unique<AdditionalItemsSelector>(prefix_length);
					result.rules->add_rule(std::make_unique<SelectorRule>(std::move(selector), std::move(additional_result.rules)));
				}
			}
		}
		// Note: additionalItems: true (default) means additional items are allowed with no constraints
	}

	// contains - at least one array item must validate against the Schema
	if (schema_def.has("contains")) {
		Ref<Schema> child_schema = schema->get_child("contains");
		if (child_schema.is_valid()) {
			Dictionary child_def = child_schema->get_schema_definition();

			// Parse minContains/maxContains (Draft 2020-12)
			int64_t min_contains = 1;
			if (schema_def.has("minContains")) {
				SchemaUtil::try_get_non_negative_int(schema_def["minContains"], min_contains);
			}
			int64_t max_contains = -1; // -1 means no limit
			if (schema_def.has("maxContains")) {
				SchemaUtil::try_get_non_negative_int(schema_def["maxContains"], max_contains);
			}

			// Check if this is boolean Schema detection
			if (child_def.is_empty()) {
				// contains: true (empty Schema) - always matches any item
				auto selector = std::make_unique<ValueSelector>();
				auto rule = std::make_shared<TrueRule>();
				auto contains_rule = std::make_unique<ContainsRule>(rule, min_contains, max_contains);
				result.rules->add_rule(std::make_unique<SelectorRule>(std::move(selector), std::move(contains_rule)));
			} else if (child_def.size() == 1 && child_def.has("not") &&
					child_def["not"].get_type() == Variant::DICTIONARY &&
					child_def["not"].operator Dictionary().is_empty()) {
				// contains: false pattern {"not": {}} - never matches any item
				auto selector = std::make_unique<ValueSelector>();
				auto rule = std::make_shared<FalseRule>();
				auto contains_rule = std::make_unique<ContainsRule>(rule, min_contains, max_contains);
				result.rules->add_rule(std::make_unique<SelectorRule>(std::move(selector), std::move(contains_rule)));
			} else {
				// Normal Schema - compile recursively
				auto contains_result = create_rules(child_schema);
				result.errors.insert(result.errors.end(), contains_result.errors.begin(), contains_result.errors.end());

				if (contains_result.is_valid()) {
					auto selector = std::make_unique<ValueSelector>();
					auto rule = std::make_unique<ContainsRule>(contains_result.rules, min_contains, max_contains);
					result.rules->add_rule(std::make_unique<SelectorRule>(std::move(selector), std::move(rule)));
				}
			}
		}
	}
}
