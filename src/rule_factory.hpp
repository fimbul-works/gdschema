#pragma once

#include "rule/rule_group.hpp"
#include "schema.hpp"
#include "schema_compile_error.hpp"
#include "selector/selector.hpp"

#include <godot_cpp/classes/mutex.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/variant.hpp>

#include <functional>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace godot {

// Forward declarations
class Schema;

/**
 * @class RuleFactory
 * @brief Factory for creating validation rules from JSON Schema definitions
 */
class RuleFactory {
public:
	/**
	 * @struct RuleCompileResult
	 * @brief Result of compiling a schema into validation rules
	 */
	struct RuleCompileResult {
		std::shared_ptr<RuleGroup> rules;
		std::vector<SchemaCompileError> errors;

		RuleCompileResult() : rules(std::make_shared<RuleGroup>()) {}

		bool has_errors() const { return !errors.empty(); }
		bool is_valid() const { return !has_errors(); }

		void add_error(const String &message, const String &schema_path_suffix = "") {
			TypedArray<String> path_parts;
			if (!schema_path_suffix.is_empty()) {
				// Split path like "properties/name/minLength" into parts
				PackedStringArray parts = schema_path_suffix.split("/");
				for (int i = 0; i < parts.size(); i++) {
					path_parts.push_back(parts[i]);
				}
			}
			errors.emplace_back(message, path_parts);
		}
	};

private:
	/**
	 * @brief Cache of compiled rules for reuse indexed by Dictionary.hash()
	 */
	std::unordered_map<uint64_t, std::shared_ptr<RuleGroup>> rule_cache;

	/**
	 * @brief Currently compiling schemas list
	 */
	std::unordered_set<uint64_t> compiling_schemas;

	/**
	 * @brief Mutex for thread-safe cache access
	 */
	Ref<Mutex> cache_mutex;

	/**
	 * @brief Factory functions for custom rules
	 */
	std::unordered_map<String, std::function<void(const Dictionary &, const Ref<Schema> &, RuleCompileResult &)>>
			custom_rule_factories;

	RuleFactory() { cache_mutex = Ref<Mutex>(memnew(Mutex)); }

	RuleFactory(RuleFactory const &); // Don't Implement
	void operator=(RuleFactory const &); // Don't implement

public:
	/**
	 * @brief Singleton instance
	 */
	static RuleFactory &get_singleton() {
		static RuleFactory instance;
		return instance;
	}

	/**
	 * @brief Creates validation rules from a schema definition
	 * @param schema The schema to create rules for
	 * @return Compile result with rules and any errors
	 */
	RuleCompileResult create_rules(const Ref<Schema> &schema);

	/**
	 * @brief Register a new schema rule factory function
	 * @param keyword The schema keyword to look for
	 * @param factory The factory function
	 */
	void register_rule_factory(const String &keyword,
			std::function<void(const Dictionary &, const Ref<Schema> &, RuleCompileResult &)> factory) {
		custom_rule_factories[keyword] = factory;
	}

private:
	/**
	 * @brief Creates $ref validation rules
	 * @param ref_def The reference definition
	 * @param result Result to add rules/errors to
	 */
	void create_ref_rules(const Ref<Schema> schema, const Dictionary &ref_def, RuleCompileResult &result);

	/**
	 * @brief Creates type validation rules
	 * @param type_def The type definition (string or array)
	 * @param result Result to add rules/errors to
	 */
	void create_type_rules(const Variant &type_def, RuleCompileResult &result);

	/**
	 * @brief Creates string constraint rules
	 * @param schema_def The schema definition
	 * @param result Result to add rules/errors to
	 */
	void create_string_rules(const Dictionary &schema_def, RuleCompileResult &result);

	/**
	 * @brief Creates numeric constraint rules
	 * @param schema_def The schema definition
	 * @param result Result to add rules/errors to
	 */
	void create_numeric_rules(const Dictionary &schema_def, RuleCompileResult &result);

