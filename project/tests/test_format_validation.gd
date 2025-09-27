extends TestSuite

func _init() -> void:
	icon = "✅"

# ========== EMAIL FORMAT TESTS ==========

func test_email_format_valid() -> void:
	var schema = Schema.build_schema({
		"type": "string",
		"format": "email"
	})

	expect(schema.validate("user@example.com").is_valid(), "Simple email should validate")
	expect(schema.validate("test.email@domain.co.uk").is_valid(), "Email with dots and subdomain should validate")
	expect(schema.validate("user+tag@example.org").is_valid(), "Email with plus addressing should validate")
	expect(schema.validate("very.long.email.address@very.long.domain.name.com").is_valid(), "Long email should validate")
	expect(schema.validate("123@456.com").is_valid(), "Numeric email should validate")

func test_email_format_invalid() -> void:
	var schema = Schema.build_schema({
		"type": "string",
		"format": "email"
	})

	expect(!schema.validate("notanemail").is_valid(), "String without @ should not validate")
	expect(!schema.validate("@example.com").is_valid(), "Email without local part should not validate")
	expect(!schema.validate("user@").is_valid(), "Email without domain should not validate")
	expect(!schema.validate("user@.com").is_valid(), "Email with domain starting with dot should not validate")
	expect(!schema.validate("user..name@example.com").is_valid(), "Email with consecutive dots should not validate")
	expect(!schema.validate("user name@example.com").is_valid(), "Email with spaces should not validate")

# ========== DATE-TIME FORMAT TESTS ==========

func test_date_time_format_valid() -> void:
	var schema = Schema.build_schema({
		"type": "string",
		"format": "date-time"
	})

	expect(schema.validate("2023-12-25T10:30:00Z").is_valid(), "ISO 8601 with Z timezone should validate")
	expect(schema.validate("2023-12-25T10:30:00+05:00").is_valid(), "ISO 8601 with positive timezone should validate")
	expect(schema.validate("2023-12-25T10:30:00-08:00").is_valid(), "ISO 8601 with negative timezone should validate")
	expect(schema.validate("2023-01-01T00:00:00.000Z").is_valid(), "ISO 8601 with milliseconds should validate")
	expect(schema.validate("2023-12-31T23:59:59+00:00").is_valid(), "End of year datetime should validate")

func test_date_time_format_invalid() -> void:
	var schema = Schema.build_schema({
		"type": "string",
		"format": "date-time"
	})

	expect(!schema.validate("2023-12-25").is_valid(), "Date without time should not validate")
	expect(!schema.validate("10:30:00").is_valid(), "Time without date should not validate")
	expect(!schema.validate("2023/12/25 10:30:00").is_valid(), "Wrong date format should not validate")
	expect(!schema.validate("2023-13-25T10:30:00Z").is_valid(), "Invalid month should not validate")
	expect(!schema.validate("2023-12-32T10:30:00Z").is_valid(), "Invalid day should not validate")
	expect(!schema.validate("2023-12-25T25:30:00Z").is_valid(), "Invalid hour should not validate")

# ========== DATE FORMAT TESTS ==========

func test_date_format_valid() -> void:
	var schema = Schema.build_schema({
		"type": "string",
		"format": "date"
	})

	expect(schema.validate("2023-12-25").is_valid(), "Valid date should validate")
	expect(schema.validate("2023-01-01").is_valid(), "First day of year should validate")
	expect(schema.validate("2023-12-31").is_valid(), "Last day of year should validate")
	expect(schema.validate("2024-02-29").is_valid(), "Leap year date should validate")

func test_date_format_invalid() -> void:
	var schema = Schema.build_schema({
		"type": "string",
		"format": "date"
	})

	expect(!schema.validate("2023/12/25").is_valid(), "Wrong separator should not validate")
	expect(!schema.validate("25-12-2023").is_valid(), "Wrong order should not validate")
	expect(!schema.validate("2023-13-01").is_valid(), "Invalid month should not validate")
	expect(!schema.validate("2023-12-32").is_valid(), "Invalid day should not validate")
	expect(!schema.validate("2023-02-29").is_valid(), "Non-leap year Feb 29 should not validate")

# ========== TIME FORMAT TESTS ==========

