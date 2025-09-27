#include "register_types.hpp"

#include "../schema.hpp"
#include "../validation_result.hpp"

#include <gdextension_interface.h>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>

// #ifdef TESTS_ENABLED
// #include "../tests/test_runner.hpp" // Inlcude test runner headers
// #endif

namespace godot {

void initialize_schema_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}

	if (!ClassDB::class_exists("Schema")) {
		GDREGISTER_CLASS(Schema);
	}

	if (!ClassDB::class_exists("SchemaValidationResult")) {
		GDREGISTER_CLASS(SchemaValidationResult);
	}

	// #ifdef TESTS_ENABLED
	// 	run_all_tests(); // Run tests after registration
	// #endif
}

void uninitialize_schema_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
}

} // namespace godot

extern "C" {
GDExtensionBool GDE_EXPORT schema_library_init(GDExtensionInterfaceGetProcAddress p_get_proc_address,
		const GDExtensionClassLibraryPtr p_library,
		GDExtensionInitialization *r_initialization) {
	godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

	init_obj.register_initializer(godot::initialize_schema_module);
	init_obj.register_terminator(godot::uninitialize_schema_module);
	init_obj.set_minimum_library_initialization_level(godot::MODULE_INITIALIZATION_LEVEL_SCENE);

	return init_obj.init();
}
}
