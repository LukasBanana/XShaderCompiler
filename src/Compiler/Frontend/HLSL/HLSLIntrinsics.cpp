/*
 * HLSLIntrinsics.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "HLSLIntrinsics.h"
#include "AST.h"
#include "Helper.h"
#include "Exception.h"


namespace Xsc
{


/* ----- HLSLIntrinsics ----- */

static HLSLIntrinsicsMap GenerateIntrinsicMap()
{
    using T = Intrinsic;

    return
    {
        { "abort",                            { T::Abort,                            4, 0 } },
        { "abs",                              { T::Abs,                              1, 1 } },
        { "acos",                             { T::ACos,                             1, 1 } },
        { "all",                              { T::All,                              1, 1 } },
        { "AllMemoryBarrier",                 { T::AllMemoryBarrier,                 5, 0 } },
        { "AllMemoryBarrierWithGroupSync",    { T::AllMemoryBarrierWithGroupSync,    5, 0 } },
        { "any",                              { T::Any,                              1, 1 } },
        { "asdouble",                         { T::AsDouble,                         5, 0 } },
        { "asfloat",                          { T::AsFloat,                          4, 0 } },
        { "asin",                             { T::ASin,                             1, 1 } },
        { "asint",                            { T::AsInt,                            4, 0 } },
        { "asuint",                           { T::AsUInt_1,                         4, 0 } }, // AsUInt_3: 5.0
        { "atan",                             { T::ATan,                             1, 1 } },
        { "atan2",                            { T::ATan2,                            1, 1 } },
        { "ceil",                             { T::Ceil,                             1, 1 } },
        { "CheckAccessFullyMapped",           { T::CheckAccessFullyMapped,           5, 0 } },
        { "clamp",                            { T::Clamp,                            1, 1 } },
        { "clip",                             { T::Clip,                             1, 1 } },
        { "cos",                              { T::Cos,                              1, 1 } },
        { "cosh",                             { T::CosH,                             1, 1 } },
        { "countbits",                        { T::CountBits,                        5, 0 } },
        { "cross",                            { T::Cross,                            1, 1 } },
        { "D3DCOLORtoUBYTE4",                 { T::D3DCOLORtoUBYTE4,                 1, 1 } },
        { "ddx",                              { T::DDX,                              2, 1 } },
        { "ddx_coarse",                       { T::DDXCoarse,                        5, 0 } },
        { "ddx_fine",                         { T::DDXFine,                          5, 0 } },
        { "ddy",                              { T::DDY,                              2, 1 } },
        { "ddy_coarse",                       { T::DDYCoarse,                        5, 0 } },
        { "ddy_fine",                         { T::DDYFine,                          5, 0 } },
        { "degrees",                          { T::Degrees,                          1, 1 } },
        { "determinant",                      { T::Determinant,                      1, 1 } },
        { "DeviceMemoryBarrier",              { T::DeviceMemoryBarrier,              5, 0 } },
        { "DeviceMemoryBarrierWithGroupSync", { T::DeviceMemoryBarrierWithGroupSync, 5, 0 } },
        { "distance",                         { T::Distance,                         1, 1 } },
        { "dot",                              { T::Dot,                              1, 0 } },
        { "dst",                              { T::Dst,                              5, 0 } },
        { "errorf",                           { T::ErrorF,                           4, 0 } },
        { "EvaluateAttributeAtCentroid",      { T::EvaluateAttributeAtCentroid,      5, 0 } },
        { "EvaluateAttributeAtSample",        { T::EvaluateAttributeAtSample,        5, 0 } },
        { "EvaluateAttributeSnapped",         { T::EvaluateAttributeSnapped,         5, 0 } },
        { "exp",                              { T::Exp,                              1, 1 } },
        { "exp2",                             { T::Exp2,                             1, 1 } },
        { "f16tof32",                         { T::F16toF32,                         5, 0 } },
        { "f32tof16",                         { T::F32toF16,                         5, 0 } },
        { "faceforward",                      { T::FaceForward,                      1, 1 } },
        { "firstbithigh",                     { T::FirstBitHigh,                     5, 0 } },
        { "firstbitlow",                      { T::FirstBitLow,                      5, 0 } },
        { "floor",                            { T::Floor,                            1, 1 } },
        { "fma",                              { T::FMA,                              5, 0 } },
        { "fmod",                             { T::FMod,                             1, 1 } },
        { "frac",                             { T::Frac,                             1, 1 } },
        { "frexp",                            { T::FrExp,                            2, 1 } },
        { "fwidth",                           { T::FWidth,                           2, 1 } },
        { "GetRenderTargetSampleCount",       { T::GetRenderTargetSampleCount,       4, 0 } },
        { "GetRenderTargetSamplePosition",    { T::GetRenderTargetSamplePosition,    4, 0 } },
        { "GroupMemoryBarrier",               { T::GroupMemoryBarrier,               5, 0 } },
        { "GroupMemoryBarrierWithGroupSync",  { T::GroupMemoryBarrierWithGroupSync,  5, 0 } },
        { "InterlockedAdd",                   { T::InterlockedAdd,                   5, 0 } },
        { "InterlockedAnd",                   { T::InterlockedAnd,                   5, 0 } },
        { "InterlockedCompareExchange",       { T::InterlockedCompareExchange,       5, 0 } },
        { "InterlockedCompareStore",          { T::InterlockedCompareStore,          5, 0 } },
        { "InterlockedExchange",              { T::InterlockedExchange,              5, 0 } },
        { "InterlockedMax",                   { T::InterlockedMax,                   5, 0 } },
        { "InterlockedMin",                   { T::InterlockedMin,                   5, 0 } },
        { "InterlockedOr",                    { T::InterlockedOr,                    5, 0 } },
        { "InterlockedXor",                   { T::InterlockedXor,                   5, 0 } },
        { "isfinite",                         { T::IsFinite,                         1, 1 } },
        { "isinf",                            { T::IsInf,                            1, 1 } },
        { "isnan",                            { T::IsNaN,                            1, 1 } },
        { "ldexp",                            { T::LdExp,                            1, 1 } },
        { "length",                           { T::Length,                           1, 1 } },
        { "lerp",                             { T::Lerp,                             1, 1 } },
        { "lit",                              { T::Lit,                              1, 1 } },
        { "log",                              { T::Log,                              1, 1 } },
        { "log10",                            { T::Log10,                            1, 1 } },
        { "log2",                             { T::Log2,                             1, 1 } },
        { "mad",                              { T::MAD,                              5, 0 } },
        { "max",                              { T::Max,                              1, 1 } },
        { "min",                              { T::Min,                              1, 1 } },
        { "modf",                             { T::ModF,                             1, 1 } },
        { "msad4",                            { T::MSAD4,                            5, 0 } },
        { "mul",                              { T::Mul,                              1, 0 } },
        { "noise",                            { T::Noise,                            1, 1 } },
        { "normalize",                        { T::Normalize,                        1, 1 } },
        { "pow",                              { T::Pow,                              1, 1 } },
        { "printf",                           { T::PrintF,                           4, 0 } },
        { "Process2DQuadTessFactorsAvg",      { T::Process2DQuadTessFactorsAvg,      5, 0 } },
        { "Process2DQuadTessFactorsMax",      { T::Process2DQuadTessFactorsMax,      5, 0 } },
        { "Process2DQuadTessFactorsMin",      { T::Process2DQuadTessFactorsMin,      5, 0 } },
        { "ProcessIsolineTessFactors",        { T::ProcessIsolineTessFactors,        5, 0 } },
        { "ProcessQuadTessFactorsAvg",        { T::ProcessQuadTessFactorsAvg,        5, 0 } },
        { "ProcessQuadTessFactorsMax",        { T::ProcessQuadTessFactorsMax,        5, 0 } },
        { "ProcessQuadTessFactorsMin",        { T::ProcessQuadTessFactorsMin,        5, 0 } },
        { "ProcessTriTessFactorsAvg",         { T::ProcessTriTessFactorsAvg,         5, 0 } },
        { "ProcessTriTessFactorsMax",         { T::ProcessTriTessFactorsMax,         5, 0 } },
        { "ProcessTriTessFactorsMin",         { T::ProcessTriTessFactorsMin,         5, 0 } },
        { "radians",                          { T::Radians,                          1, 0 } },
        { "rcp",                              { T::Rcp,                              5, 0 } },
        { "reflect",                          { T::Reflect,                          1, 0 } },
        { "refract",                          { T::Refract,                          1, 1 } },
        { "reversebits",                      { T::ReverseBits,                      5, 0 } },
        { "round",                            { T::Round,                            1, 1 } },
        { "rsqrt",                            { T::RSqrt,                            1, 1 } },
        { "saturate",                         { T::Saturate,                         1, 0 } },
        { "sign",                             { T::Sign,                             1, 1 } },
        { "sin",                              { T::Sin,                              1, 1 } },
        { "sincos",                           { T::SinCos,                           1, 1 } },
        { "sinh",                             { T::SinH,                             1, 1 } },
        { "smoothstep",                       { T::SmoothStep,                       1, 1 } },
        { "sqrt",                             { T::Sqrt,                             1, 1 } },
        { "step",                             { T::Step,                             1, 1 } },
        { "tan",                              { T::Tan,                              1, 1 } },
        { "tanh",                             { T::TanH,                             1, 1 } },
        { "tex1D",                            { T::Tex1D_2,                          1, 0 } }, // Tex1D_4: 2.1
        { "tex1Dbias",                        { T::Tex1DBias,                        2, 1 } },
        { "tex1Dgrad",                        { T::Tex1DGrad,                        2, 1 } },
        { "tex1Dlod",                         { T::Tex1DLod,                         3, 1 } },
        { "tex1Dproj",                        { T::Tex1DProj,                        2, 1 } },
        { "tex2D",                            { T::Tex2D_2,                          1, 1 } }, // Tex2D_4: 2.1
        { "tex2Dbias",                        { T::Tex2DBias,                        2, 1 } },
        { "tex2Dgrad",                        { T::Tex2DGrad,                        2, 1 } },
        { "tex2Dlod",                         { T::Tex2DLod,                         3, 0 } },
        { "tex2Dproj",                        { T::Tex2DProj,                        2, 1 } },
        { "tex3D",                            { T::Tex3D_2,                          1, 1 } }, // Tex3D_4: 2.1
        { "tex3Dbias",                        { T::Tex3DBias,                        2, 1 } },
        { "tex3Dgrad",                        { T::Tex3DGrad,                        2, 1 } },
        { "tex3Dlod",                         { T::Tex3DLod,                         3, 1 } },
        { "tex3Dproj",                        { T::Tex3DProj,                        2, 1 } },
        { "texCUBE",                          { T::TexCube_2,                        1, 1 } }, // TexCube_4: 2.1
        { "texCUBEbias",                      { T::TexCubeBias,                      2, 1 } },
        { "texCUBEgrad",                      { T::TexCubeGrad,                      2, 1 } },
        { "texCUBElod",                       { T::TexCubeLod,                       3, 1 } },
        { "texCUBEproj",                      { T::TexCubeProj,                      2, 1 } },
        { "transpose",                        { T::Transpose,                        1, 0 } },
        { "trunc",                            { T::Trunc,                            1, 0 } },

        { "GetDimensions",                    { T::Texture_GetDimensions,            5, 0 } },
        { "Load",                             { T::Texture_Load_1,                   4, 0 } },
        { "Sample",                           { T::Texture_Sample_2,                 4, 0 } },
        { "SampleBias",                       { T::Texture_SampleBias_3,             4, 0 } },
        { "SampleCmp",                        { T::Texture_SampleCmp_3,              4, 0 } },
        { "SampleCmpLevelZero",               { T::Texture_SampleCmp_3,              4, 0 } }, // Identical to SampleCmp (but only for Level 0)
        { "SampleGrad",                       { T::Texture_SampleGrad_4,             4, 0 } },
        { "SampleLevel",                      { T::Texture_SampleLevel_3,            4, 0 } },
        { "CalculateLevelOfDetail",           { T::Texture_QueryLod,                 4, 1 } }, // Fragment shader only
        { "CalculateLevelOfDetailUnclamped",  { T::Texture_QueryLodUnclamped,        4, 1 } }, // Fragment shader only

        { "Append",                           { T::StreamOutput_Append,              4, 0 } },
        { "RestartStrip",                     { T::StreamOutput_RestartStrip,        4, 0 } },
    };
}

