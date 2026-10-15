# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT license.

cmake_minimum_required(VERSION 3.25)

###################################################
# Project Kuku includes the following components: #
#   1. Kuku C++ library                           #
#   2. Kuku C export library                      #
#   3. Kuku C++ examples                          #
#   4. Kuku C++ tests                             #
###################################################

# Conditionally enable the vcpkg "tests" feature when building tests. This must be declared
# before project(), because that is when vcpkg performs the manifest install.
option(KUKU_BUILD_TESTS "Build C++ tests for Kuku" OFF)
if(KUKU_BUILD_TESTS)
    list(APPEND VCPKG_MANIFEST_FEATURES "tests")
endif()

project(Kuku VERSION 4.0.0 LANGUAGES CXX C)

# [option] CMAKE_BUILD_TYPE (default: "Release")
# Build in one of the following modes: Release, Debug, MinSizeRel, or RelWithDebInfo.
#
# CMAKE_CONFIGURATION_TYPES is non-empty only for multi-configuration generators, which pick
# the configuration at build time; it is populated by project(), hence the placement here.
# Without a default the compiler gets no optimization level and no NDEBUG.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
        "Release" "Debug" "MinSizeRel" "RelWithDebInfo")
endif()

if(CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Multi-config generator detected. Configurations: ${CMAKE_CONFIGURATION_TYPES}")
else()
    message(STATUS "Build type (CMAKE_BUILD_TYPE): ${CMAKE_BUILD_TYPE}")
endif()

########################
# Global configuration #
########################

include(CheckTypeSize)
include(CMakeDependentOption)

# Extra modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

# Always build position-independent-code
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Give Debug artifacts a distinct name so that a Debug and a Release build can be installed
# into the same prefix without overwriting each other.
set(CMAKE_DEBUG_POSTFIX "d")

# Make the install target depend on the all target (required by vcpkg)
set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY OFF)

# Enable security hardening compile and link flags (cross-platform).
# Applied as directory-scope options so they do NOT propagate to downstream
# consumers via the install/export interface.
option(KUKU_ENABLE_HARDENING "Enable security hardening compile and link flags." ON)

