cmake_minimum_required(VERSION 3.3)
project(VoxelCraft)

# Will compile to release by default
set(default_build_type "Release")

# Create custom_cache directory for precompiled.h.gch
set(CUSTOM_CACHE ${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/custom_cache)
# Make the directory if it doesn't exist yet
file(MAKE_DIRECTORY ${CUSTOM_CACHE})

# Keep track of the precompile gch cache
# If src/includes.hpp changes, the precompiled header must be recreated
# This is done be restarting the build from the beginning.
# precompile.h.gch is just a dummy file with nothing in it
ADD_CUSTOM_COMMAND(OUTPUT "${CUSTOM_CACHE}/precompiled.h.gch"
  COMMAND ${CMAKE_COMMAND} -E echo "Compiled GCH!" > "${CUSTOM_CACHE}/precompiled.h.gch"
  DEPENDS "src/includes.hpp"
  IMPLICIT_DEPENDS CXX "src/includes.hpp"
  COMMENT "Checking precompiled src/includes.hpp"
)

file(GLOB_RECURSE wasm_sources CONFIGURE_DEPENDS "mods/*.vs")

if (WIN32)
ADD_CUSTOM_COMMAND(OUTPUT "${CUSTOM_CACHE}/main.wasm"
  COMMAND ${CMAKE_COMMAND} -E echo "Must compile WASM!" > "${CUSTOM_CACHE}/main.wasm"
  DEPENDS ${wasm_sources}
  COMMENT "Checking wasm Compilation"
)
else()
ADD_CUSTOM_COMMAND(OUTPUT "${CUSTOM_CACHE}/main.wasm"
  COMMAND voxelc ./mods/main_mod -o "${CUSTOM_CACHE}/main.wasm"
  DEPENDS ${wasm_sources}
  COMMENT "Checking wasm compilation"
)
endif ()

# Add release build
ADD_CUSTOM_TARGET(release
  COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Release ${CMAKE_SOURCE_DIR}
  COMMAND ${CMAKE_COMMAND} --build ${CMAKE_SOURCE_DIR} --target all --parallel 4
  DEPENDS "${CUSTOM_CACHE}/precompiled.h.gch"
          "${CUSTOM_CACHE}/main.wasm"
  COMMENT "Creating the executable in release mode."
)

# Add debug build
ADD_CUSTOM_TARGET(debug
  COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Debug ${CMAKE_SOURCE_DIR}
  COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target all --parallel 4
  DEPENDS "${CUSTOM_CACHE}/precompiled.h.gch"
          "${CUSTOM_CACHE}/main.wasm"
  COMMENT "Creating the executable in debug mode."
)

# Print build type
if (CMAKE_BUILD_TYPE MATCHES Release)
  message(STATUS "Release Mode")
elseif (CMAKE_BUILD_TYPE MATCHES Debug)
  message(STATUS "Debug Mode")
else ()
  message(STATUS "No target given")
  return ()
endif ()

# Set requirement for C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED on)

if (WIN32)
  # Add cl.exe commandline options
  add_compile_options(/WX /wd4305 /wd4244 /wd4267 /wd4996 /we4457)
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Zi")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /GL /O2 /GS-")
  if (CMAKE_BUILD_TYPE MATCHES Release)
    add_link_options(/LTCG)
  endif ()
else ()
  # Add gcc commandline options
  add_compile_options(-Wshadow=local -Wall -Wextra -pedantic -Wno-unused-variable -Wno-unused-but-set-variable -Wno-unused-function -Werror -Wfatal-errors -DSCITER_LITE)
  add_link_options(-rdynamic)
  set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
  set(CMAKE_CXX_FLAGS_RELEASE "-Ofast -flto -g")
endif ()

# don't read CMAKE_BUILD_TYPE, it has no meaning with multiconfig
# generators
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/build/debug")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/build/release")

# PROJECT_BINARY_DIR should not be set at all
# You establish the BINARY_DIR with the initial cmake command
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

