#include "schema_registry.hpp"

using namespace godot;

void SchemaRegistry::register_schema(const StringName &id, Ref<Schema> schema) {
	if (has_schema(id)) {
		UtilityFunctions::push_warning(vformat("Schema %s already registered", id));
		return;
	}

	registry_mutex->lock();
	schemas[id] = schema;
	registry_mutex->unlock();

#ifdef GODOT_YAML_DEBUG
	UtilityFunctions::print(vformat("Registered Schema %s", id));
#endif
}

bool SchemaRegistry::has_schema(const StringName &id) {
	registry_mutex->lock();
	bool result = schemas.find(id) != schemas.end();
	registry_mutex->unlock();
	return result;
}

Ref<Schema> SchemaRegistry::get_schema(const StringName &id) {
	registry_mutex->lock();

	auto it = schemas.find(id);
	if (it != schemas.end()) {
		auto schema = it->second;
		registry_mutex->unlock();
		return schema;
	}

	registry_mutex->unlock();
	return Ref<Schema>();
}

PackedStringArray SchemaRegistry::get_schema_ids() {
	registry_mutex->lock();
	PackedStringArray id_list;
	for (const auto &[id, schema] : schemas) {
		id_list.append(id);
	}
	registry_mutex->unlock();
	return id_list;
}

void SchemaRegistry::unregister_schema(const StringName &id) {
	if (!has_schema(id)) {
		WARN_PRINT(vformat("Schema %s is not registered", id));
		return;
	}

	registry_mutex->lock();
	schemas.erase(id);
	registry_mutex->unlock();

#ifdef GODOT_SCHEMA_DEBUG
	UtilityFunctions::print(vformat("Unregistered Schema %s", id));
#endif
}
