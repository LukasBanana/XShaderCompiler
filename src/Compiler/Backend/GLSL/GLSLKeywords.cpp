/*
 * GLSLKeywords.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLSLKeywords.h"
#include "Helper.h"
#include <set>
#include <map>


namespace Xsc
{


/*
Here are a few references for HLSL-to-GLSL mappings:
https://anteru.net/blog/2016/mapping-between-hlsl-and-glsl/
https://msdn.microsoft.com/en-us/windows/uwp/gaming/glsl-to-hlsl-reference
*/

template <typename Key, typename Value>
const Value* MapTypeToKeyword(const std::map<Key, Value>& typeMap, const Key type)
{
    auto it = typeMap.find(type);
    return (it != typeMap.end() ? &(it->second) : nullptr);
}

/* ------ GLSL Keywords ----- */

static std::set<std::string> GenerateKeywordSet()
{
    return
    {
        "main",
        //TODO: add all GLSL keywords ...
    };
}

bool IsGLSLKeyword(const std::string& ident)
{
    static const auto keywordSet = GenerateKeywordSet();
    return (keywordSet.find(ident) != keywordSet.end());
}


/* ----- DataType Mapping ----- */

static std::map<DataType, std::string> GenerateDataTypeMap()
{
    using T = DataType;

    return
    {
        { T::Bool,      "bool"    },
        { T::Int,       "int"     },
        { T::UInt,      "uint"    },
        { T::Half,      "float"   },
        { T::Float,     "float"   },
        { T::Double,    "double"  },

        { T::Bool2,     "bvec2"   },
        { T::Bool3,     "bvec3"   },
        { T::Bool4,     "bvec4"   },
        { T::Int2,      "ivec2"   },
        { T::Int3,      "ivec3"   },
        { T::Int4,      "ivec4"   },
        { T::UInt2,     "uvec2"   },
        { T::UInt3,     "uvec3"   },
        { T::UInt4,     "uvec4"   },
        { T::Half2,     "vec2"    },
        { T::Half3,     "vec3"    },
        { T::Half4,     "vec4"    },
        { T::Float2,    "vec2"    },
        { T::Float3,    "vec3"    },
        { T::Float4,    "vec4"    },
        { T::Double2,   "dvec2"   },
        { T::Double3,   "dvec3"   },
        { T::Double4,   "dvec4"   },

        { T::Half2x2,   "mat2"    },
        { T::Half2x3,   "mat2x3"  },
        { T::Half2x4,   "mat2x4"  },
        { T::Half3x2,   "mat3x2"  },
        { T::Half3x3,   "mat3"    },
        { T::Half3x4,   "mat3x4"  },
        { T::Half4x2,   "mat4x2"  },
        { T::Half4x3,   "mat4x3"  },
        { T::Half4x4,   "mat4"    },
        { T::Float2x2,  "mat2"    },
        { T::Float2x3,  "mat2x3"  },
        { T::Float2x4,  "mat2x4"  },
        { T::Float3x2,  "mat3x2"  },
        { T::Float3x3,  "mat3"    },
        { T::Float3x4,  "mat3x4"  },
        { T::Float4x2,  "mat4x2"  },
        { T::Float4x3,  "mat4x3"  },
        { T::Float4x4,  "mat4"    },
        /*
        TODO: currently disabled
        -> "0.0" is read as double precision per default, which results in double precision matrices in most cases
        */
        #if 0
        { T::Double2x2, "dmat2"   },
        { T::Double2x3, "dmat2x3" },
        { T::Double2x4, "dmat2x4" },
        { T::Double3x2, "dmat3x2" },
        { T::Double3x3, "dmat3"   },
        { T::Double3x4, "dmat3x4" },
        { T::Double4x2, "dmat4x2" },
        { T::Double4x3, "dmat4x3" },
        { T::Double4x4, "dmat4"   },
        #else
        { T::Double2x2, "mat2"    },
        { T::Double2x3, "mat2x3"  },
        { T::Double2x4, "mat2x4"  },
        { T::Double3x2, "mat3x2"  },
        { T::Double3x3, "mat3"    },
        { T::Double3x4, "mat3x4"  },
        { T::Double4x2, "mat4x2"  },
        { T::Double4x3, "mat4x3"  },
        { T::Double4x4, "mat4"    },
        #endif
    };
}

