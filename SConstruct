#!/usr/bin/env python
import os
import shutil
import glob

# Use cache for faster build times
cache_dir = os.path.abspath('build/scons_cache')
if not os.path.exists(cache_dir):
    os.makedirs(cache_dir, exist_ok=True)
os.environ['SCONS_CACHE'] = cache_dir

base_env = SConscript('ext/godot-cpp/SConstruct')

def get_library_name(env):
    platform = env.get('platform', 'unknown')
    target = env.get('target', 'unknown')
    arch = env.get('arch', 'unknown')

    debug_or_release = 'release' if target == 'template_release' else 'debug'

    if platform == 'windows':
        return f'libgdschema.windows.{debug_or_release}.{arch}.dll'
    elif platform in ['linux', 'android']:
        return f'libgdschema.{platform}.{debug_or_release}.{arch}.so'
    elif platform == 'macos':
        return f'libgdschema.{platform}.{debug_or_release}.{arch}.dylib'
    elif platform == 'ios':
        return f'libgdschema.ios.{debug_or_release}.xcframework'
    else:
        print(f'Unsupported platform: {platform}')
        return f'libgdschema.{platform}.{debug_or_release}.{arch}'

def setup_build_env(base_env):
    env = base_env.Clone()

    platform = env.get('platform', '')
    is_debug = env.get('target', '') != 'template_release'
    arch = env.get('arch', 'x86_64')

    # C++ standard configuration
    cpp20_platforms = ['windows']  # Add 'linux', 'macos' when ready
    use_cpp20 = platform in cpp20_platforms
    cpp_std_version = '20' if use_cpp20 else '17'
    env['USE_CPP20'] = use_cpp20

    print(f"Building for {platform} with C++{cpp_std_version}")

    # ========== WINDOWS ==========
    if platform == 'windows':
        # C++ standard and exception handling
        env.Append(CCFLAGS=[f'/std:c++{cpp_std_version}', '/EHsc'])

        if use_cpp20:
            env.Append(CCFLAGS=['/Zc:preprocessor'])

        # Debug vs Release
        if is_debug:
            env.Append(CCFLAGS=['/Z7'])    # Debug info
        else:
            env.Append(CCFLAGS=[
                '/O2',    # Optimize for speed
                '/Oi',    # Intrinsic functions
            ])
            env.Append(CPPDEFINES=['NDEBUG'])

        # Architecture
        if arch == 'x86_32':
            env.Append(LINKFLAGS=['/MACHINE:X86'])
        else:
            env.Append(LINKFLAGS=['/MACHINE:X64'])

        env.Append(LINKFLAGS=['/IMPLIB:${TARGET.base}.lib'])

    # ========== UNIX-LIKE ==========
    else:
        env.Append(CCFLAGS=[f'-std=c++{cpp_std_version}', '-fexceptions'])

        # macOS-specific deployment target
        if platform == 'macos':
            env.Append(CCFLAGS=['-mmacosx-version-min=10.15'])
            env.Append(LINKFLAGS=['-mmacosx-version-min=10.15'])

        if platform == 'linux':
            env.Append(LINKFLAGS=['-static-libstdc++', '-static-libgcc'])

        if is_debug:
            env.Append(CCFLAGS=['-O0', '-g'])
        else:
            env.Append(CCFLAGS=['-O3', '-ffast-math'])
            env.Append(CPPDEFINES=['NDEBUG'])

        # Architecture
        if arch == 'x86_32':
            env.Append(CCFLAGS=['-m32'])
            env.Append(LINKFLAGS=['-m32'])
        elif arch == 'x86_64':
            env.Append(CCFLAGS=['-m64'])
            env.Append(LINKFLAGS=['-m64'])

    # ========== COMMON ==========
    if is_debug:
        env.Append(CPPDEFINES=['GODOT_SCHEMA_DEBUG'])
        if use_cpp20:
            env.Append(CPPDEFINES=['TESTS_ENABLED'])

    env.Append(CPPPATH=['src'])
    return env

