# used the cmake stuff of csync (https://dev.csync.org/) as template
# ./src/CMakeLists.txt might be the more interesting file

project(tpfand)

cmake_minimum_required(VERSION 2.6)

set(APPLICATION_NAME ${PROJECT_NAME})

set(APPLICATION_VERSION "0.9.0")

set(APPLICATION_VERSION_MAJOR "0")
set(APPLICATION_VERSION_MINOR "9")
set(APPLICATION_VERSION_PATCH "0")

# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
set(CMAKE_MODULE_PATH
  ${CMAKE_SOURCE_DIR}/cmake/Modules
)

# disallow in-source build
include(MacroEnsureOutOfSourceBuild)
macro_ensure_out_of_source_build("${PROJECT_NAME} requires an out of source build. Please create a separate build directory and run 'cmake /path/to/${PROJECT_NAME} [options]' there.")

# add definitions
include(DefineCMakeDefaults)
include(DefineCompilerFlags)
include(DefineInstallationPaths)

configure_file(config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

add_subdirectory(src)

# vim: set ts=4 sw=4 et:
