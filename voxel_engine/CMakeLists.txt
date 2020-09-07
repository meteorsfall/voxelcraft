cmake_minimum_required(VERSION 3.3)
project(VoxelCraft)

set(default_build_type "Release")

set(CUSTOM_CACHE ${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/custom_cache)
file(MAKE_DIRECTORY ${CUSTOM_CACHE})

ADD_CUSTOM_COMMAND(OUTPUT "precompiled.h.gch"
  COMMAND ${CMAKE_COMMAND} -E echo "Compiled GCH!" > "${CUSTOM_CACHE}/precompiled.h.gch"
  DEPENDS "src/includes.hpp"
  IMPLICIT_DEPENDS CXX "src/includes.hpp"
  COMMENT "Checking GCH Compilation"
)

message(STATUS "Test ${CUSTOM_CACHE}")

ADD_CUSTOM_TARGET(release
  COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Release ${CMAKE_SOURCE_DIR}
  COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target all
  DEPENDS "precompiled.h.gch"
  COMMENT "Creating the executable in release mode."
)

ADD_CUSTOM_TARGET(debug
  COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Debug ${CMAKE_SOURCE_DIR}
  COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target all
  DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/precompiled.h.gch"
  COMMENT "Creating the executable in debug mode."
)

if (CMAKE_BUILD_TYPE MATCHES Release)
  message(STATUS "Release Mode")
elseif (CMAKE_BUILD_TYPE MATCHES Debug)
  message(STATUS "Debug Mode")
endif ()

set(CMAKE_CXX_STANDARD 17) # no explicit compiler flags if possible
set(CMAKE_CXX_STANDARD_REQUIRED on)

if (WIN32)
  add_compile_options(/WX /wd4305 /wd4244 /wd4267 /wd4996 /we4457)
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Zi")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /GL /O2 /GS-")
  if (CMAKE_BUILD_TYPE MATCHES Release)
    add_link_options(/LTCG)
  endif ()
else ()
  add_compile_options(-Wshadow=local -Wall -Wextra -pedantic -Wno-unused-variable -Wno-unused-but-set-variable -Werror -Wfatal-errors)
  add_link_options(-rdynamic)
  set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
  set(CMAKE_CXX_FLAGS_RELEASE "-Ofast")
endif ()

# don't read CMAKE_BUILD_TYPE, it has no meaning with multiconfig
# generators
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/build/debug")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/build/release")

# PROJECT_BINARY_DIR should not be set at all
# You establish the BINARY_DIR with the initial cmake command
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

# OpenGL
set(OpenGL_GL_PREFERENCE LEGACY)
find_package(OpenGL REQUIRED)
message(STATUS "Found OpenGL at ${OPENGL_LIBRARY}")

if (WIN32)
  include_directories(SYSTEM include)
  link_directories(lib)
else ()
  set(CMAKE_MODULE_PATH /usr/local/lib/cmake /usr/local/lib/x86_64-linux-gnu/cmake)
  set(CMAKE_PREFIX_PATH /usr/local/lib/cmake/glfw)

  # GLM
  find_package(glm REQUIRED)
  message(STATUS "Found GLM in ${GLM_INCLUDE_DIRS}")

  # GLFW
  find_package(glfw3 REQUIRED)
  message(STATUS "Found GLFW3 in ${GLFW3_LIBRARY}")

  # FreeType
  find_package(Freetype REQUIRED)
  message(STATUS "Found FreeType at ${FREETYPE_LIBRARIES}")
endif ()

include_directories(SYSTEM ${OPENGL_INCLUDE_DIR})
include_directories(SYSTEM ${FREETYPE_INCLUDE_DIRS})
include_directories(SYSTEM extras)

# Define VoxelCraft executable
add_executable(vc src/main.cpp src/mesh.cpp src/entity.cpp src/rigid_body.cpp src/utils.cpp src/universe.cpp src/player.cpp src/world.cpp src/camera.cpp src/input.cpp src/aabb.cpp src/block.cpp src/chunk.cpp src/megachunk.cpp src/texture.cpp src/UI.cpp src/gl_utils.cpp src/font.cpp src/texture_renderer.cpp src/texture_atlasser.cpp src/bmp.cpp src/example/main_ui.cpp src/example/main_game.cpp src/example/page_ui.cpp src/example/world_gen.cpp extras/zip.cpp)

target_precompile_headers(vc PRIVATE src/includes.hpp)

if (WIN32)
  target_link_libraries(vc ${OPENGL_LIBRARY} glfw3 glew32 noise freetype)
  # Copy .dll's into build directory
  add_custom_command(TARGET vc POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_directory
      "${PROJECT_SOURCE_DIR}/bin"
      $<TARGET_FILE_DIR:vc>)
else ()
  target_link_libraries(vc ${OPENGL_LIBRARY} glfw GLEW noise ${FREETYPE_LIBRARIES})
  add_custom_command(TARGET vc POST_BUILD
      COMMAND doxygen &>/dev/null)
endif ()

# Define pixel_helper executable
add_executable(pixel_helper pixel_helper_src/main.cpp)
target_link_libraries(pixel_helper)
