/*
 * GLSLExtensions.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_GLSL_EXTENSIONS_H
#define XSC_GLSL_EXTENSIONS_H


#include <string>
#include <map>


namespace Xsc
{


/*
GLSL extensions (adopted from glslangValidator)
see https://github.com/KhronosGroup/glslang
see https://github.com/KhronosGroup/glslang/blob/master/glslang/MachineIndependent/Versions.h
*/

// 3DL
static const char* E_GL_3DL_array_objects                           = "GL_3DL_array_objects";

// AMD
static const char* E_GL_AMD_gcn_shader                              = "GL_AMD_gcn_shader";
static const char* E_GL_AMD_gpu_shader_half_float                   = "GL_AMD_gpu_shader_half_float";
static const char* E_GL_AMD_shader_ballot                           = "GL_AMD_shader_ballot";
static const char* E_GL_AMD_shader_explicit_vertex_parameter        = "GL_AMD_shader_explicit_vertex_parameter";
static const char* E_GL_AMD_shader_trinary_minmax                   = "GL_AMD_shader_trinary_minmax";

// ANDROID
static const char* E_GL_ANDROID_extension_pack_es31a                = "GL_ANDROID_extension_pack_es31a";

// ARB
static const char* E_GL_ARB_arrays_of_arrays                        = "GL_ARB_arrays_of_arrays";
static const char* E_GL_ARB_compute_shader                          = "GL_ARB_compute_shader";
static const char* E_GL_ARB_cull_distance                           = "GL_ARB_cull_distance";
static const char* E_GL_ARB_derivative_control                      = "GL_ARB_derivative_control";
static const char* E_GL_ARB_enhanced_layouts                        = "GL_ARB_enhanced_layouts";
static const char* E_GL_ARB_explicit_attrib_location                = "GL_ARB_explicit_attrib_location";
static const char* E_GL_ARB_fragment_coord_conventions              = "GL_ARB_fragment_coord_conventions";
static const char* E_GL_ARB_gpu_shader5                             = "GL_ARB_gpu_shader5";
static const char* E_GL_ARB_gpu_shader_fp64                         = "GL_ARB_gpu_shader_fp64";
static const char* E_GL_ARB_gpu_shader_int64                        = "GL_ARB_gpu_shader_int64";
static const char* E_GL_ARB_separate_shader_objects                 = "GL_ARB_separate_shader_objects";
static const char* E_GL_ARB_shading_language_420pack                = "GL_ARB_shading_language_420pack";
static const char* E_GL_ARB_shader_atomic_counters                  = "GL_ARB_shader_atomic_counters";
static const char* E_GL_ARB_shader_ballot                           = "GL_ARB_shader_ballot";
static const char* E_GL_ARB_shader_draw_parameters                  = "GL_ARB_shader_draw_parameters";
static const char* E_GL_ARB_shader_group_vote                       = "GL_ARB_shader_group_vote";
static const char* E_GL_ARB_shader_image_load_store                 = "GL_ARB_shader_image_load_store";
static const char* E_GL_ARB_shader_texture_image_samples            = "GL_ARB_shader_texture_image_samples";
static const char* E_GL_ARB_shader_texture_lod                      = "GL_ARB_shader_texture_lod";
static const char* E_GL_ARB_shader_viewport_layer_array             = "GL_ARB_shader_viewport_layer_array";
static const char* E_GL_ARB_sparse_texture2                         = "GL_ARB_sparse_texture2";
static const char* E_GL_ARB_sparse_texture_clamp                    = "GL_ARB_sparse_texture_clamp";
static const char* E_GL_ARB_tessellation_shader                     = "GL_ARB_tessellation_shader";
static const char* E_GL_ARB_texture_cube_map_array                  = "GL_ARB_texture_cube_map_array";
static const char* E_GL_ARB_texture_gather                          = "GL_ARB_texture_gather";
static const char* E_GL_ARB_texture_multisample                     = "GL_ARB_texture_multisample";
static const char* E_GL_ARB_texture_query_lod                       = "GL_ARB_texture_query_lod";
static const char* E_GL_ARB_texture_rectangle                       = "GL_ARB_texture_rectangle";
static const char* E_GL_ARB_uniform_buffer_object                   = "GL_ARB_uniform_buffer_object";
static const char* E_GL_ARB_viewport_array                          = "GL_ARB_viewport_array";