func test_time_format_valid() -> void:
	var schema = Schema.build_schema({
		"type": "string",
		"format": "time"
	})

	expect(schema.validate("10:30:00").is_valid(), "Basic time should validate")
	expect(schema.validate("00:00:00").is_valid(), "Midnight should validate")
	expect(schema.validate("23:59:59").is_valid(), "End of day should validate")
	expect(schema.validate("10:30:00.123").is_valid(), "Time with milliseconds should validate")
	expect(schema.validate("10:30:00Z").is_valid(), "Time with timezone should validate")
	expect(schema.validate("10:30:00+05:00").is_valid(), "Time with offset should validate")

func test_time_format_invalid() -> void:
	var schema = Schema.build_schema({
		"type": "string",
		"format": "time"
	})

	expect(!schema.validate("25:00:00").is_valid(), "Invalid hour should not validate")
	expect(!schema.validate("10:60:00").is_valid(), "Invalid minute should not validate")
	expect(!schema.validate("10:30:60").is_valid(), "Invalid second should not validate")
	expect(!schema.validate("10:30").is_valid(), "Missing seconds should not validate")
	expect(!schema.validate("10-30-00").is_valid(), "Wrong separator should not validate")

# ========== URI FORMAT TESTS ==========

func test_uri_format_valid() -> void:
	var schema = Schema.build_schema({
		"type": "string",
		"format": "uri"
	})

	expect(schema.validate("https://example.com").is_valid(), "HTTPS URL should validate")
	print(schema.validate("https://example.com").get_summary())
	expect(schema.validate("http://example.com/path?query=value#fragment").is_valid(), "Full URL should validate")
	expect(schema.validate("ftp://files.example.com/file.txt").is_valid(), "FTP URL should validate")
	expect(schema.validate("mailto:user@example.com").is_valid(), "Mailto URI should validate")
	expect(schema.validate("file:///home/user/file.txt").is_valid(), "File URI should validate")
	expect(schema.validate("urn:isbn:0451450523").is_valid(), "URN should validate")

func test_uri_format_invalid() -> void:
	var schema = Schema.build_schema({
		"type": "string",
		"format": "uri"
	})

	expect(!schema.validate("not a uri").is_valid(), "Plain text should not validate")
	expect(!schema.validate("http://").is_valid(), "Incomplete URI should not validate")
	expect(!schema.validate("://example.com").is_valid(), "URI without scheme should not validate")

# ========== URI-REFERENCE FORMAT TESTS ==========

func test_uri_reference_format_valid() -> void:
	var schema = Schema.build_schema({
		"type": "string",
		"format": "uri-reference"
	})

	expect(schema.validate("https://example.com").is_valid(), "Absolute URI should validate")
	expect(schema.validate("/path/to/resource").is_valid(), "Absolute path should validate")
	expect(schema.validate("../relative/path").is_valid(), "Relative path should validate")
	expect(schema.validate("?query=value").is_valid(), "Query-only reference should validate")
	expect(schema.validate("#fragment").is_valid(), "Fragment-only reference should validate")
	expect(schema.validate("resource").is_valid(), "Simple resource name should validate")

# ========== IPV4 FORMAT TESTS ==========

func test_ipv4_format_valid() -> void:
	var schema = Schema.build_schema({
		"type": "string",
		"format": "ipv4"
	})

	expect(schema.validate("192.168.1.1").is_valid(), "Private IP should validate")
	expect(schema.validate("127.0.0.1").is_valid(), "Localhost should validate")
	expect(schema.validate("0.0.0.0").is_valid(), "Zero IP should validate")
	expect(schema.validate("255.255.255.255").is_valid(), "Broadcast IP should validate")
	expect(schema.validate("8.8.8.8").is_valid(), "Public DNS should validate")

func test_ipv4_format_invalid() -> void:
	var schema = Schema.build_schema({
		"type": "string",
		"format": "ipv4"
	})

	expect(!schema.validate("256.1.1.1").is_valid(), "Octet > 255 should not validate")
	expect(!schema.validate("192.168.1").is_valid(), "Missing octet should not validate")
	expect(!schema.validate("192.168.1.1.1").is_valid(), "Extra octet should not validate")
	expect(!schema.validate("192.168.01.1").is_valid(), "Leading zero should not validate")
	expect(!schema.validate("192.168.1.a").is_valid(), "Non-numeric octet should not validate")

# ========== IPV6 FORMAT TESTS ==========

