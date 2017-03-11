
# === Preprocessor definitions ===

# Disable "/EHsc" flag, that is incompatible with "/CLR"
#list(REMOVE_ITEM CMAKE_CXX_FLAGS "/EHsc")
set(CMAKE_CXX_FLAGS "/DWIN32 /D_WINDOWS /W3 /GR")

# Disable "/RTC1" flag, that is incompatible with "/CLR"
#list(REMOVE_ITEM CMAKE_CXX_FLAGS_DEBUG "/RTC1")
set(CMAKE_CXX_FLAGS_DEBUG "/D_DEBUG /MDd /Zi /Ob0 /Od")


# === Global files ===

file(GLOB FilesSrcWrapperCSharp ${PROJECT_SOURCE_DIR}/src/Wrapper/CSharp/*.*)
#file(GLOB FilesSrcWrapperCSharp ${PROJECT_SOURCE_DIR}/*.*)


# === Executable ===

# C# wrapper
add_library(xsc_core_csharp SHARED ${FilesSrcWrapperCSharp})
set_target_properties(xsc_core_csharp PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(xsc_core_csharp PROPERTIES COMPILE_FLAGS "/CLR")
target_link_libraries(xsc_core_csharp xsc_core)
target_compile_features(xsc_core_csharp PRIVATE cxx_range_for)



