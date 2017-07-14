/*
 * GLSLKeywords.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLSLKeywords.h"
#include "Dictionary.h"
#include "Helper.h"
#include "ReportIdents.h"
#include "Exception.h"


namespace Xsc
{


/* 
 * Internal functions
 */

/*
Here are a few references for HLSL-to-GLSL mappings:
https://anteru.net/blog/2016/mapping-between-hlsl-and-glsl/
https://msdn.microsoft.com/en-us/windows/uwp/gaming/glsl-to-hlsl-reference
*/

template <typename Key, typename Value>
const Value* MapTypeToKeyword(const std::map<Key, Value>& typeMap, const Key& type)
{
    auto it = typeMap.find(type);
    return (it != typeMap.end() ? &(it->second) : nullptr);
}

template <typename T>
T MapKeywordToType(const Dictionary<T>& typeDict, const std::string& keyword, const std::string& typeName)
{
    if (auto type = typeDict.StringToEnum(keyword))
        return *type;
    else
        RuntimeErr(R_FailedToMapFromGLSLKeyword(keyword, typeName));
}


/* ----- GLSL Keywords ----- */

// see https://www.khronos.org/registry/OpenGL/specs/gl/GLSLangSpec.4.50.pdf

static KeywordMapType GenerateKeywordMap()
{
    using T = Token::Types;

    return
    {
        { "true",                    T::BoolLiteral        },
        { "false",                   T::BoolLiteral        },

        { "bool",                    T::ScalarType         },
        { "int",                     T::ScalarType         },
        { "uint",                    T::ScalarType         },
        { "float",                   T::ScalarType         },
        { "double",                  T::ScalarType         },

        { "bvec2",                   T::VectorType         },
        { "bvec3",                   T::VectorType         },
        { "bvec4",                   T::VectorType         },
        { "ivec2",                   T::VectorType         },
        { "ivec3",                   T::VectorType         },
        { "ivec4",                   T::VectorType         },
        { "uint2",                   T::VectorType         },
        { "uint3",                   T::VectorType         },
        { "uint4",                   T::VectorType         },
        { "vec2",                    T::VectorType         },
        { "vec3",                    T::VectorType         },
        { "vec4",                    T::VectorType         },
        { "dvec2",                   T::VectorType         },
        { "dvec3",                   T::VectorType         },
        { "dvec4",                   T::VectorType         },

        { "mat2",                    T::MatrixType         },
        { "mat2x3",                  T::MatrixType         },
        { "mat2x4",                  T::MatrixType         },
        { "mat3x2",                  T::MatrixType         },
        { "mat3",                    T::MatrixType         },
        { "mat3x4",                  T::MatrixType         },
        { "mat4x2",                  T::MatrixType         },
        { "mat4x3",                  T::MatrixType         },
        { "mat4",                    T::MatrixType         },
        { "dmat2",                   T::MatrixType         },
        { "dmat2x3",                 T::MatrixType         },
        { "dmat2x4",                 T::MatrixType         },
        { "dmat3x2",                 T::MatrixType         },
        { "dmat3",                   T::MatrixType         },
        { "dmat3x4",                 T::MatrixType         },
        { "dmat4x2",                 T::MatrixType         },
        { "dmat4x3",                 T::MatrixType         },
        { "dmat4",                   T::MatrixType         },

        { "void",                    T::Void               },

        { "atomic_uint",             T::AtomicCounter      },

        { "do",                      T::Do                 },
        { "while",                   T::While              },
        { "for",                     T::For                },

        { "if",                      T::If                 },
        { "else",                    T::Else               },

        { "switch",                  T::Switch             },
        { "case",                    T::Case               },
        { "default",                 T::Default            },

        { "struct",                  T::Struct             },
        { "layout",                  T::LayoutQualifier    },
        { "attribute",               T::Attribute          },
        { "varying",                 T::Varying            },
        { "precision",               T::Precision          },

        { "lowp",                    T::PrecisionQualifier },
        { "mediump",                 T::PrecisionQualifier },
        { "highp",                   T::PrecisionQualifier },

        { "sampler1D",               T::Sampler            },
        { "sampler2D",               T::Sampler            },
        { "sampler3D",               T::Sampler            },
        { "samplerCube",             T::Sampler            },
        { "sampler1DShadow",         T::Sampler            },
        { "sampler2DShadow",         T::Sampler            },
        { "samplerCubeShadow",       T::Sampler            },
        { "sampler1DArray",          T::Sampler            },
        { "sampler2DArray",          T::Sampler            },
        { "sampler1DArrayShadow",    T::Sampler            },
        { "sampler2DArrayShadow",    T::Sampler            },
        { "sampler2DRect",           T::Sampler            },
        { "sampler2DRectShadow",     T::Sampler            },
        { "samplerBuffer",           T::Sampler            },
        { "sampler2DMS",             T::Sampler            },
        { "sampler2DMSArray",        T::Sampler            },
        { "samplerCubeArray",        T::Sampler            },
        { "samplerCubeArrayShadow",  T::Sampler            },

        { "isampler1D",              T::Sampler            },
        { "isampler2D",              T::Sampler            },
        { "isampler3D",              T::Sampler            },
        { "isamplerCube",            T::Sampler            },
        { "isampler1DArray",         T::Sampler            },
        { "isampler2DArray",         T::Sampler            },
        { "isampler2DRect",          T::Sampler            },
        { "isamplerBuffer",          T::Sampler            },
        { "isampler2DMS",            T::Sampler            },
        { "isampler2DMSArray",       T::Sampler            },
        { "isamplerCubeArray",       T::Sampler            },

        { "usampler1D",              T::Sampler            },
        { "usampler2D",              T::Sampler            },
        { "usampler3D",              T::Sampler            },
        { "usamplerCube",            T::Sampler            },
        { "usampler1DArray",         T::Sampler            },
        { "usampler2DArray",         T::Sampler            },
        { "usampler2DRect",          T::Sampler            },
        { "usamplerBuffer",          T::Sampler            },
        { "usampler2DMS",            T::Sampler            },
        { "usampler2DMSArray",       T::Sampler            },
        { "usamplerCubeArray",       T::Sampler            },

        { "image1D",                 T::Image              },
        { "image2D",                 T::Image              },
        { "image3D",                 T::Image              },
        { "image2DRect",             T::Image              },
        { "imageCube",               T::Image              },
        { "imageBuffer",             T::Image              },
        { "image1DArray",            T::Image              },
        { "image2DArray",            T::Image              },
        { "imageCubeArray",          T::Image              },
        { "image2DMS",               T::Image              },
        { "image2DMSArray",          T::Image              },

        { "iimage1D",                T::Image              },
        { "iimage2D",                T::Image              },
        { "iimage3D",                T::Image              },
        { "iimage2DRect",            T::Image              },
        { "iimageCube",              T::Image              },
        { "iimageBuffer",            T::Image              },
        { "iimage1DArray",           T::Image              },
        { "iimage2DArray",           T::Image              },
        { "iimageCubeArray",         T::Image              },
        { "iimage2DMS",              T::Image              },
        { "iimage2DMSArray",         T::Image              },

        { "uimage1D",                T::Image              },
        { "uimage2D",                T::Image              },
        { "uimage3D",                T::Image              },
        { "uimage2DRect",            T::Image              },
        { "uimageCube",              T::Image              },
        { "uimageBuffer",            T::Image              },
        { "uimage1DArray",           T::Image              },
        { "uimage2DArray",           T::Image              },
        { "uimageCubeArray",         T::Image              },
        { "uimage2DMS",              T::Image              },
        { "uimage2DMSArray",         T::Image              },

        { "uniform",                 T::UniformBuffer      },
        { "buffer",                  T::StorageBuffer      },

        { "break",                   T::CtrlTransfer       },
        { "continue",                T::CtrlTransfer       },
        { "discard",                 T::CtrlTransfer       },

        { "return",                  T::Return             },

        { "in",                      T::InputModifier      },
        { "out",                     T::InputModifier      },
        { "inout",                   T::InputModifier      },

        { "smooth",                  T::InterpModifier     },
        { "centroid",                T::InterpModifier     },
        { "flat",                    T::InterpModifier     },
        { "noperspective",           T::InterpModifier     },
        { "sample",                  T::InterpModifier     },

        { "const",                   T::TypeModifier       },

        { "precise",                 T::StorageClass       },
        { "shared",                  T::StorageClass       },
        { "patch",                   T::StorageClass       },

        { "coherent",                T::MemoryQualifier    },
        { "volatile",                T::MemoryQualifier    },
        { "restrict",                T::MemoryQualifier    },
        { "readonly",                T::MemoryQualifier    },
        { "writeonly",               T::MemoryQualifier    },

        { "invariant",               T::InvariantQualifier },

        { "common",                  T::Reserved           },
        { "partition",               T::Reserved           },
        { "active",                  T::Reserved           },
        { "asm",                     T::Reserved           },
        { "class",                   T::Reserved           },
        { "union",                   T::Reserved           },
        { "enum",                    T::Reserved           },
        { "typedef",                 T::Reserved           },
        { "template",                T::Reserved           },
        { "this",                    T::Reserved           },
        { "resource",                T::Reserved           },
        { "goto",                    T::Reserved           },
        { "inline",                  T::Reserved           },
        { "noinline",                T::Reserved           },
        { "public",                  T::Reserved           },
        { "static",                  T::Reserved           },
        { "extern",                  T::Reserved           },
        { "external",                T::Reserved           },
        { "interface",               T::Reserved           },
        { "long",                    T::Reserved           },
        { "short",                   T::Reserved           },
        { "half",                    T::Reserved           },
        { "fixed",                   T::Reserved           },
        { "unsigned",                T::Reserved           },
        { "superp",                  T::Reserved           },
        { "input",                   T::Reserved           },
        { "output",                  T::Reserved           },
        { "hvec2",                   T::Reserved           },
        { "hvec3",                   T::Reserved           },
        { "hvec4",                   T::Reserved           },
        { "fvec2",                   T::Reserved           },
        { "fvec3",                   T::Reserved           },
        { "fvec4",                   T::Reserved           },
        { "sampler3DRect",           T::Reserved           },
        { "filter",                  T::Reserved           },
        { "sizeof",                  T::Reserved           },
        { "cast",                    T::Reserved           },
        { "namespace",               T::Reserved           },
        { "using",                   T::Reserved           },

        { "subroutine",              T::Unsupported        },
    };
}

const KeywordMapType& GLSLKeywords()
{
    static const auto keywordMap = GenerateKeywordMap();
    return keywordMap;
}


/* ----- DataType Mapping ----- */

static Dictionary<DataType> GenerateDataTypeDict()
{
    using T = DataType;

    return
    {
        { "bool",    T::Bool      },
        { "int",     T::Int       },
        { "uint",    T::UInt      },
        { "float",   T::Half      },
        { "float",   T::Float     },
        { "double",  T::Double    },

        { "bvec2",   T::Bool2     },
        { "bvec3",   T::Bool3     },
        { "bvec4",   T::Bool4     },
        { "ivec2",   T::Int2      },
        { "ivec3",   T::Int3      },
        { "ivec4",   T::Int4      },
        { "uvec2",   T::UInt2     },
        { "uvec3",   T::UInt3     },
        { "uvec4",   T::UInt4     },
        { "vec2",    T::Float2    },
        { "vec3",    T::Float3    },
        { "vec4",    T::Float4    },
        { "vec2",    T::Half2     },
        { "vec3",    T::Half3     },
        { "vec4",    T::Half4     },
        { "dvec2",   T::Double2   },
        { "dvec3",   T::Double3   },
        { "dvec4",   T::Double4   },

        { "mat2",    T::Float2x2  },
        { "mat2x3",  T::Float2x3  },
        { "mat2x4",  T::Float2x4  },
        { "mat3x2",  T::Float3x2  },
        { "mat3",    T::Float3x3  },
        { "mat3x4",  T::Float3x4  },
        { "mat4x2",  T::Float4x2  },
        { "mat4x3",  T::Float4x3  },
        { "mat4",    T::Float4x4  },
        { "mat2",    T::Half2x2   },
        { "mat2x3",  T::Half2x3   },
        { "mat2x4",  T::Half2x4   },
        { "mat3x2",  T::Half3x2   },
        { "mat3",    T::Half3x3   },
        { "mat3x4",  T::Half3x4   },
        { "mat4x2",  T::Half4x2   },
        { "mat4x3",  T::Half4x3   },
        { "mat4",    T::Half4x4   },
        /*
        TODO: currently disabled
        -> "0.0" is read as double precision per default, which results in double precision matrices in most cases
        */
        #if 0
        { "dmat2",   T::Double2x2 },
        { "dmat2x3", T::Double2x3 },
        { "dmat2x4", T::Double2x4 },
        { "dmat3x2", T::Double3x2 },
        { "dmat3",   T::Double3x3 },
        { "dmat3x4", T::Double3x4 },
        { "dmat4x2", T::Double4x2 },
        { "dmat4x3", T::Double4x3 },
        { "dmat4",   T::Double4x4 },
        #else
        { "mat2",    T::Double2x2 },
        { "mat2x3",  T::Double2x3 },
        { "mat2x4",  T::Double2x4 },
        { "mat3x2",  T::Double3x2 },
        { "mat3",    T::Double3x3 },
        { "mat3x4",  T::Double3x4 },
        { "mat4x2",  T::Double4x2 },
        { "mat4x3",  T::Double4x3 },
        { "mat4",    T::Double4x4 },
        #endif
    };
}