# Find OpenGL
set(OpenGL_GL_PREFERENCE LEGACY)
find_package(OpenGL REQUIRED)
message(STATUS "Found OpenGL at ${OPENGL_LIBRARY}")

# Include from /include
include_directories(SYSTEM include)
if (WIN32)
# Link from /lib on windows (For static lib files)
link_directories(lib)
else ()
# And /bin on linux (For shared lib files)
link_directories(bin)
endif ()

if (WIN32)
else ()
  # Import packages on linux
  set(CMAKE_MODULE_PATH /usr/local/lib/cmake /usr/local/lib/x86_64-linux-gnu/cmake)
  set(CMAKE_PREFIX_PATH /usr/local/lib/cmake/glfw)

  # Find GLM
  find_package(glm REQUIRED)
  message(STATUS "Found GLM in ${GLM_INCLUDE_DIRS}")

  # Find GLFW
  find_package(glfw3 REQUIRED)
  message(STATUS "Found GLFW3 in ${GLFW3_LIBRARY}")

  # Find FreeType
  find_package(Freetype REQUIRED)
  message(STATUS "Found FreeType at ${FREETYPE_LIBRARIES}")
endif ()

# Add include directories.
include_directories(SYSTEM ${OPENGL_INCLUDE_DIR})
include_directories(SYSTEM ${FREETYPE_INCLUDE_DIRS})
include_directories(SYSTEM extras)

# Configure rpath
if (WIN32)
else ()
# Make linux look in current directory for .so files (Just like .dll's)
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH $ORIGIN)
endif ()

# Define VoxelCraft executable
add_executable(vc src/main.cpp src/modloader.cpp src/api.cpp src/mesh.cpp src/event.cpp src/model.cpp src/example/drop.cpp src/entity.cpp src/rigid_body.cpp src/utils.cpp src/universe.cpp src/example/player.cpp src/world.cpp src/camera.cpp src/input.cpp src/aabb.cpp src/block.cpp src/chunk.cpp src/megachunk.cpp src/texture.cpp src/UI.cpp src/gl_utils.cpp src/font.cpp src/texture_renderer.cpp src/texture_atlasser.cpp src/bmp.cpp src/example/main_ui.cpp src/example/main_game.cpp src/example/page_ui.cpp src/example/world_gen.cpp extras/zip.cpp)

# Precompile includes.hpp header file
target_precompile_headers(vc PRIVATE src/includes.hpp)

if (WIN32)
  # Link windows libraries to voxelcraft
  target_link_libraries(vc ${OPENGL_LIBRARY} glfw3 glew32 libWAVM noise freetype ws2_32 userenv)
else ()
  # Link linux system libraries to voxelcraft
  # sciter: needs dl
  target_link_libraries(vc ${OPENGL_LIBRARY} glfw GLEW noise ${FREETYPE_LIBRARIES} stdc++fs dl)
  # Link linux .so libraries to voxelcraft
  target_link_libraries(vc WAVM sciter)
  # Run doxygen after compiling on linux
  add_custom_command(TARGET vc POST_BUILD
    COMMAND doxygen
  )
endif ()
# Copy .dll/.so's into build directory
add_custom_command(TARGET vc POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  "${PROJECT_SOURCE_DIR}/bin"
  $<TARGET_FILE_DIR:vc>
)
# Copy mods into build directory
add_custom_command(TARGET vc POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:vc>/mods
  COMMAND ${CMAKE_COMMAND} -E copy
  "${CUSTOM_CACHE}/main.wasm"
  $<TARGET_FILE_DIR:vc>/mods/main.wasm
)
# Copy assets into build directory
add_custom_command(TARGET vc POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:vc>/mods
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  "assets"
  $<TARGET_FILE_DIR:vc>/assets
)

target_compile_definitions(vc PUBLIC "WAVM_API=")

# Define pixel_helper executable
add_executable(pixel_helper pixel_helper_src/main.cpp)
target_link_libraries(pixel_helper)
