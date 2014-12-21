
# Custom CMake module for finding "HLSLTranslator" files
# Written by Lukas Hermanns on 21/12/2014

# Macros

macro(_HTLIB_APPEND_LIBRARIES _list _release)
   set(_debug ${_release}_DEBUG)
   if(${_debug})
      set(${_list} ${${_list}} optimized ${${_release}} debug ${${_debug}})
   else()
      set(${_list} ${${_list}} ${${_release}})
   endif()
endmacro()

# Find library

find_path(HTLib_INCLUDE_DIR HT/Translator.h)

find_library(HTLib_LIBRARY NAMES HLSLTranslator)
find_library(HTLib_LIBRARY_DEBUG NAMES HLSLTranslator)

# Setup package handle

include(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(
	HTLib
	DEFAULT_MSG
	HTLib_INCLUDE_DIR
    HTLib_LIBRARY
    HTLib_LIBRARY_DEBUG
)

if(HTLIB_FOUND)
	set(HTLib_FOUND TRUE)
	_HTLIB_APPEND_LIBRARIES(HTLib_LIBRARIES HTLib_LIBRARY)
endif(HTLIB_FOUND)
