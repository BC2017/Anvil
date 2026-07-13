cmake_minimum_required(VERSION 3.28)

project(
    Anvil
    VERSION 0.1.0
    DESCRIPTION "A robust, cross-platform 3D game engine"
    LANGUAGES CXX
)

set(CMAKE_CXX_EXTENSIONS OFF)

include(CTest)
include(FetchContent)

option(ANVIL_BUILD_SANDBOX "Build the SDL3 sandbox application" ON)
option(ANVIL_BUILD_RENDERER "Build rendering modules" ON)
option(ANVIL_BUILD_EDITOR "Build editor modules" ON)
option(ANVIL_ENABLE_ASAN "Enable AddressSanitizer where supported" OFF)
option(ANVIL_ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer where supported" OFF)
option(ANVIL_ENABLE_CLANG_TIDY "Run clang-tidy while compiling" OFF)
option(ANVIL_WARNINGS_AS_ERRORS "Treat Anvil compiler warnings as errors" ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(AnvilCompilerWarnings)
include(AnvilSanitizers)
include(AnvilStaticAnalysis)

add_library(anvil_project_options INTERFACE)
target_compile_features(anvil_project_options INTERFACE cxx_std_23)

add_library(anvil_project_warnings INTERFACE)
anvil_set_project_warnings(anvil_project_warnings)
anvil_enable_sanitizers(anvil_project_options)

add_subdirectory(engine/core)
add_subdirectory(engine/assets)
add_subdirectory(engine/gameplay)

if(ANVIL_BUILD_RENDERER)
    find_package(Vulkan 1.3 REQUIRED)
    add_subdirectory(engine/render)
endif()

if(ANVIL_BUILD_EDITOR)
    add_subdirectory(tools/editor)
endif()

if(ANVIL_BUILD_SANDBOX)
    find_package(SDL3 CONFIG QUIET)
    if(NOT SDL3_FOUND)
        message(STATUS "SDL3 was not found locally; fetching pinned release-3.4.12")
        set(SDL_SHARED OFF CACHE BOOL "Build SDL as a shared library" FORCE)
        set(SDL_STATIC ON CACHE BOOL "Build SDL as a static library" FORCE)
        set(SDL_TEST_LIBRARY OFF CACHE BOOL "Build the SDL test library" FORCE)
        set(SDL_TESTS OFF CACHE BOOL "Build SDL tests" FORCE)
        set(SDL_EXAMPLES OFF CACHE BOOL "Build SDL examples" FORCE)
        FetchContent_Declare(
            SDL3
            GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
            GIT_TAG release-3.4.12
            GIT_SHALLOW TRUE
            EXCLUDE_FROM_ALL
        )
        FetchContent_MakeAvailable(SDL3)
    endif()

    add_subdirectory(engine/platform)
    add_subdirectory(apps/sandbox)
endif()

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

if(TARGET anvil_sandbox)
    include(AnvilPackaging)
endif()
