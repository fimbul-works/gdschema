#include "format_rule.hpp"
#include "../validation_context.hpp"

#include <godot_cpp/classes/json.hpp>
#include <godot_cpp/classes/reg_ex.hpp>
#include <godot_cpp/classes/reg_ex_match.hpp>

using namespace godot;

bool FormatRule::validate(const Variant &target, ValidationContext &context) const {
	// Format validation only applies to strings - ignore other types
	if (target.get_type() != Variant::STRING && target.get_type() != Variant::STRING_NAME) {
		return true; // Format is ignored for non-strings
	}

	const String str = target.operator String();

	if (format == "date-time") {
		return validate_date_time(str, context);
	} else if (format == "date") {
		return validate_date(str, context);
	} else if (format == "time") {
		return validate_time(str, context);
	} else if (format == "email") {
		return validate_email(str, context);
	} else if (format == "hostname") {
		const String hostname_pattern = "^[a-zA-Z0-9]([a-zA-Z0-9\\-]{0,61}[a-zA-Z0-9])?(\\.[a-zA-Z0-9]([a-zA-Z0-9\\-]{0,61}[a-zA-Z0-9])?)*$";
		return validate_regex(hostname_pattern, str, context);
	} else if (format == "ipv4") {
		return validate_ipv4(str, context);
	} else if (format == "ipv6") {
		return validate_ipv6(str, context);
	} else if (format == "uri") {
		return validate_uri(str, context);
	} else if (format == "uri-reference") {
		return validate_uri_reference(str, context);
	} else if (format == "json-pointer") {
		return validate_json_pointer(str, context);
	} else if (format == "relative-json-pointer") {
		return validate_relative_json_pointer(str, context);
	} else if (format == "regex") {
		return validate_regex_format(str, context);
	} else if (format == "uuid") {
		const String uuid_pattern = "^[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}$";
		return validate_regex(uuid_pattern, str, context);
	} else if (format == "base64") {
		return validate_base64(str, context);
	} else if (format == "base64url") {
		return validate_base64url(str, context);
	} else {
		// Unknown formats are treated as annotations and should pass
		return true;
	}
}

String FormatRule::get_description() const {
	return vformat("format(%s)", format);
}

bool FormatRule::validate_regex(const String &regex_pattern, const String &str, ValidationContext &context) const {
	Ref<RegEx> regex = RegEx::create_from_string(regex_pattern);
	if (!regex.is_valid()) {
		context.add_error(vformat("Invalid %s regex: \"%s\"", format, regex_pattern), "format", str);
		return false;
	}

	Ref<RegExMatch> match = regex->search(str);
	if (!match.is_valid()) {
		context.add_error(vformat("Invalid %s: \"%s\"", format, str), "format", str);
		return false;
	}

	return true;
}

bool FormatRule::validate_email(const String &str, ValidationContext &context) const {
	// Check for consecutive dots
	if (str.contains("..")) {
		context.add_error(vformat("Email cannot contain consecutive dots: \"%s\"", str), "format", str);
		return false;
	}

	// Check for leading/trailing dots in local part
	int at_pos = str.find("@");
	if (at_pos == -1) {
		context.add_error(vformat("Email must contain @: \"%s\"", str), "format", str);
		return false;
	}

	String local_part = str.substr(0, at_pos);
	if (local_part.begins_with(".") || local_part.ends_with(".")) {
		context.add_error(vformat("Email local part cannot start or end with dot: \"%s\"", str), "format", str);
		return false;
	}

	// Basic email pattern validation
	const String email_pattern = "^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$";
	return validate_regex(email_pattern, str, context);
}

