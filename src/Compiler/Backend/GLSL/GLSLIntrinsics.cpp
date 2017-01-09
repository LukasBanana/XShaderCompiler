/*
 * GLSLIntrinsics.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLSLIntrinsics.h"
#include <map>


namespace Xsc
{


static std::map<Intrinsic, std::string> GenerateIntrinsicMap()
{
    using T = Intrinsic;

    return
    {
      //{ T::Abort,                             ""                      },
        { T::Abs,                               "abs"                   },
        { T::ACos,                              "acos"                  },
        { T::All,                               "all"                   },
        { T::AllMemoryBarrier,                  "memoryBarrier"         },
        { T::AllMemoryBarrierWithGroupSync,     "barrier"               }, //???
        { T::Any,                               "any"                   },
        { T::AsDouble,                          "uint64BitsToDouble"    },
        { T::AsFloat,                           "uintBitsToFloat"       },
        { T::ASin,                              "asin"                  },
        { T::AsInt,                             "floatBitsToInt"        },
        { T::AsUInt_1,                          "floatBitsToUint"       },
      //{ T::AsUInt_3,                          ""                      },
        { T::ATan,                              "atan"                  },
        { T::ATan2,                             "atan"                  },
        { T::Ceil,                              "ceil"                  },
      //{ T::CheckAccessFullyMapped,            ""                      },
        { T::Clamp,                             "clamp"                 },
      //{ T::Clip,                              ""                      },
        { T::Cos,                               "cos"                   },
        { T::CosH,                              "cosh"                  },
        { T::CountBits,                         ""                      },
        { T::Cross,                             "cross"                 },
      //{ T::D3DCOLORtoUBYTE4,                  ""                      },
        { T::DDX,                               "dFdx"                  },
        { T::DDXCoarse,                         "dFdxCoarse"            },
        { T::DDXFine,                           "dFdxFine"              },
        { T::DDY,                               "dFdy"                  },
        { T::DDYCoarse,                         "dFdyCoarse"            },
        { T::DDYFine,                           "dFdyFine"              },
        { T::Degrees,                           "degrees"               },
        { T::Determinant,                       "determinant"           },
        { T::DeviceMemoryBarrier,               "barrier"               }, // ??? memoryBarrier, memoryBarrierImage, memoryBarrierImage, and barrier
        { T::DeviceMemoryBarrierWithGroupSync,  "barrier"               }, // ??? memoryBarrier, memoryBarrierImage, memoryBarrierImage
        { T::Distance,                          "distance"              },
        { T::Dot,                               "dot"                   },
      //{ T::Dst,                               ""                      },
      //{ T::ErrorF,                            ""                      },
        { T::EvaluateAttributeAtCentroid,       "interpolateAtCentroid" },
        { T::EvaluateAttributeAtSample,         "interpolateAtSample"   },
        { T::EvaluateAttributeSnapped,          "interpolateAtOffset"   },
        { T::Exp,                               "exp"                   },
        { T::Exp2,                              "exp2"                  },
      //{ T::F16toF32,                          ""                      },
      //{ T::F32toF16,                          ""                      },
        { T::FaceForward,                       "faceforward"           },
        { T::FirstBitHigh,                      "findMSB"               },
        { T::FirstBitLow,                       "findLSB"               },
        { T::Floor,                             "floor"                 },
        { T::FMA,                               "fma"                   },
        { T::FMod,                              "mod"                   },
        { T::Frac,                              "fract"                 },
        { T::FrExp,                             "frexp"                 },
        { T::FWidth,                            "fwidth"                },
      //{ T::GetRenderTargetSampleCount,        ""                      },
      //{ T::GetRenderTargetSamplePosition,     ""                      },
        { T::GroupMemoryBarrier,                "groupMemoryBarrier"    },
        { T::GroupMemoryBarrierWithGroupSync,   "barrier"               }, // ??? groupMemoryBarrier and barrier
        { T::InterlockedAdd,                    "atomicAdd"             },
        { T::InterlockedAnd,                    "atomicAnd"             },
        { T::InterlockedCompareExchange,        "atomicCompSwap"        },
      //{ T::InterlockedCompareStore,           ""                      },
        { T::InterlockedExchange,               "atomicExchange"        },
        { T::InterlockedMax,                    "atomicMax"             },
        { T::InterlockedMin,                    "atomicMin"             },
        { T::InterlockedOr,                     "atomicOr"              },
        { T::InterlockedXor,                    "atomicXor"             },
      //{ T::IsFinite,                          ""                      },
        { T::IsInf,                             "isinf"                 },
        { T::IsNaN,                             "isnan"                 },
        { T::LdExp,                             "ldexp"                 },
        { T::Length,                            "length"                },
        { T::Lerp,                              "mix"                   },
      //{ T::Lit,                               ""                      },
        { T::Log,                               "log"                   },
      //{ T::Log10,                             ""                      },
        { T::Log2,                              "log2"                  },
        { T::MAD,                               "fma"                   },
        { T::Max,                               "max"                   },
        { T::Min,                               "min"                   },
        { T::ModF,                              "modf"                  },
      //{ T::MSAD4,                             ""                      },
      //{ T::Mul,                               ""                      },
        { T::Noise,                             "noise"                 },
        { T::Normalize,                         "normalize"             },
        { T::Pow,                               "pow"                   },
      //{ T::PrintF,                            ""                      },
      //{ T::Process2DQuadTessFactorsAvg,       ""                      },
      //{ T::Process2DQuadTessFactorsMax,       ""                      },
      //{ T::Process2DQuadTessFactorsMin,       ""                      },
      //{ T::ProcessIsolineTessFactors,         ""                      },
      //{ T::ProcessQuadTessFactorsAvg,         ""                      },
      //{ T::ProcessQuadTessFactorsMax,         ""                      },
      //{ T::ProcessQuadTessFactorsMin,         ""                      },
      //{ T::ProcessTriTessFactorsAvg,          ""                      },
      //{ T::ProcessTriTessFactorsMax,          ""                      },
      //{ T::ProcessTriTessFactorsMin,          ""                      },
        { T::Radians,                           "radians"               },
      //{ T::Rcp,                               ""                      },
        { T::Reflect,                           "reflect"               },
        { T::Refract,                           "refract"               },
      //{ T::ReverseBits,                       ""                      },
        { T::Round,                             "round"                 },
        { T::RSqrt,                             "inversesqrt"           },
      //{ T::Saturate,                          ""                      },
        { T::Sign,                              "sign"                  },
        { T::Sin,                               "sin"                   },
      //{ T::SinCos,                            ""                      },
        { T::SinH,                              "sinh"                  },
        { T::SmoothStep,                        "smoothstep"            },
        { T::Sqrt,                              "sqrt"                  },
        { T::Step,                              "step"                  },
        { T::Tan,                               "tan"                   },
        { T::TanH,                              "tanh"                  },
        { T::Tex1D_2,                           "texture"               },
        { T::Tex1D_4,                           "texture"               },
        { T::Tex1DBias,                         "texture"               },
        { T::Tex1DGrad,                         "textureGrad"           },
        { T::Tex1DLod,                          "textureLod"            },
        { T::Tex1DProj,                         "textureProj"           },
        { T::Tex2D_2,                           "texture"               },
        { T::Tex2D_4,                           "texture"               },
        { T::Tex2DBias,                         "texture"               },
        { T::Tex2DGrad,                         "textureGrad"           },
        { T::Tex2DLod,                          "textureLod"            },
        { T::Tex2DProj,                         "textureProj"           },
        { T::Tex3D_2,                           "texture"               },
        { T::Tex3D_4,                           "texture"               },
        { T::Tex3DBias,                         "texture"               },
        { T::Tex3DGrad,                         "textureGrad"           },
        { T::Tex3DLod,                          "textureLod"            },
        { T::Tex3DProj,                         "textureProj"           },
        { T::TexCube_2,                         "texture"               },
        { T::TexCube_4,                         "texture"               },
        { T::TexCubeBias,                       "texture"               },
        { T::TexCubeGrad,                       "textureGrad"           },
        { T::TexCubeLod,                        "textureLod"            },
      //{ T::TexCubeProj,                       ""                      },
        { T::Transpose,                         "transpose"             },
        { T::Trunc,                             "trunc"                 },

        { T::Texture_GetDimensions,             "textureSize"           },
        { T::Texture_Load_1,                    "texelFetch"            },
        { T::Texture_Load_2,                    "texelFetch"            },
        { T::Texture_Load_3,                    "texelFetchOffset"      },
        { T::Texture_Sample_2,                  "texture"               },
        { T::Texture_Sample_3,                  "textureOffset"         },
      //{ T::Texture_Sample_4,                  ""                      },
      //{ T::Texture_Sample_5,                  ""                      },
        { T::Texture_SampleBias_3,              "texture"               },
        { T::Texture_SampleBias_4,              "textureOffset"         },
      //{ T::Texture_SampleBias_5,              ""                      },
      //{ T::Texture_SampleBias_6,              ""                      },
      //{ T::Texture_SampleCmp_3,               ""                      },
      //{ T::Texture_SampleCmp_4,               ""                      },
      //{ T::Texture_SampleCmp_5,               ""                      },
      //{ T::Texture_SampleCmp_6,               ""                      },
        { T::Texture_SampleGrad_4,              "textureGrad"           },
        { T::Texture_SampleGrad_5,              "textureGradOffset"     },
      //{ T::Texture_SampleGrad_6,              ""                      },
      //{ T::Texture_SampleGrad_7,              ""                      },
        { T::Texture_SampleLevel_3,             "textureLod"            },
        { T::Texture_SampleLevel_4,             "textureLodOffset"      },
      //{ T::Texture_SampleLevel_5,             ""                      },

        { T::StreamOutput_Append,               "EmitVertex"            },
        { T::StreamOutput_RestartStrip,         "EndPrimitive"          },
    };
}

const std::string* IntrinsicToGLSLKeyword(const Intrinsic intr)
{
    static const auto intrinsicMap = GenerateIntrinsicMap();
    auto it = intrinsicMap.find(intr);
    return (it != intrinsicMap.end() ? &(it->second) : nullptr);
}


} // /namespace Xsc



// ================================================================================