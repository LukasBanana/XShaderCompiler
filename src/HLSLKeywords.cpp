/*
 * HLSLKeywords.cpp
 * 
 * This file is part of the "HLSL Translator" (Copyright (c) 2014 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "HLSLKeywords.h"


namespace HTLib
{


static KeywordMapType GenerateKeywordMap()
{
    typedef Token::Types Ty;

    return
    {
        { "true",             Ty::BoolLiteral   },
        { "false",            Ty::BoolLiteral   },

        { "void",             Ty::Void          },

        { "bool",             Ty::ScalarType    },
        { "bool2",            Ty::VectorType    },
        { "bool3",            Ty::VectorType    },
        { "bool4",            Ty::VectorType    },
        { "int",              Ty::ScalarType    },
        { "int2",             Ty::VectorType    },
        { "int3",             Ty::VectorType    },
        { "int4",             Ty::VectorType    },
        { "uint",             Ty::ScalarType    },
        { "uint2",            Ty::VectorType    },
        { "uint3",            Ty::VectorType    },
        { "uint4",            Ty::VectorType    },
        { "half",             Ty::ScalarType    },
        { "half2",            Ty::VectorType    },
        { "half3",            Ty::VectorType    },
        { "half4",            Ty::VectorType    },
        { "float",            Ty::ScalarType    },
        { "float2",           Ty::VectorType    },
        { "float3",           Ty::VectorType    },
        { "float4",           Ty::VectorType    },
        { "float2x2",         Ty::MatrixType    },
        { "float3x3",         Ty::MatrixType    },
        { "float4x4",         Ty::MatrixType    },
        { "float2x3",         Ty::MatrixType    },
        { "float2x4",         Ty::MatrixType    },
        { "float3x2",         Ty::MatrixType    },
        { "float3x4",         Ty::MatrixType    },
        { "float4x2",         Ty::MatrixType    },
        { "float4x3",         Ty::MatrixType    },
        { "double",           Ty::ScalarType    },
        { "double2",          Ty::VectorType    },
        { "double3",          Ty::VectorType    },
        { "double4",          Ty::VectorType    },
        { "double2x2",        Ty::MatrixType    },
        { "double3x3",        Ty::MatrixType    },
        { "double4x4",        Ty::MatrixType    },
        { "double2x3",        Ty::MatrixType    },
        { "double2x4",        Ty::MatrixType    },
        { "double3x2",        Ty::MatrixType    },
        { "double3x4",        Ty::MatrixType    },
        { "double4x2",        Ty::MatrixType    },
        { "double4x3",        Ty::MatrixType    },

        { "uniform",          Ty::InputModifier },
        { "in",               Ty::InputModifier },
        { "out",              Ty::InputModifier },
        { "inout",            Ty::InputModifier },

        { "do",               Ty::Do            },
        { "while",            Ty::While         },
        { "for",              Ty::For           },

        { "if",               Ty::If            },
        { "else",             Ty::Else          },

        { "switch",           Ty::Switch        },
        { "case",             Ty::Case          },
        { "default",          Ty::Default       },

        { "struct",           Ty::Struct        },
        { "register",         Ty::Register      },

        { "texture",          Ty::Texture       },
        { "Texture1D",        Ty::Texture       },
        { "Texture1DArray",   Ty::Texture       },
        { "Texture2D",        Ty::Texture       },
        { "Texture2DArray",   Ty::Texture       },
        { "Texture3D",        Ty::Texture       },
        { "TextureCube",      Ty::Texture       },
        { "TextureCubeArray", Ty::Texture       },
        { "Texture2DMS",      Ty::Texture       },
        { "Texture2DMSArray", Ty::Texture       },
        { "Buffer",           Ty::Texture       },

        { "SamplerState",     Ty::SamplerState  },

        { "cbuffer",          Ty::Buffer        },
        { "tbuffer",          Ty::Buffer        },

        { "break",            Ty::CtrlTransfer  },
        { "continue",         Ty::CtrlTransfer  },
        { "discard",          Ty::CtrlTransfer  },
        { "return",           Ty::Return        },
    };
}

static KeywordMapType keywordMap = GenerateKeywordMap();

const KeywordMapType& HLSLKeywords()
{
    return keywordMap;
}


} // /namespace HTLib



// ================================================================================