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
	/**
	 * @brief Map of registered schemas by their unique ID.
	 */
	std::unordered_map<StringName, Ref<Schema>, StringNameHasher, StringNameEqual> schemas;

	/**
	 * @brief Mutex for thread-safe registry access.
	 */
	Ref<Mutex> registry_mutex;

	/**
	 * @brief Private constructor for singleton pattern.
	 */
	SchemaRegistry() { registry_mutex = Ref<Mutex>(memnew(Mutex)); }

	SchemaRegistry(SchemaRegistry const &); // Don't Implement
	void operator=(SchemaRegistry const &); // Don't implement

public:
	/**
	 * @brief Singleton instance
	 */
	static SchemaRegistry &get_singleton() {
		static SchemaRegistry instance;
		return instance;
	}

	/**
	 * @brief Registers a schema with a unique ID.
	 * @param id The unique schema ID (from `$id`).
	 * @param schema The schema instance to register.
	 * @return True if registration succeeded, false if ID already exists.
	 */
	bool register_schema(const StringName &id, Ref<Schema> schema);

	/**
	 * @brief Checks if a schema with the given ID is registered.
	 * @param id The schema ID to check.
	 * @return True if the schema is registered, false otherwise.
	 */
	bool has_schema(const StringName &id);

	/**
	 * @brief Retrieves a registered schema by its ID.
	 * @param id The schema ID to retrieve.
	 * @return The schema instance if found, null otherwise.
	 */
	Ref<Schema> get_schema(const StringName &id);

	/**
	 * @brief Retrieves all registered schema IDs.
	 * @return An array of all registered schema IDs.
	 */
	PackedStringArray get_schema_ids();

	/**
	 * @brief Unregisters a schema by its ID.
	 * @param id The schema ID to unregister.
	 * @return True if unregistration succeeded, false if ID was not found.
	 */
	bool unregister_schema(const StringName &id);
};

} //namespace godot
