#include "validation_context.hpp"
#include "schema.hpp"

using namespace godot;

ValidationContext::ValidationContext(const Schema *schema,
		const PackedStringArray &inst_parts,
		const PackedStringArray &sch_parts,
		int depth) :
		source_schema(schema), instance_path_parts(inst_parts), schema_path_parts(sch_parts), validation_depth(depth) {}

ValidationContext::ValidationContext(const ValidationContext &other) :
		instance_path_parts(other.instance_path_parts),
		schema_path_parts(other.schema_path_parts),
		source_schema(other.source_schema),
		errors(other.errors),
		custom_data(other.custom_data),
		evaluated_properties(other.evaluated_properties),
		evaluated_items(other.evaluated_items),
		dynamic_scope(other.dynamic_scope),
		validation_depth(other.validation_depth) {}

ValidationContext &ValidationContext::operator=(const ValidationContext &other) {
	if (this != &other) {
		instance_path_parts = other.instance_path_parts;
		schema_path_parts = other.schema_path_parts;
		source_schema = other.source_schema;
		errors = other.errors;
		custom_data = other.custom_data;
		evaluated_properties = other.evaluated_properties;
		evaluated_items = other.evaluated_items;
		dynamic_scope = other.dynamic_scope;
		validation_depth = other.validation_depth;
	}
	return *this;
}

ValidationContext::~ValidationContext() {
	// Destructor implementation in .cpp allows Ref<Schema> to be incomplete in the header
}

ValidationContext ValidationContext::create_child_instance(const String &segment) const {
	PackedStringArray new_parts = instance_path_parts;
	if (!segment.is_empty()) {
		new_parts.push_back(segment);
	}
	ValidationContext child(source_schema, new_parts, schema_path_parts, validation_depth);
	child.dynamic_scope = dynamic_scope;
	return child;
}

ValidationContext ValidationContext::create_child_schema(const String &segment) const {
	PackedStringArray new_parts = schema_path_parts;
	if (!segment.is_empty()) {
		new_parts.push_back(segment);
	}
	ValidationContext child(source_schema, instance_path_parts, new_parts, validation_depth);
	child.evaluated_properties = evaluated_properties;
	child.evaluated_items = evaluated_items;
	child.dynamic_scope = dynamic_scope;
	return child;
}

ValidationContext ValidationContext::create_child_context(const String &instance_segment, const String &schema_segment) const {
	PackedStringArray new_instance_parts = instance_path_parts;
	if (!instance_segment.is_empty()) {
		new_instance_parts.push_back(instance_segment);
	}

	PackedStringArray new_schema_parts = schema_path_parts;
	if (!schema_segment.is_empty()) {
		new_schema_parts.push_back(schema_segment);
	}
	ValidationContext child(source_schema, new_instance_parts, new_schema_parts, validation_depth);
	child.dynamic_scope = dynamic_scope;
	return child;
}

void ValidationContext::push_dynamic_scope(const Ref<Schema> &schema) {
	dynamic_scope.push_back(schema);
}

const std::vector<Ref<Schema>> &ValidationContext::get_dynamic_scope() const {
	return dynamic_scope;
}

ValidationContext ValidationContext::with_incremented_depth() const {
	ValidationContext child = *this;
	child.validation_depth++;
	return child;
}
