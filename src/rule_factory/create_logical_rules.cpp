#include "../rule/all_of_rule.hpp"
#include "../rule/any_of_rule.hpp"
#include "../rule/conditional_rule.hpp"
#include "../rule/not_rule.hpp"
#include "../rule/one_of_rule.hpp"
#include "../rule/selector_rule.hpp"
#include "../schema.hpp"
#include "../selector/value_selector.hpp"
#include "../util.hpp"
#include "rule_factory.hpp"

using namespace godot;

void RuleFactory::create_logical_rules(const Dictionary &schema_def, const Ref<Schema> &schema, RuleCompileResult &result) {
	// allOf
	if (schema_def.has("allOf")) {
		Variant all_of_var = schema_def["allOf"];
		if (all_of_var.get_type() == Variant::ARRAY) {
			Array all_of_array = all_of_var.operator Array();
			auto all_of_rule = std::make_unique<AllOfRule>();

			for (int64_t i = 0; i < all_of_array.size(); i++) {
				Ref<Schema> child_schema = schema->get_child(vformat("allOf/%d", i));
				if (child_schema.is_valid()) {
					auto sub_result = create_rules(child_schema);
					result.errors.insert(result.errors.end(), sub_result.errors.begin(), sub_result.errors.end());

					if (sub_result.is_valid()) {
						all_of_rule->add_sub_rule(sub_result.rules);
					}
				}
			}

			auto selector = std::make_unique<ValueSelector>();
			result.rules->add_rule(std::make_unique<SelectorRule>(std::move(selector), std::move(all_of_rule)));
		}
	}

	// anyOf
	if (schema_def.has("anyOf")) {
		Variant any_of_var = schema_def["anyOf"];
		if (any_of_var.get_type() == Variant::ARRAY) {
			Array any_of_array = any_of_var.operator Array();
			auto any_of_rule = std::make_unique<AnyOfRule>();

			for (int64_t i = 0; i < any_of_array.size(); i++) {
				Ref<Schema> child_schema = schema->get_child(vformat("anyOf/%d", i));
				if (child_schema.is_valid()) {
					auto sub_result = create_rules(child_schema);
					result.errors.insert(result.errors.end(), sub_result.errors.begin(), sub_result.errors.end());

					if (sub_result.is_valid()) {
						any_of_rule->add_sub_rule(sub_result.rules);
					}
				}
			}

			auto selector = std::make_unique<ValueSelector>();
			result.rules->add_rule(std::make_unique<SelectorRule>(std::move(selector), std::move(any_of_rule)));
		}
	}

	// oneOf
	if (schema_def.has("oneOf")) {
		Variant one_of_var = schema_def["oneOf"];
		if (one_of_var.get_type() == Variant::ARRAY) {
			Array one_of_array = one_of_var.operator Array();
			auto one_of_rule = std::make_unique<OneOfRule>();

			for (int64_t i = 0; i < one_of_array.size(); i++) {
				Ref<Schema> child_schema = schema->get_child(vformat("oneOf/%d", i));
				if (child_schema.is_valid()) {
					auto sub_result = create_rules(child_schema);
					result.errors.insert(result.errors.end(), sub_result.errors.begin(), sub_result.errors.end());

					if (sub_result.is_valid()) {
						one_of_rule->add_sub_rule(sub_result.rules);
					}
				}
			}

			auto selector = std::make_unique<ValueSelector>();
			result.rules->add_rule(std::make_unique<SelectorRule>(std::move(selector), std::move(one_of_rule)));
		}
	}

	// not
	if (schema_def.has("not")) {
		Ref<Schema> child_schema = schema->get_child("not");
		if (child_schema.is_valid()) {
			auto not_result = create_rules(child_schema);
			result.errors.insert(result.errors.end(), not_result.errors.begin(), not_result.errors.end());

			if (not_result.is_valid()) {
				auto not_rule = std::make_unique<NotRule>(std::move(not_result.rules));
				auto selector = std::make_unique<ValueSelector>();
				result.rules->add_rule(std::make_unique<SelectorRule>(std::move(selector), std::move(not_rule)));
			}
		}
	}

	// ========== CONDITIONAL SCHEMAS ==========
	if (schema_def.has("if")) {
		// We have a conditional Schema
		std::shared_ptr<ValidationRule> if_rule = nullptr;
		std::shared_ptr<ValidationRule> then_rule = nullptr;
		std::shared_ptr<ValidationRule> else_rule = nullptr;

		// Compile 'if' Schema (required)
		Ref<Schema> if_schema = schema->get_child("if");
		if (if_schema.is_valid()) {
			auto if_result = create_rules(if_schema);
			result.errors.insert(result.errors.end(), if_result.errors.begin(), if_result.errors.end());

			if (if_result.is_valid()) {
				if_rule = if_result.rules;
			}
		}

		// Compile 'then' Schema (optional)
		if (schema_def.has("then")) {
			Ref<Schema> then_schema = schema->get_child("then");
			if (then_schema.is_valid()) {
				auto then_result = create_rules(then_schema);
				result.errors.insert(result.errors.end(), then_result.errors.begin(), then_result.errors.end());

				if (then_result.is_valid()) {
					then_rule = then_result.rules;
				}
			}
		}

		// Compile 'else' Schema (optional)
		if (schema_def.has("else")) {
			Ref<Schema> else_schema = schema->get_child("else");
			if (else_schema.is_valid()) {
				auto else_result = create_rules(else_schema);
				result.errors.insert(result.errors.end(), else_result.errors.begin(), else_result.errors.end());

				if (else_result.is_valid()) {
					else_rule = else_result.rules;
				}
			}
		}

		// Create the conditional rule if we have at least an 'if' rule
		if (if_rule) {
			auto conditional_rule = std::make_unique<ConditionalRule>(if_rule, then_rule, else_rule);
			auto selector = std::make_unique<ValueSelector>();
			result.rules->add_rule(std::make_unique<SelectorRule>(std::move(selector), std::move(conditional_rule)));
		}
	}
}