if(KUKU_ENABLE_HARDENING)
    if(MSVC)
        include(CheckCXXCompilerFlag)
        include(CheckCXXSourceCompiles)

        # Control Flow Guard.
        add_compile_options(/guard:cf)
        add_link_options(/guard:cf)

        # Spectre variant 1 mitigations. Probed rather than assumed: /Qspectre needs the
        # "Spectre-mitigated libs" toolset component, and when it is missing the compiler
        # only emits command-line warning D9002 and carries on, which is easy to miss in a
        # build log and would silently leave the mitigation off.
        check_cxx_compiler_flag(/Qspectre KUKU_HAS_QSPECTRE)
        if(KUKU_HAS_QSPECTRE)
            add_compile_options(/Qspectre)
        endif()

        # The next two need the compiler's own target macros: the generator platform
        # variables are not reliably set under every generator, and MSVC's linker accepts
        # essentially any flag, so a link-flag probe cannot gate them either. Each probe
        # names the targets it wants and rejects everything else.

        # EH continuation metadata, available on 64-bit targets, x64 and ARM64 alike.
        # Worth having here because the C export library is an exception boundary.
        check_cxx_source_compiles("
            #if defined(_M_X64) || defined(_M_ARM64) || defined(_M_ARM64EC)
            int main() { return 0; }
            #else
            #error Not a 64-bit target.
            #endif
            " KUKU_MSVC_TARGET_IS_64BIT)

        if(KUKU_MSVC_TARGET_IS_64BIT)
            check_cxx_compiler_flag(/guard:ehcont KUKU_HAS_GUARD_EHCONT)
            if(KUKU_HAS_GUARD_EHCONT)
                # Applied per target, never at directory scope, and never to the tests. The
                # linker can only emit EH continuation metadata when every module in the link
                # was compiled with the flag, and under the vcpkg *-static-md triplets
                # GoogleTest is a static library that was not, so the test binaries fail to
                # link. Setting only the compile option does not avoid this: the Visual Studio
                # generator turns it into GuardEHContMetadata, which MSBuild carries to the
                # link step by itself. See kuku_set_ehcont() below.
                set(KUKU_EHCONT_OPTION /guard:ehcont)
            endif()
        endif()

        # Shadow-stack (CET) marking applies to x64 only. ARM64EC has to be excluded by
        # name because it defines _M_X64 as well, being x64-compatible by design.
        check_cxx_source_compiles("
            #if defined(_M_X64) && !defined(_M_ARM64EC)
            int main() { return 0; }
            #else
            #error Not an x64 target.
            #endif
            " KUKU_MSVC_TARGET_IS_X64)

        if(KUKU_MSVC_TARGET_IS_X64)
            add_link_options(/CETCOMPAT)
        endif()

        # Randomized base address; debug symbols help static analysis tools.
        add_link_options(/DYNAMICBASE /DEBUG)
    else()
        # Gate the compile flags to clang/gcc-driven languages so they can never reach
        # another compile driver, such as Swift's on Apple targets. Kuku compiles C - the
        # vendored BLAKE2 sources - as well as C++, so the gate has to name both, and the
        # flags below then apply to BLAKE2 too rather than silently skipping it.
        set(_kuku_c_family_langs "$<COMPILE_LANGUAGE:C,CXX,OBJC,OBJCXX>")

        # Stack canaries.
        add_compile_options($<${_kuku_c_family_langs}:-fstack-protector-strong>)

        # Stack-clash protection (no-op on Apple Clang; gated for older GCC/Clang versions
        # that don't support the flag). Probed with the C compiler, not the C++ one,
        # because the flag is applied to the C sources as well.
        if(NOT APPLE)
            include(CheckCCompilerFlag)
            check_c_compiler_flag(-fstack-clash-protection KUKU_HAS_STACK_CLASH_PROTECTION)
            if(KUKU_HAS_STACK_CLASH_PROTECTION)
                add_compile_options($<${_kuku_c_family_langs}:-fstack-clash-protection>)
            endif()
        endif()

        # Standard library hardening (bounds-checked containers, etc.).
        add_compile_definitions(
            $<$<AND:$<NOT:$<CONFIG:Debug>>,${_kuku_c_family_langs}>:_GLIBCXX_ASSERTIONS>
            $<$<AND:$<NOT:$<CONFIG:Debug>>,${_kuku_c_family_langs}>:_LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_FAST>
        )

        # Full RELRO, immediate binding, non-executable stack (ELF only).
        if(CMAKE_SYSTEM_NAME STREQUAL "Linux" OR CMAKE_SYSTEM_NAME STREQUAL "Android")
            add_link_options(-Wl,-z,relro -Wl,-z,now -Wl,-z,noexecstack)
        endif()

        # Fortified libc functions (requires -O1+, skip on Apple).
        if(NOT APPLE)
            add_compile_options($<$<AND:$<NOT:$<CONFIG:Debug>>,${_kuku_c_family_langs}>:-U_FORTIFY_SOURCE>)
            add_compile_definitions($<$<AND:$<NOT:$<CONFIG:Debug>>,${_kuku_c_family_langs}>:_FORTIFY_SOURCE=2>)
        endif()
    endif()
endif()

###########################
# Compiler warning policy #
###########################

# Kuku's correctness rests on uint32/uint64 index arithmetic, which is the class of bug
# -Wconversion catches, so warnings are enabled in every configuration rather than in Debug
# only. The whole codebase, including the vendored BLAKE2 C sources, is clean under this set.
#
# Warnings are not errors by default: a consumer building Kuku with a newer toolchain that
# emits a brand-new diagnostic must still get a working build. Pass the standard
# -DCMAKE_COMPILE_WARNING_AS_ERROR=ON to make them errors, as the CI pipelines do.

set(KUKU_WARNING_FLAGS "")
if(MSVC)
    # CMP0092 is NEW here, so CMake does not put a default /W3 in the flags and /W4 cannot
    # provoke a D9025 "overriding /W3" warning.
    list(APPEND KUKU_WARNING_FLAGS /W4)
else()
    include(CheckCXXCompilerFlag)
    # Probe rather than assume: an unsupported -W flag is an error on GCC and would otherwise
    # break the build on compilers that predate a given warning.
    foreach(_kuku_flag -Wall -Wextra -Wconversion -Wsign-conversion -Wshadow)
        string(MAKE_C_IDENTIFIER "KUKU_HAS${_kuku_flag}" _kuku_flag_var)
        check_cxx_compiler_flag(${_kuku_flag} ${_kuku_flag_var})
        if(${_kuku_flag_var})
            list(APPEND KUKU_WARNING_FLAGS ${_kuku_flag})
        endif()
    endforeach()
    unset(_kuku_flag)
    unset(_kuku_flag_var)
endif()

# Required files and directories
include(GNUInstallDirs)

# Runtime path setup. Apple: loader-relative entries so installed dylibs resolve
# siblings without depending on the install prefix. ELF: absolute install lib
# dir matching the configure-time prefix. _USE_LINK_PATH FALSE prevents leaking
# build-machine paths (vcpkg _deps/, etc.) into the installed artifact's RPATH.
if(APPLE)
    set(CMAKE_INSTALL_RPATH "@loader_path;@loader_path/../${CMAKE_INSTALL_LIBDIR}")
else()
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")
endif()
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)

# Source Tree
set(KUKU_INCLUDES_DIR ${CMAKE_CURRENT_LIST_DIR}/src)
set(KUKU_CONFIG_IN_FILENAME ${CMAKE_CURRENT_LIST_DIR}/cmake/KukuConfig.cmake.in)

# Build tree
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(KUKU_TARGETS_FILENAME ${CMAKE_CURRENT_BINARY_DIR}/cmake/KukuTargets.cmake)
set(KUKU_CONFIG_FILENAME ${CMAKE_CURRENT_BINARY_DIR}/cmake/KukuConfig.cmake)
set(KUKU_CONFIG_VERSION_FILENAME ${CMAKE_CURRENT_BINARY_DIR}/cmake/KukuConfigVersion.cmake)

# Install
set(KUKU_CONFIG_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/Kuku)
set(KUKU_INCLUDES_INSTALL_DIR ${CMAKE_INSTALL_INCLUDEDIR})

##################
# Various macros #
##################

# Set the C++ language version
macro(kuku_set_language target)
    target_compile_features(${target} PUBLIC cxx_std_17)
endmacro()

# Apply the warning policy to one target. PRIVATE keeps the flags out of KukuTargets.cmake so
# consumers never inherit them; the COMPILE_LANGUAGE gate keeps them off any other language
# driver. The variable is quoted so the generator expression expands to separate options.
macro(kuku_set_warnings target)
    target_compile_options(${target} PRIVATE
        "$<$<COMPILE_LANGUAGE:C,CXX>:${KUKU_WARNING_FLAGS}>")
endmacro()

# Apply EH continuation metadata to one target, compiling and linking alike. See the
# /guard:ehcont probe above for why this is per target. KUKU_EHCONT_OPTION is empty unless the
# flag was probed and accepted, so this is a no-op elsewhere. Never apply it to the tests.
macro(kuku_set_ehcont target)
    if(KUKU_EHCONT_OPTION)
        target_compile_options(${target} PRIVATE ${KUKU_EHCONT_OPTION})
        target_link_options(${target} PRIVATE ${KUKU_EHCONT_OPTION})
    endif()
endmacro()

# Set the VERSION property
macro(kuku_set_version target)
    set_target_properties(${target} PROPERTIES VERSION ${Kuku_VERSION})
endmacro()

# Set the SOVERSION property
macro(kuku_set_soversion target)
    set_target_properties(${target} PROPERTIES
        SOVERSION ${Kuku_VERSION_MAJOR}.${Kuku_VERSION_MINOR})
endmacro()

# Set include directories for build and install interfaces.
macro(kuku_set_include_directories target)
    target_include_directories(${target} PUBLIC
        $<BUILD_INTERFACE:${KUKU_INCLUDES_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/src/>
        $<INSTALL_INTERFACE:${KUKU_INCLUDES_INSTALL_DIR}>)
endmacro()

# Include target to given export
macro(kuku_install_target target export)
    install(TARGETS ${target} EXPORT ${export}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endmacro()

####################
# Kuku C++ library #
####################

# Should we build also the shared library?
set(BUILD_SHARED_LIBS_STR "Build shared library")
option(BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS_STR} OFF)
if(WIN32 AND BUILD_SHARED_LIBS)
    message(FATAL_ERROR "On Windows only static build is supported; set `BUILD_SHARED_LIBS=OFF`")
endif()

# Add source files to library and header files to install
set(KUKU_SOURCE_FILES "")
add_subdirectory(src/kuku)

set(KUKU_LIBRARY_NAME "")
if(NOT BUILD_SHARED_LIBS)
    add_library(kuku STATIC ${KUKU_SOURCE_FILES})
    kuku_set_language(kuku)
    kuku_set_warnings(kuku)
    # A static archive has no link step, so this only sets the compile half - which is the
    # half that matters here: it is what lets a consumer who links kuku.lib into an
    # EHCONT-enabled binary of their own actually get the metadata.
    kuku_set_ehcont(kuku)
    kuku_set_include_directories(kuku)
    kuku_set_version(kuku)
    kuku_install_target(kuku KukuTargets)
    set(KUKU_LIBRARY_NAME "kuku")

# Build only a shared library
else()
    add_library(kuku_shared SHARED ${KUKU_SOURCE_FILES})
    kuku_set_soversion(kuku_shared)
    set_target_properties(kuku_shared PROPERTIES OUTPUT_NAME kuku)
    kuku_set_language(kuku_shared)
    kuku_set_warnings(kuku_shared)
    kuku_set_ehcont(kuku_shared)
    kuku_set_include_directories(kuku_shared)
    kuku_set_version(kuku_shared)
    kuku_install_target(kuku_shared KukuTargets)
    set(KUKU_LIBRARY_NAME "kuku_shared")
endif()

# Add standard alias targets for Kuku::kuku and Kuku::kuku_shared
if(TARGET kuku)
    add_library(Kuku::kuku ALIAS kuku)
endif()
if(TARGET kuku_shared)
    add_library(Kuku::kuku_shared ALIAS kuku_shared)
endif()

#########################
# Kuku C export library #
#########################

set(KUKU_BUILD_KUKU_C_OPTION_STR "Build C export library for Kuku")
option(KUKU_BUILD_KUKU_C ${KUKU_BUILD_KUKU_C_OPTION_STR} OFF)

# [option] KUKU_BUILD_STATIC_KUKU_C (default: OFF)
# Build the C export library as a static archive instead of a shared library. Useful for
# consumers that must ship a single binary.
set(KUKU_BUILD_STATIC_KUKU_C_OPTION_STR "Build the C export library as a static library")
cmake_dependent_option(
    KUKU_BUILD_STATIC_KUKU_C ${KUKU_BUILD_STATIC_KUKU_C_OPTION_STR} OFF "KUKU_BUILD_KUKU_C" OFF)

if(KUKU_BUILD_KUKU_C)
    # Check that size_t is 8 bytes (CheckTypeSize is included at the top of this file).
    check_type_size("size_t" SIZET LANGUAGE C)
    if(NOT ${SIZET} EQUAL 8)
        unset(SIZET CACHE)
        unset(HAVE_SIZET CACHE)
        message(FATAL_ERROR "kukuc requires a 64-bit platform")
    endif()
    unset(SIZET CACHE)
    unset(HAVE_SIZET CACHE)

    if(BUILD_SHARED_LIBS)
        message(FATAL_ERROR "kukuc (shared or static) requires a static Kuku; set "
                            "`BUILD_SHARED_LIBS=OFF` and control kukuc by setting "
                            "`KUKU_BUILD_STATIC_KUKU_C` to `OFF` (default) or `ON`")
    endif()

    if(KUKU_BUILD_STATIC_KUKU_C)
        add_library(kukuc STATIC)
    else()
        add_library(kukuc SHARED)
        # Tell kuku.h that a shared kukuc exists, so it decorates the exported functions
        # with dllexport while building and dllimport while consuming. PUBLIC so the
        # definition travels to consumers through the exported CMake target; a static kukuc
        # deliberately gets no definition and therefore no decoration.
        target_compile_definitions(kukuc PUBLIC KUKU_C_IS_SHARED)
    endif()

    # Add source files to library and header files to install
    add_subdirectory(src/kuku/c)
    kuku_set_version(kukuc)
    # SOVERSION is an ELF/Mach-O soname concept and is meaningless for a static archive,
    # matching how the static core target is handled above.
    if(NOT KUKU_BUILD_STATIC_KUKU_C)
        kuku_set_soversion(kukuc)
    endif()
    kuku_set_language(kukuc)
    kuku_set_warnings(kukuc)
    kuku_set_ehcont(kukuc)
    kuku_set_include_directories(kukuc)

    target_link_libraries(kukuc PUBLIC ${KUKU_LIBRARY_NAME})

    kuku_install_target(kukuc KukuTargets)
endif()

# Add standard alias target for Kuku::kukuc
if(TARGET kukuc)
    add_library(Kuku::kukuc ALIAS kukuc)
endif()

#################################
# Installation and CMake config #
#################################

# Create the CMake config file
include(CMakePackageConfigHelpers)
configure_package_config_file(
    ${KUKU_CONFIG_IN_FILENAME} ${KUKU_CONFIG_FILENAME}
    INSTALL_DESTINATION ${KUKU_CONFIG_INSTALL_DIR})

# Install the export
install(
    EXPORT KukuTargets
    NAMESPACE Kuku::
    DESTINATION ${KUKU_CONFIG_INSTALL_DIR})

# Version file; downstream must match on major.minor (COMPATIBILITY SameMinorVersion)
write_basic_package_version_file(
    ${KUKU_CONFIG_VERSION_FILENAME}
    VERSION ${Kuku_VERSION}
    COMPATIBILITY SameMinorVersion)

# Install config and version files
install(
    FILES
        ${KUKU_CONFIG_FILENAME}
        ${KUKU_CONFIG_VERSION_FILENAME}
    DESTINATION ${KUKU_CONFIG_INSTALL_DIR})

# We export KukuTargets from the build tree so it can be used by other projects
# without requiring an install.
export(
    EXPORT KukuTargets
    NAMESPACE Kuku::
    FILE ${KUKU_TARGETS_FILENAME})

#####################
# Kuku C++ examples #
#####################

set(KUKU_BUILD_EXAMPLES_OPTION_STR "Build C++ examples for Kuku")
option(KUKU_BUILD_EXAMPLES ${KUKU_BUILD_EXAMPLES_OPTION_STR} OFF)

if(KUKU_BUILD_EXAMPLES)
    add_executable(kukuexamples)
    add_subdirectory(examples)
    target_link_libraries(kukuexamples PRIVATE ${KUKU_LIBRARY_NAME})
    kuku_set_warnings(kukuexamples)
    kuku_set_ehcont(kukuexamples)
endif()

##################
# Kuku C++ tests #
##################

# KUKU_BUILD_TESTS option is declared above project() so VCPKG_MANIFEST_FEATURES
# can include "tests" before vcpkg runs.
if(KUKU_BUILD_TESTS)
    find_package(GTest CONFIG REQUIRED)
    include(GoogleTest)
    enable_testing()

    # Test binaries deliberately do NOT get kuku_set_ehcont(); see the /guard:ehcont probe above.
    add_executable(kukutest)
    add_subdirectory(tests/kuku)
    target_link_libraries(kukutest PRIVATE ${KUKU_LIBRARY_NAME} GTest::gtest)
    # GTest is an IMPORTED target, so its headers are SYSTEM includes and the warning policy
    # applies to Kuku's own test code only.
    kuku_set_warnings(kukutest)

    # Register every GoogleTest case with CTest individually, so `ctest` reports and
    # can filter them one by one. DISCOVERY_MODE PRE_TEST defers enumeration to ctest
    # time instead of running the freshly linked binary as a post-build step; that
    # matters for kukuctest below, which loads the kukuc shared library at runtime.
    gtest_discover_tests(kukutest DISCOVERY_MODE PRE_TEST)

    # Links kukuc and NOT the C++ core, so every call crosses the real C boundary and
    # exercises the exported symbols, calling convention and dllimport declarations.
    if(KUKU_BUILD_KUKU_C)
        add_executable(kukuctest)
        add_subdirectory(tests/kuku/c)
        target_link_libraries(kukuctest PRIVATE Kuku::kukuc GTest::gtest)
        kuku_set_warnings(kukuctest)
        gtest_discover_tests(kukuctest DISCOVERY_MODE PRE_TEST)
    endif()
endif()