static const auto g_dataTypeDictGLSL = GenerateDataTypeDict();

const std::string* DataTypeToGLSLKeyword(const DataType t)
{
    return g_dataTypeDictGLSL.EnumToString(t);
}

DataType GLSLKeywordToDataType(const std::string& keyword)
{
    return MapKeywordToType(g_dataTypeDictGLSL, keyword, R_DataType);
}


/* ----- StorageClass Mapping ----- */

static Dictionary<StorageClass> GenerateStorageClassDict()
{
    using T = StorageClass;

    return
    {
        { "extern",   T::Extern      },
        { "precise",  T::Precise,    },
        { "shared",   T::Shared      },
        { "shared",   T::GroupShared },
      //{ "static",   T::Static      }, // reserved GLSL keyword
        { "volatile", T::Volatile    },
    };
}

static const auto g_storageClassDictGLSL = GenerateStorageClassDict();

const std::string* StorageClassToGLSLKeyword(const StorageClass t)
{
    return g_storageClassDictGLSL.EnumToString(t);
}

StorageClass GLSLKeywordToStorageClass(const std::string& keyword)
{
    return MapKeywordToType(g_storageClassDictGLSL, keyword, R_StorageClass);
}


/* ----- InterpModifier Mapping ----- */

static Dictionary<InterpModifier> GenerateInterpModifierDict()
{
    using T = InterpModifier;

    return
    {
        { "smooth",        T::Linear          },
        { "centroid",      T::Centroid        },
        { "flat",          T::NoInterpolation },
        { "noperspective", T::NoPerspective   },
        { "sample",        T::Sample          },
    };
}