const std::string* DataTypeToGLSLKeyword(const DataType t)
{
    static const auto typeMap = GenerateDataTypeMap();
    return MapTypeToKeyword(typeMap, t);
}

/* ----- DataType to ImageLayoutFormat Mapping ----- */

static std::map<DataType, ImageLayoutFormat> GenerateDataTypeImageLayoutFormatMap()
{
    using T = DataType;
    using U = ImageLayoutFormat;

    return
    {
        { T::Int,       U::I32X1    },
        { T::Int2,      U::I32X2    },
        { T::Int4,      U::I32X4    },
        { T::UInt,      U::UI32X1   },
        { T::UInt2,     U::UI32X2   },
        { T::UInt4,     U::UI32X4   },
        { T::Float,     U::F32X1    },
        { T::Float2,    U::F32X2    },
        { T::Float4,    U::F32X4    },
    };
}

ImageLayoutFormat DataTypeToImageLayoutFormat(const DataType t)
{
    static const auto typeMap = GenerateDataTypeImageLayoutFormatMap();

    auto it = typeMap.find(t);
    return (it != typeMap.end() ? it->second : ImageLayoutFormat::Undefined);
}


/* ----- StorageClass Mapping ----- */

static std::map<StorageClass, std::string> GenerateStorageClassMap()
{
    using T = StorageClass;

    return
    {
        { T::Extern,          "extern"        },
      //{ T::Precise,         ""              },
        { T::Shared,          "shared"        },
        { T::GroupShared,     "shared"        },
        { T::Static,          "static"        },
        { T::Volatile,        "volatile"      },
    };
}

const std::string* StorageClassToGLSLKeyword(const StorageClass t)
{
    static const auto typeMap = GenerateStorageClassMap();
    return MapTypeToKeyword(typeMap, t);
}


/* ----- InterpModifier Mapping ----- */

static std::map<InterpModifier, std::string> GenerateInterpModifierMap()
{
    using T = InterpModifier;

    return
    {
        { T::Linear,          "smooth"        },
        { T::Centroid,        "centroid"      },
        { T::NoInterpolation, "flat"          },
        { T::NoPerspective,   "noperspective" },
        { T::Sample,          "sample"        },
    };
}

const std::string* InterpModifierToGLSLKeyword(const InterpModifier t)
{
    static const auto typeMap = GenerateInterpModifierMap();
    return MapTypeToKeyword(typeMap, t);
}


/* ----- BufferType Mapping ----- */

static std::map<BufferType, std::string> GenerateBufferTypeMap()
{
    using T = BufferType;

    return
    {
        { T::Buffer,                  "samplerBuffer"    },
        { T::StructuredBuffer,        "buffer"           },
        { T::ByteAddressBuffer,       "buffer"           },

        { T::RWBuffer,                "imageBuffer"      },
        { T::RWStructuredBuffer,      "buffer"           },
        { T::RWByteAddressBuffer,     "buffer"           },
        { T::AppendStructuredBuffer,  "buffer"           },
        { T::ConsumeStructuredBuffer, "buffer"           },

        { T::RWTexture1D,             "image1D"          },
        { T::RWTexture1DArray,        "image1DArray"     },
        { T::RWTexture2D,             "image2D"          },
        { T::RWTexture2DArray,        "image2DArray"     },
        { T::RWTexture3D,             "image3D"          },

        { T::Texture1D,               "sampler1D"        },
        { T::Texture1DArray,          "sampler1DArray"   },
        { T::Texture2D,               "sampler2D"        },
        { T::Texture2DArray,          "sampler2DArray"   },
        { T::Texture3D,               "sampler3D"        },
        { T::TextureCube,             "samplerCube"      },
        { T::TextureCubeArray,        "samplerCubeArray" },
        { T::Texture2DMS,             "sampler2DMS"      },
        { T::Texture2DMSArray,        "sampler2DMSArray" },

        { T::GenericTexture,          "sampler2D"        }, //TODO: determine correct sampler type by its use

      //{ T::InputPatch,              ""                 },
      //{ T::OutputPatch,             ""                 },

        { T::PointStream,             "points"           },
        { T::LineStream,              "line_strip"       },
        { T::TriangleStream,          "triangle_strip"   },
    };
}

