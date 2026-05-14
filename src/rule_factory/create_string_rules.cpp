#include "../rule/content_encoding_rule.hpp"
#include "../rule/content_media_type_rule.hpp"
#include "../rule/format_rule.hpp"
#include "../rule/max_length_rule.hpp"
#include "../rule/min_length_rule.hpp"
#include "../rule/pattern_rule.hpp"
#include "../rule/selector_rule.hpp"
#include "../schema.hpp"
#include "../selector/value_selector.hpp"
#include "../util.hpp"
#include "rule_factory.hpp"

using namespace godot;

void RuleFactory::create_string_rules(const Dictionary &schema_def, RuleCompileResult &result) {
	// minLength
	if (schema_def.has("minLength")) {
		Variant min_length_var = schema_def["minLength"];
		int64_t min_length;
		if (SchemaUtil::try_get_non_negative_int(min_length_var, min_length)) {
			auto selector = std::make_unique<ValueSelector>();
			auto rule = std::make_unique<MinLengthRule>(min_length);
			result.rules->add_rule(std::make_unique<SelectorRule>(std::move(selector), std::move(rule)));
		}
	}

	// maxLength
	if (schema_def.has("maxLength")) {
		Variant max_length_var = schema_def["maxLength"];
		int64_t max_length;
		if (SchemaUtil::try_get_non_negative_int(max_length_var, max_length)) {
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
