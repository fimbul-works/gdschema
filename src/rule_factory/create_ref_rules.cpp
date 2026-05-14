#include "../rule/dynamic_ref_rule.hpp"
#include "../rule/ref_rule.hpp"
#include "../rule/selector_rule.hpp"
#include "../schema.hpp"
#include "../selector/value_selector.hpp"
#include "../util.hpp"
#include "rule_factory.hpp"

using namespace godot;

void RuleFactory::create_ref_rules(const Ref<Schema> schema, const Dictionary &ref_def, RuleCompileResult &result) {
	// Handle $ref
	if (ref_def.has("$ref")) {
		Variant ref_var = ref_def["$ref"];
		if (SchemaUtil::is_string(ref_var)) {
			String ref_uri = ref_var.operator String();
			if (ref_uri.is_empty()) {
				result.add_error("$ref cannot be empty", "ref");
			} else {
				auto selector = std::make_unique<ValueSelector>();
				auto ref_rule = std::make_unique<RefRule>(ref_uri, schema.ptr());
				result.rules->add_rule(std::make_unique<SelectorRule>(std::move(selector), std::move(ref_rule)));
			}
		}
	}

	// Handle $dynamicRef (JSON Schema Draft 2020-12)
	if (ref_def.has("$dynamicRef")) {
		Variant dref_var = ref_def["$dynamicRef"];
		if (SchemaUtil::is_string(dref_var)) {
			String dref_uri = dref_var.operator String();
			if (!dref_uri.is_empty()) {
				auto selector = std::make_unique<ValueSelector>();
				auto dref_rule = std::make_unique<DynamicRefRule>(dref_uri, schema.ptr());
				result.rules->add_rule(std::make_unique<SelectorRule>(std::move(selector), std::move(dref_rule)));
			}
		}
	}
}
