cmake_minimum_required(VERSION 3.3)
project(VoxelCraft)

set(default_build_type "Release")

ADD_CUSTOM_TARGET(debug
  COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Debug ${CMAKE_SOURCE_DIR}
  COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target all
  COMMENT "Creating the executable in the debug mode."
)

ADD_CUSTOM_TARGET(release
  COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Release ${CMAKE_SOURCE_DIR}
  COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target all
  COMMENT "Creating the executable in the release mode."
)

set(CMAKE_CXX_STANDARD 17) # no explicit compiler flags if possible
set(CMAKE_CXX_STANDARD_REQUIRED on)

if (WIN32)
add_compile_options(/WX /wd4305 /wd4244 /wd4267 /wd4996)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Od")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2")
else ()
add_compile_options(-Wall -Wextra -pedantic -Werror -Wfatal-errors)
set(CMAKE_CXX_FLAGS_DEBUG "-pg -O0 -g")
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
message(STATUS "Including windows /include and /lib directories!")
message(STATUS "Including windows ${CMAKE_SOURCE_DIR}/include and /lib directories!")
include_directories(include)
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

include_directories(${OPENGL_INCLUDE_DIR})
include_directories(${FREETYPE_INCLUDE_DIRS})

add_executable(pixel_helper pixel_helper_src/main.cpp)
target_link_libraries(pixel_helper)

add_executable(vc src/main.cpp src/utils.cpp src/player.cpp src/world.cpp src/camera.cpp src/input.cpp src/aabb.cpp src/block.cpp src/chunk.cpp src/texture.cpp src/UI.cpp src/gl_utils.cpp src/font.cpp src/texture_renderer.cpp src/example/main_ui.cpp src/example/main_game.cpp)

if (WIN32)
target_link_libraries(vc ${OPENGL_LIBRARY} glfw3 glew32 freetype)
add_custom_command(TARGET vc POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${PROJECT_SOURCE_DIR}/bin"
    $<TARGET_FILE_DIR:vc>)
else ()
target_link_libraries(vc ${OPENGL_LIBRARY} glfw GLEW ${FREETYPE_LIBRARIES})
endif ()
