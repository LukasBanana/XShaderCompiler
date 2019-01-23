
# === CMake lists for the XShaderCompiler - (09/10/2014) ===

cmake_minimum_required(VERSION 2.8)
project(XShaderCompiler)


# === Build path ===

set(BUILD_OUTPUT_PATH "${CMAKE_CURRENT_BINARY_DIR}/build")
set(INSTALL_OUTPUT_PATH "${CMAKE_CURRENT_BINARY_DIR}/install" CACHE PATH "Installation directory" FORCE)

macro(XSC_OUTPUT_PATHS TARGET_PROJECT)
	set_target_properties(
		${TARGET_PROJECT}
		PROPERTIES
		ARCHIVE_OUTPUT_DIRECTORY "${BUILD_OUTPUT_PATH}"
		LIBRARY_OUTPUT_DIRECTORY "${BUILD_OUTPUT_PATH}"
		RUNTIME_OUTPUT_DIRECTORY "${BUILD_OUTPUT_PATH}"
	)
endmacro()


# === Options ===

# Main
option(XSC_BUILD_SHELL "Build XShaderCompiler shell 'xsc'" ON)
option(XSC_BUILD_DEBUGGER "Build XShaderCompiler debugger 'xsc_debugger' (requires wxWidgets library)" OFF)
option(XSC_BUILD_TESTS "Build all test applications" OFF)
option(XSC_SHARED_LIB "Build XShaderCompiler as a shared library instead of a static library" OFF)

# Wrappers
option(XSC_BUILD_WRAPPER_C "Build C wrapper (ISO C99)" OFF)
option(XSC_BUILD_WRAPPER_CSHARP "Build C# wrapper" OFF)

# Specials
option(XSC_ENABLE_EASTER_EGGS "Enables little easter eggs" OFF)
option(XSC_ENABLE_POST_VALIDATION "Enables the post validation in presettings with 'glslangValidator'" OFF)
option(XSC_ENABLE_LANGUAGE_EXT "Enables a few language extensions (e.g. 'space' attribute for stronger type system)" OFF)
option(XSC_ENABLE_SPIRV "Enables all SPIR-V related features (experimental; requires submodule in 'external/SPIR-V')" OFF)

#set(XSC_REPORT_LANGUAGE "EN" CACHE STRING "Language of the report output (supported values: 'EN')")


# === Preprocessor definitions ===

add_definitions(-D_CRT_SECURE_NO_WARNINGS)

if(XSC_SHARED_LIB)
	add_definitions(-DXSC_SHARED_LIB)
endif()

if(XSC_ENABLE_EASTER_EGGS)
	add_definitions(-DXSC_ENABLE_EASTER_EGGS)
endif()

if(XSC_ENABLE_POST_VALIDATION)
	add_definitions(-DXSC_ENABLE_POST_VALIDATION)
endif()

if(XSC_ENABLE_LANGUAGE_EXT)
    add_definitions(-DXSC_ENABLE_LANGUAGE_EXT)
endif()

if(XSC_ENABLE_SPIRV)
	add_definitions(-DXSC_ENABLE_SPIRV)
endif()


# === Global files ===

