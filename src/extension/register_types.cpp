#include "register_types.hpp"

#include "../schema.hpp"
#include "../schema_registry.hpp"
#include "../rule_factory/rule_factory.hpp"
#include "../validation_result.hpp"

#include <gdextension_interface.h>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>

namespace godot {

void initialize_schema_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}

	// Initialize singletons
	SchemaRegistry::get_singleton();
	RuleFactory::get_singleton();

	if (!ClassDB::class_exists("Schema")) {
		GDREGISTER_CLASS(Schema);
	}

	if (!ClassDB::class_exists("SchemaValidationResult")) {
		GDREGISTER_CLASS(SchemaValidationResult);
	}

	// Preload and register the JSON Schema Draft-07 and 2020-12 meta-schemas
	Schema::load_from_json_file("res://addons/GDSchema/schema/json_schema_draft_07.json", false);
	Schema::load_from_json_file("res://addons/GDSchema/schema/json_schema_draft_2020-12.json", false);
}

void uninitialize_schema_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}

	SchemaRegistry::destroy_singleton();
	RuleFactory::destroy_singleton();
}

} // namespace godot

extern "C" {
GDExtensionBool GDE_EXPORT schema_library_init(GDExtensionInterfaceGetProcAddress p_get_proc_address,
		const GDExtensionClassLibraryPtr p_library, GDExtensionInitialization *r_initialization) {
	godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

	init_obj.register_initializer(godot::initialize_schema_module);
	init_obj.register_terminator(godot::uninitialize_schema_module);
	init_obj.set_minimum_library_initialization_level(godot::MODULE_INITIALIZATION_LEVEL_SCENE);

	return init_obj.init();
}
}
