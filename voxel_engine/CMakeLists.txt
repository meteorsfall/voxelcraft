cmake_minimum_required(VERSION 3.3)
project(VoxelCraft)

# Set C++17 requirement
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED on)

# Create custom_cache directory for various cmake cache reasons
set(CUSTOM_CACHE ${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/custom_cache)
# Make the directory if it doesn't exist yet
file(MAKE_DIRECTORY ${CUSTOM_CACHE})

# Print build type, set target build directory
if (CMAKE_BUILD_TYPE MATCHES Release)
  message(STATUS "Release Mode")
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build/release")
elseif (CMAKE_BUILD_TYPE MATCHES Debug)
  message(STATUS "Debug Mode")
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build/debug")
else ()
  message(STATUS "Can't compile, No CMAKE_BUILD_TYPE given")
  return ()
endif ()

# Keep track of the precompiled gch file
# If src/includes.hpp or something it includes changes,
# the precompiled header must be recreated
# This is done by regenerating the dummy file precompiled.h.gch,
# which will trigger a rebuild of the `voxelcraft` target
add_custom_command(OUTPUT "${CUSTOM_CACHE}/precompiled.h.gch"
  COMMENT "Precompiled src/includes.hpp header out-of-date, recompiling..."
  COMMAND ${CMAKE_COMMAND} -E touch "${CUSTOM_CACHE}/precompiled.h.gch"
  DEPENDS "src/includes.hpp"
  IMPLICIT_DEPENDS CXX "src/includes.hpp"
)

file(GLOB_RECURSE voxelscript_files CONFIGURE_DEPENDS "mods/*.vs")
file(GLOB_RECURSE asset_files CONFIGURE_DEPENDS "assets/*")
file(GLOB_RECURSE bin_files CONFIGURE_DEPENDS "bin/*")

if (WIN32)
add_custom_command(OUTPUT "${CUSTOM_CACHE}/main.wasm"
  COMMENT "Compiling VoxelScript to WASM"
  COMMAND wsl -- /bin/bash -i -c "\"voxelc \\$$(wslpath \\\"${CMAKE_SOURCE_DIR}/mods/main_mod\\\") -o \\$$(wslpath \\\"${CUSTOM_CACHE}/main.wasm\\\")\""
  DEPENDS ${voxelscript_files}
)
else()
add_custom_command(OUTPUT "${CUSTOM_CACHE}/main.wasm"
  COMMENT "Compiling VoxelScript to WASM"
  COMMAND voxelc "${CMAKE_SOURCE_DIR}/mods/main_mod" -o "${CUSTOM_CACHE}/main.wasm"
  DEPENDS ${voxelscript_files}
)
endif ()

# Custom command for copying build files
add_custom_command(OUTPUT "${CUSTOM_CACHE}/build_files"
  COMMENT "Copying build files into build folder"
  COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/bin" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/assets" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets"
  COMMAND ${CMAKE_COMMAND} -E copy "${CUSTOM_CACHE}/main.wasm" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/mods/main.wasm"
  COMMAND ${CMAKE_COMMAND} -E touch "${CUSTOM_CACHE}/build_files"
  DEPENDS "${CUSTOM_CACHE}/main.wasm"
          ${asset_files}
          ${bin_files}
)

# Add build
add_custom_target(voxelcraft
  COMMENT "Creating the executable in ${CMAKE_BUILD_TYPE} mode"
  COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target vc --parallel 4
  DEPENDS "${CUSTOM_CACHE}/precompiled.h.gch"
          "${CUSTOM_CACHE}/build_files"
)

if (WIN32)
  # Add cl.exe commandline options
  add_compile_options(/WX /wd4305 /wd4244 /wd4267 /wd4996 /we4457 /DSCITER_LITE /DUNICODE)
  # Link against MDd, not MD, WAVM seems to crash if this isn't done
  string(REPLACE "/MDd" "/MD" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
  # Create .pdb files
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Zi")
  # /GL for global optimization, /O2 for optimization level
  # /GS- to remove unneeded stack overflow nonce check
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /GL /O2 /GS-")
  if (CMAKE_BUILD_TYPE MATCHES Release)
    # Link-time optimization (Cross-file)
    add_link_options(/LTCG)
  endif ()
else ()
  # Add gcc commandline options
  add_compile_options(-Wshadow=local -Wall -Wextra -pedantic -Wno-unused-variable -Wno-unused-but-set-variable -Wno-unused-function -Werror -Wfatal-errors -DSCITER_LITE)
  add_link_options(-rdynamic)
  set(CMAKE_CXX_FLAGS_DEBUG "-Og")
  set(CMAKE_CXX_FLAGS_RELEASE "-Ofast -flto -g")
endif ()

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

  # Find Bfd [Optional]
  find_library(LIBBFD_LIB NAMES bfd)
  if(LIBBFD_LIB)
    message(STATUS "Found Libbfd")
    add_definitions(-DBACKWARD_HAS_BFD=1)
    link_libraries(bfd)
  else()
    message(STATUS "Not Found Libbfd")
  endif()
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
file(GLOB_RECURSE voxelcraft_cpp_files CONFIGURE_DEPENDS "src/*.cpp" "extras/*.cpp")
add_executable(vc ${voxelcraft_cpp_files})
target_compile_definitions(vc PUBLIC "WAVM_API=")

# Precompile includes.hpp header file
target_precompile_headers(vc PRIVATE src/includes.hpp)

# Link libraries to the executable
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
    COMMAND ${CMAKE_COMMAND} -E chdir "${CMAKE_SOURCE_DIR}" doxygen
  )
endif ()

# Define pixel_helper executable
add_executable(pixel_helper pixel_helper_src/main.cpp)
target_link_libraries(pixel_helper)