static std::map<BufferType, std::string> GenerateBufferTypeMapVKSL()
{
    using T = BufferType;

    return
    {
        { T::Buffer,                  "samplerBuffer"    },
        { T::StructuredBuffer,        "buffer"           },
        { T::ByteAddressBuffer,       "buffer"           },

        { T::RWBuffer,                "imageBuffer"      },
        { T::RWStructuredBuffer,      "buffer"           },
        { T::RWByteAddressBuffer,     "buffer"           },
        { T::AppendStructuredBuffer,  "buffer"           },
        { T::ConsumeStructuredBuffer, "buffer"           },

        { T::RWTexture1D,             "image1D"          },
        { T::RWTexture1DArray,        "image1DArray"     },
        { T::RWTexture2D,             "image2D"          },
        { T::RWTexture2DArray,        "image2DArray"     },
        { T::RWTexture3D,             "image3D"          },

        { T::Texture1D,               "texture1D"        },
        { T::Texture1DArray,          "texture1DArray"   },
        { T::Texture2D,               "texture2D"        },
        { T::Texture2DArray,          "texture2DArray"   },
        { T::Texture3D,               "texture3D"        },
        { T::TextureCube,             "textureCube"      },
        { T::TextureCubeArray,        "textureCubeArray" },
        { T::Texture2DMS,             "texture2DMS"      },
        { T::Texture2DMSArray,        "texture2DMSArray" },

      //{ T::GenericTexture,          ""                 },

      //{ T::InputPatch,              ""                 },
      //{ T::OutputPatch,             ""                 },

        { T::PointStream,             "points"           },
        { T::LineStream,              "line_strip"       },
        { T::TriangleStream,          "triangle_strip"   },
    };
}

const std::string* BufferTypeToGLSLKeyword(const BufferType t, bool useVulkanGLSL, bool separateSamplers)
{
    static const auto typeMapGLSL = GenerateBufferTypeMap();
    static const auto typeMapVKSL = GenerateBufferTypeMapVKSL();

    if (useVulkanGLSL && !separateSamplers)
    {
        auto samplerType = TextureTypeToSamplerType(t);
        if (samplerType != SamplerType::Undefined)
            useVulkanGLSL = false;
    }

    return MapTypeToKeyword((useVulkanGLSL ? typeMapVKSL : typeMapGLSL), t);
}


/* ----- BufferType Mapping ----- */

static std::map<SamplerType, std::string> GenerateSamplerTypeMap()
{
    using T = SamplerType;

    return
    {
        { T::Sampler1D,              "sampler1D"              },
        { T::Sampler2D,              "sampler2D"              },
        { T::Sampler3D,              "sampler3D"              },
        { T::SamplerCube,            "samplerCube"            },
        { T::Sampler2DRect,          "sampler2DRect"          },
        { T::Sampler1DArray,         "sampler1DArray"         },
        { T::Sampler2DArray,         "sampler2DArray"         },
        { T::SamplerCubeArray,       "samplerCubeArray"       },
        { T::SamplerBuffer,          "samplerBuffer"          },
        { T::Sampler2DMS,            "sampler2DMS"            },
        { T::Sampler2DMSArray,       "sampler2DMSArray"       },
        { T::Sampler1DShadow,        "sampler1DShadow"        },
        { T::Sampler2DShadow,        "sampler2DShadow"        },
        { T::SamplerCubeShadow,      "samplerCubeShadow"      },
        { T::Sampler2DRectShadow,    "sampler2DRectShadow"    },
        { T::Sampler1DArrayShadow,   "sampler1DArrayShadow"   },
        { T::Sampler2DArrayShadow,   "sampler2DArrayShadow"   },
        { T::SamplerCubeArrayShadow, "samplerCubeArrayShadow" },
        { T::SamplerState,           "sampler"                }, // Only for Vulkan
        { T::SamplerComparisonState, "samplerShadow"          }, // Only for Vulkan
    };
}

const std::string* SamplerTypeToGLSLKeyword(const SamplerType t)
{
    static const auto typeMap = GenerateSamplerTypeMap();
    return MapTypeToKeyword(typeMap, t);
}


