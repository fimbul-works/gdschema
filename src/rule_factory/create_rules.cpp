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

	bool has_refs = schema_def.has("$ref") || schema_def.has("$dynamicRef");

	// Check cache first (only for schemas without references)
	if (!has_refs) {
		auto cache_it = rule_cache.find(hash);
		if (cache_it != rule_cache.end()) {
			result.rules = cache_it->second;
			cache_mutex->unlock();

			// Update Schema atomically ONLY if not already compiled
			schema->compilation_mutex->lock();
			if (!schema->is_compiled) {
				schema->rules = result.rules;
				schema->compile_errors.clear(); // No errors for cached successful compilation
				schema->is_compiled = true;
			}

			schema->compilation_mutex->unlock();
			return result;
		}
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

		// Cache successful results (only for schemas without references)
		if (result.is_valid() && !has_refs) {
			cache_mutex->lock();
			if (rule_cache.find(hash) == rule_cache.end()) {
				rule_cache[hash] = result.rules;
			}
			cache_mutex->unlock();
		}

		// Apply dynamic scope wrapping if needed
		std::shared_ptr<ValidationRule> final_rules = result.rules;
		if (!schema->get_id().is_empty() || !schema->dynamic_anchor.is_empty()) {
			final_rules = std::make_shared<DynamicScopeRule>(schema, final_rules);
		}

		// Update Schema atomically (whether valid or not)
		schema->set_compilation_result(final_rules, result.errors);
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
