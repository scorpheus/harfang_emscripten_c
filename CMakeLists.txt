cmake_minimum_required(VERSION 3.20)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)


# create project
project(harfang_emscripten_c CXX C)

include(GetPrerequisites)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR})

add_compile_definitions(CMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD})

if(MSVC)
	# Make Visual Studio report accurate C++ version
	# See: https://devblogs.microsoft.com/cppblog/msvc-now-correctly-reports-__cplusplus/
  # /Wall emits warnings about the C++ standard library
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc /GS- /Zc:__cplusplus")

	add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
	add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
else()
	# Ignore Visual Studio pragma regions
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unknown-pragmas")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} --coverage -Og")
endif(MSVC)

# debug to list every path cmake check for find_library, very useful
#set(CMAKE_FIND_DEBUG_MODE TRUE)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# add harfang sdk
set(HG_SRC_DIR "../harfang3d_github" CACHE PATH "Path to the Harfang source directory")

if(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
	set(HG_BUILD_HARFANG_STATIC ON)
	set(HG_USE_GLFW OFF)
	set(HG_BUILD_GLTF_EXPORTER OFF)
	set(HG_BUILD_GLTF_IMPORTER OFF)
	set(HG_BUILD_SPHERICAL_HARMONICS_EXTRACTOR OFF)
	set(HG_BUILD_ASSETC OFF)
	set(HG_ENABLE_OPENVR_API OFF)
	set(HG_ENABLE_OPENXR_API OFF)
	set(HG_ENABLE_BULLET3_SCENE_PHYSICS OFF)
elseif(UNIX)
	set(HG_BUILD_GLTF_EXPORTER ON)
	set(HG_BUILD_GLTF_IMPORTER ON)
	set(HG_BUILD_SPHERICAL_HARMONICS_EXTRACTOR ON)
	set(HG_BUILD_ASSETC ON)
	set(HG_ENABLE_OPENVR_API OFF)
	set(HG_ENABLE_OPENXR_API OFF)
	set(HG_ENABLE_BULLET3_SCENE_PHYSICS ON)
else()
	set(HG_BUILD_GLTF_EXPORTER ON)
	set(HG_BUILD_GLTF_IMPORTER ON)
	set(HG_BUILD_SPHERICAL_HARMONICS_EXTRACTOR ON)
	set(HG_BUILD_ASSETC ON)
	set(HG_ENABLE_OPENVR_API ON)
	set(HG_ENABLE_OPENXR_API ON)
	set(HG_ENABLE_BULLET3_SCENE_PHYSICS ON)
endif()

set(HG_BUILD_CPP_SDK OFF)
set(HG_BUILD_ASSIMP_CONVERTER OFF)
set(HG_BUILD_FBX_CONVERTER OFF)
set(HG_BUILD_HG_PYTHON OFF)
set(HG_BUILD_TESTS OFF)
	
add_subdirectory(${HG_SRC_DIR} hg EXCLUDE_FROM_ALL)
	
set(HG_ASSETC_PATH ${CMAKE_INSTALL_PREFIX}/assetc)
set(HG_GLTF_IMPORTER_PATH ${CMAKE_INSTALL_PREFIX}/gltf_importer)

add_library(hg::engine ALIAS engine)
add_library(hg::foundation ALIAS foundation)
add_library(hg::platform ALIAS platform)

include_directories(${HG_SRC_DIR}/extern)

link_libraries(hg::engine hg::foundation hg::platform)

if(NOT APPLE)
	set(CMAKE_BUILD_RPATH_USE_ORIGIN TRUE)
	set(CMAKE_SKIP_BUILD_RPATH FALSE)
	set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
	set(CMAKE_INSTALL_RPATH $ORIGIN)
endif()

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
	set(HG_ENABLE_BULLET3_SCENE_PHYSICS ON)
	add_compile_definitions(HG_ENABLE_SCENE_PHYSICS HG_ENABLE_BULLET3_SCENE_PHYSICS)
endif()


# emscripten
if(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
	set(CONAN_DISABLE_CHECK_COMPILER ON)
	message(STATUS "Emscripten sets")
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -DEMSCRIPTEN -s MODULARIZE=1 -s EXPORT_NAME='Harfang' -s 'EXPORTED_RUNTIME_METHODS=[\"FS\"]' -std=c++20 -s MAX_WEBGL_VERSION=2 -s USE_SDL=2 -s USE_WEBGL2=1 -s FULL_ES3=1 -s ALLOW_MEMORY_GROWTH=1 -s OFFSCREENCANVAS_SUPPORT=1 -s LLD_REPORT_UNDEFINED -s DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR=1 -s FORCE_FILESYSTEM=1 -DBGFX_CONFIG_RENDERER_OPENGLES_MIN_VERSION=30")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -DEMSCRIPTEN -s MODULARIZE=1 -s EXPORT_NAME='Harfang' -s 'EXPORTED_RUNTIME_METHODS=[\"FS\"]' -std=c++20 -s MAX_WEBGL_VERSION=2 -s USE_SDL=2 -s USE_WEBGL2=1 -s FULL_ES3=1 -s ALLOW_MEMORY_GROWTH=1 -s OFFSCREENCANVAS_SUPPORT=1 -s LLD_REPORT_UNDEFINED -s DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR=1 -s FORCE_FILESYSTEM=1 -DBGFX_CONFIG_RENDERER_OPENGLES_MIN_VERSION=30")

	# without pthread and with wasm
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -s WASM=1 -s WASM_BIGINT=1 -Oz -DNDEBUG -s DISABLE_EXCEPTION_CATCHING=0")
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS} -s WASM=1 -s WASM_BIGINT=1 -Oz -DNDEBUG -s DISABLE_EXCEPTION_CATCHING=0")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS}  -s WASM=1 -s WASM_BIGINT=1 -O0 -DNDEBUG -gsource-map -s ASSERTIONS=2 -s DEMANGLE_SUPPORT=1")
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS}  --s WASM=1 -s WASM_BIGINT=1 -O0 -DNDEBUG -gsource-map -s ASSERTIONS=2 -s DEMANGLE_SUPPORT=1")
endif()

# create global lib to be sure to have a big lib above all to contain all preprocessor definition
add_library(global_lib INTERFACE)

# create ἔχιδνα project
add_executable(harfang_emscripten_c 
					app.cpp)
target_link_libraries(harfang_emscripten_c global_lib)
target_compile_definitions( harfang_emscripten_c PRIVATE BGFX_CONFIG_RENDERER_OPENGLES_MIN_VERSION=30 )

if(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
	set_target_properties(harfang_emscripten_c PROPERTIES SUFFIX ".js")
	#// "assetc\\assetc.exe" -api GLES -quiet -t assetc\\toolchains\\host-windows-x64-target-windows-x64 project project_compiled -platform linux
	#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}		--preload-file \"${CMAKE_SOURCE_DIR}/project_compiled@project_compiled/\"")
	#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}			--preload-file \"${CMAKE_SOURCE_DIR}/project_compiled@project_compiled/\"")

	set_target_properties(harfang_emscripten_c PROPERTIES LINK_FLAGS "--bind")
else()
endif()