/* ----- BufferType Mapping ----- */

static std::map<AttributeValue, std::string> GenerateAttributeValueMap()
{
    using T = AttributeValue;

    return
    {
        { T::DomainTri,                  "triangles"               },
        { T::DomainQuad,                 "quads"                   },
        { T::DomainIsoline,              "isolines"                },

      //{ T::OutputTopologyPoint,        ""                        }, // ignored in GLSL
      //{ T::OutputTopologyLine,         ""                        }, // ignored in GLSL
        { T::OutputTopologyTriangleCW,   "cw"                      },
        { T::OutputTopologyTriangleCCW,  "ccw"                     },

        { T::PartitioningInteger,        "equal_spacing"           },
        { T::PartitioningPow2,           "equal_spacing"           }, // ???
        { T::PartitioningFractionalEven, "fractional_even_spacing" },
        { T::PartitioningFractionalOdd,  "fractional_odd_spacing"  },
    };
}

const std::string* AttributeValueToGLSLKeyword(const AttributeValue t)
{
    static const auto typeMap = GenerateAttributeValueMap();
    return MapTypeToKeyword(typeMap, t);
}


/* ----- PrimitiveType Mapping ----- */

static std::map<PrimitiveType, std::string> GeneratePrimitiveTypeMap()
{
    using T = PrimitiveType;

    return
    {
        { T::Point,       "points"              },
        { T::Line,        "lines"               },
        { T::LineAdj,     "lines_adjacency"     },
        { T::Triangle,    "triangles"           },
        { T::TriangleAdj, "triangles_adjacency" },
    };
}

const std::string* PrimitiveTypeToGLSLKeyword(const PrimitiveType t)
{
    static const auto typeMap = GeneratePrimitiveTypeMap();
    return MapTypeToKeyword(typeMap, t);
}

/* ----- ImageLayoutFormat Mapping ----- */

static std::map<ImageLayoutFormat, std::string> GenerateImageLayoutFormatMap()
{
    using T = ImageLayoutFormat;

    return
    {
        { T::F32X4,         "rgba32f"           },
        { T::F32X2,         "rg32f"             },
        { T::F32X1,         "r32f"              },
        { T::F16X4,         "rgba16f"           },
        { T::F16X2,         "rg16f"             },
        { T::F16X1,         "r16f"              },
        { T::F11R11G10B,    "r11f_g11f_b10f"    },
        { T::UN32X4,        "rgba16"            },
        { T::UN16X2,        "rg16"              },
        { T::UN16X1,        "r16"               },
        { T::UN10R10G10B2A, "rgb10_a2"          },
        { T::UN8X4,         "rgba8"             },
        { T::UN8X2,         "rg8"               },
        { T::UN8X1,         "r8"                },
        { T::SN16X4,        "rgba16_snorm"      },
        { T::SN16X2,        "rg16_snorm"        },
        { T::SN16X1,        "r16_snorm"         },
        { T::SN8X4,         "rgba8_snorm"       },
        { T::SN8X2,         "rg8_snorm"         },
        { T::SN8X1,         "r8_snorm"          },
        { T::I32X4,         "rgba32i"           },
        { T::I32X2,         "rg32i"             },
        { T::I32X1,         "r32i"              },
        { T::I16X4,         "rgba16i"           },
        { T::I16X2,         "rg16i"             },
        { T::I16X1,         "r16i"              },
        { T::I8X4,          "rgba8i"            },
        { T::I8X2,          "rg8i"              },
        { T::I8X1,          "r8i"               },
        { T::UI32X4,        "rgba32ui"          },
        { T::UI32X2,        "rg32ui"            },
        { T::UI32X1,        "r32ui"             },
        { T::UI16X4,        "rgba16ui"          },
        { T::UI16X2,        "rg16ui"            },
        { T::UI16X1,        "r16ui"             },
        { T::UI10R10G10B2A, "rgb10_a2ui"        },
        { T::UI8X4,         "rgba8ui"           },
        { T::UI8X2,         "rg8ui"             },
        { T::UI8X1,         "r8ui"              },
    };
}

const std::string* ImageLayoutFormatToGLSLKeyword(const ImageLayoutFormat t)
{
    static const auto typeMap = GenerateImageLayoutFormatMap();
    return MapTypeToKeyword(typeMap, t);
}