// EXT
static const char* E_GL_EXT_device_group                            = "GL_EXT_device_group";
static const char* E_GL_EXT_gpu_shader4                             = "GL_EXT_gpu_shader4";
static const char* E_GL_EXT_multiview                               = "GL_EXT_multiview";
static const char* E_GL_EXT_shader_image_load_formatted             = "GL_EXT_shader_image_load_formatted";
static const char* E_GL_EXT_shader_non_constant_global_initializers = "GL_EXT_shader_non_constant_global_initializers"; // ESSL
static const char* E_GL_EXT_geometry_shader                         = "GL_EXT_geometry_shader";
static const char* E_GL_EXT_geometry_point_size                     = "GL_EXT_geometry_point_size";
static const char* E_GL_EXT_gpu_shader5                             = "GL_EXT_gpu_shader5";
static const char* E_GL_EXT_primitive_bounding_box                  = "GL_EXT_primitive_bounding_box";
static const char* E_GL_EXT_shader_io_blocks                        = "GL_EXT_shader_io_blocks";
static const char* E_GL_EXT_tessellation_shader                     = "GL_EXT_tessellation_shader";
static const char* E_GL_EXT_tessellation_point_size                 = "GL_EXT_tessellation_point_size";
static const char* E_GL_EXT_texture_buffer                          = "GL_EXT_texture_buffer";
static const char* E_GL_EXT_texture_cube_map_array                  = "GL_EXT_texture_cube_map_array";
static const char* E_GL_EXT_frag_depth                              = "GL_EXT_frag_depth";
static const char* E_GL_EXT_shader_texture_lod                      = "GL_EXT_shader_texture_lod";

// GOOGLE
static const char* E_GL_GOOGLE_cpp_style_line_directive             = "GL_GOOGLE_cpp_style_line_directive";
static const char* E_GL_GOOGLE_include_directive                    = "GL_GOOGLE_include_directive";

// KHR
static const char* E_GL_KHR_blend_equation_advanced                 = "GL_KHR_blend_equation_advanced";

// NV
static const char* E_GL_NV_geometry_shader_passthrough              = "GL_NV_geometry_shader_passthrough";
static const char* E_GL_NV_sample_mask_override_coverage            = "GL_NV_sample_mask_override_coverage";
static const char* E_GL_NV_stereo_view_rendering                    = "GL_NV_stereo_view_rendering";
static const char* E_GL_NV_viewport_array2                          = "GL_NV_viewport_array2";

// NVX
static const char* E_GL_NVX_multiview_per_view_attributes           = "GL_NVX_multiview_per_view_attributes";

// OES
static const char* E_GL_OES_EGL_image_external                      = "GL_OES_EGL_image_external";
static const char* E_GL_OES_geometry_point_size                     = "GL_OES_geometry_point_size";
static const char* E_GL_OES_geometry_shader                         = "GL_OES_geometry_shader";
static const char* E_GL_OES_gpu_shader5                             = "GL_OES_gpu_shader5";
static const char* E_GL_OES_primitive_bounding_box                  = "GL_OES_primitive_bounding_box";
static const char* E_GL_OES_sample_variables                        = "GL_OES_sample_variables";
static const char* E_GL_OES_shader_image_atomic                     = "GL_OES_shader_image_atomic";
static const char* E_GL_OES_shader_io_blocks                        = "GL_OES_shader_io_blocks";
static const char* E_GL_OES_shader_multisample_interpolation        = "GL_OES_shader_multisample_interpolation";
static const char* E_GL_OES_standard_derivatives                    = "GL_OES_standard_derivatives";
static const char* E_GL_OES_tessellation_point_size                 = "GL_OES_tessellation_point_size";
static const char* E_GL_OES_tessellation_shader                     = "GL_OES_tessellation_shader";
static const char* E_GL_OES_texture_3D                              = "GL_OES_texture_3D";
static const char* E_GL_OES_texture_buffer                          = "GL_OES_texture_buffer";
static const char* E_GL_OES_texture_cube_map_array                  = "GL_OES_texture_cube_map_array";
static const char* E_GL_OES_texture_storage_multisample_2d_array    = "GL_OES_texture_storage_multisample_2d_array";


// Returns a map of GLSL extension strings with their minimum required version number (default is 110).
const std::map<std::string, int>& GetGLSLExtensionVersionMap();


} // /namespace Xsc


#endif



// ================================================================================