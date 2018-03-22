/*
 * GLSLExtensions.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLSLExtensions.h"


namespace Xsc
{


const std::map<std::string, int>& GetGLSLExtensionVersionMap()
{
    static const std::map<std::string, int> refList
    {
        // 3DL
        { E_GL_3DL_array_objects,                           110 },

        // AMD
        { E_GL_AMD_gcn_shader,                              110 },
        { E_GL_AMD_gpu_shader_half_float,                   110 },
        { E_GL_AMD_shader_ballot,                           110 },
        { E_GL_AMD_shader_explicit_vertex_parameter,        110 },
        { E_GL_AMD_shader_trinary_minmax,                   110 },

        // ANDROID
        { E_GL_ANDROID_extension_pack_es31a,                110 },

        // ARB
        { E_GL_ARB_arrays_of_arrays,                        430 },
        { E_GL_ARB_compute_shader,                          430 },
        { E_GL_ARB_cull_distance,                           440 },
        { E_GL_ARB_derivative_control,                      450 },
        { E_GL_ARB_enhanced_layouts,                        430 },
        { E_GL_ARB_explicit_attrib_location,                330 },
        { E_GL_ARB_fragment_coord_conventions,              150 },
        { E_GL_ARB_gpu_shader5,                             400 },
        { E_GL_ARB_gpu_shader_fp64,                         400 },
        { E_GL_ARB_gpu_shader_int64,                        450 },
        { E_GL_ARB_separate_shader_objects,                 410 },
        { E_GL_ARB_shading_language_420pack,                420 },
        { E_GL_ARB_shader_atomic_counters,                  410 },
        { E_GL_ARB_shader_ballot,                           450 },
        { E_GL_ARB_shader_bit_encoding,                     330 },
        { E_GL_ARB_shader_draw_parameters,                  430 },
        { E_GL_ARB_shader_group_vote,                       430 },
        { E_GL_ARB_shader_image_load_store,                 420 },
        { E_GL_ARB_shader_storage_buffer_object,            430 },
        { E_GL_ARB_shader_texture_image_samples,            110 },
        { E_GL_ARB_shader_texture_lod,                      110 },
        { E_GL_ARB_shader_viewport_layer_array,             110 },
        { E_GL_ARB_sparse_texture2,                         110 },
        { E_GL_ARB_sparse_texture_clamp,                    110 },
        { E_GL_ARB_tessellation_shader,                     400 },
        { E_GL_ARB_texture_cube_map_array,                  400 },
        { E_GL_ARB_texture_gather,                          110 },
        { E_GL_ARB_texture_multisample,                     150 },
        { E_GL_ARB_texture_query_lod,                       400 },
        { E_GL_ARB_texture_rectangle,                       110 },
        { E_GL_ARB_uniform_buffer_object,                   140 },
        { E_GL_ARB_viewport_array,                          320 },

        // EXT
        { E_GL_EXT_device_group,                            110 },
        { E_GL_EXT_gpu_shader4,                             130 },
        { E_GL_EXT_multiview,                               110 },
        { E_GL_EXT_shader_image_load_formatted,             110 },
        { E_GL_EXT_shader_non_constant_global_initializers, 110 }, // ESSL
        { E_GL_EXT_geometry_shader,                         110 },
        { E_GL_EXT_geometry_point_size,                     110 },
        { E_GL_EXT_gpu_shader5,                             400 },
        { E_GL_EXT_primitive_bounding_box,                  110 },
        { E_GL_EXT_shader_io_blocks,                        110 },
        { E_GL_EXT_tessellation_shader,                     400 },
        { E_GL_EXT_tessellation_point_size,                 400 },
        { E_GL_EXT_texture_buffer,                          110 },
        { E_GL_EXT_texture_cube_map_array,                  110 },
        { E_GL_EXT_frag_depth,                              110 },
        { E_GL_EXT_shader_texture_lod,                      110 },

        // GOOGLE
        { E_GL_GOOGLE_cpp_style_line_directive,             110 },
        { E_GL_GOOGLE_include_directive,                    110 },

        // KHR
        { E_GL_KHR_blend_equation_advanced,                 110 },

        // NV
        { E_GL_NV_geometry_shader_passthrough,              110 },
        { E_GL_NV_sample_mask_override_coverage,            110 },
        { E_GL_NV_stereo_view_rendering,                    110 },
        { E_GL_NV_viewport_array2,                          110 },

        // NVX
        { E_GL_NVX_multiview_per_view_attributes,           110 },

        // OES
        { E_GL_OES_EGL_image_external,                      110 },
        { E_GL_OES_geometry_point_size,                     110 },
        { E_GL_OES_geometry_shader,                         110 },
        { E_GL_OES_gpu_shader5,                             400 },
        { E_GL_OES_primitive_bounding_box,                  110 },
        { E_GL_OES_sample_variables,                        110 },
        { E_GL_OES_shader_image_atomic,                     110 },
        { E_GL_OES_shader_io_blocks,                        110 },
        { E_GL_OES_shader_multisample_interpolation,        110 },
        { E_GL_OES_standard_derivatives,                    110 },
        { E_GL_OES_tessellation_point_size,                 400 },
        { E_GL_OES_tessellation_shader,                     400 },
        { E_GL_OES_texture_3D,                              110 },
        { E_GL_OES_texture_buffer,                          110 },
        { E_GL_OES_texture_cube_map_array,                  110 },
        { E_GL_OES_texture_storage_multisample_2d_array,    110 },
    };
    return refList;
}


} // /namespace Xsc



// ================================================================================