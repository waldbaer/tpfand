add_executable(tpfand main.cpp TpFan.cpp TpTemperature.cpp TpTempRegulator.cpp)

include(CheckIncludeFile)
include(CheckFunctionExists)
include(FindPkgConfig)

pkg_check_modules (GLIB2 REQUIRED glib-2.0>=2.14)

# for config.h
add_definitions(-DHAVE_CONFIG_H)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/..)
include_directories(${GLIB2_INCLUDE_DIRS})

target_link_libraries(tpfand ${GLIB2_LIBRARIES})

macro(require_include_file _header)
    check_include_file(${_header} ${_header}_INCLUDE_FILE)
    if (NOT ${_header}_INCLUDE_FILE)
        message(FATAL_ERROR "${_header} was not found")
    endif (NOT ${_header}_INCLUDE_FILE)
endmacro(require_include_file)

macro(require_function_exists _function)
    check_function_exists(${_function} ${_function}_FUNCTION_EXISTS)
    if (NOT ${_function}_FUNCTION_EXISTS)
        message(FATAL_ERROR "${_function} does not exist")
    endif (NOT ${_function}_FUNCTION_EXISTS)
endmacro(require_function_exists)

install(
  TARGETS
    tpfand
  RUNTIME DESTINATION bin
)

if(CMAKE_HOST_UNIX)
    message("is host unix destdir=$ENV{DESTDIR} installdir=${BIN_INSTALL_DIR}")
    install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
    tpfand \"\$ENV{DESTDIR}${BIN_INSTALL_DIR}/tpfand\" )" )
endif(CMAKE_HOST_UNIX)

# vim: set ts=4 sw=4 et:
