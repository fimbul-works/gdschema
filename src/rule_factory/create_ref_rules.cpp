#include "../rule/ref_rule.hpp"
#include "../rule/selector_rule.hpp"
#include "../schema.hpp"
#include "../selector/value_selector.hpp"
#include "../util.hpp"
#include "rule_factory.hpp"

using namespace godot;

void RuleFactory::create_ref_rules(const Ref<Schema> schema, const Dictionary &ref_def, RuleCompileResult &result) {
	Variant ref_var = ref_def["$ref"];

	if (ref_var.get_type() != Variant::STRING) {
		result.add_error("$ref must be a string", "ref");
		return;
	}

	String ref_uri = ref_var.operator String();

	if (ref_uri.is_empty()) {
		result.add_error("$ref cannot be empty", "ref");
		return;
	}

	// Create RefRule that will handle resolution and validation at runtime
	auto selector = std::make_unique<ValueSelector>();
	auto ref_rule = std::make_unique<RefRule>(ref_uri, schema.ptr());

	// UtilityFunctions::print("Created $ref ", ref_uri);

	result.rules->add_rule(std::make_unique<SelectorRule>(std::move(selector), std::move(ref_rule)));
}
