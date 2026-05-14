#include "../rule/exclusive_maximum_rule.hpp"
#include "../rule/exclusive_minimum_rule.hpp"
#include "../rule/maximum_rule.hpp"
#include "../rule/minimum_rule.hpp"
#include "../rule/multiple_of_rule.hpp"
#include "../rule/selector_rule.hpp"
#include "../schema.hpp"
#include "../selector/value_selector.hpp"
#include "../util.hpp"
#include "rule_factory.hpp"

using namespace godot;

void RuleFactory::create_numeric_rules(const Dictionary &schema_def, RuleCompileResult &result) {
	// minimum
	if (schema_def.has("minimum")) {
		Variant minimum_var = schema_def["minimum"];
		double minimum;
		if (SchemaUtil::try_get_numeric_value(minimum_var, minimum)) {
			auto selector = std::make_unique<ValueSelector>();
			auto rule = std::make_unique<MinimumRule>(minimum);
			result.rules->add_rule(std::make_unique<SelectorRule>(std::move(selector), std::move(rule)));
		}
	}

	// maximum
	if (schema_def.has("maximum")) {
		Variant maximum_var = schema_def["maximum"];
		double maximum;
		if (SchemaUtil::try_get_numeric_value(maximum_var, maximum)) {
			auto selector = std::make_unique<ValueSelector>();
			auto rule = std::make_unique<MaximumRule>(maximum);
			result.rules->add_rule(std::make_unique<SelectorRule>(std::move(selector), std::move(rule)));
		}
	}

	// exclusiveMinimum
	if (schema_def.has("exclusiveMinimum")) {
		Variant exclusive_minimum_var = schema_def["exclusiveMinimum"];
		double exclusive_minimum;
		if (SchemaUtil::try_get_numeric_value(exclusive_minimum_var, exclusive_minimum)) {
			auto selector = std::make_unique<ValueSelector>();
			auto rule = std::make_unique<ExclusiveMinimumRule>(exclusive_minimum);
			result.rules->add_rule(std::make_unique<SelectorRule>(std::move(selector), std::move(rule)));
		}
	}

	// exclusiveMaximum
	if (schema_def.has("exclusiveMaximum")) {
		Variant exclusive_maximum_var = schema_def["exclusiveMaximum"];
		double exclusive_maximum;
		if (SchemaUtil::try_get_numeric_value(exclusive_maximum_var, exclusive_maximum)) {
			auto selector = std::make_unique<ValueSelector>();
			auto rule = std::make_unique<ExclusiveMaximumRule>(exclusive_maximum);
			result.rules->add_rule(std::make_unique<SelectorRule>(std::move(selector), std::move(rule)));
		}
	}

	// multipleOf
	if (schema_def.has("multipleOf")) {
		Variant multiple_of_var = schema_def["multipleOf"];
		double multiple_of;
		if (SchemaUtil::try_get_numeric_value(multiple_of_var, multiple_of)) {
			auto selector = std::make_unique<ValueSelector>();
			auto rule = std::make_unique<MultipleOfRule>(multiple_of);
			result.rules->add_rule(std::make_unique<SelectorRule>(std::move(selector), std::move(rule)));
		}
	}
}