/* ----- Semantic Mapping ----- */

struct GLSLSemanticDescriptor
{
    inline GLSLSemanticDescriptor(const std::string& keyword, bool hasIndex = false) :
        keyword { keyword  },
        hasIndex{ hasIndex }
    {
    }

    std::string keyword;
    bool        hasIndex    = false;
};

static std::map<Semantic, GLSLSemanticDescriptor> GenerateSemanticMap()
{
    using T = Semantic;

    return
    {
        { T::ClipDistance,           { "gl_ClipDistance",        true } },
        { T::CullDistance,           { "gl_CullDistance",        true } }, // if ARB_cull_distance is present
        { T::Coverage,               { "gl_SampleMask"                } },
        { T::Depth,                  { "gl_FragDepth"                 } },
        { T::DepthGreaterEqual,      { "gl_FragDepth"                 } }, // layout(depth_greater) out float gl_FragDepth;
        { T::DepthLessEqual,         { "gl_FragDepth"                 } }, // layout(depth_less) out float gl_FragDepth;
        { T::DispatchThreadID,       { "gl_GlobalInvocationID"        } },
        { T::DomainLocation,         { "gl_TessCoord"                 } },
        { T::FragCoord,              { "gl_FragCoord"                 } },
        { T::GroupID,                { "gl_WorkGroupID"               } },
        { T::GroupIndex,             { "gl_LocalInvocationIndex"      } },
        { T::GroupThreadID,          { "gl_LocalInvocationID"         } },
        { T::GSInstanceID,           { "gl_InvocationID"              } },
        { T::InnerCoverage,          { "gl_SampleMaskIn"              } },
        { T::InsideTessFactor,       { "gl_TessLevelInner"            } },
        { T::InstanceID,             { "gl_InstanceID"                } }, // gl_InstanceID (GLSL), gl_InstanceIndex (Vulkan)
        { T::IsFrontFace,            { "gl_FrontFacing"               } },
        { T::OutputControlPointID,   { "gl_InvocationID"              } },
        { T::PointSize,              { "gl_PointSize"                 } },
        { T::PrimitiveID,            { "gl_PrimitiveID"               } },
        { T::RenderTargetArrayIndex, { "gl_Layer"                     } },
        { T::SampleIndex,            { "gl_SampleID"                  } },
        { T::StencilRef,             { "gl_FragStencilRef"            } }, // if ARB_shader_stencil_export is present
        { T::Target,                 { "gl_FragData",            true } }, // only for GLSL 1.10
        { T::TessFactor,             { "gl_TessLevelOuter"            } },
        { T::VertexID,               { "gl_VertexID"                  } }, // gl_VertexID (GLSL), gl_VertexIndex (Vulkan)
        { T::VertexPosition,         { "gl_Position"                  } },
        { T::ViewportArrayIndex,     { "gl_ViewportIndex"             } },
    };
}

static std::unique_ptr<std::string> SemanticToGLSLKeywordPrimary(const IndexedSemantic& semantic)
{
    static const auto typeMap = GenerateSemanticMap();
    if (auto type = MapTypeToKeyword(typeMap, Semantic(semantic)))
    {
        if (type->hasIndex)
            return MakeUnique<std::string>(type->keyword + "[" + std::to_string(semantic.Index()) + "]");
        else
            return MakeUnique<std::string>(type->keyword);
    }
    return nullptr;
}

std::unique_ptr<std::string> SemanticToGLSLKeyword(const IndexedSemantic& semantic, bool useVulkanGLSL)
{
    if (useVulkanGLSL)
    {
        switch (semantic)
        {
            case Semantic::VertexID:
                return MakeUnique<std::string>("gl_VertexIndex");
            case Semantic::InstanceID:
                return MakeUnique<std::string>("gl_InstanceIndex");
            default:
                break;
        }
    }
    return SemanticToGLSLKeywordPrimary(semantic);
}


/* ----- Reserved GLSL Keywrods ----- */