bool FormatRule::validate_date(const String &str, ValidationContext &context) const {
	// Simple but effective date pattern: YYYY-MM-DD
	const String date_pattern = "^([0-9]{4})-([0-9]{2})-([0-9]{2})$";
	Ref<RegEx> regex = RegEx::create_from_string(date_pattern);
	if (!regex.is_valid()) {
		context.add_error("Internal error: invalid date regex", "format", str);
		return false;
	}

	Ref<RegExMatch> match = regex->search(str);
	if (!match.is_valid()) {
		context.add_error(vformat("Invalid date format: \"%s\" (expected YYYY-MM-DD)", str), "format", str);
		return false;
	}

	PackedStringArray groups = match->get_strings();
	if (groups.size() < 4) {
		context.add_error("Internal error: date regex groups", "format", str);
		return false;
	}

	int year = groups[1].to_int();
	int month = groups[2].to_int();
	int day = groups[3].to_int();

	// Validate ranges
	if (month < 1 || month > 12) {
		context.add_error(vformat("Month must be 1-12, got %d", month), "format", str);
		return false;
	}

	if (day < 1 || day > 31) {
		context.add_error(vformat("Day must be 1-31, got %d", day), "format", str);
		return false;
	}

	// Days per month validation
	const int days_in_month[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	bool is_leap = (year % 4 == 0) && ((year % 100 != 0) || (year % 400 == 0));
	int max_day = days_in_month[month - 1];
	if (month == 2 && is_leap) {
		max_day = 29;
	}

	if (day > max_day) {
		context.add_error(vformat("Day %d is invalid for month %d in year %d", day, month, year), "format", str);
		return false;
	}

	return true;
}

bool FormatRule::validate_time(const String &str, ValidationContext &context) const {
	// RFC 3339 time format: HH:MM:SS[.sss][Z|±HH:MM]
	const String time_pattern = "^([0-9]{2}):([0-9]{2}):([0-9]{2})(?:\\.([0-9]+))?(?:([Zz])|([+-])([0-9]{2}):([0-9]{2}))?$";
	Ref<RegEx> regex = RegEx::create_from_string(time_pattern);
	if (!regex.is_valid()) {
		context.add_error("Internal error: invalid time regex", "format", str);
		return false;
	}

	Ref<RegExMatch> match = regex->search(str);
	if (!match.is_valid()) {
		context.add_error(vformat("Invalid time format: \"%s\"", str), "format", str);
		return false;
	}

	PackedStringArray groups = match->get_strings();
	if (groups.size() < 4) {
		context.add_error("Internal error: time regex groups", "format", str);
		return false;
	}

	int hour = groups[1].to_int();
	int minute = groups[2].to_int();
	int second = groups[3].to_int();

	if (hour < 0 || hour > 23) {
		context.add_error(vformat("Hour must be 0-23, got %d", hour), "format", str);
		return false;
	}

	if (minute < 0 || minute > 59) {
		context.add_error(vformat("Minute must be 0-59, got %d", minute), "format", str);
		return false;
	}

	if (second < 0 || second > 59) {
		context.add_error(vformat("Second must be 0-59, got %d", second), "format", str);
		return false;
	}

	// Validate timezone offset if present (groups 6-8 for offset)
	if (groups.size() >= 9 && !groups[6].is_empty()) { // Has timezone offset
		int tz_hour = groups[7].to_int();
		int tz_minute = groups[8].to_int();

		if (tz_hour < 0 || tz_hour > 23) {
			context.add_error(vformat("Timezone hour must be 0-23, got %d", tz_hour), "format", str);
			return false;
		}

		if (tz_minute < 0 || tz_minute > 59) {
			context.add_error(vformat("Timezone minute must be 0-59, got %d", tz_minute), "format", str);
			return false;
		}
	}

	return true;
}

bool FormatRule::validate_date_time(const String &str, ValidationContext &context) const {
	// RFC 3339 date-time: YYYY-MM-DDTHH:MM:SS[.sss][Z|±HH:MM]
	const String dt_pattern = "^([0-9]{4})-([0-9]{2})-([0-9]{2})[Tt]([0-9]{2}):([0-9]{2}):([0-9]{2})(?:\\.([0-9]+))?(?:([Zz])|([+-])([0-9]{2}):([0-9]{2}))?$";
	Ref<RegEx> regex = RegEx::create_from_string(dt_pattern);
	if (!regex.is_valid()) {
		context.add_error("Internal error: invalid date-time regex", "format", str);
		return false;
	}

	Ref<RegExMatch> match = regex->search(str);
	if (!match.is_valid()) {
		context.add_error(vformat("Invalid date-time format: \"%s\"", str), "format", str);
		return false;
	}

	PackedStringArray groups = match->get_strings();
	if (groups.size() < 7) {
		context.add_error("Internal error: date-time regex groups", "format", str);
		return false;
	}

	// Validate date part
	String date_part = vformat("%s-%s-%s", groups[1], groups[2], groups[3]);
	if (!validate_date(date_part, context)) {
		return false;
	}

	// Validate time components
	int hour = groups[4].to_int();
	int minute = groups[5].to_int();
	int second = groups[6].to_int();

	if (hour < 0 || hour > 23) {
		context.add_error(vformat("Hour must be 0-23, got %d", hour), "format", str);
		return false;
	}

	if (minute < 0 || minute > 59) {
		context.add_error(vformat("Minute must be 0-59, got %d", minute), "format", str);
		return false;
	}

	if (second < 0 || second > 59) {
		context.add_error(vformat("Second must be 0-59, got %d", second), "format", str);
		return false;
	}

	// Validate timezone offset if present
	if (groups.size() >= 12 && !groups[9].is_empty()) { // Has timezone offset
		int tz_hour = groups[10].to_int();
		int tz_minute = groups[11].to_int();

		if (tz_hour < 0 || tz_hour > 23) {
			context.add_error(vformat("Timezone hour must be 0-23, got %d", tz_hour), "format", str);
			return false;
		}

		if (tz_minute < 0 || tz_minute > 59) {
			context.add_error(vformat("Timezone minute must be 0-59, got %d", tz_minute), "format", str);
			return false;
		}
	}

	return true;
}

bool FormatRule::validate_ipv4(const String &str, ValidationContext &context) const {
	PackedStringArray octets = str.split(".");
	if (octets.size() != 4) {
		context.add_error(vformat("IPv4 address must have exactly 4 octets, got %d", octets.size()), "format", str);
		return false;
	}

	for (int i = 0; i < 4; i++) {
		String octet = octets[i];

		// Check for leading zeros (except "0" itself)
		if (octet.length() > 1 && octet[0] == '0') {
			context.add_error(vformat("IPv4 octet cannot have leading zeros: \"%s\"", octet), "format", str);
			return false;
		}

		// Must be numeric
		if (!octet.is_valid_int()) {
			context.add_error(vformat("IPv4 octet must be numeric: \"%s\"", octet), "format", str);
			return false;
		}

		int value = octet.to_int();
		if (value < 0 || value > 255) {
			context.add_error(vformat("IPv4 octet must be 0-255, got %d", value), "format", str);
			return false;
		}
	}

	return true;
}

bool FormatRule::validate_ipv6(const String &str, ValidationContext &context) const {
	// Check for double :: (invalid)
	if (str.count("::") > 1) {
		context.add_error("IPv6 address cannot contain multiple '::'", "format", str);
		return false;
	}

	// Handle special cases
	if (str == "::") {
		return true; // All zeros
	}
	if (str == "::1") {
		return true; // Loopback
	}

	// Check for IPv4-mapped IPv6 addresses (::ffff:192.168.1.1)
	if (str.contains(".")) {
		// This might be an IPv4-mapped or IPv4-compatible IPv6 address
		return validate_ipv6_with_ipv4(str, context);
	}

	// Split by :: to handle compression
	PackedStringArray parts;
	if (str.contains("::")) {
		parts = str.split("::", false);
		if (parts.size() > 2) {
			context.add_error("IPv6 address has invalid :: usage", "format", str);
			return false;
		}
	} else {
		// No compression - should have exactly 8 groups
		PackedStringArray groups = str.split(":");
		if (groups.size() != 8) {
			context.add_error(vformat("IPv6 address without :: must have exactly 8 groups, got %d", groups.size()), "format", str);
			return false;
		}
		parts.push_back(str);
	}

	// Validate each part
	for (int p = 0; p < parts.size(); p++) {
		String part = parts[p];
		if (part.is_empty() && p == 0) {
			continue; // Leading ::
		}
		if (part.is_empty() && p == 1) {
			continue; // Trailing ::
		}

		PackedStringArray groups = part.split(":");
		for (int i = 0; i < groups.size(); i++) {
			String group = groups[i];
			if (group.is_empty()) {
				continue;
			}

			// Each group must be 1-4 hex digits
			if (group.length() > 4) {
				context.add_error(vformat("IPv6 group cannot be longer than 4 characters: \"%s\"", group), "format", str);
				return false;
			}

			// Check for valid hex characters
			for (int j = 0; j < group.length(); j++) {
				char c = group[j];
				if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))) {
					context.add_error(vformat("IPv6 group contains invalid hex character: \"%s\"", group), "format", str);
					return false;
				}
			}
		}
	}

	return true;
}

bool FormatRule::validate_ipv6_with_ipv4(const String &str, ValidationContext &context) const {
	// Handle IPv4-mapped IPv6 addresses like ::ffff:192.168.1.1
	// or IPv4-compatible like ::192.168.1.1

	int last_colon = str.rfind(":");
	if (last_colon == -1) {
		context.add_error("Invalid IPv6 with IPv4 format", "format", str);
		return false;
	}

	String ipv6_part = str.substr(0, last_colon + 1);
	String ipv4_part = str.substr(last_colon + 1);

	// Validate the IPv4 part
	if (!validate_ipv4(ipv4_part, context)) {
		return false;
	}

	// Handle special cases for IPv4-mapped addresses
	if (str.begins_with("::ffff:") || str.begins_with("::FFFF:")) {
		return true; // IPv4-mapped IPv6
	}

	if (str.begins_with("::") && str.count(":") == 3) {
		return true; // IPv4-compatible IPv6
	}

	// For other cases, we need to validate that the IPv6 part is valid
	// and that the total doesn't exceed 8 groups
	String ipv6_without_ipv4 = ipv6_part.substr(0, ipv6_part.length() - 1); // Remove trailing colon

	// Count colons to estimate groups
	int colon_count = str.count(":");
	bool has_compression = str.contains("::");

	// IPv4 takes up 2 groups worth of space (32 bits = 2 x 16-bit groups)
	int estimated_groups = colon_count + 1 - 2; // -2 for the IPv4 part taking 2 groups

	if (!has_compression && estimated_groups != 6) {
		context.add_error("IPv6 with embedded IPv4 must have 6 IPv6 groups", "format", str);
		return false;
	}

	return true;
}

bool FormatRule::validate_uri(const String &str, ValidationContext &context) const {
	// URI must have a scheme
	if (!str.contains(":")) {
		context.add_error("URI must contain a scheme (e.g., \"http:\")", "format", str);
		return false;
	}

	// Find the scheme part
	int colon_pos = str.find(":");
	String scheme = str.substr(0, colon_pos);

	// Scheme must not be empty and must start with letter
	if (scheme.is_empty()) {
		context.add_error("URI scheme cannot be empty", "format", str);
		return false;
	}

	// Scheme format: starts with letter, contains only letters, digits, +, -, .
	if (!validate_regex("[a-zA-Z][a-zA-Z0-9\\+\\-\\.]+:(\\/\\/)?([^\\.]+\\.|[^\\:]+\\:)(.*)", str, context)) {
		return false;
	}

	// Check that we have content after the scheme
	String remainder = str.substr(colon_pos + 1);
	if (remainder.is_empty()) {
		context.add_error("URI must have content after scheme", "format", str);
		return false;
	}

	return true;
}

bool FormatRule::validate_uri_reference(const String &str, ValidationContext &context) const {
	// URI reference can be absolute URI or relative reference
	// For simplicity, we'll accept any non-empty string with valid characters
	const String uri_ref_pattern = "^[^\\s]*$"; // No whitespace
	return validate_regex(uri_ref_pattern, str, context);
}

bool FormatRule::validate_json_pointer(const String &str, ValidationContext &context) const {
	// JSON Pointer: empty string or starts with '/' and has valid escape sequences
	if (str.is_empty()) {
		return true; // Empty string is valid
	}

	if (!str.begins_with("/")) {
		context.add_error("JSON Pointer must start with '/' or be empty", "format", str);
		return false;
	}

	// Check for valid escape sequences (~0 for ~, ~1 for /)
	for (int i = 0; i < str.length(); i++) {
		if (str[i] == '~') {
			if (i + 1 >= str.length()) {
				context.add_error("JSON Pointer has incomplete escape sequence", "format", str);
				return false;
			}
			char next = str[i + 1];
			if (next != '0' && next != '1') {
				context.add_error("JSON Pointer has invalid escape sequence", "format", str);
				return false;
			}
			i++; // Skip next character
		}
	}

	return true;
}

bool FormatRule::validate_relative_json_pointer(const String &str, ValidationContext &context) const {
	// Relative JSON Pointer: non-negative integer followed by optional '#' or JSON Pointer
	const String rel_ptr_pattern = "^[0-9]+(?:#|/.*)?$";
	return validate_regex(rel_ptr_pattern, str, context);
}

bool FormatRule::validate_base64(const String &str, ValidationContext &context) const {
	if (str.is_empty()) {
		return true; // Empty string is valid base64
	}

	// Base64 can only contain A-Z, a-z, 0-9, +, /, and = for padding
	const String base64_pattern = "^[A-Za-z0-9+/]*={0,2}$";

	if (!validate_regex(base64_pattern, str, context)) {
		return false;
	}

	// Check length is multiple of 4
	if (str.length() % 4 != 0) {
		context.add_error("Base64 string length must be multiple of 4", "format", str);
		return false;
	}

	return true;
}

bool FormatRule::validate_base64url(const String &str, ValidationContext &context) const {
	if (str.is_empty()) {
		return true; // Empty string is valid
	}

	// Base64url uses - and _ instead of + and /, and no padding
	const String base64url_pattern = "^[A-Za-z0-9_-]*$";
	return validate_regex(base64url_pattern, str, context);
}

bool FormatRule::validate_regex_format(const String &str, ValidationContext &context) const {
	// Try to create a RegEx to validate the pattern
	Ref<RegEx> test_regex = RegEx::create_from_string(str);
	if (!test_regex.is_valid()) {
		context.add_error(vformat("Invalid regular expression: \"%s\"", str), "format", str);
		return false;
	}
	return true;
}
