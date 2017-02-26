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
        { T::Bool,      "bool"   },
        { T::Int,       "int"    },
        { T::UInt,      "uint"   },
        { T::Half,      "float"  },
        { T::Float,     "float"  },
        { T::Double,    "double" },

        { T::Bool2,     "bvec2"  },
        { T::Bool3,     "bvec3"  },
        { T::Bool4,     "bvec4"  },
        { T::Int2,      "ivec2"  },
        { T::Int3,      "ivec3"  },
        { T::Int4,      "ivec4"  },
        { T::UInt2,     "uvec2"  },
        { T::UInt3,     "uvec3"  },
        { T::UInt4,     "uvec4"  },
        { T::Half2,     "vec2"   },
        { T::Half3,     "vec3"   },
        { T::Half4,     "vec4"   },
        { T::Float2,    "vec2"   },
        { T::Float3,    "vec3"   },
        { T::Float4,    "vec4"   },
        { T::Double2,   "dvec2"  },
        { T::Double3,   "dvec3"  },
        { T::Double4,   "dvec4"  },

        { T::Float2x2,  "mat2"   },
        { T::Float2x3,  "mat2x3" },
        { T::Float2x4,  "mat2x4" },
        { T::Float3x2,  "mat3x2" },
        { T::Float3x3,  "mat3"   },
        { T::Float3x4,  "mat3x4" },
        { T::Float4x2,  "mat4x2" },
        { T::Float4x3,  "mat4x3" },
        { T::Float4x4,  "mat4"   },
        { T::Double2x2, "mat2"   },
        { T::Double2x3, "mat2x3" },
        { T::Double2x4, "mat2x4" },
        { T::Double3x2, "mat3x2" },
        { T::Double3x3, "mat3"   },
        { T::Double3x4, "mat3x4" },
        { T::Double4x2, "mat4x2" },
        { T::Double4x3, "mat4x3" },
        { T::Double4x4, "mat4"   },
    };
}

const std::string* DataTypeToGLSLKeyword(const DataType t)
{
    static const auto typeMap = GenerateDataTypeMap();
    return MapTypeToKeyword(typeMap, t);
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
        { T::Buffer,                  "buffer"           },
        { T::StructuredBuffer,        "buffer"           },
        { T::ByteAddressBuffer,       "buffer"           },

        { T::RWBuffer,                "buffer"           },
        { T::RWStructuredBuffer,      "buffer"           },
        { T::RWByteAddressBuffer,     "buffer"           },
        { T::AppendStructuredBuffer,  "buffer"           },
        { T::ConsumeStructuredBuffer, "buffer"           },

      //{ T::RWTexture1D,             ""                 },
      //{ T::RWTexture1DArray,        ""                 },
      //{ T::RWTexture2D,             ""                 },
      //{ T::RWTexture2DArray,        ""                 },
      //{ T::RWTexture3D,             ""                 },

        { T::Texture1D,               "sampler1D"        },
        { T::Texture1DArray,          "sampler1DArray"   },
        { T::Texture2D,               "sampler2D"        },
        { T::Texture2DArray,          "sampler2DArray"   },
        { T::Texture3D,               "sampler3D"        },
        { T::TextureCube,             "samplerCube"      },
        { T::TextureCubeArray,        "samplerCubeArray" },
        { T::Texture2DMS,             "sampler2DMS"      },
        { T::Texture2DMSArray,        "sampler2DMSArray" },

      //{ T::GenericTexture,          ""                 },

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
        { T::Buffer,                  "buffer"           },
        { T::StructuredBuffer,        "buffer"           },
        { T::ByteAddressBuffer,       "buffer"           },

        { T::RWBuffer,                "buffer"           },
        { T::RWStructuredBuffer,      "buffer"           },
        { T::RWByteAddressBuffer,     "buffer"           },
        { T::AppendStructuredBuffer,  "buffer"           },
        { T::ConsumeStructuredBuffer, "buffer"           },

      //{ T::RWTexture1D,             ""                 },
      //{ T::RWTexture1DArray,        ""                 },
      //{ T::RWTexture2D,             ""                 },
      //{ T::RWTexture2DArray,        ""                 },
      //{ T::RWTexture3D,             ""                 },

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

const std::string* BufferTypeToGLSLKeyword(const BufferType t, bool useVulkanGLSL)
{
    static const auto typeMapGLSL = GenerateBufferTypeMap();
    static const auto typeMapVKSL = GenerateBufferTypeMapVKSL();
    return MapTypeToKeyword((useVulkanGLSL ? typeMapVKSL : typeMapGLSL), t);
}


/* ----- BufferType Mapping ----- */

static std::map<SamplerType, std::string> GenerateSamplerTypeMap()
{
    using T = SamplerType;

    return
    {
        { T::Sampler1D,              "sampler1D"   },
        { T::Sampler2D,              "sampler2D"   },
        { T::Sampler3D,              "sampler3D"   },
        { T::SamplerCube,            "samplerCube" },
      //{ T::SamplerState,           ""            },
      //{ T::SamplerComparisonState, ""            },
    };
}

const std::string* SamplerTypeToGLSLKeyword(const SamplerType t, bool useVulkanGLSL)
{
    static const auto typeMap = GenerateSamplerTypeMap();
    static const std::string samplerTypeVKSL = "sampler";
    return (useVulkanGLSL ? (&samplerTypeVKSL) : MapTypeToKeyword(typeMap, t));
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
        { T::GroupID,                { "gl_WorkGroupID"               } },
        { T::GroupIndex,             { "gl_LocalInvocationIndex"      } },
        { T::GroupThreadID,          { "gl_LocalInvocationID"         } },
        { T::GSInstanceID,           { "gl_InvocationID"              } },
        { T::InnerCoverage,          { "gl_SampleMaskIn"              } },
        { T::InsideTessFactor,       { "gl_TessLevelInner"            } },
        { T::InstanceID,             { "gl_InstanceID"                } }, // gl_InstanceID (GLSL), gl_InstanceIndex (Vulkan)
        { T::IsFrontFace,            { "gl_FrontFacing"               } },
        { T::OutputControlPointID,   { "gl_InvocationID"              } },
        { T::FragCoord,               { "gl_FragCoord"                 } },
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
        "groupMemoryBarrier",
        "memoryBarrierImage",
        "memoryBarrier",
        "distance",
        "dot",
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
        "log",
        "log2",
        "fma",
        "max",
        "min",
        "modf",
        "noise",
        "normalize",
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


} // /namespace Xsc



// ================================================================================