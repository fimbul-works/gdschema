#include "rule_factory.hpp"
#include "meta_schema_definitions.hpp"
#include "rule/all_of_rule.hpp"
#include "rule/any_of_rule.hpp"
#include "rule/conditional_rule.hpp"
#include "rule/const_rule.hpp"
#include "rule/contains_rule.hpp"
#include "rule/content_encoding_rule.hpp"
#include "rule/content_media_type_rule.hpp"
#include "rule/dependency_rule.hpp"
#include "rule/enum_rule.hpp"
#include "rule/exclusive_maximum_rule.hpp"
#include "rule/exclusive_minimum_rule.hpp"
#include "rule/false_rule.hpp"
#include "rule/format_rule.hpp"
#include "rule/max_items_rule.hpp"
#include "rule/max_length_rule.hpp"
#include "rule/max_properties_rule.hpp"
#include "rule/maximum_rule.hpp"
#include "rule/min_items_rule.hpp"
#include "rule/min_length_rule.hpp"
#include "rule/min_properties_rule.hpp"
#include "rule/minimum_rule.hpp"
#include "rule/multiple_of_rule.hpp"
#include "rule/not_rule.hpp"
#include "rule/one_of_rule.hpp"
#include "rule/pattern_rule.hpp"
#include "rule/ref_rule.hpp"
#include "rule/required_properties_rule.hpp"
#include "rule/selector_rule.hpp"
#include "rule/true_rule.hpp"
#include "rule/type_rule.hpp"
#include "rule/unique_items_rule.hpp"
#include "schema.hpp"
#include "selector/additional_items_selector.hpp"
#include "selector/additional_properties_selector.hpp"
#include "selector/array_item_selector.hpp"
#include "selector/array_items_selector.hpp"
#include "selector/object_keys_selector.hpp"
#include "selector/object_values_selector.hpp"
#include "selector/pattern_properties_selector.hpp"
#include "selector/property_selector.hpp"
#include "selector/value_selector.hpp"

using namespace godot;

RuleFactory::RuleCompileResult RuleFactory::create_rules(const Ref<Schema> &schema) {
	RuleCompileResult result;
	Dictionary schema_def = schema->get_schema_definition();
	int64_t hash = schema_def.hash();

	// UtilityFunctions::print("Create Rules: ", schema_def);

	// CRITICAL: Check for $ref FIRST
	if (schema_def.has("$ref")) {
		create_ref_rules(schema, schema_def, result);
		schema->set_compilation_result(result.rules, result.errors);
		return result;
	}

	cache_mutex->lock();

	// Check compilation cycle
	bool is_cycling = compiling_schemas.find(hash) != compiling_schemas.end();
	if (is_cycling) {
		cache_mutex->unlock();
		return result; // Empty rules to break cycle
	}

	// Check cache first
	auto cache_it = rule_cache.find(hash);
	if (cache_it != rule_cache.end()) {
		result.rules = cache_it->second;
		cache_mutex->unlock();

		// Update schema atomically ONLY if not already compiled
		schema->compilation_mutex->lock();
		if (!schema->is_compiled) {
			schema->rules = result.rules;
			schema->compile_errors.clear(); // No errors for cached successful compilation
			schema->is_compiled = true;
		}

		schema->compilation_mutex->unlock();
		return result;
	}

	// Mark as compiling
	compiling_schemas.insert(hash);
	cache_mutex->unlock();

	// Compile the schema
	try {
		if (schema_def.has("type")) {
			create_type_rules(schema_def["type"], result);
		}

		create_value_rules(schema_def, result);
		create_string_rules(schema_def, result);
		create_numeric_rules(schema_def, result);
		create_array_rules(schema_def, schema, result);
		create_object_rules(schema_def, schema, result);
		create_logical_rules(schema_def, schema, result);
		create_custom_rules(schema_def, schema, result);

		// Cache successful results
		if (result.is_valid()) {
			cache_mutex->lock();
			if (rule_cache.find(hash) == rule_cache.end()) {
				rule_cache[hash] = result.rules;
			}
			cache_mutex->unlock();
		}

		// Update schema atomically (whether valid or not)
		schema->set_compilation_result(result.rules, result.errors);
	} catch (...) {
		// Ensure cleanup on exception
		cache_mutex->lock();
		compiling_schemas.erase(hash);
		cache_mutex->unlock();
		throw;
	}

	// Clean up compilation state
	cache_mutex->lock();
	compiling_schemas.erase(hash);
	cache_mutex->unlock();

	return result;
}

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