	/**
	 * @brief Creates array constraint rules
	 * @param schema_def The schema definition
	 * @param schema The source schema for sub-schema creation
	 * @param result Result to add rules/errors to
	 */
	void create_array_rules(const Dictionary &schema_def, const Ref<Schema> &schema, RuleCompileResult &result);

	/**
	 * @brief Creates object constraint rules
	 * @param schema_def The schema definition
	 * @param schema The source schema for sub-schema creation
	 * @param result Result to add rules/errors to
	 */
	void create_object_rules(const Dictionary &schema_def, const Ref<Schema> &schema, RuleCompileResult &result);

	/**
	 * @brief Creates value constraint rules (const, enum)
	 * @param schema_def The schema definition
	 * @param result Result to add rules/errors to
	 */
	void create_value_rules(const Dictionary &schema_def, RuleCompileResult &result);

	/**
	 * @brief Creates logical composition rules (allOf, anyOf, oneOf, not)
	 * @param schema_def The schema definition
	 * @param schema The source schema for sub-schema creation
	 * @param result Result to add rules/errors to
	 */
	void create_logical_rules(const Dictionary &schema_def, const Ref<Schema> &schema, RuleCompileResult &result);

	/**
	 * @brief Creates custom rules using factory functions
	 * @param schema_def The schema definition
	 * @param schema The source schema
	 * @param result Result to add rules/errors to
	 */
	void create_custom_rules(const Dictionary &schema_def, const Ref<Schema> &schema, RuleCompileResult &result) {
		for (const auto &[keyword, factory] : custom_rule_factories) {
			if (schema_def.has(keyword)) {
				factory(schema_def, schema, result);
			}
		}
	}

	/**
	 * @brief Helper to get variant type name for error messages
	 * @param value The variant to analyze
	 * @return Type name string
	 */
	static String get_variant_type_name(const Variant &value) {
		switch (value.get_type()) {
			case Variant::NIL:
				return "null";
			case Variant::BOOL:
				return "boolean";
			case Variant::INT:
				return "integer";
			case Variant::FLOAT:
				return "number";
			case Variant::STRING:
			case Variant::STRING_NAME:
				return "string";
			case Variant::ARRAY:
			case Variant::PACKED_COLOR_ARRAY:
			case Variant::PACKED_FLOAT32_ARRAY:
			case Variant::PACKED_FLOAT64_ARRAY:
			case Variant::PACKED_INT32_ARRAY:
			case Variant::PACKED_INT64_ARRAY:
			case Variant::PACKED_STRING_ARRAY:
			case Variant::PACKED_VECTOR2_ARRAY:
			case Variant::PACKED_VECTOR3_ARRAY:
			case Variant::PACKED_VECTOR4_ARRAY:
				return "array";
			case Variant::DICTIONARY:
			case Variant::OBJECT:
				return "object";
			default:
				return "unknown";
		}
	}

	/**
	 * @brief Helper to convert numeric variant to double safely
	 * @param value The variant to convert
	 * @param out_value Output double value
	 * @return True if conversion successful
	 */
	static bool try_get_numeric_value(const Variant &value, double &out_value) {
		if (value.get_type() == Variant::INT) {
			out_value = static_cast<double>(value.operator int64_t());
			return true;
		} else if (value.get_type() == Variant::FLOAT) {
			out_value = value.operator double();
			return true;
		}
		return false;
	}

	/**
	 * @brief Helper to convert variant to non-negative integer safely
	 * @param value The variant to convert
	 * @param out_value Output integer value
	 * @return True if conversion successful and value is non-negative
	 */
	static bool try_get_non_negative_int(const Variant &value, int64_t &out_value) {
		if (value.get_type() == Variant::INT) {
			int64_t int_val = value.operator int64_t();
			if (int_val >= 0) {
				out_value = int_val;
				return true;
			}
		}
		return false;
	}
};

} // namespace godot