const std::set<std::string>& ReservedGLSLKeywords()
{
    static const std::set<std::string> reservedNames
    {
        // Functions
        "main",

        // Keywords
        "layout",
        "attribute",
        "varying",
        "patch",

        // Types
        "bool",
        "int",
        "uint",
        "float",
        "float",
        "double",

        "bvec2",
        "bvec3",
        "bvec4",
        "ivec2",
        "ivec3",
        "ivec4",
        "uvec2",
        "uvec3",
        "uvec4",
        "vec2",
        "vec3",
        "vec4",
        "dvec2",
        "dvec3",
        "dvec4",

        "mat2",
        "mat2x3",
        "mat2x4",
        "mat3x2",
        "mat3",
        "mat3x4",
        "mat4x2",
        "mat4x3",
        "mat4",
        "mat2",
        "mat2x3",
        "mat2x4",
        "mat3x2",
        "mat3",
        "mat3x4",
        "mat4x2",
        "mat4x3",
        "mat4",
        "dmat2",
        "dmat2x3",
        "dmat2x4",
        "dmat3x2",
        "dmat3",
        "dmat3x4",
        "dmat4x2",
        "dmat4x3",
        "dmat4",
        "dmat2",
        "dmat2x3",
        "dmat2x4",
        "dmat3x2",
        "dmat3",
        "dmat3x4",
        "dmat4x2",
        "dmat4x3",
        "dmat4",

        "buffer",

        // Sampler types
        "sampler1D",
        "sampler2D",
        "sampler3D",
        "samplerCube",
        "sampler2DRect",
        "sampler1DArray",
        "sampler2DArray",
        "samplerCubeArray",
        "samplerBuffer",
        "sampler2DMS",
        "sampler2DMSArray",

        "isampler1D",
        "isampler2D",
        "isampler3D",
        "isamplerCube",
        "isampler2DRect",
        "isampler1DArray",
        "isampler2DArray",
        "isamplerCubeArray",
        "isamplerBuffer",
        "isampler2DMS",
        "isampler2DMSArray",

        "usampler1D",
        "usampler2D",
        "usampler3D",
        "usamplerCube",
        "usampler2DRect",
        "usampler1DArray",
        "usampler2DArray",
        "usamplerCubeArray",
        "usamplerBuffer",
        "usampler2DMS",
        "usampler2DMSArray",

        "sampler1DShadow",
        "sampler2DShadow",
        "samplerCubeShadow",
        "sampler2DRectShadow",
        "sampler1DArrayShadow",
        "sampler2DArrayShadow",
        "samplerCubeArrayShadow",

        // Image types
        "image1D",
        "image2D",
        "image3D",
        "image2DRect",
        "imageCube",
        "imageBuffer",
        "image1DArray",
        "image2DArray",
        "imageCubeArray",
        "image2DMS",
        "image2DMSArray",

        "iimage1D",
        "iimage2D",
        "iimage3D",
        "iimage2DRect",
        "iimageCube",
        "iimageBuffer",
        "iimage1DArray",
        "iimage2DArray",
        "iimageCubeArray",
        "iimage2DMS",
        "iimage2DMSArray",

        "uimage1D",
        "uimage2D",
        "uimage3D",
        "uimage2DRect",
        "uimageCube",
        "uimageBuffer",
        "uimage1DArray",
        "uimage2DArray",
        "uimageCubeArray",
        "uimage2DMS",
        "uimage2DMSArray",

        // Built-in variables,
        "gl_ClipDistance",
        "gl_CullDistance",
        "gl_FragCoord",
        "gl_FragData",
        "gl_FragDepth",
        "gl_FragStencilRef",
        "gl_FrontFacing",
        "gl_GlobalInvocationID",
        "gl_InvocationID",
        "gl_InstanceID",
        "gl_InstanceIndex",
        "gl_InvocationID",
        "gl_Layer",
        "gl_LocalInvocationIndex",
        "gl_LocalInvocationID",
        "gl_Position",
        "gl_PointSize",
        "gl_PrimitiveID",
        "gl_SampleID",
        "gl_SampleMask",
        "gl_SampleMaskIn",
        "gl_TessCoord",
        "gl_TessLevelInner",
        "gl_TessLevelOuter",
        "gl_VertexID",
        "gl_VertexIndex",
        "gl_ViewportIndex",
        "gl_WorkGroupID",

        // Built-in arrays
        "gl_in",
        "gl_out",

        // Intrinsics
        "abs",
        "acos",
        "all",
        "any",
        "asin",
        "barrier",
        "uint64BitsToDouble",
        "uintBitsToFloat",
        "floatBitsToInt",
        "floatBitsToUint",
        "atan",
        "atan",
        "ceil",
        "clamp",
        "cos",
        "cosh",
        "cross",
        "dFdx",
        "dFdxCoarse",
        "dFdxFine",
        "dFdy",
        "dFdyCoarse",
        "dFdyFine",
        "degrees",
        "determinant",
        "greaterThan",
        "greaterThanEqual",
        "groupMemoryBarrier",
        "memoryBarrierImage",
        "memoryBarrier",
        "distance",
        "dot",
        "equal",
        "interpolateAtCentroid",
        "interpolateAtSample",
        "interpolateAtOffset",
        "exp",
        "exp2",
        "faceforward",
        "findMSB",
        "findLSB",
        "floor",
        "fma",
        "mod",
        "fract",
        "frexp",
        "fwidth",
        "atomicAdd",
        "atomicAnd",
        "atomicCompSwap",
        "atomicExchange",
        "atomicMax",
        "atomicMin",
        "atomicOr",
        "atomicXor",
        "isinf",
        "isnan",
        "ldexp",
        "length",
        "mix",
        "lessThan",
        "lessThanEqual",
        "log",
        "log2",
        "fma",
        "max",
        "min",
        "modf",
        "noise1",
        "noise2",
        "noise3",
        "noise4",
        "normalize",
        "notEqual",
        "pow",
        "radians",
        "reflect",
        "refract",
        "round",
        "inversesqrt",
        "sign",
        "sin",
        "sinh",
        "smoothstep",
        "sqrt",
        "step",
        "tan",
        "tanh",
        "texture",
        "textureGrad",
        "textureGradOffset",
        "textureLod",
        "textureLodOffset",
        "textureProj",
        "textureSize",
        "texelFetch",
        "texelFetchOffset",
        "transpose",
        "trunc",

        "EmitVertex",
        "EmitStreamVertex",
        "EndPrimitive",
        "EndStreamPrimitive",

        // Reserved words (for future use)
        "input",
        "output",
        "typedef",
        "template",
        "this",
        "goto",
        "inline",
        "noinline",
    };

    return reservedNames;
}


