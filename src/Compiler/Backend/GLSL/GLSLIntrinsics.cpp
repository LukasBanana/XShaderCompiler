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
    using I = Intrinsic;

    return
    {
      //{ I::Abort,                             ""                      },
        { I::Abs,                               "abs"                   },
        { I::ACos,                              "acos"                  },
        { I::All,                               "all"                   },
        { I::AllMemoryBarrier,                  "memoryBarrier"         },
      //{ I::AllMemoryBarrierWithGroupSync,     "barrier"               }, //???
        { I::Any,                               "any"                   },
      //{ I::AsDouble,                          ""                      },
      //{ I::AsFloat,                           ""                      },
        { I::ASin,                              "asin"                  },
      //{ I::AsInt,                             ""                      },
      //{ I::AsUInt,                            ""                      },
      //{ I::AsUInt_2,                          ""                      },
        { I::ATan,                              "atan"                  },
        { I::ATan2,                             "atan"                  },
        { I::Ceil,                              "ceil"                  },
      //{ I::CheckAccessFullyMapped,            ""                      },
        { I::Clamp,                             "clamp"                 },
      //{ I::Clip,                              ""                      },
        { I::Cos,                               "cos"                   },
        { I::CosH,                              "cosh"                  },
        { I::CountBits,                         ""                      },
        { I::Cross,                             "cross"                 },
      //{ I::D3DCOLORtoUBYTE4,                  ""                      },
        { I::DDX,                               "dFdx"                  },
        { I::DDXCoarse,                         "dFdxCoarse"            },
        { I::DDXFine,                           "dFdxFine"              },
        { I::DDY,                               "dFdy"                  },
        { I::DDYCoarse,                         "dFdyCoarse"            },
        { I::DDYFine,                           "dFdyFine"              },
        { I::Degrees,                           "degrees"               },
        { I::Determinant,                       "determinant"           },
      //{ I::DeviceMemoryBarrier,               ""                      },
      //{ I::DeviceMemoryBarrierWithGroupSync,  ""                      },
        { I::Distance,                          "distance"              },
        { I::Dot,                               "dot"                   },
      //{ I::Dst,                               ""                      },
      //{ I::ErrorF,                            ""                      },
      //{ I::EvaluateAttributeAtCentroid,       ""                      },
      //{ I::EvaluateAttributeAtSample,         ""                      },
      //{ I::EvaluateAttributeSnapped,          ""                      },
        { I::Exp,                               "exp"                   },
        { I::Exp2,                              "exp2"                  },
      //{ I::F16toF32,                          ""                      },
      //{ I::F32toF16,                          ""                      },
        { I::FaceForward,                       "faceforward"           },
        { I::FirstBitHigh,                      "findMSB"               },
        { I::FirstBitLow,                       "findLSB"               },
        { I::Floor,                             "floor"                 },
        { I::FMA,                               "fma"                   },
        { I::FMod,                              "mod"                   },
        { I::Frac,                              "fract"                 },
        { I::FrExp,                             "frexp"                 },
        { I::FWidth,                            "fwidth"                },
      //{ I::GetRenderTargetSampleCount,        ""                      },
      //{ I::GetRenderTargetSamplePosition,     ""                      },
        { I::GroupMemoryBarrier,                "groupMemoryBarrier"    },
      //{ I::GroupMemoryBarrierWithGroupSync,   "barrier"               }, //???
        { I::InterlockedAdd,                    "atomicAdd"             },
        { I::InterlockedAnd,                    "atomicAnd"             },
        { I::InterlockedCompareExchange,        "atomicCompSwap"        },
      //{ I::InterlockedCompareStore,           ""                      },
        { I::InterlockedExchange,               "atomicExchange"        },
        { I::InterlockedMax,                    "atomicMax"             },
        { I::InterlockedMin,                    "atomicMin"             },
        { I::InterlockedOr,                     "atomicOr"              },
        { I::InterlockedXor,                    "atomicXor"             },
      //{ I::IsFinite,                          ""                      },
        { I::IsInf,                             "isinf"                 },
        { I::IsNaN,                             "isnan"                 },
        { I::LdExp,                             "ldexp"                 },
        { I::Length,                            "length"                },
        { I::Lerp,                              "mix"                   },
      //{ I::Lit,                               ""                      },
        { I::Log,                               "log"                   },
      //{ I::Log10,                             ""                      },
        { I::Log2,                              "log2"                  },
      //{ I::MAD,                               ""                      },
        { I::Max,                               "max"                   },
        { I::Min,                               "min"                   },
        { I::ModF,                              "modf"                  },
      //{ I::MSAD4,                             ""                      },
      //{ I::Mul,                               ""                      },
        { I::Noise,                             "noise"                 },
        { I::Normalize,                         "normalize"             },
        { I::Pow,                               "pow"                   },
      //{ I::PrintF,                            ""                      },
      //{ I::Process2DQuadTessFactorsAvg,       ""                      },
      //{ I::Process2DQuadTessFactorsMax,       ""                      },
      //{ I::Process2DQuadTessFactorsMin,       ""                      },
      //{ I::ProcessIsolineTessFactors,         ""                      },
      //{ I::ProcessQuadTessFactorsAvg,         ""                      },
      //{ I::ProcessQuadTessFactorsMax,         ""                      },
      //{ I::ProcessQuadTessFactorsMin,         ""                      },
      //{ I::ProcessTriTessFactorsAvg,          ""                      },
      //{ I::ProcessTriTessFactorsMax,          ""                      },
      //{ I::ProcessTriTessFactorsMin,          ""                      },
        { I::Radians,                           "radians"               },
      //{ I::Rcp,                               ""                      },
        { I::Reflect,                           "reflect"               },
        { I::Refract,                           "refract"               },
      //{ I::ReverseBits,                       ""                      },
        { I::Round,                             "round"                 },
        { I::RSqrt,                             "inversesqrt"           },
        { I::Saturate,                          "clamp"                 }, //TODO: make this unavailable as mapping (must be converted by an "GLSLConverter" or the like)
        { I::Sign,                              "sign"                  },
        { I::Sin,                               "sin"                   },
      //{ I::SinCos,                            ""                      },
        { I::SinH,                              "sinh"                  },
        { I::SmoothStep,                        "smoothstep"            },
        { I::Sqrt,                              "sqrt"                  },
        { I::Step,                              "step"                  },
        { I::Tan,                               "tan"                   },
        { I::TanH,                              "tanh"                  },
        { I::Tex1D,                             "texture"               },
        { I::Tex1D_2,                           "texture"               },
        { I::Tex1DBias,                         "texture"               },
        { I::Tex1DGrad,                         "textureGrad"           },
        { I::Tex1DLod,                          "textureLod"            },
        { I::Tex1DProj,                         "textureProj"           },
        { I::Tex2D,                             "texture"               },
        { I::Tex2D_2,                           "texture"               },
        { I::Tex2DBias,                         "texture"               },
        { I::Tex2DGrad,                         "textureGrad"           },
        { I::Tex2DLod,                          "textureLod"            },
        { I::Tex2DProj,                         "textureProj"           },
        { I::Tex3D,                             "texture"               },
        { I::Tex3D_2,                           "texture"               },
        { I::Tex3DBias,                         "texture"               },
        { I::Tex3DGrad,                         "textureGrad"           },
        { I::Tex3DLod,                          "textureLod"            },
        { I::Tex3DProj,                         "textureProj"           },
        { I::TexCube,                           "texture"               },
        { I::TexCube_2,                         "texture"               },
        { I::TexCubeBias,                       "texture"               },
        { I::TexCubeGrad,                       "textureGrad"           },
        { I::TexCubeLod,                        "textureLod"            },
      //{ I::TexCubeProj,                       ""                      },
        { I::Transpose,                         "transpose"             },
        { I::Trunc,                             "trunc"                 },
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