file(GLOB FilesInc ${PROJECT_SOURCE_DIR}/inc/Xsc/*.*)
file(GLOB FilesIncC ${PROJECT_SOURCE_DIR}/inc/XscC/*.*)
file(GLOB FilesSrcCompiler ${PROJECT_SOURCE_DIR}/src/Compiler/*.*)
file(GLOB FilesSrcCompilerAST ${PROJECT_SOURCE_DIR}/src/Compiler/AST/*.*)
file(GLOB FilesSrcCompilerASTVisitor ${PROJECT_SOURCE_DIR}/src/Compiler/AST/Visitor/*.*)
file(GLOB FilesSrcCompilerFrontend ${PROJECT_SOURCE_DIR}/src/Compiler/Frontend/*.*)
file(GLOB FilesSrcCompilerFrontendHLSL ${PROJECT_SOURCE_DIR}/src/Compiler/Frontend/HLSL/*.*)
file(GLOB FilesSrcCompilerFrontendGLSL ${PROJECT_SOURCE_DIR}/src/Compiler/Frontend/GLSL/*.*)
file(GLOB FilesSrcCompilerFrontendMetal ${PROJECT_SOURCE_DIR}/src/Compiler/Frontend/Metal/*.*)
file(GLOB FilesSrcCompilerBackend ${PROJECT_SOURCE_DIR}/src/Compiler/Backend/*.*)
file(GLOB FilesSrcCompilerBackendGLSL ${PROJECT_SOURCE_DIR}/src/Compiler/Backend/GLSL/*.*)
file(GLOB FilesSrcCompilerBackendMetal ${PROJECT_SOURCE_DIR}/src/Compiler/Backend/Metal/*.*)
file(GLOB FilesSrcCompilerReport ${PROJECT_SOURCE_DIR}/src/Compiler/Report/*.*)
file(GLOB FilesSrcCompilerCFG ${PROJECT_SOURCE_DIR}/src/Compiler/CFG/*.*)
file(GLOB FilesSrcShell ${PROJECT_SOURCE_DIR}/src/Shell/*.*)
file(GLOB FilesSrcDebugger ${PROJECT_SOURCE_DIR}/src/Debugger/*.*)
file(GLOB FilesSrcWrapperC ${PROJECT_SOURCE_DIR}/src/Wrapper/C/*.*)
#file(GLOB FilesSrcWrapperCSharp ${PROJECT_SOURCE_DIR}/src/Wrapper/CSharp/*.*)

set(FilesTest ${PROJECT_SOURCE_DIR}/test)

if(WIN32)
	file(GLOB FilesSrcCompilerPlatform ${PROJECT_SOURCE_DIR}/src/Compiler/Platform/Win32/*.*)
else()
	file(GLOB FilesSrcCompilerPlatform ${PROJECT_SOURCE_DIR}/src/Compiler/Platform/Unix/*.*)
endif()

set(
	FilesCompilerAll
	${FilesInc}
	${FilesSrcCompiler}
	${FilesSrcCompilerAST}
	${FilesSrcCompilerASTVisitor}
	${FilesSrcCompilerPlatform}
	${FilesSrcCompilerFrontend}
	${FilesSrcCompilerFrontendHLSL}
	${FilesSrcCompilerFrontendGLSL}
	${FilesSrcCompilerFrontendMetal}
	${FilesSrcCompilerBackend}
	${FilesSrcCompilerBackendGLSL}
	${FilesSrcCompilerBackendMetal}
	${FilesSrcCompilerReport}
)

if(XSC_ENABLE_SPIRV)
	set(FilesCompilerAll ${FilesCompilerAll} ${FilesSrcCompilerCFG})
endif()


# === Source group folders ===

source_group("inc\\Xsc" FILES ${FilesInc})
source_group("inc\\XscC" FILES ${FilesIncC})
source_group("src" FILES ${FilesSrcCompiler} ${FilesSrcShell} ${FilesSrcDebugger})
source_group("src\\AST" FILES ${FilesSrcCompilerAST})
source_group("src\\AST\\Visitor" FILES ${FilesSrcCompilerASTVisitor})
source_group("src\\Platform" FILES ${FilesSrcCompilerPlatform})
source_group("src\\Frontend" FILES ${FilesSrcCompilerFrontend})
source_group("src\\Frontend\\HLSL" FILES ${FilesSrcCompilerFrontendHLSL})
source_group("src\\Frontend\\GLSL" FILES ${FilesSrcCompilerFrontendGLSL})
source_group("src\\Frontend\\Metal" FILES ${FilesSrcCompilerFrontendMetal})
source_group("src\\Backend" FILES ${FilesSrcCompilerBackend})
source_group("src\\Backend\\GLSL" FILES ${FilesSrcCompilerBackendGLSL})
source_group("src\\Backend\\Metal" FILES ${FilesSrcCompilerBackendMetal})
source_group("src\\Report" FILES ${FilesSrcCompilerReport})

if(XSC_ENABLE_SPIRV)
	source_group("src\\CFG" FILES ${FilesSrcCompilerCFG})
endif()


# === Include directories ===

include_directories("${PROJECT_SOURCE_DIR}/inc")

include_directories("${PROJECT_SOURCE_DIR}/src/Compiler")
include_directories("${PROJECT_SOURCE_DIR}/src/Compiler/AST")
include_directories("${PROJECT_SOURCE_DIR}/src/Compiler/AST/Visitor")
include_directories("${PROJECT_SOURCE_DIR}/src/Compiler/Frontend")
include_directories("${PROJECT_SOURCE_DIR}/src/Compiler/Frontend/HLSL")
include_directories("${PROJECT_SOURCE_DIR}/src/Compiler/Frontend/GLSL")
include_directories("${PROJECT_SOURCE_DIR}/src/Compiler/Frontend/Metal")
include_directories("${PROJECT_SOURCE_DIR}/src/Compiler/Backend")
include_directories("${PROJECT_SOURCE_DIR}/src/Compiler/Backend/GLSL")
include_directories("${PROJECT_SOURCE_DIR}/src/Compiler/Backend/Metal")
include_directories("${PROJECT_SOURCE_DIR}/src/Compiler/Report")

if(XSC_ENABLE_SPIRV)
	include_directories("${PROJECT_SOURCE_DIR}/src/Compiler/CFG")
	include_directories("${PROJECT_SOURCE_DIR}/external/SPIR-V/include")
endif()


# === Executable ===

# Library core
if(XSC_SHARED_LIB)
	add_library(xsc_core SHARED ${FilesCompilerAll})
else()
	add_library(xsc_core STATIC ${FilesCompilerAll})
endif()

XSC_OUTPUT_PATHS(xsc_core)
set_target_properties(xsc_core PROPERTIES LINKER_LANGUAGE CXX)
target_compile_features(xsc_core PRIVATE cxx_range_for)

set(XSC_INSTALL_TARGETS "xsc_core")

# Shell application
if(XSC_BUILD_SHELL)
	add_executable(xsc ${FilesSrcShell})
	XSC_OUTPUT_PATHS(xsc)
	set_target_properties(xsc PROPERTIES LINKER_LANGUAGE CXX)
	target_link_libraries(xsc xsc_core)
	target_compile_features(xsc PRIVATE cxx_range_for)
	set(XSC_INSTALL_TARGETS ${XSC_INSTALL_TARGETS} "xsc")
endif()

# Debugger UI aplication
if(XSC_BUILD_DEBUGGER)
	if(WIN32)
		add_executable(xsc_debugger WIN32 ${FilesSrcDebugger} "${PROJECT_SOURCE_DIR}/src/Debugger/Resources.rc")
	else()
		add_executable(xsc_debugger ${FilesSrcDebugger})
	endif()
	
	XSC_OUTPUT_PATHS(xsc_debugger)
	set_target_properties(xsc_debugger PROPERTIES LINKER_LANGUAGE CXX)
	target_link_libraries(xsc_debugger xsc_core)
	target_compile_features(xsc_debugger PRIVATE cxx_range_for)
	
	# wxWidgets Libs
	find_package(wxWidgets REQUIRED adv aui base core stc propgrid richtext html xml scintilla)
	if(wxWidgets_FOUND)
		message("Found wxWidgets library")
		include_directories(${wxWidgets_INCLUDE_DIRS})
		target_link_libraries(xsc_debugger ${wxWidgets_LIBRARIES})
	else(wxWidgets_FOUND)
		message(SEND_ERROR "Missing wxWidgets library")
	endif(wxWidgets_FOUND)
	
	set(XSC_INSTALL_TARGETS ${XSC_INSTALL_TARGETS} "xsc_debugger")
endif()

# C wrapper
if(XSC_BUILD_WRAPPER_C)
	if(XSC_SHARED_LIB)
		add_library(xsc_core_c SHARED ${FilesSrcWrapperC})
	else()
		add_library(xsc_core_c STATIC ${FilesSrcWrapperC})
	endif()
	
	XSC_OUTPUT_PATHS(xsc_core_c)
	set_target_properties(xsc_core_c PROPERTIES LINKER_LANGUAGE CXX)
	target_link_libraries(xsc_core_c xsc_core)
	target_compile_features(xsc_core_c PRIVATE cxx_range_for)
	
	set(XSC_INSTALL_TARGETS ${XSC_INSTALL_TARGETS} "xsc_core_c")
endif()

# C# wrapper
if(WIN32 AND XSC_BUILD_WRAPPER_CSHARP)
	add_subdirectory(src/Wrapper/CSharp)
endif()

if(XSC_BUILD_TESTS)
	# Test C wrapper
	if(XSC_BUILD_WRAPPER_C)
		add_executable(XscTest_CWrapper "${FilesTest}/XscTest_CWrapper.c")
		XSC_OUTPUT_PATHS(XscTest_CWrapper)
		set_target_properties(XscTest_CWrapper PROPERTIES LINKER_LANGUAGE C)
		target_link_libraries(XscTest_CWrapper xsc_core_c)
	endif()
endif()


# === Show summary ===

message("~~~ Build Summary ~~~")

if(XSC_SHARED_LIB)
	message("Build Core: xsc_core (SHARED)")
else()
	message("Build Core: xsc_core (STATIC)")
endif()

if(XSC_BUILD_SHELL)
	message("Build Shell: xsc")
endif()

if(XSC_BUILD_DEBUGGER)
	message("Build Debugger: xsc_debugger")
endif()

if(XSC_BUILD_WRAPPER_C)
	message("Build C Wrapper: xsc_core_c")
endif()

if(XSC_BUILD_WRAPPER_CSHARP)
	message("Build C# Wrapper: xsc_core_csharp")
endif()

if(XSC_ENABLE_SPIRV)
	message("Include SPIR-V (Experimental)")
endif()


# === Installation Rules ===

install(
	TARGETS ${XSC_INSTALL_TARGETS}
	DESTINATION ${INSTALL_OUTPUT_PATH}
)