static const auto g_inperpModifierDictGLSL = GenerateInterpModifierDict();

const std::string* InterpModifierToGLSLKeyword(const InterpModifier t)
{
    return g_inperpModifierDictGLSL.EnumToString(t);
}

InterpModifier GLSLKeywordToInterpModifier(const std::string& keyword)
{
    return MapKeywordToType(g_inperpModifierDictGLSL, keyword, R_InterpModifier);
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
        if (samplerType != SamplerType::Undefined || t == BufferType::Texture2DMS || t == BufferType::Texture2DMSArray)
            useVulkanGLSL = false;
    }

    return MapTypeToKeyword((useVulkanGLSL ? typeMapVKSL : typeMapGLSL), t);
}


/* ----- SamplerType Mapping ----- */

static Dictionary<SamplerType> GenerateSamplerTypeDict()
{
    using T = SamplerType;

    return
    {
        { "sampler1D",              T::Sampler1D              },
        { "sampler2D",              T::Sampler2D              },
        { "sampler3D",              T::Sampler3D              },
        { "samplerCube",            T::SamplerCube            },
        { "sampler2DRect",          T::Sampler2DRect          },
        { "sampler1DArray",         T::Sampler1DArray         },
        { "sampler2DArray",         T::Sampler2DArray         },
        { "samplerCubeArray",       T::SamplerCubeArray       },
        { "samplerBuffer",          T::SamplerBuffer          },
        { "sampler2DMS",            T::Sampler2DMS            },
        { "sampler2DMSArray",       T::Sampler2DMSArray       },
        { "sampler1DShadow",        T::Sampler1DShadow        },
        { "sampler2DShadow",        T::Sampler2DShadow        },
        { "samplerCubeShadow",      T::SamplerCubeShadow      },
        { "sampler2DRectShadow",    T::Sampler2DRectShadow    },
        { "sampler1DArrayShadow",   T::Sampler1DArrayShadow   },
        { "sampler2DArrayShadow",   T::Sampler2DArrayShadow   },
        { "samplerCubeArrayShadow", T::SamplerCubeArrayShadow },

        { "isampler1D",             T::Sampler1D              },
        { "isampler2D",             T::Sampler2D              },
        { "isampler3D",             T::Sampler3D              },
        { "isamplerCube",           T::SamplerCube            },
        { "isampler2DRect",         T::Sampler2DRect          },
        { "isampler1DArray",        T::Sampler1DArray         },
        { "isampler2DArray",        T::Sampler2DArray         },
        { "isamplerCubeArray",      T::SamplerCubeArray       },
        { "isamplerBuffer",         T::SamplerBuffer          },
        { "isampler2DMS",           T::Sampler2DMS            },
        { "isampler2DMSArray",      T::Sampler2DMSArray       },

        { "usampler1D",             T::Sampler1D              },
        { "usampler2D",             T::Sampler2D              },
        { "usampler3D",             T::Sampler3D              },
        { "usamplerCube",           T::SamplerCube            },
        { "usampler2DRect",         T::Sampler2DRect          },
        { "usampler1DArray",        T::Sampler1DArray         },
        { "usampler2DArray",        T::Sampler2DArray         },
        { "usamplerCubeArray",      T::SamplerCubeArray       },
        { "usamplerBuffer",         T::SamplerBuffer          },
        { "usampler2DMS",           T::Sampler2DMS            },
        { "usampler2DMSArray",      T::Sampler2DMSArray       },

        { "sampler",                T::SamplerState           }, // Only for Vulkan
        { "samplerShadow",          T::SamplerComparisonState }, // Only for Vulkan
    };
}

