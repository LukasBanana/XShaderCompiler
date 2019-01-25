/*
 * MetalIntrinsics.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "MetalIntrinsics.h"
#include <map>


namespace Xsc
{


static std::map<Intrinsic, MetalIntrinsic> GenerateIntrinsicMapMetal()
{
    using T = Intrinsic;

    return
    {
      //{ T::Abort,                            ""                      },
        { T::Abs,                              "abs"                   },
        { T::ACos,                             "acos"                  },
      //{ T::All,                              ""                      },
      //{ T::AllMemoryBarrier,                 ""                      },
        { T::AllMemoryBarrierWithGroupSync,    "threadgroup_barrier"   },
      //{ T::Any,                              ""                      },
      //{ T::AsDouble,                         ""                      },
        { T::AsFloat,                          { "as_type", true }     },
        { T::ASin,                             "asin"                  },
        { T::AsInt,                            { "as_type", true }     },
        { T::AsUInt_1,                         { "as_type", true }     },
      //{ T::AsUInt_3,                         ""                      },
        { T::ATan,                             "atan"                  },
        { T::ATan2,                            "atan2"                 },
        { T::Ceil,                             "ceil"                  },
      //{ T::CheckAccessFullyMapped,           ""                      },
        { T::Clamp,                            "clamp"                 },
      //{ T::Clip,                             ""                      },
        { T::Cos,                              "cos"                   },
        { T::CosH,                             "cosh"                  },
      //{ T::CountBits,                        ""                      },
        { T::Cross,                            "cross"                 },
      //{ T::D3DCOLORtoUBYTE4,                 ""                      },
        { T::DDX,                              "dfdx"                  },
        { T::DDXCoarse,                        "dfdx"                  },
        { T::DDXFine,                          "dfdx"                  },
        { T::DDY,                              "dfdy"                  },
        { T::DDYCoarse,                        "dfdy"                  },
        { T::DDYFine,                          "dfdy"                  },
      //{ T::Degrees,                          ""                      },
        { T::Determinant,                      "determinant"           },
      //{ T::DeviceMemoryBarrier,              ""                      }, // memoryBarrier, memoryBarrierImage, memoryBarrierImage, and barrier
      //{ T::DeviceMemoryBarrierWithGroupSync, ""                      }, // memoryBarrier, memoryBarrierImage, memoryBarrierImage
        { T::Distance,                         "distance"              },
        { T::Dot,                              "dot"                   },
      //{ T::Dst,                              ""                      },
      //{ T::Equal,                            ""                      }, // GLSL only
      //{ T::ErrorF,                           ""                      },
      //{ T::EvaluateAttributeAtCentroid,      ""                      },
      //{ T::EvaluateAttributeAtSample,        ""                      },
      //{ T::EvaluateAttributeSnapped,         ""                      },
        { T::Exp,                              "exp"                   },
        { T::Exp2,                             "exp2"                  },
      //{ T::F16toF32,                         ""                      },
      //{ T::F32toF16,                         ""                      },
        { T::FaceForward,                      "faceforward"           },
      //{ T::FirstBitHigh,                     ""                      },
      //{ T::FirstBitLow,                      ""                      },
        { T::Floor,                            "floor"                 },
        { T::FMA,                              "fma"                   },
        { T::FMod,                             "fmod"                  },
        { T::Frac,                             "fract"                 },
        { T::FrExp,                            "frexp"                 },
        { T::FWidth,                           "fwidth"                },
      //{ T::GetRenderTargetSampleCount,       ""                      },
      //{ T::GetRenderTargetSamplePosition,    ""                      },
      //{ T::GreaterThan,                      ""                      }, // GLSL only
      //{ T::GreaterThanEqual,                 ""                      }, // GLSL only
      //{ T::GroupMemoryBarrier,               ""                      },
      //{ T::GroupMemoryBarrierWithGroupSync,  ""                      }, // groupMemoryBarrier and barrier
        { T::InterlockedAdd,                   "atomicAdd"             },
        { T::InterlockedAnd,                   "atomicAnd"             },
        { T::InterlockedCompareExchange,       "atomicCompSwap"        },
      //{ T::InterlockedCompareStore,          ""                      },
        { T::InterlockedExchange,              "atomicExchange"        },
        { T::InterlockedMax,                   "atomicMax"             },
        { T::InterlockedMin,                   "atomicMin"             },
        { T::InterlockedOr,                    "atomicOr"              },
        { T::InterlockedXor,                   "atomicXor"             },
      //{ T::IsFinite,                         ""                      },
        { T::IsInf,                            "isinf"                 },
        { T::IsNaN,                            "isnan"                 },
        { T::LdExp,                            "ldexp"                 },
        { T::Length,                           "length"                },
        { T::Lerp,                             "mix"                   },
      //{ T::LessThan,                         ""                      }, // GLSL only
      //{ T::LessThanEqual,                    ""                      }, // GLSL only
      //{ T::Lit,                              ""                      },
        { T::Log,                              "log"                   },
      //{ T::Log10,                            ""                      },
        { T::Log2,                             "log2"                  },
        { T::MAD,                              "fma"                   },
        { T::Max,                              "max"                   },
        { T::Min,                              "min"                   },
        { T::ModF,                             "modf"                  },
      //{ T::MSAD4,                            ""                      },
      //{ T::Mul,                              ""                      },
        { T::Normalize,                        "normalize"             },
      //{ T::NotEqual,                         ""                      }, // GLSL only
      //{ T::Not,                              ""                      }, // GLSL only
        { T::Pow,                              "pow"                   },
      //{ T::PrintF,                           ""                      },
      //{ T::Process2DQuadTessFactorsAvg,      ""                      },
      //{ T::Process2DQuadTessFactorsMax,      ""                      },
      //{ T::Process2DQuadTessFactorsMin,      ""                      },
      //{ T::ProcessIsolineTessFactors,        ""                      },
      //{ T::ProcessQuadTessFactorsAvg,        ""                      },
      //{ T::ProcessQuadTessFactorsMax,        ""                      },
      //{ T::ProcessQuadTessFactorsMin,        ""                      },
      //{ T::ProcessTriTessFactorsAvg,         ""                      },
      //{ T::ProcessTriTessFactorsMax,         ""                      },
      //{ T::ProcessTriTessFactorsMin,         ""                      },
      //{ T::Radians,                          ""                      },
      //{ T::Rcp,                              ""                      },
        { T::Reflect,                          "reflect"               },
        { T::Refract,                          "refract"               },
      //{ T::ReverseBits,                      ""                      },
        { T::Round,                            "round"                 },
        { T::RSqrt,                            "rsqrt"                 },
      //{ T::Saturate,                         ""                      },
        { T::Sign,                             "sign"                  },
        { T::Sin,                              "sin"                   },
      //{ T::SinCos,                           ""                      },
        { T::SinH,                             "sinh"                  },
        { T::SmoothStep,                       "smoothstep"            },
        { T::Sqrt,                             "sqrt"                  },
        { T::Step,                             "step"                  },
        { T::Tan,                              "tan"                   },
        { T::TanH,                             "tanh"                  },
        { T::Transpose,                        "transpose"             },
        { T::Trunc,                            "trunc"                 },

        { T::Tex1D_2,                          "sample"                },
        { T::Tex1D_4,                          "sample"                },
        { T::Tex1DBias,                        "sample"                },
        { T::Tex1DGrad,                        "sample"                },
        { T::Tex1DLod,                         "sample"                },
        { T::Tex1DProj,                        "sample"                },
        { T::Tex2D_2,                          "sample"                },
        { T::Tex2D_4,                          "sample"                },
        { T::Tex2DBias,                        "sample"                },
        { T::Tex2DGrad,                        "sample"                },
        { T::Tex2DLod,                         "sample"                },
        { T::Tex2DProj,                        "sample"                },
        { T::Tex3D_2,                          "sample"                },
        { T::Tex3D_4,                          "sample"                },
        { T::Tex3DBias,                        "sample"                },
        { T::Tex3DGrad,                        "sample"                },
        { T::Tex3DLod,                         "sample"                },
        { T::Tex3DProj,                        "sample"                },
        { T::TexCube_2,                        "sample"                },
        { T::TexCube_4,                        "sample"                },
        { T::TexCubeBias,                      "sample"                },
        { T::TexCubeGrad,                      "sample"                },
        { T::TexCubeLod,                       "sample"                },
      //{ T::TexCubeProj,                      ""                      },

      //{ T::Texture_GetDimensions,            ""                      }, // get_width(), get_height(), ...
        { T::Texture_Load_1,                   "read"                  },
        { T::Texture_Load_2,                   "read"                  },
        { T::Texture_Load_3,                   "read"                  },
        { T::Texture_Gather_2,                 "gather"                },
        { T::Texture_Gather_3,                 "gather"                },
        { T::Texture_Gather_4,                 "gather"                },
      //{ T::Texture_GatherRed_2,              ""                      },
      //{ T::Texture_GatherRed_3,              ""                      },
      //{ T::Texture_GatherRed_4,              ""                      },
      //{ T::Texture_GatherRed_6,              ""                      },
      //{ T::Texture_GatherRed_7,              ""                      },
      //{ T::Texture_GatherGreen_2,            ""                      },
      //{ T::Texture_GatherGreen_3,            ""                      },
      //{ T::Texture_GatherGreen_4,            ""                      },
      //{ T::Texture_GatherGreen_6,            ""                      },
      //{ T::Texture_GatherGreen_7,            ""                      },
      //{ T::Texture_GatherBlue_2,             ""                      },
      //{ T::Texture_GatherBlue_3,             ""                      },
      //{ T::Texture_GatherBlue_4,             ""                      },
      //{ T::Texture_GatherBlue_6,             ""                      },
      //{ T::Texture_GatherBlue_7,             ""                      },
      //{ T::Texture_GatherAlpha_2,            ""                      },
      //{ T::Texture_GatherAlpha_3,            ""                      },
      //{ T::Texture_GatherAlpha_4,            ""                      },
      //{ T::Texture_GatherAlpha_6,            ""                      },
      //{ T::Texture_GatherAlpha_7,            ""                      },
        { T::Texture_GatherCmp_3,              "gather_compare"        },
        { T::Texture_GatherCmp_4,              "gather_compare"        },
        { T::Texture_GatherCmp_5,              "gather_compare"        },
      //{ T::Texture_GatherCmpRed_3,           ""                      },
      //{ T::Texture_GatherCmpRed_4,           ""                      },
      //{ T::Texture_GatherCmpRed_5,           ""                      },
      //{ T::Texture_GatherCmpRed_7,           ""                      },
      //{ T::Texture_GatherCmpRed_8,           ""                      },
      //{ T::Texture_GatherCmpGreen_3,         ""                      },
      //{ T::Texture_GatherCmpGreen_4,         ""                      },
      //{ T::Texture_GatherCmpGreen_5,         ""                      },
      //{ T::Texture_GatherCmpGreen_7,         ""                      },
      //{ T::Texture_GatherCmpGreen_8,         ""                      },
      //{ T::Texture_GatherCmpBlue_3,          ""                      },
      //{ T::Texture_GatherCmpBlue_4,          ""                      },
      //{ T::Texture_GatherCmpBlue_5,          ""                      },
      //{ T::Texture_GatherCmpBlue_7,          ""                      },
      //{ T::Texture_GatherCmpBlue_8,          ""                      },
      //{ T::Texture_GatherCmpAlpha_3,         ""                      },
      //{ T::Texture_GatherCmpAlpha_4,         ""                      },
      //{ T::Texture_GatherCmpAlpha_5,         ""                      },
      //{ T::Texture_GatherCmpAlpha_7,         ""                      },
      //{ T::Texture_GatherCmpAlpha_8,         ""                      },
        { T::Texture_Sample_2,                 "sample"                },
        { T::Texture_Sample_3,                 "sample"                },
        { T::Texture_Sample_4,                 "sample"                },
        { T::Texture_Sample_5,                 "sample"                },
        { T::Texture_SampleBias_3,             "sample"                },
        { T::Texture_SampleBias_4,             "sample"                },
        { T::Texture_SampleBias_5,             "sample"                },
        { T::Texture_SampleBias_6,             "sample"                },
        { T::Texture_SampleCmp_3,              "sample_compare"        },
        { T::Texture_SampleCmp_4,              "sample_compare"        },
        { T::Texture_SampleCmp_5,              "sample_compare"        },
        { T::Texture_SampleCmp_6,              "sample_compare"        },
        { T::Texture_SampleCmpLevelZero_3,     "sample_compare"        },
        { T::Texture_SampleCmpLevelZero_4,     "sample_compare"        },
        { T::Texture_SampleCmpLevelZero_5,     "sample_compare"        },
        { T::Texture_SampleGrad_4,             "sample"                }, // lod_options: gradient2d or gradient3d
        { T::Texture_SampleGrad_5,             "sample"                }, // lod_options: gradient2d or gradient3d
        { T::Texture_SampleGrad_6,             "sample"                }, // lod_options: gradient2d or gradient3d
        { T::Texture_SampleGrad_7,             "sample"                }, // lod_options: gradient2d or gradient3d
        { T::Texture_SampleLevel_3,            "sample"                }, // lod_options: level
        { T::Texture_SampleLevel_4,            "sample"                }, // lod_options: level
        { T::Texture_SampleLevel_5,            "sample"                }, // lod_options: level
      //{ T::Texture_QueryLod,                 ""                      },
      //{ T::Texture_QueryLodUnclamped,        ""                      },

      //{ T::StreamOutput_Append,              ""                      },
      //{ T::StreamOutput_RestartStrip,        ""                      },

      //{ T::Image_Load,                       ""                      }, // GLSL only
      //{ T::Image_Store,                      ""                      }, // GLSL only
      //{ T::Image_AtomicAdd,                  ""                      }, // GLSL only
      //{ T::Image_AtomicAnd,                  ""                      }, // GLSL only
      //{ T::Image_AtomicCompSwap,             ""                      }, // GLSL only
      //{ T::Image_AtomicExchange,             ""                      }, // GLSL only
      //{ T::Image_AtomicMax,                  ""                      }, // GLSL only
      //{ T::Image_AtomicMin,                  ""                      }, // GLSL only
      //{ T::Image_AtomicOr,                   ""                      }, // GLSL only
      //{ T::Image_AtomicXor,                  ""                      }, // GLSL only

      //{ T::PackHalf2x16,                     ""                      }, // GLSL only
    };
}

static const auto g_intrinsicMapMetal = GenerateIntrinsicMapMetal();

const MetalIntrinsic* IntrinsicToMetalKeyword(const Intrinsic intr)
{
    auto it = g_intrinsicMapMetal.find(intr);
    return (it != g_intrinsicMapMetal.end() ? &(it->second) : nullptr);
}


} // /namespace Xsc



// ================================================================================