func test_ipv6_format_valid() -> void:
	var schema = Schema.build_schema({
		"type": "string",
		"format": "ipv6"
	})

	expect(schema.validate("2001:0db8:85a3:0000:0000:8a2e:0370:7334").is_valid(), "Full IPv6 should validate")
	expect(schema.validate("2001:db8:85a3::8a2e:370:7334").is_valid(), "Compressed IPv6 should validate")
	expect(schema.validate("::1").is_valid(), "Localhost IPv6 should validate")
	expect(schema.validate("::").is_valid(), "Zero IPv6 should validate")
	expect(schema.validate("2001:db8::1").is_valid(), "Mixed compression should validate")
	expect(schema.validate("::ffff:192.168.1.1").is_valid(), "IPv4-mapped IPv6 should validate")

func test_ipv6_format_invalid() -> void:
	var schema = Schema.build_schema({
		"type": "string",
		"format": "ipv6"
	})

	expect(!schema.validate("2001:0db8:85a3::8a2e::7334").is_valid(), "Double compression should not validate")
	expect(!schema.validate("2001:0db8:85a3:0000:0000:8a2e:0370:7334:extra").is_valid(), "Too many groups should not validate")
	expect(!schema.validate("gggg::1").is_valid(), "Invalid hex should not validate")
	expect(!schema.validate("2001:db8").is_valid(), "Too few groups should not validate")

# ========== HOSTNAME FORMAT TESTS ==========

func test_hostname_format_valid() -> void:
	var schema = Schema.build_schema({
		"type": "string",
		"format": "hostname"
	})

	expect(schema.validate("example.com").is_valid(), "Simple hostname should validate")
	expect(schema.validate("sub.example.com").is_valid(), "Subdomain should validate")
	expect(schema.validate("localhost").is_valid(), "Single label should validate")
	expect(schema.validate("my-server.example.org").is_valid(), "Hostname with hyphens should validate")
	expect(schema.validate("server01.datacenter.company.com").is_valid(), "Multi-level hostname should validate")

func test_hostname_format_invalid() -> void:
	var schema = Schema.build_schema({
		"type": "string",
		"format": "hostname"
	})

	expect(!schema.validate("example..com").is_valid(), "Double dot should not validate")
	expect(!schema.validate("-example.com").is_valid(), "Leading hyphen should not validate")
	expect(!schema.validate("example-.com").is_valid(), "Trailing hyphen should not validate")
	expect(!schema.validate("exam_ple.com").is_valid(), "Underscore should not validate")
	expect(!schema.validate("").is_valid(), "Empty string should not validate")

# ========== JSON-POINTER FORMAT TESTS ==========

func test_json_pointer_format_valid() -> void:
	var schema = Schema.build_schema({
		"type": "string",
		"format": "json-pointer"
	})

	expect(schema.validate("").is_valid(), "Empty pointer should validate")
	expect(schema.validate("/foo").is_valid(), "Simple pointer should validate")
	expect(schema.validate("/foo/bar").is_valid(), "Nested pointer should validate")
	expect(schema.validate("/0").is_valid(), "Array index pointer should validate")
	expect(schema.validate("/foo~0bar").is_valid(), "Escaped tilde should validate")
	expect(schema.validate("/foo~1bar").is_valid(), "Escaped slash should validate")

func test_json_pointer_format_invalid() -> void:
	var schema = Schema.build_schema({
		"type": "string",
		"format": "json-pointer"
	})

	expect(!schema.validate("foo").is_valid(), "Pointer without leading slash should not validate")
	expect(!schema.validate("/foo~").is_valid(), "Incomplete escape should not validate")
	expect(!schema.validate("/foo~2").is_valid(), "Invalid escape should not validate")

# ========== REGEX FORMAT TESTS ==========

func test_regex_format_valid() -> void:
	var schema = Schema.build_schema({
		"type": "string",
		"format": "regex"
	})

	expect(schema.validate("^[a-z]+$").is_valid(), "Simple regex should validate")
	expect(schema.validate("\\d{3}-\\d{3}-\\d{4}").is_valid(), "Phone regex should validate")
	expect(schema.validate("[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}").is_valid(), "Email regex should validate")
	expect(schema.validate(".*").is_valid(), "Match all regex should validate")

# ========== UUID FORMAT TESTS ==========

