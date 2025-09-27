#pragma once

#include "hashers.hpp"
#include "schema.hpp"

#include <godot_cpp/classes/mutex.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/variant.hpp>

#include <mutex>
#include <unordered_map>

namespace godot {

class SchemaRegistry {
private:
	std::unordered_map<StringName, Ref<Schema>, StringNameHasher, StringNameEqual> schemas;

	/**
	 * @brief Mutex for thread-safe registry access.
	 */
	Ref<Mutex> registry_mutex;

	SchemaRegistry() {
		registry_mutex = Ref<Mutex>(memnew(Mutex));
	}

	SchemaRegistry(SchemaRegistry const &); // Don't Implement
	void operator=(SchemaRegistry const &); // Don't implement

public:
	static SchemaRegistry &get_singleton() {
		static SchemaRegistry instance;
		return instance;
	}

	// Core registry
	void register_schema(const StringName &id, Ref<Schema> schema);
	bool has_schema(const StringName &id);
	Ref<Schema> get_schema(const StringName &id);
	PackedStringArray get_schema_ids();
	void unregister_schema(const StringName &id);
};

} //namespace godot