const HLSLIntrinsicsMap& HLSLIntrinsics()
{
    static const HLSLIntrinsicsMap intrinsicMap = GenerateIntrinsicMap();
    return intrinsicMap;
}


/* ----- GetTypeDenoterForHLSLIntrinsicWithArgs ----- */

enum ArgumentIndex
{
    ArgUndefined    = -1,

    Arg1            = 0,
    Arg2            = 1,
    Arg3            = 2,
};

class IntrinsicDescriptor
{
    
    public:

        IntrinsicDescriptor(int numArgs = 0);
        IntrinsicDescriptor(int numArgsMin, int numArgsMax);
        IntrinsicDescriptor(DataType dataType, int numArgs = 0);
        IntrinsicDescriptor(ArgumentIndex takeByArgIndex, int numArgs = 0);

        TypeDenoterPtr GetTypeDenoterWithArgs(const std::vector<ExprPtr>& args) const;

    private:

        int             numArgsMin_     = 0;
        int             numArgsMax_     = 0;
        DataType        dataType_       = DataType::Undefined;
        ArgumentIndex   takeByArgIndex_ = ArgUndefined;

};

IntrinsicDescriptor::IntrinsicDescriptor(int numArgs) :
    numArgsMin_{ numArgs },
    numArgsMax_{ numArgs }
{
}

