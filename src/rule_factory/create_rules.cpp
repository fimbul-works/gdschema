#include "../schema.hpp"
#include "../util.hpp"
#include "rule_factory.hpp"

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

		// Cache successful results
		if (result.is_valid()) {
			cache_mutex->lock();
			if (rule_cache.find(hash) == rule_cache.end()) {
				rule_cache[hash] = result.rules;
			}
			cache_mutex->unlock();
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