static const auto g_samplerTypeDictGLSL = GenerateSamplerTypeDict();

const std::string* SamplerTypeToGLSLKeyword(const SamplerType t)
{
    return g_samplerTypeDictGLSL.EnumToString(t);
}

SamplerType GLSLKeywordToSamplerType(const std::string& keyword)
{
    return MapKeywordToType(g_samplerTypeDictGLSL, keyword, R_SamplerType);
}


/* ----- AttributeType Mapping ----- */

static Dictionary<AttributeType> GenerateAttributeTypeDict()
{
    using T = AttributeType;

    return
    {
        { "align",                   T::Align                 },
        { "binding",                 T::Binding               },
        { "cw",                      T::CW                    },
        { "ccw",                     T::CCW                   },
        { "column_major",            T::ColumnMajor           },
        { "component",               T::Component             },
        { "depth_any",               T::DepthAny              },
        { "depth_greater",           T::DepthGreater          },
        { "depth_less",              T::DepthLess             },
        { "depth_unchanged",         T::DepthUnchanged        },
        { "early_fragment_tests",    T::EarlyFragmentTests    },
        { "equal_spacing",           T::EqualSpacing          },
        { "fractional_even_spacing", T::FractionalEvenSpacing },
        { "fractional_odd_spacing",  T::FractionalOddSpacing  },
        { "index",                   T::Index                 },
        { "invocations",             T::Invocations           },
        { "isolines",                T::Isolines              },
        { "lines",                   T::Lines                 },
        { "lines_adjacency",         T::LinesAdjacency        },
        { "line_strip",              T::LineStrip             },
        { "local_size_x",            T::LocalSizeX            },
        { "local_size_y",            T::LocalSizeY            },
        { "local_size_z",            T::LocalSizeZ            },
        { "location",                T::Location              },
        { "max_vertices",            T::MaxVertices           },
        { "origin_upper_left",       T::OriginUpperLeft       },
        { "offset",                  T::Offset                },
        { "packed",                  T::Packed                },
        { "pixel_center_integer",    T::PixelCenterInteger    },
        { "points",                  T::Points                },
        { "point_mode",              T::PointMode             },
        { "quads",                   T::Quads                 },
        { "row_major",               T::RowMajor              },
        { "shared",                  T::Shared                },
        { "std140",                  T::Std140                },
        { "std430",                  T::Std430                },
        { "stream",                  T::Stream                },
        { "triangles",               T::Triangles             },
        { "triangles_adjacency",     T::TrianglesAdjacency    },
        { "triangles_strip",         T::TriangleStrip         },
        { "vertices",                T::Vertices              },
        { "xfb_buffer",              T::XfbBuffer             },
        { "xfb_offset",              T::XfbOffset             },
        { "xfb_stride",              T::XfbStride             },
    };
}