IntrinsicDescriptor::IntrinsicDescriptor(int numArgsMin, int numArgsMax) :
    numArgsMin_{ numArgsMin },
    numArgsMax_{ numArgsMax }
{
}

IntrinsicDescriptor::IntrinsicDescriptor(DataType dataType, int numArgs) :
    numArgsMin_ { numArgs  },
    numArgsMax_ { numArgs  },
    dataType_   { dataType }
{
}

IntrinsicDescriptor::IntrinsicDescriptor(ArgumentIndex takeByArgIndex, int numArgs) :
    numArgsMin_     { numArgs        },
    numArgsMax_     { numArgs        },
    takeByArgIndex_ { takeByArgIndex }
{
}

TypeDenoterPtr IntrinsicDescriptor::GetTypeDenoterWithArgs(const std::vector<ExprPtr>& args) const
{
    /* Validate number of arguments */
    if (numArgsMin_ >= 0)
    {
        auto numMin = static_cast<std::size_t>(numArgsMin_);
        auto numMax = static_cast<std::size_t>(numArgsMax_);

        if (args.size() < numMin || args.size() > numMax)
        {
            RuntimeErr(
                "invalid number of arguments for intrinsic (expected " +
                (numMin < numMax ? std::to_string(numMin) + "-" + std::to_string(numMax) : std::to_string(numMin)) +
                ", but got " + std::to_string(args.size()) + ")"
            );
        }
    }

    /* Find return type denoter with the specified arguments */
    if (dataType_ != DataType::Undefined)
    {
        /* Return fixed base type denoter */
        return MakeShared<BaseTypeDenoter>(dataType_);
    }
    else if (takeByArgIndex_ != ArgUndefined)
    {
        /* Take type denoter from argument */
        auto idx = static_cast<std::size_t>(takeByArgIndex_);
        return args[idx]->GetTypeDenoter();
    }
    else
    {
        /* Return default void type denoter */
        return MakeShared<VoidTypeDenoter>();
    }
}

