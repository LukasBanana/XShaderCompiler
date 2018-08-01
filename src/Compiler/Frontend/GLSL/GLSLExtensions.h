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

// Declares an extension string as 'static const char* E_<NAME> = "<NAME>"' where <NAME> is the specified identifier
#define DECL_EXTENSION(NAME) \
    static const char* const E_##NAME = #NAME

// 3DL
DECL_EXTENSION( GL_3DL_array_objects                            );

// AMD
DECL_EXTENSION( GL_AMD_gcn_shader                               );
DECL_EXTENSION( GL_AMD_gpu_shader_half_float                    );
DECL_EXTENSION( GL_AMD_shader_ballot                            );
DECL_EXTENSION( GL_AMD_shader_explicit_vertex_parameter         );
DECL_EXTENSION( GL_AMD_shader_trinary_minmax                    );

// ANDROID
DECL_EXTENSION( GL_ANDROID_extension_pack_es31a                 );

// ARB
DECL_EXTENSION( GL_ARB_arrays_of_arrays                         );
DECL_EXTENSION( GL_ARB_compute_shader                           );
DECL_EXTENSION( GL_ARB_cull_distance                            );
DECL_EXTENSION( GL_ARB_derivative_control                       );
DECL_EXTENSION( GL_ARB_enhanced_layouts                         );
DECL_EXTENSION( GL_ARB_explicit_attrib_location                 );
DECL_EXTENSION( GL_ARB_fragment_coord_conventions               );
DECL_EXTENSION( GL_ARB_gpu_shader5                              );
DECL_EXTENSION( GL_ARB_gpu_shader_fp64                          );
DECL_EXTENSION( GL_ARB_gpu_shader_int64                         );
DECL_EXTENSION( GL_ARB_separate_shader_objects                  );
DECL_EXTENSION( GL_ARB_shading_language_420pack                 );
DECL_EXTENSION( GL_ARB_explicit_uniform_location                );
DECL_EXTENSION( GL_ARB_shader_atomic_counters                   );
DECL_EXTENSION( GL_ARB_shader_ballot                            );
DECL_EXTENSION( GL_ARB_shader_bit_encoding                      );
DECL_EXTENSION( GL_ARB_shader_draw_parameters                   );
DECL_EXTENSION( GL_ARB_shader_group_vote                        );
DECL_EXTENSION( GL_ARB_shader_image_load_store                  );
DECL_EXTENSION( GL_ARB_shader_storage_buffer_object             );
DECL_EXTENSION( GL_ARB_shader_texture_image_samples             );
DECL_EXTENSION( GL_ARB_shader_texture_lod                       );
DECL_EXTENSION( GL_ARB_shader_viewport_layer_array              );
DECL_EXTENSION( GL_ARB_sparse_texture2                          );
DECL_EXTENSION( GL_ARB_sparse_texture_clamp                     );
DECL_EXTENSION( GL_ARB_tessellation_shader                      );
DECL_EXTENSION( GL_ARB_texture_cube_map_array                   );
DECL_EXTENSION( GL_ARB_texture_gather                           );
DECL_EXTENSION( GL_ARB_texture_multisample                      );
DECL_EXTENSION( GL_ARB_texture_query_lod                        );
DECL_EXTENSION( GL_ARB_texture_rectangle                        );
DECL_EXTENSION( GL_ARB_uniform_buffer_object                    );
DECL_EXTENSION( GL_ARB_viewport_array                           );

// EXT
DECL_EXTENSION( GL_EXT_device_group                             );
DECL_EXTENSION( GL_EXT_gpu_shader4                              );
DECL_EXTENSION( GL_EXT_multiview                                );
DECL_EXTENSION( GL_EXT_shader_image_load_formatted              );
DECL_EXTENSION( GL_EXT_shader_non_constant_global_initializers  ); // ESSL
DECL_EXTENSION( GL_EXT_geometry_shader                          );
DECL_EXTENSION( GL_EXT_geometry_point_size                      );
DECL_EXTENSION( GL_EXT_gpu_shader5                              );
DECL_EXTENSION( GL_EXT_primitive_bounding_box                   );
DECL_EXTENSION( GL_EXT_shader_io_blocks                         );
DECL_EXTENSION( GL_EXT_tessellation_shader                      );
DECL_EXTENSION( GL_EXT_tessellation_point_size                  );
DECL_EXTENSION( GL_EXT_texture_buffer                           );
DECL_EXTENSION( GL_EXT_texture_cube_map_array                   );
DECL_EXTENSION( GL_EXT_frag_depth                               );
DECL_EXTENSION( GL_EXT_shader_texture_lod                       );

// GOOGLE
DECL_EXTENSION( GL_GOOGLE_cpp_style_line_directive              );
DECL_EXTENSION( GL_GOOGLE_include_directive                     );

// KHR
DECL_EXTENSION( GL_KHR_blend_equation_advanced                  );

// NV
DECL_EXTENSION( GL_NV_geometry_shader_passthrough               );
DECL_EXTENSION( GL_NV_sample_mask_override_coverage             );
DECL_EXTENSION( GL_NV_stereo_view_rendering                     );
DECL_EXTENSION( GL_NV_viewport_array2                           );

// NVX
DECL_EXTENSION( GL_NVX_multiview_per_view_attributes            );

// OES
DECL_EXTENSION( GL_OES_EGL_image_external                       );
DECL_EXTENSION( GL_OES_geometry_point_size                      );
DECL_EXTENSION( GL_OES_geometry_shader                          );
DECL_EXTENSION( GL_OES_gpu_shader5                              );
DECL_EXTENSION( GL_OES_primitive_bounding_box                   );
DECL_EXTENSION( GL_OES_sample_variables                         );
DECL_EXTENSION( GL_OES_shader_image_atomic                      );
DECL_EXTENSION( GL_OES_shader_io_blocks                         );
DECL_EXTENSION( GL_OES_shader_multisample_interpolation         );
DECL_EXTENSION( GL_OES_standard_derivatives                     );
DECL_EXTENSION( GL_OES_tessellation_point_size                  );
DECL_EXTENSION( GL_OES_tessellation_shader                      );
DECL_EXTENSION( GL_OES_texture_3D                               );
DECL_EXTENSION( GL_OES_texture_buffer                           );
DECL_EXTENSION( GL_OES_texture_cube_map_array                   );
DECL_EXTENSION( GL_OES_texture_storage_multisample_2d_array     );

#undef DECL_EXTENSION


// Returns a map of GLSL extension strings with their minimum required version number (default is 110).
const std::map<std::string, int>& GetGLSLExtensionVersionMap();


} // /namespace Xsc


#endif



// ================================================================================