cmake_minimum_required(VERSION 3.3)
project(LearnGLSL)

set(CMAKE_CXX_STANDARD 11) # no explicit compiler flags if possible

if (NOT WIN32)
set(CMAKE_MODULE_PATH /usr/local/lib/cmake /usr/local/lib/x86_64-linux-gnu/cmake)
set(CMAKE_PREFIX_PATH /usr/local/lib/cmake/glfw)
endif (NOT WIN32)

# don't read CMAKE_BUILD_TYPE, it has no meaning with multiconfig
# generators
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/build/debug")

# PROJECT_BINARY_DIR should not be set at all
# You establish the BINARY_DIR with the initial cmake command
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

# OpenGL
find_package(OpenGL REQUIRED)

# GLM
find_package(glm REQUIRED)
message(STATUS "GLM included at ${GLM_INCLUDE_DIR}")

# GLFW
find_package(glfw3 REQUIRED)
message(STATUS "Found GLFW3 in ${GLFW3_INCLUDE_DIR}")

include_directories(${OPENGL_INCLUDE_DIR})

add_executable(voxelscript src/main.cpp src/utils.cpp src/player.cpp src/world.cpp)
target_link_libraries(voxelscript ${OPENGL_LIBRARY} glfw GLEW ${GLFW_LIBRARIES})
