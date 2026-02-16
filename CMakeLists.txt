cmake_minimum_required(VERSION 3.16)
project(LuauExecutor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ============================================================================
# Source files
# ============================================================================
set(EXECUTOR_SOURCES
    src/main.cpp
    src/LuauExecutor.cpp
    src/UNCFunctions.cpp
)

set(EXECUTOR_HEADERS
    include/LuauExecutor.h
    environment/LuauEnvironment.h
    offsets/InternalOffsets.h
    offsets/ExternalOffsets.h
)

# ============================================================================
# Main executable
# ============================================================================
add_executable(LuauExecutor ${EXECUTOR_SOURCES} ${EXECUTOR_HEADERS})

target_include_directories(LuauExecutor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/environment
    ${CMAKE_CURRENT_SOURCE_DIR}/offsets
)

# ============================================================================
# Luau library linkage
#
# Option A: Build Luau from source (recommended)
#   1. Clone https://github.com/luau-lang/luau into extern/luau
#   2. Uncomment the lines below
#
# Option B: Link against pre-built Luau libraries
#   1. Set LUAU_LIB_DIR to the directory containing the .a / .lib files
#   2. Set LUAU_INCLUDE_DIR to the Luau headers
# ============================================================================

# --- Option A: Luau from source ---
# add_subdirectory(extern/luau)
# target_link_libraries(LuauExecutor PRIVATE
#     Luau.Compiler
#     Luau.VM
#     Luau.Ast
# )
# target_include_directories(LuauExecutor PRIVATE
#     extern/luau/VM/include
#     extern/luau/Compiler/include
#     extern/luau/Ast/include
#     extern/luau/Common/include
# )

# --- Option B: Pre-built libraries ---
# set(LUAU_LIB_DIR "" CACHE PATH "Path to Luau libraries")
# set(LUAU_INCLUDE_DIR "" CACHE PATH "Path to Luau headers")
# target_include_directories(LuauExecutor PRIVATE ${LUAU_INCLUDE_DIR})
# target_link_libraries(LuauExecutor PRIVATE
#     ${LUAU_LIB_DIR}/libLuau.Compiler.a
#     ${LUAU_LIB_DIR}/libLuau.VM.a
#     ${LUAU_LIB_DIR}/libLuau.Ast.a
# )

# ============================================================================
# Platform-specific settings
# ============================================================================
if(WIN32)
    target_compile_definitions(LuauExecutor PRIVATE _CRT_SECURE_NO_WARNINGS)
    # For DLL injection builds:
    # add_library(LuauExecutorDLL SHARED ${EXECUTOR_SOURCES} ${EXECUTOR_HEADERS})
    # target_include_directories(LuauExecutorDLL PRIVATE ...)
    # target_link_libraries(LuauExecutorDLL PRIVATE ...)
endif()

if(UNIX AND NOT APPLE)
    target_link_libraries(LuauExecutor PRIVATE pthread dl)
endif()

# ============================================================================
# Compiler warnings
# ============================================================================
if(MSVC)
    target_compile_options(LuauExecutor PRIVATE /W4)
else()
    target_compile_options(LuauExecutor PRIVATE -Wall -Wextra -Wpedantic)
endif()