static const auto g_attributeTypeDictGLSL = GenerateAttributeTypeDict();

const std::string* AttributeTypeToGLSLKeyword(const AttributeType t)
{
    return g_attributeTypeDictGLSL.EnumToString(t);
}

AttributeType GLSLKeywordToAttributeType(const std::string& keyword)
{
    return g_attributeTypeDictGLSL.StringToEnumOrDefault(keyword, AttributeType::Undefined);
}


/* ----- AttributeValue Mapping ----- */

static Dictionary<AttributeValue> GenerateAttributeValueDict()
{
    using T = AttributeValue;

    return
    {
        { "triangles",               T::DomainTri                  },
        { "quads",                   T::DomainQuad                 },
        { "isolines",                T::DomainIsoline              },

      //{ "",                        T::OutputTopologyPoint        }, // ignored in GLSL
      //{ "",                        T::OutputTopologyLine         }, // ignored in GLSL
        { "cw",                      T::OutputTopologyTriangleCW   },
        { "ccw",                     T::OutputTopologyTriangleCCW  },

        { "equal_spacing",           T::PartitioningInteger        },
      //{ "",                        T::PartitioningPow2           }, // ???
        { "fractional_even_spacing", T::PartitioningFractionalEven },
        { "fractional_odd_spacing",  T::PartitioningFractionalOdd  },
    };
}

static const auto g_attributeValueDictGLSL = GenerateAttributeValueDict();

const std::string* AttributeValueToGLSLKeyword(const AttributeValue t)
{
    return g_attributeValueDictGLSL.EnumToString(t);
}

AttributeValue GLSLKeywordToAttributeValue(const std::string& keyword)
{
    return g_attributeValueDictGLSL.StringToEnumOrDefault(keyword, AttributeValue::Undefined);
}


/* ----- PrimitiveType Mapping ----- */

static Dictionary<PrimitiveType> GeneratePrimitiveTypeDict()
{
    using T = PrimitiveType;

    return
    {
        { "points",              T::Point       },
        { "lines",               T::Line        },
        { "lines_adjacency",     T::LineAdj     },
        { "triangles",           T::Triangle    },
        { "triangles_adjacency", T::TriangleAdj },
    };
}

