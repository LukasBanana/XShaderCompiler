/*
 * GLSLKeywords.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
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
        { T::Uniform,         "uniform"       },
        { T::Volatile,        "volatile"      },

        { T::NoInterpolation, "flat"          },
        { T::Linear,          "smooth"        },
        { T::Centroid,        "centroid"      },
        { T::NoPerspective,   "noperspective" },
        { T::Sample,          "sample"        },
    };
}

const std::string* StorageClassToGLSLKeyword(const StorageClass t)
{
    static const auto typeMap = GenerateStorageClassMap();
    return MapTypeToKeyword(typeMap, t);
}


/* ----- BufferType Mapping ----- */

static std::map<BufferType, std::string> GenerateBufferTypeMap()
{
    using T = BufferType;

    return
    {
        { T::Buffer,                  "buffer"           },
        { T::StucturedBuffer,         "buffer"           },
        { T::ByteAddressBuffer,       "buffer"           },

        { T::RWBuffer,                "buffer"           },
        { T::RWStucturedBuffer,       "buffer"           },
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
    };
}

const std::string* BufferTypeToGLSLKeyword(const BufferType t)
{
    static const auto typeMap = GenerateBufferTypeMap();
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
        { T::DomainLocation,         { "gl_TessCord"                  } },
        { T::GroupID,                { "gl_WorkGroupID"               } },
        { T::GroupIndex,             { "gl_LocalInvocationIndex"      } },
        { T::GroupThreadID,          { "gl_LocalInvocationID"         } },
        { T::GSInstanceID,           { "gl_InvocationID"              } },
        { T::InnerCoverage,          { "gl_SampleMaskIn"              } },
        { T::InsideTessFactor,       { "gl_TessLevelInner"            } },
        { T::InstanceID,             { "gl_InstanceID"                } }, // gl_InstanceID (GLSL), gl_InstanceIndex (Vulkan)
        { T::IsFrontFace,            { "gl_FrontFacing"               } },
        { T::OutputControlPointID,   { "gl_InvocationID"              } },
        { T::Position,               { "gl_FragCoord"                 } },
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

std::unique_ptr<std::string> SemanticToGLSLKeyword(const IndexedSemantic& semantic)
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


} // /namespace Xsc



// ================================================================================