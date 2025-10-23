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
	 * @brief Result of compiling a Schema into validation rules
	 */
	struct RuleCompileResult {
		std::shared_ptr<RuleGroup> rules;
		std::vector<SchemaCompileError> errors;

		RuleCompileResult() :
				rules(std::make_shared<RuleGroup>()) {}

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
	std::unordered_map<String, std::function<void(const Dictionary &, const Ref<Schema> &, RuleCompileResult &)>> custom_rule_factories;

	/**
	 * @brief Private constructor for singleton pattern
	 */
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
	 * @brief Creates validation rules from a Schema definition
	 * @param schema The Schema to create rules for
	 * @return Compile result with rules and any errors
	 */
	RuleCompileResult create_rules(const Ref<Schema> &schema);

	/**
	 * @brief Register a new Schema rule factory function to enable custom keywords and rules
	 * @param keyword The Schema keyword to look for
	 * @param factory The factory function
	 */
	void register_rule_factory(const String &keyword, std::function<void(const Dictionary &, const Ref<Schema> &, RuleCompileResult &)> factory) {
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
	 * @param schema_def The Schema definition
	 * @param result Result to add rules/errors to
	 */
	void create_string_rules(const Dictionary &schema_def, RuleCompileResult &result);

	/**
	 * @brief Creates numeric constraint rules
	 * @param schema_def The Schema definition
	 * @param result Result to add rules/errors to
	 */
	void create_numeric_rules(const Dictionary &schema_def, RuleCompileResult &result);

	/**
	 * @brief Creates array constraint rules
	 * @param schema_def The Schema definition
	 * @param schema The source Schema for sub-schema creation
	 * @param result Result to add rules/errors to
	 */
	void create_array_rules(const Dictionary &schema_def, const Ref<Schema> &schema, RuleCompileResult &result);

	/**
	 * @brief Creates object constraint rules
	 * @param schema_def The Schema definition
	 * @param schema The source Schema for sub-schema creation
	 * @param result Result to add rules/errors to
	 */
	void create_object_rules(const Dictionary &schema_def, const Ref<Schema> &schema, RuleCompileResult &result);

	/**
	 * @brief Creates value constraint rules (const, enum)
	 * @param schema_def The Schema definition
	 * @param result Result to add rules/errors to
	 */
	void create_value_rules(const Dictionary &schema_def, RuleCompileResult &result);

	/**
	 * @brief Creates logical composition rules (allOf, anyOf, oneOf, not)
	 * @param schema_def The Schema definition
	 * @param schema The source Schema for sub-schema creation
	 * @param result Result to add rules/errors to
	 */
	void create_logical_rules(const Dictionary &schema_def, const Ref<Schema> &schema, RuleCompileResult &result);

	/**
	 * @brief Creates custom rules using factory functions
	 * @param schema_def The Schema definition
	 * @param schema The source Schema
	 * @param result Result to add rules/errors to
	 */
	void create_custom_rules(const Dictionary &schema_def, const Ref<Schema> &schema, RuleCompileResult &result);

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
		} else if (value.get_type() == Variant::FLOAT) {
			double float_val = value.operator double();
			if (float_val >= 0 && float_val == static_cast<double>(static_cast<int64_t>(float_val))) {
				out_value = static_cast<int64_t>(float_val);
				return true;
			}
		}
		return false;
	}
};

} // namespace godot