static const auto g_primitiveTypeDictGLSL = GeneratePrimitiveTypeDict();

const std::string* PrimitiveTypeToGLSLKeyword(const PrimitiveType t)
{
    return g_primitiveTypeDictGLSL.EnumToString(t);
}

PrimitiveType GLSLKeywordToPrimitiveType(const std::string& keyword)
{
    return MapKeywordToType(g_primitiveTypeDictGLSL, keyword, R_PrimitiveType);
}


/* ----- ImageLayoutFormat Mapping ----- */

static Dictionary<ImageLayoutFormat> GenerateImageLayoutFormatDict()
{
    using T = ImageLayoutFormat;

    return
    {
        { "rgba32f",        T::F32X4         },
        { "rg32f",          T::F32X2         },
        { "r32f",           T::F32X1         },
        { "rgba16f",        T::F16X4         },
        { "rg16f",          T::F16X2         },
        { "r16f",           T::F16X1         },
        { "r11f_g11f_b10f", T::F11R11G10B    },
        { "rgba16",         T::UN32X4        },
        { "rg16",           T::UN16X2        },
        { "r16",            T::UN16X1        },
        { "rgb10_a2",       T::UN10R10G10B2A },
        { "rgba8",          T::UN8X4         },
        { "rg8",            T::UN8X2         },
        { "r8",             T::UN8X1         },
        { "rgba16_snorm",   T::SN16X4        },
        { "rg16_snorm",     T::SN16X2        },
        { "r16_snorm",      T::SN16X1        },
        { "rgba8_snorm",    T::SN8X4         },
        { "rg8_snorm",      T::SN8X2         },
        { "r8_snorm",       T::SN8X1         },
        { "rgba32i",        T::I32X4         },
        { "rg32i",          T::I32X2         },
        { "r32i",           T::I32X1         },
        { "rgba16i",        T::I16X4         },
        { "rg16i",          T::I16X2         },
        { "r16i",           T::I16X1         },
        { "rgba8i",         T::I8X4          },
        { "rg8i",           T::I8X2          },
        { "r8i",            T::I8X1          },
        { "rgba32ui",       T::UI32X4        },
        { "rg32ui",         T::UI32X2        },
        { "r32ui",          T::UI32X1        },
        { "rgba16ui",       T::UI16X4        },
        { "rg16ui",         T::UI16X2        },
        { "r16ui",          T::UI16X1        },
        { "rgb10_a2ui",     T::UI10R10G10B2A },
        { "rgba8ui",        T::UI8X4         },
        { "rg8ui",          T::UI8X2         },
        { "r8ui",           T::UI8X1         },
    };
}

const std::string* ImageLayoutFormatToGLSLKeyword(const ImageLayoutFormat t)
{
    static const auto typeDict = GenerateImageLayoutFormatDict();
    return typeDict.EnumToString(t);
}


/* ----- Semantic Mapping ----- */

struct GLSLSemanticDescriptor
{
    inline GLSLSemanticDescriptor(const std::string& keyword, bool hasIndex = false) :
        keyword  { keyword  },
        hasIndex { hasIndex }
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
        { T::GroupIndex,             D::UInt   },
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


/* ----- Reserved GLSL Keywords ----- */

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
        "active",
        "asm",
        "cast",
        "class",
        "common",
        "enum",
        "extern",
        "external",
        "filter",
        "fixed",
        "fvec2",
        "fvec3",
        "fvec4",
        "goto",
        "half",
        "hvec2",
        "hvec3",
        "hvec4",
        "inline",
        "input",
        "interface",
        "long",
        "namespace",
        "noinline",
        "output",
        "partition",
        "public",
        "sampler3DRect",
        "short",
        "sizeof",
        "static",
        "typedef",
        "template",
        "this",
        "union",
        "unsigned",
        "using",
    };

    return reservedNames;
}


} // /namespace Xsc



// ================================================================================