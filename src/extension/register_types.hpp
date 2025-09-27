/**
 * @file register_types.hpp
 * @brief Defines functions for registering the YAML module with Godot.
 *
 * This file contains the functions needed to register the YAML module
 * with the Godot engine. It handles initialization and cleanup of the
 * module during the Godot startup and shutdown process.
 */
#pragma once

#include <godot_cpp/core/class_db.hpp>

namespace godot {

/**
 * @brief Initializes the Schema module.
 *
 * This function is called by Godot during engine initialization and
 * registers all the module's classes with the ClassDB. It only
 * performs initialization during the SCENE module initialization level.
 *
 * @param p_level The current module initialization level
 */
void initialize_schema_module(ModuleInitializationLevel p_level);

/**
 * @brief Uninitializes the Schema module.
 *
 * This function is called by Godot during engine shutdown and
 * performs cleanup operations for the module.
 *
 * @param p_level The current module initialization level
 */
void uninitialize_schema_module(ModuleInitializationLevel p_level);

} // namespace godot
