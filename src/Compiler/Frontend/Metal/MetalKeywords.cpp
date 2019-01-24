/*
 * MetalKeywords.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "MetalKeywords.h"
#include "Dictionary.h"
#include "Helper.h"
#include "ReportIdents.h"
#include "Exception.h"


namespace Xsc
{


/* 
 * Internal functions
 */

template <typename Key, typename Value>
const Value* MapTypeToKeyword(const std::map<Key, Value>& typeMap, const Key& type)
{
    auto it = typeMap.find(type);
    return (it != typeMap.end() ? &(it->second) : nullptr);
}


/* ----- DataType Mapping ----- */

static Dictionary<DataType> GenerateDataTypeDict()
{
    using T = DataType;

    return
    {
        { "bool",       T::Bool      },
        { "int",        T::Int       },
        { "uint",       T::UInt      },
        { "half",       T::Half      },
        { "float",      T::Float     },
        { "double",     T::Double    },

        { "bool2",      T::Bool2     },
        { "bool3",      T::Bool3     },
        { "bool4",      T::Bool4     },
        { "int2",   	T::Int2      },
        { "int3",       T::Int3      },
        { "int4",       T::Int4      },
        { "uint2",      T::UInt2     },
        { "uint3",      T::UInt3     },
        { "uint4",      T::UInt4     },
        { "float2",     T::Float2    },
        { "float3",     T::Float3    },
        { "float4",     T::Float4    },
        { "half2",      T::Half2     },
        { "half3",      T::Half3     },
        { "half4",      T::Half4     },
        { "double2",    T::Double2   },
        { "double3",    T::Double3   },
        { "double4",    T::Double4   },

        { "float2",     T::Float2x2  },
        { "float2x3",   T::Float2x3  },
        { "float2x4",   T::Float2x4  },
        { "float3x2",   T::Float3x2  },
        { "float3",     T::Float3x3  },
        { "float3x4",   T::Float3x4  },
        { "float4x2",   T::Float4x2  },
        { "float4x3",   T::Float4x3  },
        { "float4",     T::Float4x4  },
        { "half2",      T::Half2x2   },
        { "half2x3",    T::Half2x3   },
        { "half2x4",    T::Half2x4   },
        { "half3x2",    T::Half3x2   },
        { "half3",      T::Half3x3   },
        { "half3x4",    T::Half3x4   },
        { "half4x2",    T::Half4x2   },
        { "half4x3",    T::Half4x3   },
        { "half4",      T::Half4x4   },
        /*
        TODO: currently disabled
        -> "0.0" is read as double precision per default, which results in double precision matrices in most cases
        */
        #if 0
        { "double2",    T::Double2x2 },
        { "double2x3",  T::Double2x3 },
        { "double2x4",  T::Double2x4 },
        { "double3x2",  T::Double3x2 },
        { "double3",    T::Double3x3 },
        { "double3x4",  T::Double3x4 },
        { "double4x2",  T::Double4x2 },
        { "double4x3",  T::Double4x3 },
        { "double4",    T::Double4x4 },
        #else
        { "float2",     T::Double2x2 },
        { "float2x3",   T::Double2x3 },
        { "float2x4",   T::Double2x4 },
        { "float3x2",   T::Double3x2 },
        { "float3",     T::Double3x3 },
        { "float3x4",   T::Double3x4 },
        { "float4x2",   T::Double4x2 },
        { "float4x3",   T::Double4x3 },
        { "float4",     T::Double4x4 },
        #endif
    };
}

static const auto g_dataTypeDictMetal = GenerateDataTypeDict();

const std::string* DataTypeToMetalKeyword(const DataType t)
{
    return g_dataTypeDictMetal.EnumToString(t);
}


/* ----- StorageClass Mapping ----- */

static Dictionary<StorageClass> GenerateStorageClassDict()
{
    using T = StorageClass;

    return
    {
        { "extern",   T::Extern      },
      //{ "precise",  T::Precise,    },
      //{ "shared",   T::Shared      },
      //{ "shared",   T::GroupShared },
        { "static",   T::Static      },
        { "volatile", T::Volatile    },
    };
}

static const auto g_storageClassDictMetal = GenerateStorageClassDict();