void RuleFactory::create_type_rules(const Variant &type_def, RuleCompileResult &result) {
	if (type_def.get_type() == Variant::STRING) {
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
			if (type_array[i].get_type() == Variant::STRING) {
				types.push_back(type_array[i].operator String());
			} else {
				result.add_error(vformat("Type array element must be string, got %s", get_variant_type_name(type_array[i])), vformat("type/%d", i));
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

void RuleFactory::create_string_rules(const Dictionary &schema_def, RuleCompileResult &result) {
	// minLength
	if (schema_def.has("minLength")) {
		Variant min_length_var = schema_def["minLength"];
		int64_t min_length;
		if (try_get_non_negative_int(min_length_var, min_length)) {
			auto selector = std::make_unique<ValueSelector>();
			auto rule = std::make_unique<MinLengthRule>(min_length);
			result.rules->add_rule(std::make_unique<SelectorRule>(std::move(selector), std::move(rule)));
		}
		// Note: Meta-validation ensures this is a non-negative integer
	}

	// maxLength
	if (schema_def.has("maxLength")) {
		Variant max_length_var = schema_def["maxLength"];
		int64_t max_length;
		if (try_get_non_negative_int(max_length_var, max_length)) {
			auto selector = std::make_unique<ValueSelector>();
			auto rule = std::make_unique<MaxLengthRule>(max_length);
			result.rules->add_rule(std::make_unique<SelectorRule>(std::move(selector), std::move(rule)));
		}
	}

	// pattern
	if (schema_def.has("pattern")) {
		Variant pattern_var = schema_def["pattern"];
		if (pattern_var.get_type() == Variant::STRING) {
			String pattern_str = pattern_var.operator String();
			auto selector = std::make_unique<ValueSelector>();
			auto rule = std::make_unique<PatternRule>(pattern_str);
			result.rules->add_rule(std::make_unique<SelectorRule>(std::move(selector), std::move(rule)));
		}
	}

	// format
	if (schema_def.has("format")) {
		Variant format_var = schema_def["format"];
		if (format_var.get_type() == Variant::STRING) {
			String format = format_var.operator String();
			auto selector = std::make_unique<ValueSelector>();
			auto rule = std::make_unique<FormatRule>(format);
			result.rules->add_rule(std::make_unique<SelectorRule>(std::move(selector), std::move(rule)));
		}
	}

	// contentEncoding
	if (schema_def.has("contentEncoding")) {
		Variant format_var = schema_def["contentEncoding"];
		if (format_var.get_type() == Variant::STRING) {
			String format = format_var.operator String();
			auto selector = std::make_unique<ValueSelector>();
			auto rule = std::make_unique<ContentEncodingRule>(format);
			result.rules->add_rule(std::make_unique<SelectorRule>(std::move(selector), std::move(rule)));
		}
	}

	// contentMediaType
	if (schema_def.has("contentMediaType")) {
		Variant format_var = schema_def["contentMediaType"];
		if (format_var.get_type() == Variant::STRING) {
			String format = format_var.operator String();
			auto selector = std::make_unique<ValueSelector>();
			auto rule = std::make_unique<ContentMediaTypeRule>(format);
			result.rules->add_rule(std::make_unique<SelectorRule>(std::move(selector), std::move(rule)));
		}
	}
}

void RuleFactory::create_numeric_rules(const Dictionary &schema_def, RuleCompileResult &result) {
	// minimum
	if (schema_def.has("minimum")) {
		Variant minimum_var = schema_def["minimum"];
		double minimum;
		if (try_get_numeric_value(minimum_var, minimum)) {
			auto selector = std::make_unique<ValueSelector>();
			auto rule = std::make_unique<MinimumRule>(minimum);
			result.rules->add_rule(std::make_unique<SelectorRule>(std::move(selector), std::move(rule)));
		}
	}

	// maximum
	if (schema_def.has("maximum")) {
		Variant maximum_var = schema_def["maximum"];
		double maximum;
		if (try_get_numeric_value(maximum_var, maximum)) {
			auto selector = std::make_unique<ValueSelector>();
			auto rule = std::make_unique<MaximumRule>(maximum);
			result.rules->add_rule(std::make_unique<SelectorRule>(std::move(selector), std::move(rule)));
		}
	}

	// exclusiveMinimum
	if (schema_def.has("exclusiveMinimum")) {
		Variant exclusive_minimum_var = schema_def["exclusiveMinimum"];
		double exclusive_minimum;
		if (try_get_numeric_value(exclusive_minimum_var, exclusive_minimum)) {
			auto selector = std::make_unique<ValueSelector>();
			auto rule = std::make_unique<ExclusiveMinimumRule>(exclusive_minimum);
			result.rules->add_rule(std::make_unique<SelectorRule>(std::move(selector), std::move(rule)));
		}
	}

	// exclusiveMaximum
	if (schema_def.has("exclusiveMaximum")) {
		Variant exclusive_maximum_var = schema_def["exclusiveMaximum"];
		double exclusive_maximum;
		if (try_get_numeric_value(exclusive_maximum_var, exclusive_maximum)) {
			auto selector = std::make_unique<ValueSelector>();
			auto rule = std::make_unique<ExclusiveMaximumRule>(exclusive_maximum);
			result.rules->add_rule(std::make_unique<SelectorRule>(std::move(selector), std::move(rule)));
		}
	}

	// multipleOf
	if (schema_def.has("multipleOf")) {
		Variant multiple_of_var = schema_def["multipleOf"];
		double multiple_of;
		if (try_get_numeric_value(multiple_of_var, multiple_of)) {
			auto selector = std::make_unique<ValueSelector>();
			auto rule = std::make_unique<MultipleOfRule>(multiple_of);
			result.rules->add_rule(std::make_unique<SelectorRule>(std::move(selector), std::move(rule)));
		}
	}
}

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
		// Default value is stored in schema definition, not as validation rule
	}
}

void RuleFactory::create_object_rules(const Dictionary &schema_def, const Ref<Schema> &schema, RuleCompileResult &result) {
	// minProperties
	if (schema_def.has("minProperties")) {
		Variant min_props_var = schema_def["minProperties"];
		int64_t min_props;
		if (try_get_non_negative_int(min_props_var, min_props)) {
			auto selector = std::make_unique<ValueSelector>();
			auto rule = std::make_unique<MinPropertiesRule>(min_props);
			result.rules->add_rule(std::make_unique<SelectorRule>(std::move(selector), std::move(rule)));
		}
	}

	// maxProperties
	if (schema_def.has("maxProperties")) {
		Variant max_props_var = schema_def["maxProperties"];
		int64_t max_props;
		if (try_get_non_negative_int(max_props_var, max_props)) {
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
				if (required_array[i].get_type() == Variant::STRING) {
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
					// Recursively create rules for the child schema
					auto child_result = create_rules(child_schema);

					// Merge any compilation errors
					result.errors.insert(result.errors.end(), child_result.errors.begin(), child_result.errors.end());

					// If child schema is valid, create a selector rule for this property
					if (child_result.is_valid() && !child_result.rules->is_empty()) {
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
				Variant pattern_schema_var = pattern_properties[patterns[i]];

				if (pattern_schema_var.get_type() == Variant::DICTIONARY) {
					StringName child_path = vformat("patternProperties/%s", pattern);
					Ref<Schema> child_schema = schema->get_child(child_path);
					if (child_schema.is_valid()) {
						auto pattern_result = create_rules(child_schema);
						result.errors.insert(result.errors.end(), pattern_result.errors.begin(), pattern_result.errors.end());

						if (pattern_result.is_valid() && !pattern_result.rules->is_empty()) {
							auto selector = std::make_unique<PatternPropertiesSelector>(pattern);
							result.rules->add_rule(std::make_unique<SelectorRule>(std::move(selector), std::move(pattern_result.rules)));
						}
					}
				}
			}
		}
	}

	// additionalProperties
	if (schema_def.has("additionalProperties")) {
		Variant additional_props_var = schema_def["additionalProperties"];

		if (additional_props_var.get_type() == Variant::BOOL && !additional_props_var.operator bool()) {
			// additionalProperties: false - no additional properties allowed
			// Collect defined properties and pattern properties
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

			// Create a rule that fails validation for any additional properties
			auto selector = std::make_unique<AdditionalPropertiesSelector>(defined_properties, pattern_properties_list);
			auto rule = std::make_unique<ConstRule>(Variant()); // This will always fail since no value equals null in this context
			result.rules->add_rule(std::make_unique<SelectorRule>(std::move(selector), std::move(rule)));
		} else if (additional_props_var.get_type() == Variant::DICTIONARY) {
			// additionalProperties: {...} - additional properties must match this schema
			Dictionary additional_schema = additional_props_var.operator Dictionary();
			Ref<Schema> child_schema = schema->get_child("additionalProperties");
			if (child_schema.is_valid()) {
				auto additional_result = create_rules(child_schema);
				result.errors.insert(result.errors.end(), additional_result.errors.begin(), additional_result.errors.end());

				if (additional_result.is_valid() && !additional_result.rules->is_empty()) {
					std::vector<StringName> defined_properties;
					std::vector<String> pattern_properties_list;

					// Same collection logic as above...
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

					auto selector = std::make_unique<AdditionalPropertiesSelector>(defined_properties, pattern_properties_list);
					result.rules->add_rule(std::make_unique<SelectorRule>(std::move(selector), std::move(additional_result.rules)));
				}
			}
		}
	}

	// Dependencies - property and schema dependencies
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
					if (required_props[j].get_type() == Variant::STRING) {
						props.push_back(required_props[j].operator String());
					}
				}

				if (!props.empty()) {
					auto selector = std::make_unique<ValueSelector>();
					auto rule = std::make_unique<DependencyRule>(dep_name, props);
					result.rules->add_rule(std::make_unique<SelectorRule>(std::move(selector), std::move(rule)));
				}

			} else if (dep_value.get_type() == Variant::DICTIONARY) {
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
}

void RuleFactory::create_array_rules(const Dictionary &schema_def, const Ref<Schema> &schema, RuleCompileResult &result) {
	// minItems
	if (schema_def.has("minItems")) {
		Variant min_items_var = schema_def["minItems"];
		int64_t min_items;
		if (try_get_non_negative_int(min_items_var, min_items)) {
			auto selector = std::make_unique<ValueSelector>();
			auto rule = std::make_unique<MinItemsRule>(min_items);
			result.rules->add_rule(std::make_unique<SelectorRule>(std::move(selector), std::move(rule)));
		}
	}

	// maxItems
	if (schema_def.has("maxItems")) {
		Variant max_items_var = schema_def["maxItems"];
		int64_t max_items;
		if (try_get_non_negative_int(max_items_var, max_items)) {
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

	// Track tuple length for additionalItems
	int tuple_length = -1;

	// items - validate array items (single schema or tuple validation)
	if (schema_def.has("items")) {
		Variant items_var = schema_def["items"];

		if (items_var.get_type() == Variant::DICTIONARY) {
			// Single schema applies to all items
			Ref<Schema> child_schema = schema->get_child("items");

			if (child_schema.is_valid()) {
				auto items_result = create_rules(child_schema);
				result.errors.insert(result.errors.end(), items_result.errors.begin(), items_result.errors.end());

				if (items_result.is_valid() && !items_result.rules->is_empty()) {
					auto selector = std::make_unique<ArrayItemsSelector>();
					result.rules->add_rule(std::make_unique<SelectorRule>(std::move(selector), std::move(items_result.rules)));
				}
			}
		} else if (items_var.get_type() == Variant::ARRAY) {
			// Tuple validation - each position has its own schema
			Array items_array = items_var.operator Array();
			tuple_length = items_array.size(); // Store tuple length

			for (int64_t i = 0; i < items_array.size(); i++) {
				StringName child_key = vformat("items/%d", i);
				Ref<Schema> child_schema = schema->get_child(child_key);

				if (child_schema.is_valid()) {
					auto item_result = create_rules(child_schema);
					result.errors.insert(result.errors.end(), item_result.errors.begin(), item_result.errors.end());

					if (item_result.is_valid() && !item_result.rules->is_empty()) {
						// Create selector for this specific array position
						auto selector = std::make_unique<ArrayItemSelector>(i);
						result.rules->add_rule(std::make_unique<SelectorRule>(std::move(selector), std::move(item_result.rules)));
					}
				}
			}
		}
	}

	// additionalItems - only applies when items is an array (tuple validation)
	if (tuple_length >= 0 && schema_def.has("additionalItems")) {
		Variant additional_items_var = schema_def["additionalItems"];

		if (additional_items_var.get_type() == Variant::BOOL && !additional_items_var.operator bool()) {
			// additionalItems: false - no additional items allowed beyond tuple
			// Create a FalseRule that will fail for any additional items
			auto selector = std::make_unique<AdditionalItemsSelector>(tuple_length);
			auto rule = std::make_shared<FalseRule>();
			result.rules->add_rule(std::make_unique<SelectorRule>(std::move(selector), std::move(rule)));
		} else if (additional_items_var.get_type() == Variant::DICTIONARY) {
			// additionalItems: {...} - additional items must match this schema
			Ref<Schema> child_schema = schema->get_child("additionalItems");
			if (child_schema.is_valid()) {
				auto additional_result = create_rules(child_schema);
				result.errors.insert(result.errors.end(), additional_result.errors.begin(), additional_result.errors.end());

				if (additional_result.is_valid() && !additional_result.rules->is_empty()) {
					auto selector = std::make_unique<AdditionalItemsSelector>(tuple_length);
					result.rules->add_rule(std::make_unique<SelectorRule>(std::move(selector), std::move(additional_result.rules)));
				}
			}
		}
		// Note: additionalItems: true (default) means additional items are allowed with no constraints
	}

	// contains - at least one array item must validate against the schema
	if (schema_def.has("contains")) {
		Ref<Schema> child_schema = schema->get_child("contains");
		if (child_schema.is_valid()) {
			Dictionary child_def = child_schema->get_schema_definition();
			// Check if this is boolean schema detection
			if (child_def.is_empty()) {
				// contains: true (empty schema) - always matches any item
				auto selector = std::make_unique<ValueSelector>();
				auto rule = std::make_shared<TrueRule>();
				auto contains_rule = std::make_unique<ContainsRule>(rule);
				result.rules->add_rule(std::make_unique<SelectorRule>(std::move(selector), std::move(contains_rule)));
			} else if (child_def.size() == 1 && child_def.has("not") &&
					child_def["not"].get_type() == Variant::DICTIONARY &&
					child_def["not"].operator Dictionary().is_empty()) {
				// contains: false pattern {"not": {}} - never matches any item
				auto selector = std::make_unique<ValueSelector>();
				auto rule = std::make_shared<FalseRule>();
				auto contains_rule = std::make_unique<ContainsRule>(rule);
				result.rules->add_rule(std::make_unique<SelectorRule>(std::move(selector), std::move(contains_rule)));
			} else {
				// Normal schema - compile recursively
				auto contains_result = create_rules(child_schema);
				result.errors.insert(result.errors.end(), contains_result.errors.begin(), contains_result.errors.end());

				if (contains_result.is_valid()) {
					auto selector = std::make_unique<ValueSelector>();
					auto rule = std::make_unique<ContainsRule>(contains_result.rules);
					result.rules->add_rule(std::make_unique<SelectorRule>(std::move(selector), std::move(rule)));
				}
			}
		}
	}
}

void RuleFactory::create_logical_rules(const Dictionary &schema_def, const Ref<Schema> &schema, RuleCompileResult &result) {
	// allOf
	if (schema_def.has("allOf")) {
		Variant all_of_var = schema_def["allOf"];
		if (all_of_var.get_type() == Variant::ARRAY) {
			Array all_of_array = all_of_var.operator Array();
			auto all_of_rule = std::make_unique<AllOfRule>();

			for (int64_t i = 0; i < all_of_array.size(); i++) {
				if (all_of_array[i].get_type() == Variant::DICTIONARY) {
					Ref<Schema> child_schema = schema->get_child(vformat("allOf/%d", i));
					if (child_schema.is_valid()) {
						auto sub_result = create_rules(child_schema);
						result.errors.insert(result.errors.end(), sub_result.errors.begin(), sub_result.errors.end());

						if (sub_result.is_valid()) {
							all_of_rule->add_sub_rule(sub_result.rules);
						}
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
				if (any_of_array[i].get_type() == Variant::DICTIONARY) {
					Ref<Schema> child_schema = schema->get_child(vformat("anyOf/%d", i));
					if (child_schema.is_valid()) {
						auto sub_result = create_rules(child_schema);
						result.errors.insert(result.errors.end(), sub_result.errors.begin(), sub_result.errors.end());

						if (sub_result.is_valid()) {
							any_of_rule->add_sub_rule(sub_result.rules);
						}
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
				if (one_of_array[i].get_type() == Variant::DICTIONARY) {
					Ref<Schema> child_schema = schema->get_child(vformat("oneOf/%d", i));
					if (child_schema.is_valid()) {
						auto sub_result = create_rules(child_schema);
						result.errors.insert(result.errors.end(), sub_result.errors.begin(), sub_result.errors.end());

						if (sub_result.is_valid()) {
							one_of_rule->add_sub_rule(sub_result.rules);
						}
					}
				}
			}

			auto selector = std::make_unique<ValueSelector>();
			result.rules->add_rule(std::make_unique<SelectorRule>(std::move(selector), std::move(one_of_rule)));
		}
	}

	// not
	if (schema_def.has("not")) {
		Variant not_var = schema_def["not"];
		if (not_var.get_type() == Variant::DICTIONARY) {
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
	}

	// ========== CONDITIONAL SCHEMAS ==========
	if (schema_def.has("if")) {
		// We have a conditional schema
		std::shared_ptr<ValidationRule> if_rule = nullptr;
		std::shared_ptr<ValidationRule> then_rule = nullptr;
		std::shared_ptr<ValidationRule> else_rule = nullptr;

		// Compile 'if' schema (required)
		Ref<Schema> if_schema = schema->get_child("if");
		if (if_schema.is_valid()) {
			auto if_result = create_rules(if_schema);
			result.errors.insert(result.errors.end(), if_result.errors.begin(), if_result.errors.end());

			if (if_result.is_valid()) {
				if_rule = if_result.rules;
			}
		}

		// Compile 'then' schema (optional)
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

		// Compile 'else' schema (optional)
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

void RuleFactory::create_custom_rules(const Dictionary &schema_def, const Ref<Schema> &schema, RuleCompileResult &result) {
	for (const auto &[keyword, factory] : custom_rule_factories) {
		if (schema_def.has(keyword)) {
			factory(schema_def, schema, result);
		}
	}
}
