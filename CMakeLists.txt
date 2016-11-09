
# === CMake lists for the XShaderCompiler - (09/10/2014) ===

cmake_minimum_required(VERSION 2.8)
project(XShaderCompiler)


# === Build path ===

set(BUILD_OUTPUT_PATH "${CMAKE_CURRENT_BINARY_DIR}/build")
set(EXECUTABLE_OUTPUT_PATH ${BUILD_OUTPUT_PATH} CACHE PATH "Build directory" FORCE)
set(LIBRARY_OUTPUT_PATH ${BUILD_OUTPUT_PATH} CACHE PATH "Build directory" FORCE)
set(INSTALL_OUTPUT_PATH "${CMAKE_CURRENT_BINARY_DIR}/install" CACHE PATH "Installation directory" FORCE)


# === Options ===

option(XSC_SHARED_LIB "Build XShaderCompiler as a shared library instead of a static library" OFF)
option(XSC_BUILD_SHELL "Build XShaderCompiler shell 'xsc'" ON)


# === Preprocessor definitions ===

add_definitions(-D_CRT_SECURE_NO_WARNINGS)

if(HTLib_SHARED_LIB)
	add_definitions(-DHTLIB_SHARED_LIB)
endif()


# === Global files ===

file(GLOB FilesInc ${PROJECT_SOURCE_DIR}/inc/HT/*.*)
file(GLOB FilesSrc ${PROJECT_SOURCE_DIR}/src/*.*)
file(GLOB FilesShell ${PROJECT_SOURCE_DIR}/shell/*.*)

if(WIN32)
	file(GLOB FilesPlatform ${PROJECT_SOURCE_DIR}/src/win32/*.*)
else()
	file(GLOB FilesPlatform ${PROJECT_SOURCE_DIR}/src/unix/*.*)
endif()

set(
	FilesAll
	${FilesInc}
	${FilesSrc}
	${FilesPlatform}
)


# === Source group folders ===

source_group("inc" FILES ${FilesInc})
source_group("src" FILES ${FilesSrc})
source_group("src\\platform" FILES ${FilesPlatform})
source_group("shell" FILES ${FilesShell})


# === Include directories ===

include_directories("${PROJECT_SOURCE_DIR}/inc")
include_directories("${PROJECT_SOURCE_DIR}/src")


# === Executable ===

if(XSC_SHARED_LIB)
	add_library(xsc_core SHARED ${FilesAll})
else()
	add_library(xsc_core STATIC ${FilesAll})
endif()

set_target_properties(xsc_core PROPERTIES LINKER_LANGUAGE CXX)
target_compile_features(xsc_core PRIVATE cxx_range_for)

if(XSC_BUILD_SHELL)
	add_executable(xsc ${FilesShell})
	set_target_properties(xsc PROPERTIES LINKER_LANGUAGE CXX)
	target_link_libraries(xsc xsc_core)
	target_compile_features(xsc PRIVATE cxx_range_for)
endif()


# === Installation Rules ===

install(
	TARGETS xsc_core xsc
	DESTINATION ${INSTALL_OUTPUT_PATH}
)

