#include "content_encoding_rule.hpp"
#include "../validation_context.hpp"

using namespace godot;

bool ContentEncodingRule::validate(const Variant &target, ValidationContext &context) const {
	// Content encoding only applies to strings
	if (target.get_type() != Variant::STRING && target.get_type() != Variant::STRING_NAME) {
		return true;
	}

	const String str = target.operator String();

	if (encoding == "base64") {
		return validate_base64(str, context);
	} else if (encoding == "base64url") {
		return validate_base64url(str, context);
	} else if (encoding == "7bit" || encoding == "8bit" || encoding == "binary") {
		// These encodings are always valid for strings
		return true;
	} else if (encoding == "quoted-printable") {
		// Basic quoted-printable validation could be added here
		return true;
	} else {
		// Unknown encodings are treated as valid
		return true;
	}
}

String ContentEncodingRule::get_description() const {
	return vformat("contentEncoding(%s)", encoding);
}

bool ContentEncodingRule::validate_base64(const String &str, ValidationContext &context) const {
	if (str.is_empty()) {
		return true; // Empty string is valid base64
	}

	// Base64 can only contain A-Z, a-z, 0-9, +, /, and = for padding
	int padding_count = 0;
	bool padding_started = false;

	for (int i = 0; i < str.length(); i++) {
		char c = str[i];

		if (c == '=') {
			padding_started = true;
			padding_count++;
			if (padding_count > 2) {
				context.add_error("Base64 content has too much padding", "contentEncoding", str);
				return false;
			}
		} else if (padding_started) {
			context.add_error("Base64 content has characters after padding", "contentEncoding", str);
			return false;
		} else if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '+' || c == '/')) {
			context.add_error(vformat("Base64 content contains invalid character: '%c'", c), "contentEncoding", str);
			return false;
		}
	}

	// Check length is multiple of 4
	if (str.length() % 4 != 0) {
		context.add_error("Base64 content length must be multiple of 4", "contentEncoding", str);
		return false;
	}

	return true;
}

bool ContentEncodingRule::validate_base64url(const String &str, ValidationContext &context) const {
	if (str.is_empty()) {
		return true; // Empty string is valid
	}

	// Base64url uses - and _ instead of + and /, and no padding
	for (int i = 0; i < str.length(); i++) {
		char c = str[i];
		if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-' || c == '_')) {
			context.add_error(vformat("Base64url content contains invalid character: '%c'", c), "contentEncoding", str);
			return false;
		}
	}

	return true;
}