def get_build_path(env):
    """Create a unique build path based on platform, target, and architecture"""
    platform = env.get('platform', 'unknown')
    target = env.get('target', 'unknown')
    arch = env.get('arch', 'unknown')

    # Create a unique directory name
    build_path = os.path.join('build', f"{platform}_{target}_{arch}")
    return os.path.abspath(build_path)

def build_config(env, variant_dir):
    # Set up variant dir for our sources
    env.VariantDir(os.path.join(variant_dir, 'src'), 'src', duplicate=0)

    # Gather source files
    sources = Glob(os.path.join(variant_dir, 'src', '*.cpp'))
    sources += Glob(os.path.join(variant_dir, 'src', 'extension', '*.cpp'))
    sources += Glob(os.path.join(variant_dir, 'src', 'rule', '*.cpp'))
    sources += Glob(os.path.join(variant_dir, 'src', 'selector', '*.cpp'))

    # Add test sources only for debug builds with C++20 support
    # This prevents compilation errors on C++17 platforms
    # if env["target"] == "template_debug" and env.get('USE_CPP20', False):
    #     sources += Glob(os.path.join(variant_dir, 'src', 'tests', '*.cpp'))
    #     print(f"Including {len(Glob(os.path.join(variant_dir, 'src', 'tests', '*.cpp')))} test files")

    # Embed documentation
    if env["target"] in ["editor", "template_debug"]:
        try:
            doc_data = env.GodotCPPDocData("src/gen/doc_data.gen.cpp", source=Glob("doc_classes/*.xml"))
            sources.append(doc_data)
        except AttributeError:
            print("Not including class reference as we're targeting a pre-4.3 baseline.")

    # Set up output directories
    output_lib_dir = os.path.join(variant_dir, 'lib')
    if not os.path.exists(output_lib_dir):
        os.makedirs(output_lib_dir)

    env.Append(LIBPATH=[output_lib_dir])

    # Build the library
    library = env.SharedLibrary(
        target=os.path.join(output_lib_dir, get_library_name(env)),
        source=sources
    )

    # Install the built library to the bin directory
    bin_dir = os.path.join('project', 'addons', 'GDSchema', 'bin')
    if not os.path.exists(bin_dir):
        os.makedirs(bin_dir)

    # Handle installation differently for Windows vs other platforms
    platform = env.get('platform', '')
    if platform == 'windows':
        # For Windows, install all generated files that match our library name pattern
        lib_base_name = get_library_name(env).replace('.dll', '')

        # Create a custom install action that uses glob at build time
        def install_windows_files(target, source, env):
            # Convert SCons File objects to strings and use glob to find all related files
            lib_pattern_str = os.path.join(output_lib_dir, f"{lib_base_name}.dll")
            generated_files = glob.glob(lib_pattern_str)

            installed_files = []
            for file_path in generated_files:
                if os.path.exists(file_path):
                    dest_path = os.path.join(bin_dir, os.path.basename(file_path))
                    shutil.copy2(file_path, dest_path)
                    installed_files.append(dest_path)
                    print(f"Installed: {dest_path}")
                else:
                    print(f"Warning: {file_path} not found, skipping")
            return None

        # Create a dummy target for the install action
        install_target = os.path.join(bin_dir, get_library_name(env))
        installed_lib = env.Command(install_target, library, install_windows_files)
    else:
        # For non-Windows platforms, use the standard install
        installed_lib = env.Install(bin_dir, library)

    env.Alias('install', installed_lib)
    return library, installed_lib

# Setup the build environment
env = setup_build_env(base_env)

# Establish the variant directory based on platform, target, and architecture
variant_dir = get_build_path(env)
if not os.path.exists(variant_dir):
    os.makedirs(variant_dir)

# OBJPREFIX places object files in the variant directory
env['OBJPREFIX'] = os.path.join(variant_dir, '')

target = build_config(env, variant_dir)

Default(target)
