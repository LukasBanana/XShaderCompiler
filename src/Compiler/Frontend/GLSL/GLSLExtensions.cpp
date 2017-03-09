/*
 * GLSLExtensions.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLSLExtensions.h"


namespace Xsc
{


const std::vector<const char*>& GetGLSLExtensionRefList()
{
    static const std::vector<const char*> refList
    {
        // 3DL
        E_GL_3DL_array_objects,

        // AMD
        E_GL_AMD_gcn_shader,
        E_GL_AMD_gpu_shader_half_float,
        E_GL_AMD_shader_ballot,
        E_GL_AMD_shader_explicit_vertex_parameter,
        E_GL_AMD_shader_trinary_minmax,

        // ANDROID
        E_GL_ANDROID_extension_pack_es31a,

        // ARB
        E_GL_ARB_arrays_of_arrays,
        E_GL_ARB_compute_shader,
        E_GL_ARB_cull_distance,
        E_GL_ARB_derivative_control,
        E_GL_ARB_enhanced_layouts,
        E_GL_ARB_explicit_attrib_location,
        E_GL_ARB_fragment_coord_conventions,
        E_GL_ARB_gpu_shader5,
        E_GL_ARB_gpu_shader_fp64,
        E_GL_ARB_gpu_shader_int64,
        E_GL_ARB_separate_shader_objects,
        E_GL_ARB_shading_language_420pack,
        E_GL_ARB_shader_atomic_counters,
        E_GL_ARB_shader_ballot,
        E_GL_ARB_shader_draw_parameters,
        E_GL_ARB_shader_group_vote,
        E_GL_ARB_shader_image_load_store,
        E_GL_ARB_shader_texture_image_samples,
        E_GL_ARB_shader_texture_lod,
        E_GL_ARB_shader_viewport_layer_array,
        E_GL_ARB_sparse_texture2,
        E_GL_ARB_sparse_texture_clamp,
        E_GL_ARB_tessellation_shader,
        E_GL_ARB_texture_cube_map_array,
        E_GL_ARB_texture_gather,
        E_GL_ARB_texture_multisample,
        E_GL_ARB_texture_query_lod,
        E_GL_ARB_texture_rectangle,
        E_GL_ARB_uniform_buffer_object,
        E_GL_ARB_viewport_array,

        // EXT
        E_GL_EXT_device_group,
        E_GL_EXT_gpu_shader4,
        E_GL_EXT_multiview,
        E_GL_EXT_shader_image_load_formatted,
        E_GL_EXT_shader_non_constant_global_initializers, // ESSL
        E_GL_EXT_geometry_shader,
        E_GL_EXT_geometry_point_size,
        E_GL_EXT_gpu_shader5,
        E_GL_EXT_primitive_bounding_box,
        E_GL_EXT_shader_io_blocks,
        E_GL_EXT_tessellation_shader,
        E_GL_EXT_tessellation_point_size,
        E_GL_EXT_texture_buffer,
        E_GL_EXT_texture_cube_map_array,
        E_GL_EXT_frag_depth,
        E_GL_EXT_shader_texture_lod,

        // GOOGLE
        E_GL_GOOGLE_cpp_style_line_directive,
        E_GL_GOOGLE_include_directive,

        // KHR
        E_GL_KHR_blend_equation_advanced,

        // NV
        E_GL_NV_geometry_shader_passthrough,
        E_GL_NV_sample_mask_override_coverage,
        E_GL_NV_stereo_view_rendering,
        E_GL_NV_viewport_array2,

        // NVX
        E_GL_NVX_multiview_per_view_attributes,

        // OES
        E_GL_OES_EGL_image_external,
        E_GL_OES_geometry_point_size,
        E_GL_OES_geometry_shader,
        E_GL_OES_gpu_shader5,
        E_GL_OES_primitive_bounding_box,
        E_GL_OES_sample_variables,
        E_GL_OES_shader_image_atomic,
        E_GL_OES_shader_io_blocks,
        E_GL_OES_shader_multisample_interpolation,
        E_GL_OES_standard_derivatives,
        E_GL_OES_tessellation_point_size,
        E_GL_OES_tessellation_shader,
        E_GL_OES_texture_3D,
        E_GL_OES_texture_buffer,
        E_GL_OES_texture_cube_map_array,
        E_GL_OES_texture_storage_multisample_2d_array,
    };
    return refList;
}


} // /namespace Xsc



// ================================================================================