func test_uuid_format_valid() -> void:
	var schema = Schema.build_schema({
		"type": "string",
		"format": "uuid"
	})

	expect(schema.validate("550e8400-e29b-41d4-a716-446655440000").is_valid(), "Version 4 UUID should validate")
	expect(schema.validate("6ba7b810-9dad-11d1-80b4-00c04fd430c8").is_valid(), "Version 1 UUID should validate")
	expect(schema.validate("00000000-0000-0000-0000-000000000000").is_valid(), "Nil UUID should validate")

func test_uuid_format_invalid() -> void:
	var schema = Schema.build_schema({
		"type": "string",
		"format": "uuid"
	})

	expect(!schema.validate("550e8400-e29b-41d4-a716-44665544000").is_valid(), "Short UUID should not validate")
	expect(!schema.validate("550e8400-e29b-41d4-a716-4466554400000").is_valid(), "Long UUID should not validate")
	expect(!schema.validate("550e8400-e29b-41d4-a716-44665544000g").is_valid(), "Invalid hex should not validate")
	expect(!schema.validate("550e8400e29b41d4a716446655440000").is_valid(), "UUID without hyphens should not validate")

# ========== FORMAT VALIDATION WITH OTHER CONSTRAINTS ==========

func test_format_with_length_constraints() -> void:
	var schema = Schema.build_schema({
		"type": "string",
		"format": "email",
		"minLength": 10,
		"maxLength": 50
	})

	expect(schema.validate("user@example.com").is_valid(), "Valid email within length should validate")
	expect(!schema.validate("a@b.co").is_valid(), "Valid email too short should not validate")
	expect(!schema.validate("very.very.very.long.email.address@very.long.domain.name.extension.com").is_valid(), "Valid email too long should not validate")

# ========== CONTENT ENCODING TESTS ==========

func test_content_encoding_base64() -> void:
	var schema = Schema.build_schema({
		"type": "string",
		"contentEncoding": "base64"
	})

	expect(schema.validate("SGVsbG8gV29ybGQ=").is_valid(), "Valid base64 should validate")
	expect(schema.validate("").is_valid(), "Empty string should validate")
	expect(!schema.validate("Not base64!").is_valid(), "Invalid base64 should not validate")

func test_content_encoding_base64url() -> void:
	var schema = Schema.build_schema({
		"type": "string",
		"contentEncoding": "base64url"
	})

	expect(schema.validate("SGVsbG8gV29ybGQ").is_valid(), "Valid base64url should validate")
	expect(schema.validate("SGVsbG8-V29ybGQ_").is_valid(), "Base64url with URL-safe chars should validate")
	expect(!schema.validate("SGVsbG8+V29ybGQ/").is_valid(), "Base64 with non-URL-safe chars should not validate")

# ========== CONTENT MEDIA TYPE TESTS ==========

func test_content_media_type() -> void:
	var schema = Schema.build_schema({
		"type": "string",
		"contentMediaType": "application/json"
	})

	# Note: This typically requires contentEncoding for binary data
	# For JSON, the string itself should be valid JSON
	expect(schema.validate("{\"key\": \"value\"}").is_valid(), "Valid JSON string should validate")
	expect(!schema.validate("not json").is_valid(), "Invalid JSON should not validate")

# ========== UNKNOWN FORMAT HANDLING ==========

func test_unknown_format_handling() -> void:
	var schema = Schema.build_schema({
		"type": "string",
		"format": "unknown-format"
	})

	# Unknown formats should be ignored (treated as annotations)
	expect(schema.validate("any string").is_valid(), "Any string should validate with unknown format")
	expect(schema.validate("").is_valid(), "Empty string should validate with unknown format")

# ========== FORMAT EDGE CASES ==========

func test_format_edge_cases() -> void:
	# Test that format validation only applies to strings
	var schema = Schema.build_schema({
		"format": "email"  # No type specified
	})

	expect(schema.validate("user@example.com").is_valid(), "String should validate")
	expect(schema.validate(123).is_valid(), "Non-string should validate (format ignored)")
	expect(schema.validate(true).is_valid(), "Boolean should validate (format ignored)")

func test_multiple_format_errors() -> void:
	var schema = Schema.build_schema({
		"type": "string",
		"format": "email",
		"minLength": 20
	})

	var result = schema.validate("short")
	expect(!result.is_valid(), "Should fail validation")
	expect(result.error_count() >= 2, "Should have both format and length errors")