/* ----- Semantic/DataType Mapping ----- */

static std::map<Semantic, DataType> GenerateSemanticDataTypeMap()
{
    using T = Semantic;
    using D = DataType;

    return
    {
        { T::ClipDistance,           D::Float  },
        { T::CullDistance,           D::Float  },
        { T::Coverage,               D::Int    },
        { T::Depth,                  D::Float  },
        { T::DepthGreaterEqual,      D::Float  },
        { T::DepthLessEqual,         D::Float  },
        { T::DispatchThreadID,       D::UInt3  },
        { T::DomainLocation,         D::Float3 },
        { T::GroupID,                D::UInt3  },
        { T::GroupIndex,             D::Int    },
        { T::GroupThreadID,          D::UInt3  },
        { T::GSInstanceID,           D::Int    },
        { T::InnerCoverage,          D::Int    },
        { T::InsideTessFactor,       D::Float  },
        { T::InstanceID,             D::Int    },
        { T::IsFrontFace,            D::Bool   },
        { T::OutputControlPointID,   D::Int    },
        { T::FragCoord,              D::Float4 },
        { T::PointSize,              D::Float  },
        { T::PrimitiveID,            D::Int    },
        { T::RenderTargetArrayIndex, D::Int    },
        { T::SampleIndex,            D::Int    },
        { T::StencilRef,             D::Int    },
      //{ T::Target,                 D::Float4 }, // Custom output in GLSL
        { T::TessFactor,             D::Float  },
        { T::VertexID,               D::Int    },
        { T::VertexPosition,         D::Float4 },
        { T::ViewportArrayIndex,     D::Int    },
    };
}

DataType SemanticToGLSLDataType(const Semantic t)
{
    static const auto typeMap = GenerateSemanticDataTypeMap();
    auto it = typeMap.find(t);
    return (it != typeMap.end() ? it->second : DataType::Undefined);
}


} // /namespace Xsc



// ================================================================================