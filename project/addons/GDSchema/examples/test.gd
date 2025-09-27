extends Node2D

var schema: Schema

func _ready() -> void:
	schema = Schema.build_schema({
		"type": "string"
	})
	print(schema.validate("hello").is_valid())
	print(schema.validate(123).get_errors())
