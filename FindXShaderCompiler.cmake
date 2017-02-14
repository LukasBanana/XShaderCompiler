
# Custom CMake module for finding "XShaderCompiler" files
# Written by Lukas Hermanns on 21/12/2014

# Macros

macro(_XSC_APPEND_LIBRARIES _list _release)
   set(_debug ${_release}_DEBUG)
   if(${_debug})
      set(${_list} ${${_list}} optimized ${${_release}} debug ${${_debug}})
   else()
      set(${_list} ${${_list}} ${${_release}})
   endif()
endmacro()

# Find library

find_path(XSC_INCLUDE_DIR Xsc/Xsc.h)

find_library(XSC_LIBRARY NAMES XShaderCompiler)
find_library(XSC_LIBRARY_DEBUG NAMES XShaderCompiler)

# Setup package handle

include(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(
	XSC
	DEFAULT_MSG
	XSC_INCLUDE_DIR
    XSC_LIBRARY
    XSC_LIBRARY_DEBUG
)

if(XSC_FOUND)
	_XSC_APPEND_LIBRARIES(XSC_LIBRARIES XSC_LIBRARY)
endif(HTLIB_FOUND)