const std::string* StorageClassToMetalKeyword(const StorageClass t)
{
    return g_storageClassDictMetal.EnumToString(t);
}


/* ----- InterpModifier Mapping ----- */

static Dictionary<InterpModifier> GenerateInterpModifierDict()
{
    using T = InterpModifier;

    return
    {
        { "centroid_perspective",   T::Centroid        },
        { "center_perspective",     T::Linear          },
        { "flat",                   T::NoInterpolation },
        { "center_no_perspective",  T::NoPerspective   },
        { "sample_perspective",     T::Sample          },
    };
}

static const auto g_inperpModifierDictMetal = GenerateInterpModifierDict();

const std::string* InterpModifierToMetalKeyword(const InterpModifier t)
{
    return g_inperpModifierDictMetal.EnumToString(t);
}


/* ----- BufferType Mapping ----- */

static std::map<BufferType, std::string> GenerateBufferTypeMap()
{
    using T = BufferType;

    return
    {
        { T::Buffer,                  "buffer"            },
        { T::StructuredBuffer,        "buffer"            },
        { T::ByteAddressBuffer,       "buffer"            },

        { T::RWBuffer,                "buffer"            },
        { T::RWStructuredBuffer,      "buffer"            },
        { T::RWByteAddressBuffer,     "buffer"            },
        { T::AppendStructuredBuffer,  "buffer"            },
        { T::ConsumeStructuredBuffer, "buffer"            },

        { T::RWTexture1D,             "texture1d"         },
        { T::RWTexture1DArray,        "texture1d_array"   },
        { T::RWTexture2D,             "texture2d"         },
        { T::RWTexture2DArray,        "texture2d_array"   },
        { T::RWTexture3D,             "texture3d"         },

        { T::Texture1D,               "texture1d"         },
        { T::Texture1DArray,          "texture1d_array"   },
        { T::Texture2D,               "texture2d"         },
        { T::Texture2DArray,          "texture2d_array"   },
        { T::Texture3D,               "texture3d"         },
        { T::TextureCube,             "texturecube"       },
        { T::TextureCubeArray,        "texturecube_array" },
        { T::Texture2DMS,             "texture2d_ms"      },
      //{ T::Texture2DMSArray,        ""                  },

        { T::GenericTexture,          "texture2d"         },

      //{ T::InputPatch,              ""                  },
      //{ T::OutputPatch,             ""                  },

      //{ T::PointStream,             ""                  },
      //{ T::LineStream,              ""                  },
      //{ T::TriangleStream,          ""                  },
    };
}

const std::string* BufferTypeToMetalKeyword(const BufferType t)
{
    return MapTypeToKeyword(GenerateBufferTypeMap(), t);
}


/* ----- SamplerType Mapping ----- */

static Dictionary<SamplerType> GenerateSamplerTypeDict()
{
    using T = SamplerType;

    return
    {
        { "texture1d",              T::Sampler1D              },
        { "texture2d",              T::Sampler2D              },
        { "texture3d",              T::Sampler3D              },
        { "texturecube",            T::SamplerCube            },
        { "texture2d",              T::Sampler2DRect          },
        { "texture1d_array",        T::Sampler1DArray         },
        { "texture2d_array",        T::Sampler2DArray         },
        { "texturecube_array",      T::SamplerCubeArray       },
      //{ "",                       T::SamplerBuffer          },
        { "texture2d_ms",           T::Sampler2DMS            },
      //{ "",                       T::Sampler2DMSArray       },
      //{ "",                       T::Sampler1DShadow        },
        { "depth2d",                T::Sampler2DShadow        },
        { "depthcube",              T::SamplerCubeShadow      },
        { "depth2d",                T::Sampler2DRectShadow    },
      //{ "",                       T::Sampler1DArrayShadow   },
        { "depth2d_array",          T::Sampler2DArrayShadow   },
        { "depthcube_array",        T::SamplerCubeArrayShadow },

        { "sampler",                T::SamplerState           },
        { "sampler",                T::SamplerComparisonState },
    };
}

static const auto g_samplerTypeDictMetal = GenerateSamplerTypeDict();

const std::string* SamplerTypeToMetalKeyword(const SamplerType t)
{
    return g_samplerTypeDictMetal.EnumToString(t);
}


/* ----- Semantic Mapping ----- */

