#include "schema_registry.hpp"

using namespace godot;

SchemaRegistry *SchemaRegistry::singleton = nullptr;

bool SchemaRegistry::register_schema(const StringName &id, Ref<Schema> schema) {
	registry_mutex->lock();
	schemas[id] = schema;
	registry_mutex->unlock();

#ifdef GODOT_SCHEMA_DEBUG
	UtilityFunctions::print_verbose(vformat("Schema: Registered %s", id));
#endif
	return true;
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

bool SchemaRegistry::unregister_schema(const StringName &id) {
	if (!has_schema(id)) {
		WARN_PRINT(vformat("Schema %s is not registered", id));
		return false;
	}

	registry_mutex->lock();
	Ref<Schema> target_schema = schemas[id];
	schemas.erase(id);

	// Also check for dual registration with/without trailing '#'
	String id_str = String(id);
	if (id_str.ends_with("#") && id_str.length() > 1) {
		StringName clean_id = StringName(id_str.substr(0, id_str.length() - 1));
		auto it = schemas.find(clean_id);
		if (it != schemas.end() && it->second == target_schema) {
			schemas.erase(it);
		}
	} else if (!id_str.ends_with("#")) {
		StringName hash_id = StringName(id_str + "#");
		auto it = schemas.find(hash_id);
		if (it != schemas.end() && it->second == target_schema) {
			schemas.erase(it);
		}
	}

	registry_mutex->unlock();

#ifdef GODOT_SCHEMA_DEBUG
	UtilityFunctions::print_verbose(vformat("Schema: Unregistered %s", id));
#endif

	return true;
}

void SchemaRegistry::clear_user_schemas() {
	if (registry_mutex.is_valid()) {
		registry_mutex->lock();
	}

	std::unordered_map<StringName, Ref<Schema>, StringNameHasher, StringNameEqual> meta_schemas;
	for (const auto &[id, schema] : schemas) {
		String id_str = String(id);
		if (id_str.begins_with("http://json-schema.org/") || id_str.begins_with("https://json-schema.org/")) {
			meta_schemas[id] = schema;
		}
	}

	schemas = std::move(meta_schemas);

	if (registry_mutex.is_valid()) {
		registry_mutex->unlock();
	}
}

void SchemaRegistry::clear() {
	if (registry_mutex.is_valid()) {
		registry_mutex->lock();
	}

	schemas.clear();

	if (registry_mutex.is_valid()) {
		registry_mutex->unlock();
		registry_mutex.unref();
	}
}
