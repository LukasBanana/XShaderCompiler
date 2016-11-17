/*
 * HLSLKeywords.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "HLSLKeywords.h"


namespace Xsc
{


static KeywordMapType GenerateKeywordMap()
{
    using Ty = Token::Types;

    return
    {
        { "true",                       Ty::BoolLiteral     },
        { "false",                      Ty::BoolLiteral     },

        { "string",                     Ty::StringType      },

        { "bool",                       Ty::ScalarType      },
        { "bool1",                      Ty::ScalarType      },
        { "bool1x1",                    Ty::ScalarType      },
        { "int",                        Ty::ScalarType      },
        { "int1",                       Ty::ScalarType      },
        { "int1x1",                     Ty::ScalarType      },
        { "uint",                       Ty::ScalarType      },
        { "uint1",                      Ty::ScalarType      },
        { "uint1x1",                    Ty::ScalarType      },
        { "dword",                      Ty::ScalarType      },
        { "dword1",                     Ty::ScalarType      },
        { "dword1x1",                   Ty::ScalarType      },
        { "half",                       Ty::ScalarType      },
        { "half1",                      Ty::ScalarType      },
        { "half1x1",                    Ty::ScalarType      },
        { "float",                      Ty::ScalarType      },
        { "float1",                     Ty::ScalarType      },
        { "float1x1",                   Ty::ScalarType      },
        { "double",                     Ty::ScalarType      },
        { "double1",                    Ty::ScalarType      },
        { "double1x1",                  Ty::ScalarType      },

        { "bool2",                      Ty::VectorType      },
        { "bool3",                      Ty::VectorType      },
        { "bool4",                      Ty::VectorType      },
        { "int2",                       Ty::VectorType      },
        { "int3",                       Ty::VectorType      },
        { "int4",                       Ty::VectorType      },
        { "uint2",                      Ty::VectorType      },
        { "uint3",                      Ty::VectorType      },
        { "uint4",                      Ty::VectorType      },
        { "dword2",                     Ty::VectorType      },
        { "dword3",                     Ty::VectorType      },
        { "dword4",                     Ty::VectorType      },
        { "half2",                      Ty::VectorType      },
        { "half3",                      Ty::VectorType      },
        { "half4",                      Ty::VectorType      },
        { "float2",                     Ty::VectorType      },
        { "float3",                     Ty::VectorType      },
        { "float4",                     Ty::VectorType      },
        { "double2",                    Ty::VectorType      },
        { "double3",                    Ty::VectorType      },
        { "double4",                    Ty::VectorType      },

        { "bool2x2",                    Ty::MatrixType      },
        { "bool2x3",                    Ty::MatrixType      },
        { "bool2x4",                    Ty::MatrixType      },
        { "bool3x2",                    Ty::MatrixType      },
        { "bool3x3",                    Ty::MatrixType      },
        { "bool3x4",                    Ty::MatrixType      },
        { "bool4x2",                    Ty::MatrixType      },
        { "bool4x3",                    Ty::MatrixType      },
        { "bool4x4",                    Ty::MatrixType      },
        { "int2x2",                     Ty::MatrixType      },
        { "int2x3",                     Ty::MatrixType      },
        { "int2x4",                     Ty::MatrixType      },
        { "int3x2",                     Ty::MatrixType      },
        { "int3x3",                     Ty::MatrixType      },
        { "int3x4",                     Ty::MatrixType      },
        { "int4x2",                     Ty::MatrixType      },
        { "int4x3",                     Ty::MatrixType      },
        { "int4x4",                     Ty::MatrixType      },
        { "uint2x2",                    Ty::MatrixType      },
        { "uint2x3",                    Ty::MatrixType      },
        { "uint2x4",                    Ty::MatrixType      },
        { "uint3x2",                    Ty::MatrixType      },
        { "uint3x3",                    Ty::MatrixType      },
        { "uint3x4",                    Ty::MatrixType      },
        { "uint4x2",                    Ty::MatrixType      },
        { "uint4x3",                    Ty::MatrixType      },
        { "uint4x4",                    Ty::MatrixType      },
        { "dword2x2",                   Ty::MatrixType      },
        { "dword2x3",                   Ty::MatrixType      },
        { "dword2x4",                   Ty::MatrixType      },
        { "dword3x2",                   Ty::MatrixType      },
        { "dword3x3",                   Ty::MatrixType      },
        { "dword3x4",                   Ty::MatrixType      },
        { "dword4x2",                   Ty::MatrixType      },
        { "dword4x3",                   Ty::MatrixType      },
        { "dword4x4",                   Ty::MatrixType      },
        { "half2x2",                    Ty::MatrixType      },
        { "half2x3",                    Ty::MatrixType      },
        { "half2x4",                    Ty::MatrixType      },
        { "half3x2",                    Ty::MatrixType      },
        { "half3x3",                    Ty::MatrixType      },
        { "half3x4",                    Ty::MatrixType      },
        { "half4x2",                    Ty::MatrixType      },
        { "half4x3",                    Ty::MatrixType      },
        { "half4x4",                    Ty::MatrixType      },
        { "float2x2",                   Ty::MatrixType      },
        { "float2x3",                   Ty::MatrixType      },
        { "float2x4",                   Ty::MatrixType      },
        { "float3x2",                   Ty::MatrixType      },
        { "float3x3",                   Ty::MatrixType      },
        { "float3x4",                   Ty::MatrixType      },
        { "float4x2",                   Ty::MatrixType      },
        { "float4x3",                   Ty::MatrixType      },
        { "float4x4",                   Ty::MatrixType      },
        { "double2x2",                  Ty::MatrixType      },
        { "double2x3",                  Ty::MatrixType      },
        { "double2x4",                  Ty::MatrixType      },
        { "double3x2",                  Ty::MatrixType      },
        { "double3x3",                  Ty::MatrixType      },
        { "double3x4",                  Ty::MatrixType      },
        { "double4x2",                  Ty::MatrixType      },
        { "double4x3",                  Ty::MatrixType      },
        { "double4x4",                  Ty::MatrixType      },

        { "void",                       Ty::Void            },

        { "vector",                     Ty::Vector          },
        { "matrix",                     Ty::Matrix          },

        { "do",                         Ty::Do              },
        { "while",                      Ty::While           },
        { "for",                        Ty::For             },

        { "if",                         Ty::If              },
        { "else",                       Ty::Else            },

        { "switch",                     Ty::Switch          },
        { "case",                       Ty::Case            },
        { "default",                    Ty::Default         },

        { "typedef",                    Ty::Typedef         },
        { "struct",                     Ty::Struct          },
        { "register",                   Ty::Register        },
        { "packoffset",                 Ty::PackOffset      },

        { "sampler",                    Ty::Sampler         },
        { "sampler1D",                  Ty::Sampler         },
        { "sampler2D",                  Ty::Sampler         },
        { "sampler3D",                  Ty::Sampler         },
        { "samplerCUBE",                Ty::Sampler         },
        { "sampler_state",              Ty::Sampler         },
        { "SamplerState",               Ty::Sampler         },
        { "SamplerComparisonState",     Ty::Sampler         }, // since D3D10+

        { "Texture1D",                  Ty::Texture         },
        { "Texture1DArray",             Ty::Texture         },
        { "Texture2D",                  Ty::Texture         },
        { "Texture2DArray",             Ty::Texture         },
        { "Texture3D",                  Ty::Texture         },
        { "TextureCube",                Ty::Texture         },
        { "TextureCubeArray",           Ty::Texture         },
        { "Texture2DMS",                Ty::Texture         },
        { "Texture2DMSArray",           Ty::Texture         },
        { "RWTexture1D",                Ty::Texture         },
        { "RWTexture1DArray",           Ty::Texture         },
        { "RWTexture2D",                Ty::Texture         },
        { "RWTexture2DArray",           Ty::Texture         },
        { "RWTexture3D",                Ty::Texture         },

        { "AppendStructuredBuffer",     Ty::StorageBuffer   },
        { "Buffer",                     Ty::StorageBuffer   },
        { "ByteAddressBuffer",          Ty::StorageBuffer   },
        { "ConsumeStructuredBuffer",    Ty::StorageBuffer   },
        { "StructuredBuffer",           Ty::StorageBuffer   },
        { "RWBuffer",                   Ty::StorageBuffer   },
        { "RWByteAddressBuffer",        Ty::StorageBuffer   },
        { "RWStructuredBuffer",         Ty::StorageBuffer   },

        { "cbuffer",                    Ty::UniformBuffer   },
        { "tbuffer",                    Ty::UniformBuffer   },

        { "break",                      Ty::CtrlTransfer    },
        { "continue",                   Ty::CtrlTransfer    },
        { "discard",                    Ty::CtrlTransfer    },

        { "return",                     Ty::Return          },

        { "uniform",                    Ty::InputModifier   },
        { "in",                         Ty::InputModifier   },
        { "out",                        Ty::InputModifier   },
        { "inout",                      Ty::InputModifier   },

        { "extern",                     Ty::StorageModifier },
        { "nointerpolation",            Ty::StorageModifier },
        { "precise",                    Ty::StorageModifier },
        { "shared",                     Ty::StorageModifier },
        { "groupshared",                Ty::StorageModifier },
        { "static",                     Ty::StorageModifier },
        //{ "uniform",                    Ty::StorageModifier }, // Already used as "InputModifier"
        { "volatile",                   Ty::StorageModifier },
        { "linear",                     Ty::StorageModifier },
        { "centroid",                   Ty::StorageModifier },
        { "noperspective",              Ty::StorageModifier },
        { "sample",                     Ty::StorageModifier },

        { "const",                      Ty::TypeModifier    },
        { "row_major",                  Ty::TypeModifier    },
        { "column_major",               Ty::TypeModifier    },

        { "technique",                  Ty::Technique       },
        { "pass",                       Ty::Pass            },
        { "compile",                    Ty::Compile         },

        { "auto",                       Ty::Reserved        },
        { "catch",                      Ty::Reserved        },
        { "char",                       Ty::Reserved        },
        { "const_cast",                 Ty::Reserved        },
        { "delete",                     Ty::Reserved        },
        { "dynamic_cast",               Ty::Reserved        },
        { "enum",                       Ty::Reserved        },
        { "explicit",                   Ty::Reserved        },
        { "friend",                     Ty::Reserved        },
        { "goto",                       Ty::Reserved        },
        { "long",                       Ty::Reserved        },
        { "mutable",                    Ty::Reserved        },
        { "new",                        Ty::Reserved        },
        { "operator",                   Ty::Reserved        },
        { "private",                    Ty::Reserved        },
        { "protected",                  Ty::Reserved        },
        { "public",                     Ty::Reserved        },
        { "reinterpret_cast",           Ty::Reserved        },
        { "short",                      Ty::Reserved        },
        { "signed",                     Ty::Reserved        },
        { "sizeof",                     Ty::Reserved        },
        { "static_cast",                Ty::Reserved        },
        { "template",                   Ty::Reserved        },
        { "this",                       Ty::Reserved        },
        { "throw",                      Ty::Reserved        },
        { "try",                        Ty::Reserved        },
        { "typename",                   Ty::Reserved        },
        { "union",                      Ty::Reserved        },
        { "unsigned",                   Ty::Reserved        },
        { "using",                      Ty::Reserved        },
        { "virtual",                    Ty::Reserved        },
    };
}

static const KeywordMapType g_keywordMapHLSL = GenerateKeywordMap();

const KeywordMapType& HLSLKeywords()
{
    return g_keywordMapHLSL;
}

using DataTypeMap = std::map<std::string, DataType>;

static DataTypeMap GenerateDataTypeMap()
{
    return
    {
        { "string",     DataType::String    },

        { "bool",       DataType::Bool      },
        { "bool1",      DataType::Bool      },
        { "bool1x1",    DataType::Bool      },
        { "int",        DataType::Int       },
        { "int1",       DataType::Int       },
        { "int1x1",     DataType::Int       },
        { "uint",       DataType::UInt      },
        { "uint1",      DataType::UInt      },
        { "uint1x1",    DataType::UInt      },
        { "dword",      DataType::UInt      },
        { "dword1",     DataType::UInt      },
        { "dword1x1",   DataType::UInt      },
        { "half",       DataType::Half      },
        { "half1",      DataType::Half      },
        { "half1x1",    DataType::Half      },
        { "float",      DataType::Float     },
        { "float1",     DataType::Float     },
        { "float1x1",   DataType::Float     },
        { "double",     DataType::Double    },
        { "double1",    DataType::Double    },
        { "double1x1",  DataType::Double    },

        { "bool2",      DataType::Bool2     },
        { "bool3",      DataType::Bool3     },
        { "bool4",      DataType::Bool4     },
        { "int2",       DataType::Int2      },
        { "int3",       DataType::Int3      },
        { "int4",       DataType::Int4      },
        { "uint2",      DataType::UInt2     },
        { "uint3",      DataType::UInt3     },
        { "uint4",      DataType::UInt4     },
        { "dword2",     DataType::UInt2     },
        { "dword3",     DataType::UInt3     },
        { "dword4",     DataType::UInt4     },
        { "half2",      DataType::Half2     },
        { "half3",      DataType::Half3     },
        { "half4",      DataType::Half4     },
        { "float2",     DataType::Float2    },
        { "float3",     DataType::Float3    },
        { "float4",     DataType::Float4    },
        { "double2",    DataType::Double2   },
        { "double3",    DataType::Double3   },
        { "double4",    DataType::Double4   },

        { "bool2x2",    DataType::Bool2x2   },
        { "bool2x3",    DataType::Bool2x3   },
        { "bool2x4",    DataType::Bool2x4   },
        { "bool3x2",    DataType::Bool3x2   },
        { "bool3x3",    DataType::Bool3x3   },
        { "bool3x4",    DataType::Bool3x4   },
        { "bool4x2",    DataType::Bool4x2   },
        { "bool4x3",    DataType::Bool4x3   },
        { "bool4x4",    DataType::Bool4x4   },
        { "int2x2",     DataType::Int2x2    },
        { "int2x3",     DataType::Int2x3    },
        { "int2x4",     DataType::Int2x4    },
        { "int3x2",     DataType::Int3x2    },
        { "int3x3",     DataType::Int3x3    },
        { "int3x4",     DataType::Int3x4    },
        { "int4x2",     DataType::Int4x2    },
        { "int4x3",     DataType::Int4x3    },
        { "int4x4",     DataType::Int4x4    },
        { "uint2x2",    DataType::UInt2x2   },
        { "uint2x3",    DataType::UInt2x3   },
        { "uint2x4",    DataType::UInt2x4   },
        { "uint3x2",    DataType::UInt3x2   },
        { "uint3x3",    DataType::UInt3x3   },
        { "uint3x4",    DataType::UInt3x4   },
        { "uint4x2",    DataType::UInt4x2   },
        { "uint4x3",    DataType::UInt4x3   },
        { "uint4x4",    DataType::UInt4x4   },
        { "dword2x2",   DataType::UInt2x2   },
        { "dword2x3",   DataType::UInt2x3   },
        { "dword2x4",   DataType::UInt2x4   },
        { "dword3x2",   DataType::UInt3x2   },
        { "dword3x3",   DataType::UInt3x3   },
        { "dword3x4",   DataType::UInt3x4   },
        { "dword4x2",   DataType::UInt4x2   },
        { "dword4x3",   DataType::UInt4x3   },
        { "dword4x4",   DataType::UInt4x4   },
        { "half2x2",    DataType::Half2x2   },
        { "half2x3",    DataType::Half2x3   },
        { "half2x4",    DataType::Half2x4   },
        { "half3x2",    DataType::Half3x2   },
        { "half3x3",    DataType::Half3x3   },
        { "half3x4",    DataType::Half3x4   },
        { "half4x2",    DataType::Half4x2   },
        { "half4x3",    DataType::Half4x3   },
        { "half4x4",    DataType::Half4x4   },
        { "float2x2",   DataType::Float2x2  },
        { "float2x3",   DataType::Float2x3  },
        { "float2x4",   DataType::Float2x4  },
        { "float3x2",   DataType::Float3x2  },
        { "float3x3",   DataType::Float3x3  },
        { "float3x4",   DataType::Float3x4  },
        { "float4x2",   DataType::Float4x2  },
        { "float4x3",   DataType::Float4x3  },
        { "float4x4",   DataType::Float4x4  },
        { "double2x2",  DataType::Double2x2 },
        { "double2x3",  DataType::Double2x3 },
        { "double2x4",  DataType::Double2x4 },
        { "double3x2",  DataType::Double3x2 },
        { "double3x3",  DataType::Double3x3 },
        { "double3x4",  DataType::Double3x4 },
        { "double4x2",  DataType::Double4x2 },
        { "double4x3",  DataType::Double4x3 },
        { "double4x4",  DataType::Double4x4 },
    };
}

static const DataTypeMap g_dataTypeMapHLSL = GenerateDataTypeMap();

DataType HLSLKeywordToDataType(const std::string& keyword)
{
    auto it = g_dataTypeMapHLSL.find(keyword);
    if (it != g_dataTypeMapHLSL.end())
        return it->second;
    else
        throw std::runtime_error("failed to map keyword '" + keyword + "' to data type");
}


} // /namespace Xsc



// ================================================================================