#include "../rule/dynamic_scope_rule.hpp"
#include "../schema.hpp"
#include "../util.hpp"
#include "rule_factory.hpp"

using namespace godot;

RuleFactory::RuleCompileResult RuleFactory::create_rules(const Ref<Schema> &schema) {
	RuleCompileResult result;
	Dictionary schema_def = schema->get_schema_definition();
	int64_t hash = schema_def.hash();

	// Handle $ref and $dynamicRef
	if (schema_def.has("$ref") || schema_def.has("$dynamicRef")) {
		create_ref_rules(schema, schema_def, result);
	}

	cache_mutex->lock();

	// Check compilation cycle
	bool is_cycling = compiling_schemas.find(hash) != compiling_schemas.end();
	if (is_cycling) {
		cache_mutex->unlock();
		return result; // Empty rules to break cycle
	}

	// Mark as compiling
	compiling_schemas.insert(hash);
	cache_mutex->unlock();

	// Compile the Schema
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
		create_unevaluated_rules(schema_def, schema, result);

		// Apply dynamic scope wrapping if needed
		if (!schema->get_id().is_empty() || !schema->dynamic_anchor.is_empty() || schema->is_root()) {
			auto wrapped_group = std::make_shared<RuleGroup>();
			wrapped_group->add_rule(std::make_shared<DynamicScopeRule>(schema.ptr(), result.rules));
			result.rules = wrapped_group;
		}

		// Update Schema atomically (whether valid or not)
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
