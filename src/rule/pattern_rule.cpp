#include "pattern_rule.hpp"
#include "../validation_context.hpp"

#include <godot_cpp/classes/reg_ex_match.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

PatternRule::PatternRule(const String &pattern) :
		pattern_string(pattern) {
	pattern_regex = RegEx::create_from_string(pattern);

	if (!pattern_regex.is_valid()) {
		UtilityFunctions::push_warning(vformat("PatternRule: invalid regex pattern '%s'", pattern));
	}
}

bool PatternRule::validate(const Variant &target, ValidationContext &context) const {
	// Only validate strings
	if (target.get_type() != Variant::STRING && target.get_type() != Variant::STRING_NAME) {
		return true; // Rule doesn't apply to non-strings
	}

	// Check if regex is valid
	if (!pattern_regex.is_valid()) {
		context.add_error(vformat("Invalid regex pattern '%s'", pattern_string), "pattern");
		return false;
	}

	String str = target.operator String();
	Ref<RegExMatch> match = pattern_regex->search(str);

	if (!match.is_valid()) {
		context.add_error(vformat("String '%s' does not match pattern '%s'", str, pattern_string), "pattern", target);
		return false;
	}

	return true;
}
