
# === CMake lists for the HLSL Translator - (09/10/2014) ===

cmake_minimum_required(VERSION 2.8)
project(HLSLTranslator)


# === Build path ===

set(BUILD_OUTPUT_PATH "${CMAKE_CURRENT_BINARY_DIR}/build")
set(EXECUTABLE_OUTPUT_PATH ${BUILD_OUTPUT_PATH} CACHE PATH "Build directory" FORCE)
set(LIBRARY_OUTPUT_PATH ${BUILD_OUTPUT_PATH} CACHE PATH "Build directory" FORCE)
set(INSTALL_OUTPUT_PATH "${CMAKE_CURRENT_BINARY_DIR}/install" CACHE PATH "Installation directory" FORCE)


# === Options ===

option(HTLib_SHARED_LIB "Build HTLib as a shared library instead of a static library" OFF)
option(HTLib_BUILD_CMD_TOOL "Build HTLib command line tool 'HTLibCmd'" ON)


# === Preprocessor definitions ===

add_definitions(-D_CRT_SECURE_NO_WARNINGS)

if(HTLib_SHARED_LIB)
	add_definitions(-DHTLIB_SHARED_LIB)
endif()


# === Global files ===

file(GLOB FilesInc	${PROJECT_SOURCE_DIR}/inc/HT/*.*)
file(GLOB FilesSrc	${PROJECT_SOURCE_DIR}/src/*.*)
file(GLOB FilesTool	${PROJECT_SOURCE_DIR}/tool/*.*)

set(
	FilesAll
	${FilesInc}
	${FilesSrc}
)


# === Source group folders ===

source_group("inc" FILES ${FilesInc})
source_group("src" FILES ${FilesSrc})
source_group("tool" FILES ${FilesTool})


# === Include directories ===

include_directories("${PROJECT_SOURCE_DIR}/inc")
include_directories("${PROJECT_SOURCE_DIR}/src")


# === Executable ===

if(HTLib_SHARED_LIB)
	add_library(HTLib SHARED ${FilesAll})
else()
	add_library(HTLib STATIC ${FilesAll})
endif()
set_target_properties(HTLib PROPERTIES LINKER_LANGUAGE CXX)

if(HTLib_BUILD_CMD_TOOL)
	add_executable(HTLibCmd ${FilesTool})
	set_target_properties(HTLibCmd PROPERTIES LINKER_LANGUAGE CXX)
	target_link_libraries(HTLibCmd HTLib)
endif()


# === Installation Rules ===

install(
	TARGETS HTLib HTLibCmd
	DESTINATION ${INSTALL_OUTPUT_PATH}
)