static std::map<Intrinsic, IntrinsicDescriptor> GenerateIntrinsicDescriptorMap()
{
    using T = Intrinsic;

    return
    {
        { T::Abort,                                   {  }                    },
        { T::Abs,                                     { Arg1, 1 }             },
        { T::ACos,                                    { Arg1, 1 }             },
        { T::All,                                     { DataType::Bool, 1 }   },
        { T::AllMemoryBarrier,                        {  }                    },
        { T::AllMemoryBarrierWithGroupSync,           {  }                    },
        { T::Any,                                     { DataType::Bool, 1 }   },
        { T::AsDouble,                                { DataType::Double, 2 } },
        { T::AsFloat,                                 { Arg1, 1 }             },
        { T::ASin,                                    { Arg1, 1 }             },
        { T::AsInt,                                   { Arg1, 1 }             },
        { T::AsUInt_1,                                { Arg1, 1 }             },
        { T::AsUInt_3,                                { 3 }                   },
        { T::ATan,                                    { Arg1, 1 }             },
        { T::ATan2,                                   { Arg2, 2 }             },
        { T::Ceil,                                    { Arg1, 1 }             },
        { T::CheckAccessFullyMapped,                  { DataType::Bool, 1 }   },
        { T::Clamp,                                   { Arg1, 3 }             },
        { T::Clip,                                    { 1 }                   },
        { T::Cos,                                     { Arg1, 1 }             },
        { T::CosH,                                    { Arg1, 1 }             },
        { T::CountBits,                               { DataType::UInt, 1 }   },
        { T::Cross,                                   { DataType::Float3, 2 } },
        { T::D3DCOLORtoUBYTE4,                        { DataType::Int4, 1 }   },
        { T::DDX,                                     { Arg1, 1 }             },
        { T::DDXCoarse,                               { Arg1, 1 }             },
        { T::DDXFine,                                 { Arg1, 1 }             },
        { T::DDY,                                     { Arg1, 1 }             },
        { T::DDYCoarse,                               { Arg1, 1 }             },
        { T::DDYFine,                                 { Arg1, 1 }             },
        { T::Degrees,                                 { Arg1, 1 }             },
        { T::Determinant,                             { DataType::Float, 1 }  },
        { T::DeviceMemoryBarrier,                     {  }                    },
        { T::DeviceMemoryBarrierWithGroupSync,        {  }                    },
        { T::Distance,                                { DataType::Float, 2 }  },
        { T::Dot,                                     { DataType::Float, 2 }  }, // float or int with size of input
        { T::Dst,                                     { Arg1, 2 }             },
        { T::ErrorF,                                  { -1 }                  },
        { T::EvaluateAttributeAtCentroid,             { Arg1, 1 }             },
        { T::EvaluateAttributeAtSample,               { Arg1, 2 }             },
        { T::EvaluateAttributeSnapped,                { Arg1, 2 }             },
        { T::Exp,                                     { Arg1, 1 }             },
        { T::Exp2,                                    { Arg1, 1 }             },
        { T::F16toF32,                                { DataType::Float, 1 }  },
        { T::F32toF16,                                { DataType::UInt, 1 }   },
        { T::FaceForward,                             { Arg1, 3 }             },
        { T::FirstBitHigh,                            { DataType::Int, 1 }    },
        { T::FirstBitLow,                             { DataType::Int, 1 }    },
        { T::Floor,                                   { Arg1, 1 }             },
        { T::FMA,                                     { Arg1, 3 }             },
        { T::FMod,                                    { Arg1, 2 }             },
        { T::Frac,                                    { Arg1, 1 }             },
        { T::FrExp,                                   { Arg1, 2 }             },
        { T::FWidth,                                  { Arg1, 1 }             },
        { T::GetRenderTargetSampleCount,              { DataType::UInt }      },
        { T::GetRenderTargetSamplePosition,           { DataType::Float2, 1 } },
        { T::GroupMemoryBarrier,                      {  }                    },
        { T::GroupMemoryBarrierWithGroupSync,         {  }                    },
        { T::InterlockedAdd,                          { 2, 3 }                },
        { T::InterlockedAnd,                          { 2, 3 }                },
        { T::InterlockedCompareExchange,              { 4 }                   },
        { T::InterlockedCompareStore,                 { 3 }                   },
        { T::InterlockedExchange,                     { 3 }                   },
        { T::InterlockedMax,                          { 2, 3 }                },
        { T::InterlockedMin,                          { 2, 3 }                },
        { T::InterlockedOr,                           { 2, 3 }                },
        { T::InterlockedXor,                          { 2, 3 }                },
        { T::IsFinite,                                { Arg1, 1 }             }, // bool with size as input
        { T::IsInf,                                   { Arg1, 1 }             }, // bool with size as input
        { T::IsNaN,                                   { Arg1, 1 }             }, // bool with size as input
        { T::LdExp,                                   { Arg1, 2 }             }, // float with size as input
        { T::Length,                                  { DataType::Float, 1 }  },
        { T::Lerp,                                    { Arg1, 3 }             },
        { T::Lit,                                     { Arg1, 3 }             },
        { T::Log,                                     { Arg1, 1 }             },
        { T::Log10,                                   { Arg1, 1 }             },
        { T::Log2,                                    { Arg1, 1 }             },
        { T::MAD,                                     { Arg1, 3 }             },
        { T::Max,                                     { Arg1, 2 }             },
        { T::Min,                                     { Arg1, 2 }             },
        { T::ModF,                                    { Arg1, 2 }             },
        { T::MSAD4,                                   { DataType::UInt4, 3 }  },
      //{ T::Mul,                                     {  }                    }, // special case
        { T::Noise,                                   { DataType::Float, 1 }  },
        { T::Normalize,                               { Arg1, 1 }             },
        { T::Pow,                                     { Arg1, 2 }             },
        { T::PrintF,                                  { -1 }                  },
        { T::Process2DQuadTessFactorsAvg,             { 5 }                   },
        { T::Process2DQuadTessFactorsMax,             { 5 }                   },
        { T::Process2DQuadTessFactorsMin,             { 5 }                   },
        { T::ProcessIsolineTessFactors,               { 4 }                   },
        { T::ProcessQuadTessFactorsAvg,               { 5 }                   },
        { T::ProcessQuadTessFactorsMax,               { 5 }                   },
        { T::ProcessQuadTessFactorsMin,               { 5 }                   },
        { T::ProcessTriTessFactorsAvg,                { 5 }                   },
        { T::ProcessTriTessFactorsMax,                { 5 }                   },
        { T::ProcessTriTessFactorsMin,                { 5 }                   },
        { T::Radians,                                 { Arg1, 1 }             },
        { T::Rcp,                                     { Arg1, 1 }             },
        { T::Reflect,                                 { Arg1, 2 }             },
        { T::Refract,                                 { Arg1, 3 }             },
        { T::ReverseBits,                             { DataType::UInt, 1 }   },
        { T::Round,                                   { Arg1, 1 }             },
        { T::RSqrt,                                   { Arg1, 1 }             },
        { T::Saturate,                                { Arg1, 1 }             },
        { T::Sign,                                    { Arg1, 1 }             },
        { T::Sin,                                     { Arg1, 1 }             },
        { T::SinCos,                                  { 3 }                   },
        { T::SinH,                                    { Arg1, 1 }             },
        { T::SmoothStep,                              { Arg3, 3 }             },
        { T::Sqrt,                                    { Arg1, 1 }             },
        { T::Step,                                    { Arg1, 2 }             },
        { T::Tan,                                     { Arg1, 1 }             },
        { T::TanH,                                    { Arg1, 1 }             },
        { T::Tex1D_2,                                 { DataType::Float4, 2 } },
        { T::Tex1D_4,                                 { DataType::Float4, 4 } },
        { T::Tex1DBias,                               { DataType::Float4, 2 } },
        { T::Tex1DGrad,                               { DataType::Float4, 4 } },
        { T::Tex1DLod,                                { DataType::Float4, 2 } },
        { T::Tex1DProj,                               { DataType::Float4, 2 } },
        { T::Tex2D_2,                                 { DataType::Float4, 2 } },
        { T::Tex2D_4,                                 { DataType::Float4, 4 } },
        { T::Tex2DBias,                               { DataType::Float4, 2 } },
        { T::Tex2DGrad,                               { DataType::Float4, 4 } },
        { T::Tex2DLod,                                { DataType::Float4, 2 } },
        { T::Tex2DProj,                               { DataType::Float4, 2 } },
        { T::Tex3D_2,                                 { DataType::Float4, 2 } },
        { T::Tex3D_4,                                 { DataType::Float4, 4 } },
        { T::Tex3DBias,                               { DataType::Float4, 2 } },
        { T::Tex3DGrad,                               { DataType::Float4, 4 } },
        { T::Tex3DLod,                                { DataType::Float4, 2 } },
        { T::Tex3DProj,                               { DataType::Float4, 2 } },
        { T::TexCube_2,                               { DataType::Float4, 2 } },
        { T::TexCube_4,                               { DataType::Float4, 4 } },
        { T::TexCubeBias,                             { DataType::Float4, 2 } },
        { T::TexCubeGrad,                             { DataType::Float4, 4 } },
        { T::TexCubeLod,                              { DataType::Float4, 2 } },
        { T::TexCubeProj,                             { DataType::Float4, 2 } },
      //{ T::Transpose,                               {  }                    }, // special case
        { T::Trunc,                                   { Arg1, 1 }             },

        { T::Texture_GetDimensions,                   { 3 }                   },
        { T::Texture_Load_1,                          { DataType::Float4, 1 } },
        { T::Texture_Load_2,                          { DataType::Float4, 2 } },
        { T::Texture_Load_3,                          { DataType::Float4, 3 } },
        { T::Texture_Sample_2,                        { DataType::Float4, 2 } },
        { T::Texture_Sample_3,                        { DataType::Float4, 3 } },
        { T::Texture_Sample_4,                        { DataType::Float4, 4 } },
        { T::Texture_Sample_5,                        { DataType::Float4, 5 } },
        { T::Texture_SampleBias_3,                    { DataType::Float4, 3 } },
        { T::Texture_SampleBias_4,                    { DataType::Float4, 4 } },
        { T::Texture_SampleBias_5,                    { DataType::Float4, 5 } },
        { T::Texture_SampleBias_6,                    { DataType::Float4, 6 } },
        { T::Texture_SampleCmp_3,                     { DataType::Float4, 3 } },
        { T::Texture_SampleCmp_4,                     { DataType::Float4, 4 } },
        { T::Texture_SampleCmp_5,                     { DataType::Float4, 5 } },
        { T::Texture_SampleCmp_6,                     { DataType::Float4, 6 } },
        { T::Texture_SampleGrad_4,                    { DataType::Float4, 4 } },
        { T::Texture_SampleGrad_5,                    { DataType::Float4, 5 } },
        { T::Texture_SampleGrad_6,                    { DataType::Float4, 6 } },
        { T::Texture_SampleGrad_7,                    { DataType::Float4, 7 } },
        { T::Texture_SampleLevel_3,                   { DataType::Float4, 3 } },
        { T::Texture_SampleLevel_4,                   { DataType::Float4, 4 } },
        { T::Texture_SampleLevel_5,                   { DataType::Float4, 5 } },
        { T::Texture_QueryLod,          { DataType::Float,  2 } },
        { T::Texture_QueryLodUnclamped, { DataType::Float,  2 } },

        { T::StreamOutput_Append,                     { 1 }                   },
        { T::StreamOutput_RestartStrip,               { }                     },
    };
}

TypeDenoterPtr GetTypeDenoterForHLSLIntrinsicWithArgs(const Intrinsic intrinsic, const std::vector<ExprPtr>& args)
{
    if (intrinsic == Intrinsic::Mul)
    {
        /* Validate number of arguments */
        if (args.size() != 2)
            RuntimeErr("invalid number of arguments for intrinsic");

        auto type0 = args[0]->GetTypeDenoter();
        auto type1 = args[1]->GetTypeDenoter();

        if (type0->IsScalar())
            return type1;
        
        if (type0->IsVector())
        {
            if (type1->IsScalar())
                return type0;

            if (type1->IsVector())
            {
                auto baseDataType0 = BaseDataType(static_cast<BaseTypeDenoter&>(*type0).dataType);
                return MakeShared<BaseTypeDenoter>(baseDataType0); // scalar
            }

            if (type1->IsMatrix())
            {
                auto dataType1      = static_cast<BaseTypeDenoter&>(*type1).dataType;
                auto baseDataType1  = BaseDataType(dataType1);
                auto matrixTypeDim1 = MatrixTypeDim(dataType1);
                return MakeShared<BaseTypeDenoter>(VectorDataType(baseDataType1, matrixTypeDim1.second));
            }
        }

        if (type0->IsMatrix())
        {
            if (type1->IsScalar())
                return type0;

            if (type1->IsVector())
            {
                auto dataType0      = static_cast<BaseTypeDenoter&>(*type0).dataType;
                auto baseDataType0  = BaseDataType(dataType0);
                auto matrixTypeDim0 = MatrixTypeDim(dataType0);
                return MakeShared<BaseTypeDenoter>(VectorDataType(baseDataType0, matrixTypeDim0.first));
            }

            if (type1->IsMatrix())
            {
                auto dataType0      = static_cast<BaseTypeDenoter&>(*type0).dataType;
                auto baseDataType0  = BaseDataType(dataType0);
                auto matrixTypeDim0 = MatrixTypeDim(dataType0);
                auto dataType1      = static_cast<BaseTypeDenoter&>(*type1).dataType;
                auto matrixTypeDim1 = MatrixTypeDim(dataType1);
                return MakeShared<BaseTypeDenoter>(MatrixDataType(baseDataType0, matrixTypeDim0.first, matrixTypeDim1.second));
            }
        }

        RuntimeErr("invalid arguments in intrinsic 'mul'");
    }
    else if (intrinsic == Intrinsic::Transpose)
    {
        /* Validate number of arguments */
        if (args.size() != 1)
            RuntimeErr("invalid number of arguments for intrinsic");

        auto type0 = args[0]->GetTypeDenoter();

        if (type0->IsMatrix())
        {
            /* Convert MxN matrix type to NxM matrix type */
            auto dataType0      = static_cast<BaseTypeDenoter&>(*type0).dataType;
            auto baseDataType0  = BaseDataType(dataType0);
            auto matrixTypeDim0 = MatrixTypeDim(dataType0);
            return MakeShared<BaseTypeDenoter>(MatrixDataType(baseDataType0, matrixTypeDim0.second, matrixTypeDim0.first));
        }

        RuntimeErr("invalid arguments in intrinsic 'transpose'");
    }
    else
    {
        /* Get type denoter from intrinsic map */
        static const auto intrinsicMap = GenerateIntrinsicDescriptorMap();
        auto it = intrinsicMap.find(intrinsic);
        if (it != intrinsicMap.end())
            return it->second.GetTypeDenoterWithArgs(args);
        else
            RuntimeErr("failed to derive type denoter for intrinsic");
    }
}


} // /namespace Xsc



// ================================================================================