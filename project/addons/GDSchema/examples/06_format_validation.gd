extends Node

## Format Validation Examples
## This example demonstrates built-in format validators

func _ready() -> void:
	print("=== Format Validation Examples ===\n")

	email_validation()
	uri_validation()
	datetime_validation()
	ip_address_validation()
	uuid_validation()

## Example 1: Email Validation
func email_validation() -> void:
	print("--- Email Validation ---")

	var schema = Schema.build_schema({
		"type": "string",
		"format": "email"
	})

	# Valid emails
	var valid_emails = [
		"user@example.com",
		"john.doe@company.co.uk",
		"player+tag@game.io",
		"test_123@sub.domain.org"
	]

	for email in valid_emails:
		var result = schema.validate(email)
		print("✓ '%s': %s" % [email, result.is_valid()])

	# Invalid emails
	var invalid_emails = [
		"notanemail",
		"@example.com",
		"user@",
		"user..name@example.com"
	]

	for email in invalid_emails:
		var result = schema.validate(email)
		print("✗ '%s': %s" % [email, result.is_valid()])
	print()

## Example 2: URI Validation
func uri_validation() -> void:
	print("--- URI Validation ---")

	var uri_schema = Schema.build_schema({
		"type": "string",
		"format": "uri"
	})

	# Valid URIs
	print("✓ HTTP URL: ", uri_schema.validate("https://example.com").is_valid())
	print("✓ With path: ", uri_schema.validate("https://example.com/path?query=1").is_valid())
	print("✓ FTP: ", uri_schema.validate("ftp://files.example.com/file.txt").is_valid())
	print("✓ Mailto: ", uri_schema.validate("mailto:user@example.com").is_valid())

	# Invalid URIs
	print("✗ Plain text: ", uri_schema.validate("not a uri").is_valid())
	print("✗ Incomplete: ", uri_schema.validate("http://").is_valid())
	print()

## Example 3: Date and Time Validation
func datetime_validation() -> void:
	print("--- Date/Time Validation ---")

	# ISO 8601 date-time
	var datetime_schema = Schema.build_schema({
		"type": "string",
		"format": "date-time"
	})

	print("✓ With Z: ", datetime_schema.validate("2023-12-25T10:30:00Z").is_valid())
	print("✓ With offset: ", datetime_schema.validate("2023-12-25T10:30:00+05:00").is_valid())
	print("✓ With ms: ", datetime_schema.validate("2023-01-01T00:00:00.000Z").is_valid())
	print("✗ Date only: ", datetime_schema.validate("2023-12-25").is_valid())
	print("✗ Invalid month: ", datetime_schema.validate("2023-13-01T10:30:00Z").is_valid())

	# Date format (YYYY-MM-DD)
	var date_schema = Schema.build_schema({
		"type": "string",
		"format": "date"
	})

	print("\n✓ Valid date: ", date_schema.validate("2023-12-25").is_valid())
	print("✓ Leap year: ", date_schema.validate("2024-02-29").is_valid())
	print("✗ Wrong format: ", date_schema.validate("25-12-2023").is_valid())
	print("✗ Invalid day: ", date_schema.validate("2023-02-30").is_valid())

	# Time format (HH:MM:SS)
	var time_schema = Schema.build_schema({
		"type": "string",
		"format": "time"
	})

	print("\n✓ Valid time: ", time_schema.validate("10:30:00").is_valid())
	print("✓ With zone: ", time_schema.validate("10:30:00Z").is_valid())
	print("✗ Invalid hour: ", time_schema.validate("25:00:00").is_valid())
	print()

## Example 4: IP Address Validation
func ip_address_validation() -> void:
	print("--- IP Address Validation ---")

	# IPv4
	var ipv4_schema = Schema.build_schema({
		"type": "string",
		"format": "ipv4"
	})

	print("IPv4:")
	print("✓ Local: ", ipv4_schema.validate("192.168.1.1").is_valid())
	print("✓ Localhost: ", ipv4_schema.validate("127.0.0.1").is_valid())
	print("✓ Public DNS: ", ipv4_schema.validate("8.8.8.8").is_valid())
	print("✗ Out of range: ", ipv4_schema.validate("256.1.1.1").is_valid())
	print("✗ Incomplete: ", ipv4_schema.validate("192.168.1").is_valid())

	# IPv6
	var ipv6_schema = Schema.build_schema({
		"type": "string",
		"format": "ipv6"
	})

	print("\nIPv6:")
	print("✓ Full: ", ipv6_schema.validate("2001:0db8:85a3:0000:0000:8a2e:0370:7334").is_valid())
	print("✓ Compressed: ", ipv6_schema.validate("2001:db8:85a3::8a2e:370:7334").is_valid())
	print("✓ Localhost: ", ipv6_schema.validate("::1").is_valid())
	print("✗ Double ::: ", ipv6_schema.validate("2001:0db8::8a2e::7334").is_valid())
	print()

## Example 5: UUID Validation
func uuid_validation() -> void:
	print("--- UUID Validation ---")

	var uuid_schema = Schema.build_schema({
		"type": "string",
		"format": "uuid"
	})

	print("✓ Valid UUID: ", uuid_schema.validate("550e8400-e29b-41d4-a716-446655440000").is_valid())
	print("✓ Nil UUID: ", uuid_schema.validate("00000000-0000-0000-0000-000000000000").is_valid())
	print("✗ Too short: ", uuid_schema.validate("550e8400-e29b-41d4-a716-44665544000").is_valid())
	print("✗ No hyphens: ", uuid_schema.validate("550e8400e29b41d4a716446655440000").is_valid())
	print()

## Bonus: Combining Formats with Other Constraints
func combined_format_validation() -> void:
	print("--- Combined Format + Constraints ---")

	var user_schema = Schema.build_schema({
		"type": "object",
		"properties": {
			"email": {
				"type": "string",
				"format": "email",
				"minLength": 5,
				"maxLength": 100
			},
			"website": {
				"type": "string",
				"format": "uri"
			},
			"created_at": {
				"type": "string",
				"format": "date-time"
			},
			"user_id": {
				"type": "string",
				"format": "uuid"
			}
		},
		"required": ["email", "user_id"]
	})

	var valid_user = {
		"email": "user@example.com",
		"website": "https://player.example.com",
		"created_at": "2023-12-25T10:30:00Z",
		"user_id": "550e8400-e29b-41d4-a716-446655440000"
	}

	print("✓ Valid user with formats: ", user_schema.validate(valid_user).is_valid())

	var invalid_user = {
		"email": "a@b",  # Too short (minLength 5)
		"user_id": "not-a-uuid"
	}

	var result = user_schema.validate(invalid_user)
	print("✗ Invalid user: ", result.is_valid())
	if result.has_errors():
		print("  Errors found: %d" % result.error_count())