struct MetalSemanticDescriptor
{
    inline MetalSemanticDescriptor(const std::string& keyword, bool hasIndex = false) :
        keyword  { keyword  },
        hasIndex { hasIndex }
    {
    }

    std::string keyword;
    bool        hasIndex    = false;
};

static std::map<Semantic, MetalSemanticDescriptor> GenerateSemanticMap()
{
    using T = Semantic;

    return
    {
        { T::ClipDistance,           { "clip_distance"                } },
      //{ T::CullDistance,           { ""                             } },
        { T::Coverage,               { "sample_mask"                  } },
        { T::Depth,                  { "depth(any)"                   } },
        { T::DepthGreaterEqual,      { "depth(greater)"               } },
        { T::DepthLessEqual,         { "depth(less)"                  } },
        { T::DispatchThreadID,       { "thread_position_in_grid"      } },
      //{ T::DomainLocation,         { ""                             } },
        { T::FragCoord,              { "position"                     } }, //???
        { T::GroupID,                { "threadgroup_position_in_grid" } },
      //{ T::GroupIndex,             { ""                             } },
        { T::GroupThreadID,          { "thread_index_in_threadgroup"  } },
      //{ T::GSInstanceID,           { ""                             } },
      //{ T::InnerCoverage,          { ""                             } },
      //{ T::InsideTessFactor,       { ""                             } },
        { T::InstanceID,             { "instance_id"                  } },
        { T::IsFrontFace,            { "front_facing"                 } },
      //{ T::OutputControlPointID,   { ""                             } },
        { T::PointSize,              { "point_size"                   } },
      //{ T::PrimitiveID,            { ""                             } },
        { T::RenderTargetArrayIndex, { "render_target_array_index"    } },
        { T::SampleIndex,            { "sample_id"                    } },
      //{ T::StencilRef,             { ""                             } },
        { T::Target,                 { "color",                  true } },
      //{ T::TessFactor,             { ""                             } },
        { T::VertexID,               { "vertex_id"                    } },
        { T::VertexPosition,         { "position"                     } },
        { T::ViewportArrayIndex,     { "viewport_array_index"         } },
    };
}

std::unique_ptr<std::string> SemanticToMetalKeyword(const IndexedSemantic& semantic)
{
    static const auto typeMap = GenerateSemanticMap();
    if (auto type = MapTypeToKeyword(typeMap, Semantic(semantic)))
    {
        if (type->hasIndex)
            return MakeUnique<std::string>(type->keyword + "(" + std::to_string(semantic.Index()) + ")");
        else
            return MakeUnique<std::string>(type->keyword);
    }
    return nullptr;
}


/* ----- Reserved Metal Keywords ----- */

const std::set<std::string>& ReservedMetalKeywords()
{
    static const std::set<std::string> reservedNames
    {
        // Keywords
        "vertex",
        "kernel",
        "fragment",
        "using",
        "namespace",
        "template",
        "typename",
        //TODO...

        // Types
        "bool",
        "char",
        "uchar",
        "short",
        "ushort",
        "int",
        "uint",
        "half",
        "float",
        "double",

        "bool2",
        "bool3",
        "bool4",
        "int2",
        "int3",
        "int4",
        "uint2",
        "uint3",
        "uint4",
        "half2",
        "half3",
        "half4",
        "float2",
        "float3",
        "float4",
        "double2",
        "double3",
        "double4",

        "half2x2",
        "half2x3",
        "half2x4",
        "half3x2",
        "half3x3",
        "half3x4",
        "half4x2",
        "half4x3",
        "half4x4",
        "float2x2",
        "float2x3",
        "float2x4",
        "float3x2",
        "float3x3",
        "float3x4",
        "float4x2",
        "float4x3",
        "float4x4",
        "double2x2",
        "double2x3",
        "double2x4",
        "double3x2",
        "double3x3",
        "double3x4",
        "double4x2",
        "double4x3",
        "double4x4",

        // Semantics
        "sampler",
        "texture",
        "buffer",

        // Intrinsics
        /*"abs",
        "acos",
        "all",
        "any",
        "asin",
        "barrier",
        "bitCount",
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
        "trunc",*/
    };

    return reservedNames;
}


} // /namespace Xsc



// ================================================================================
