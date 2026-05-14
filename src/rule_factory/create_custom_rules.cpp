#include "../schema.hpp"
#include "../util.hpp"
#include "rule_factory.hpp"

using namespace godot;

void RuleFactory::create_custom_rules(const Dictionary &schema_def, const Ref<Schema> &schema, RuleCompileResult &result) {
	for (const auto &[keyword, factory] : custom_rule_factories) {
		if (schema_def.has(keyword)) {
			factory(schema_def, schema, result);
		}
	}
}
