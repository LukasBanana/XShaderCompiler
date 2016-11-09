/*
 * HLSLKeywords.cpp
 * 
 * This file is part of the "HLSL Translator" (Copyright (c) 2014 by Lukas Hermanns)
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
        { "true",                    Ty::BoolLiteral     },
        { "false",                   Ty::BoolLiteral     },

        { "void",                    Ty::Void            },

        { "bool",                    Ty::ScalarType      },
        { "bool1",                   Ty::ScalarType      },
        { "bool1x1",                 Ty::ScalarType      },
        { "int",                     Ty::ScalarType      },
        { "int1",                    Ty::ScalarType      },
        { "int1x1",                  Ty::ScalarType      },
        { "uint",                    Ty::ScalarType      },
        { "uint1",                   Ty::ScalarType      },
        { "uint1x1",                 Ty::ScalarType      },
        { "half",                    Ty::ScalarType      },
        { "half1",                   Ty::ScalarType      },
        { "half1x1",                 Ty::ScalarType      },
        { "float",                   Ty::ScalarType      },
        { "float1",                  Ty::ScalarType      },
        { "float1x1",                Ty::ScalarType      },
        { "double",                  Ty::ScalarType      },
        { "double1",                 Ty::ScalarType      },
        { "double1x1",               Ty::ScalarType      },

        { "bool2",                   Ty::VectorType      },
        { "bool3",                   Ty::VectorType      },
        { "bool4",                   Ty::VectorType      },
        { "int2",                    Ty::VectorType      },
        { "int3",                    Ty::VectorType      },
        { "int4",                    Ty::VectorType      },
        { "uint2",                   Ty::VectorType      },
        { "uint3",                   Ty::VectorType      },
        { "uint4",                   Ty::VectorType      },
        { "half2",                   Ty::VectorType      },
        { "half3",                   Ty::VectorType      },
        { "half4",                   Ty::VectorType      },
        { "float2",                  Ty::VectorType      },
        { "float3",                  Ty::VectorType      },
        { "float4",                  Ty::VectorType      },
        { "double2",                 Ty::VectorType      },
        { "double3",                 Ty::VectorType      },
        { "double4",                 Ty::VectorType      },

        { "bool2x2",                 Ty::MatrixType      },
        { "bool2x3",                 Ty::MatrixType      },
        { "bool2x4",                 Ty::MatrixType      },
        { "bool3x2",                 Ty::MatrixType      },
        { "bool3x3",                 Ty::MatrixType      },
        { "bool3x4",                 Ty::MatrixType      },
        { "bool4x2",                 Ty::MatrixType      },
        { "bool4x3",                 Ty::MatrixType      },
        { "bool4x4",                 Ty::MatrixType      },
        { "int2x2",                  Ty::MatrixType      },
        { "int2x3",                  Ty::MatrixType      },
        { "int2x4",                  Ty::MatrixType      },
        { "int3x2",                  Ty::MatrixType      },
        { "int3x3",                  Ty::MatrixType      },
        { "int3x4",                  Ty::MatrixType      },
        { "int4x2",                  Ty::MatrixType      },
        { "int4x3",                  Ty::MatrixType      },
        { "int4x4",                  Ty::MatrixType      },
        { "uint2x2",                 Ty::MatrixType      },
        { "uint2x3",                 Ty::MatrixType      },
        { "uint2x4",                 Ty::MatrixType      },
        { "uint3x2",                 Ty::MatrixType      },
        { "uint3x3",                 Ty::MatrixType      },
        { "uint3x4",                 Ty::MatrixType      },
        { "uint4x2",                 Ty::MatrixType      },
        { "uint4x3",                 Ty::MatrixType      },
        { "uint4x4",                 Ty::MatrixType      },
        { "half2x2",                 Ty::MatrixType      },
        { "half2x3",                 Ty::MatrixType      },
        { "half2x4",                 Ty::MatrixType      },
        { "half3x2",                 Ty::MatrixType      },
        { "half3x3",                 Ty::MatrixType      },
        { "half3x4",                 Ty::MatrixType      },
        { "half4x2",                 Ty::MatrixType      },
        { "half4x3",                 Ty::MatrixType      },
        { "half4x4",                 Ty::MatrixType      },
        { "float2x2",                Ty::MatrixType      },
        { "float2x3",                Ty::MatrixType      },
        { "float2x4",                Ty::MatrixType      },
        { "float3x2",                Ty::MatrixType      },
        { "float3x3",                Ty::MatrixType      },
        { "float3x4",                Ty::MatrixType      },
        { "float4x2",                Ty::MatrixType      },
        { "float4x3",                Ty::MatrixType      },
        { "float4x4",                Ty::MatrixType      },
        { "double2x2",               Ty::MatrixType      },
        { "double2x3",               Ty::MatrixType      },
        { "double2x4",               Ty::MatrixType      },
        { "double3x2",               Ty::MatrixType      },
        { "double3x3",               Ty::MatrixType      },
        { "double3x4",               Ty::MatrixType      },
        { "double4x2",               Ty::MatrixType      },
        { "double4x3",               Ty::MatrixType      },
        { "double4x4",               Ty::MatrixType      },

        { "do",                      Ty::Do              },
        { "while",                   Ty::While           },
        { "for",                     Ty::For             },

        { "if",                      Ty::If              },
        { "else",                    Ty::Else            },

        { "switch",                  Ty::Switch          },
        { "case",                    Ty::Case            },
        { "default",                 Ty::Default         },

        { "struct",                  Ty::Struct          },
        { "register",                Ty::Register        },
        { "packoffset",              Ty::PackOffset      },

        { "sampler",                 Ty::Sampler         },
        { "sampler1D",               Ty::Sampler         },
        { "sampler2D",               Ty::Sampler         },
        { "sampler3D",               Ty::Sampler         },
        { "samplerCUBE",             Ty::Sampler         },
        { "sampler_state",           Ty::Sampler         },
        { "SamplerState",            Ty::Sampler         },

        { "Texture1D",               Ty::Texture         },
        { "Texture1DArray",          Ty::Texture         },
        { "Texture2D",               Ty::Texture         },
        { "Texture2DArray",          Ty::Texture         },
        { "Texture3D",               Ty::Texture         },
        { "TextureCube",             Ty::Texture         },
        { "TextureCubeArray",        Ty::Texture         },
        { "Texture2DMS",             Ty::Texture         },
        { "Texture2DMSArray",        Ty::Texture         },
        { "RWTexture1D",             Ty::Texture         },
        { "RWTexture1DArray",        Ty::Texture         },
        { "RWTexture2D",             Ty::Texture         },
        { "RWTexture2DArray",        Ty::Texture         },
        { "RWTexture3D",             Ty::Texture         },

        { "AppendStructuredBuffer",  Ty::StorageBuffer   },
        { "Buffer",                  Ty::StorageBuffer   },
        { "ByteAddressBuffer",       Ty::StorageBuffer   },
        { "ConsumeStructuredBuffer", Ty::StorageBuffer   },
        { "StructuredBuffer",        Ty::StorageBuffer   },
        { "RWBuffer",                Ty::StorageBuffer   },
        { "RWByteAddressBuffer",     Ty::StorageBuffer   },
        { "RWStructuredBuffer",      Ty::StorageBuffer   },

        { "cbuffer",                 Ty::UniformBuffer   },
        { "tbuffer",                 Ty::UniformBuffer   },

        { "break",                   Ty::CtrlTransfer    },
        { "continue",                Ty::CtrlTransfer    },
        { "discard",                 Ty::CtrlTransfer    },

        { "return",                  Ty::Return          },

        { "uniform",                 Ty::InputModifier   },
        { "in",                      Ty::InputModifier   },
        { "out",                     Ty::InputModifier   },
        { "inout",                   Ty::InputModifier   },

        { "extern",                  Ty::StorageModifier },
        { "nointerpolation",         Ty::StorageModifier },
        { "precise",                 Ty::StorageModifier },
        { "shared",                  Ty::StorageModifier },
        { "groupshared",             Ty::StorageModifier },
        { "static",                  Ty::StorageModifier },
        //{ "uniform",                 Ty::StorageModifier }, // Already used as "InputModifier"
        { "volatile",                Ty::StorageModifier },
        { "linear",                  Ty::StorageModifier },
        { "centroid",                Ty::StorageModifier },
        { "noperspective",           Ty::StorageModifier },
        { "sample",                  Ty::StorageModifier },

        { "const",                   Ty::TypeModifier    },
        { "row_major",               Ty::TypeModifier    },
        { "column_major",            Ty::TypeModifier    },
    };
}

static KeywordMapType keywordMap = GenerateKeywordMap();

const KeywordMapType& HLSLKeywords()
{
    return keywordMap;
}


} // /namespace Xsc



// ================================================================================