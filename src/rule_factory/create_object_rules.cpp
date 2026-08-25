#include "../rule/const_rule.hpp"
#include "../rule/dependency_rule.hpp"
#include "../rule/dependent_required_rule.hpp"
#include "../rule/dependent_schemas_rule.hpp"
#include "../rule/false_rule.hpp"
#include "../rule/max_properties_rule.hpp"
#include "../rule/min_properties_rule.hpp"
#include "../rule/required_properties_rule.hpp"
#include "../rule/selector_rule.hpp"
#include "../schema.hpp"
#include "../selector/additional_properties_selector.hpp"
#include "../selector/object_keys_selector.hpp"
#include "../selector/pattern_properties_selector.hpp"
#include "../selector/property_selector.hpp"
#include "../selector/unevaluated_properties_selector.hpp"
#include "../selector/value_selector.hpp"
#include "../util.hpp"
#include "rule_factory.hpp"

using namespace godot;

void RuleFactory::create_object_rules(const Dictionary &schema_def, const Ref<Schema> &schema, RuleCompileResult &result) {
	// minProperties
	if (schema_def.has("minProperties")) {
		Variant min_props_var = schema_def["minProperties"];
		int64_t min_props;
		if (SchemaUtil::try_get_non_negative_int(min_props_var, min_props)) {
			auto selector = std::make_unique<ValueSelector>();
			auto rule = std::make_unique<MinPropertiesRule>(min_props);
			result.rules->add_rule(std::make_unique<SelectorRule>(std::move(selector), std::move(rule)));
		}
	}

	// maxProperties
	if (schema_def.has("maxProperties")) {
		Variant max_props_var = schema_def["maxProperties"];
		int64_t max_props;
		if (SchemaUtil::try_get_non_negative_int(max_props_var, max_props)) {
			auto selector = std::make_unique<ValueSelector>();
			auto rule = std::make_unique<MaxPropertiesRule>(max_props);
			result.rules->add_rule(std::make_unique<SelectorRule>(std::move(selector), std::move(rule)));
		}
	}

	// required properties
	if (schema_def.has("required")) {
		Variant required_var = schema_def["required"];
		if (required_var.get_type() == Variant::ARRAY) {
			Array required_array = required_var.operator Array();
			std::vector<String> required_props;

			for (int i = 0; i < required_array.size(); i++) {
				if (SchemaUtil::is_string(required_array[i])) {
					required_props.push_back(required_array[i].operator String());
				}
			}

			if (!required_props.empty()) {
				auto selector = std::make_unique<ValueSelector>();
				auto rule = std::make_unique<RequiredPropertiesRule>(required_props);
				result.rules->add_rule(std::make_unique<SelectorRule>(std::move(selector), std::move(rule)));
			}
		}
	}

	// properties - create rules for each property
	if (schema_def.has("properties")) {
		Variant props_var = schema_def["properties"];
		if (props_var.get_type() == Variant::DICTIONARY) {
			Dictionary properties = props_var.operator Dictionary();
			Array keys = properties.keys();

			for (int i = 0; i < keys.size(); i++) {
				StringName prop_name = keys[i];
				StringName child_name = vformat("properties/%s", prop_name);
				Ref<Schema> child_schema = schema->get_child(child_name);
				if (child_schema.is_valid()) {
					// Recursively create rules for the child Schema
					auto child_result = create_rules(child_schema);

					// Merge any compilation errors
					result.errors.insert(result.errors.end(), child_result.errors.begin(), child_result.errors.end());

					// If child Schema is valid, create a selector rule for this property
					if (child_result.is_valid()) {
						auto selector = std::make_unique<PropertySelector>(prop_name);
						result.rules->add_rule(std::make_unique<SelectorRule>(std::move(selector), std::move(child_result.rules)));
					}
				}
			}
		}
	}

	// propertyNames - validate all object keys
	if (schema_def.has("propertyNames")) {
		Ref<Schema> child_schema = schema->get_child("propertyNames");
		if (child_schema.is_valid()) {
			auto prop_names_result = create_rules(child_schema);
			result.errors.insert(result.errors.end(), prop_names_result.errors.begin(), prop_names_result.errors.end());

			if (prop_names_result.is_valid() && !prop_names_result.rules->is_empty()) {
				auto selector = std::make_unique<ObjectKeysSelector>();
				result.rules->add_rule(std::make_unique<SelectorRule>(std::move(selector), std::move(prop_names_result.rules)));
			}
		}
	}

	// patternProperties
	if (schema_def.has("patternProperties")) {
		Variant pattern_props_var = schema_def["patternProperties"];
		if (pattern_props_var.get_type() == Variant::DICTIONARY) {
			Dictionary pattern_properties = pattern_props_var.operator Dictionary();
			Array patterns = pattern_properties.keys();

			for (int i = 0; i < patterns.size(); i++) {
				String pattern = patterns[i].operator String();
				StringName child_path = vformat("patternProperties/%s", pattern);
				Ref<Schema> child_schema = schema->get_child(child_path);
				if (child_schema.is_valid()) {
					auto pattern_result = create_rules(child_schema);
					result.errors.insert(result.errors.end(), pattern_result.errors.begin(), pattern_result.errors.end());

					if (pattern_result.is_valid()) {
						auto selector = std::make_unique<PatternPropertiesSelector>(pattern);
						result.rules->add_rule(std::make_unique<SelectorRule>(std::move(selector), std::move(pattern_result.rules)));
					}
				}
			}
		}
	}

	// additionalProperties
	if (schema_def.has("additionalProperties")) {
		Variant additional_props_var = schema_def["additionalProperties"];
		std::vector<StringName> defined_properties;
		std::vector<String> pattern_properties_list;

		if (schema_def.has("properties")) {
			Dictionary properties = schema_def["properties"].operator Dictionary();
			Array prop_keys = properties.keys();
			for (int i = 0; i < prop_keys.size(); i++) {
				defined_properties.push_back(StringName(prop_keys[i].operator String()));
			}
		}

		if (schema_def.has("patternProperties")) {
			Dictionary pattern_properties = schema_def["patternProperties"].operator Dictionary();
			Array pattern_keys = pattern_properties.keys();
			for (int i = 0; i < pattern_keys.size(); i++) {
				pattern_properties_list.push_back(pattern_keys[i].operator String());
			}
		}

		if (additional_props_var.get_type() == Variant::BOOL && !additional_props_var.operator bool()) {
			// additionalProperties: false - no additional properties allowed
			auto selector = std::make_unique<AdditionalPropertiesSelector>(defined_properties, pattern_properties_list);
			auto rule = std::make_shared<FalseRule>();
			result.rules->add_rule(std::make_unique<SelectorRule>(std::move(selector), std::move(rule)));
		} else if (additional_props_var.get_type() == Variant::BOOL && additional_props_var.operator bool()) {
			// additionalProperties: true - any additional properties allowed and marked as evaluated
			auto selector = std::make_unique<AdditionalPropertiesSelector>(defined_properties, pattern_properties_list);
			auto rule = std::make_shared<RuleGroup>();
			result.rules->add_rule(std::make_unique<SelectorRule>(std::move(selector), std::move(rule)));
		} else if (additional_props_var.get_type() == Variant::DICTIONARY) {
			// additionalProperties: {...} - additional properties must match this Schema
			Ref<Schema> child_schema = schema->get_child("additionalProperties");
			if (child_schema.is_valid()) {
				auto additional_result = create_rules(child_schema);
				result.errors.insert(result.errors.end(), additional_result.errors.begin(), additional_result.errors.end());

				if (additional_result.is_valid()) {
					auto selector = std::make_unique<AdditionalPropertiesSelector>(defined_properties, pattern_properties_list);
					result.rules->add_rule(std::make_unique<SelectorRule>(std::move(selector), std::move(additional_result.rules)));
				}
			}
		}
	}

	// Dependencies - Draft-07 property and Schema dependencies
	if (schema_def.has("dependencies")) {
		Dictionary dependencies = schema_def["dependencies"].operator Dictionary();
		Array dep_keys = dependencies.keys();

		for (int i = 0; i < dep_keys.size(); i++) {
			StringName dep_name = dep_keys[i];
			Variant dep_value = dependencies[dep_keys[i]];

			if (dep_value.get_type() == Variant::ARRAY) {
				// Property dependency
				Array required_props = dep_value.operator Array();
				std::vector<String> props;

				for (int j = 0; j < required_props.size(); j++) {
					if (SchemaUtil::is_string(required_props[j])) {
						props.push_back(required_props[j].operator String());
					}
				}

				if (!props.empty()) {
					auto selector = std::make_unique<ValueSelector>();
					auto rule = std::make_unique<DependencyRule>(dep_name, props);
					result.rules->add_rule(std::make_unique<SelectorRule>(std::move(selector), std::move(rule)));
				}

			} else if (dep_value.get_type() == Variant::DICTIONARY || dep_value.get_type() == Variant::BOOL) {
				// Schema dependency
				StringName child_key = vformat("dependencies/%s", dep_name);
				Ref<Schema> child_schema = schema->get_child(child_key);

				if (child_schema.is_valid()) {
					auto dep_result = create_rules(child_schema);
					result.errors.insert(result.errors.end(), dep_result.errors.begin(), dep_result.errors.end());

					if (dep_result.is_valid() && !dep_result.rules->is_empty()) {
						auto selector = std::make_unique<ValueSelector>();
						auto rule = std::make_unique<DependencyRule>(dep_name, dep_result.rules);
						result.rules->add_rule(std::make_unique<SelectorRule>(std::move(selector), std::move(rule)));
					}
				}
			}
		}
	}

	// dependentRequired - JSON Schema 2020-12
	if (schema_def.has("dependentRequired")) {
		Variant dep_req_var = schema_def["dependentRequired"];
		if (dep_req_var.get_type() == Variant::DICTIONARY) {
			Dictionary dep_req_dict = dep_req_var.operator Dictionary();
			Array keys = dep_req_dict.keys();
			auto rule = std::make_unique<DependentRequiredRule>();

			for (int i = 0; i < keys.size(); i++) {
				StringName trigger = keys[i];
				Variant req_props_var = dep_req_dict[trigger];
				if (req_props_var.get_type() == Variant::ARRAY) {
					Array req_props_array = req_props_var.operator Array();
					std::vector<StringName> req_props;
					for (int j = 0; j < req_props_array.size(); j++) {
						if (SchemaUtil::is_string(req_props_array[j])) {
							req_props.push_back(StringName(req_props_array[j].operator String()));
						}
					}
					if (!req_props.empty()) {
						rule->add_dependency(trigger, req_props);
					}
				}
			}

			auto selector = std::make_unique<ValueSelector>();
			result.rules->add_rule(std::make_unique<SelectorRule>(std::move(selector), std::move(rule)));
		}
	}

	// dependentSchemas - JSON Schema 2020-12
	if (schema_def.has("dependentSchemas")) {
		Variant dep_sch_var = schema_def["dependentSchemas"];
		if (dep_sch_var.get_type() == Variant::DICTIONARY) {
			Dictionary dep_sch_dict = dep_sch_var.operator Dictionary();
			Array keys = dep_sch_dict.keys();
			auto rule = std::make_unique<DependentSchemasRule>();

			for (int i = 0; i < keys.size(); i++) {
				StringName trigger = keys[i];
				StringName child_key = vformat("dependentSchemas/%s", trigger);
				Ref<Schema> child_schema = schema->get_child(child_key);

				if (child_schema.is_valid()) {
					auto dep_result = create_rules(child_schema);
					result.errors.insert(result.errors.end(), dep_result.errors.begin(), dep_result.errors.end());

					if (dep_result.is_valid()) {
						rule->add_schema(trigger, dep_result.rules);
					}
				}
			}

			auto selector = std::make_unique<ValueSelector>();
			result.rules->add_rule(std::make_unique<SelectorRule>(std::move(selector), std::move(rule)));
		}
	}
}
