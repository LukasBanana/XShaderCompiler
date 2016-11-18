/*
 * HLSLIntrinsics.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "HLSLIntrinsics.h"


namespace Xsc
{


static HLSLIntrinsicsMap GenerateIntrinsicMap()
{
    using I = Intrinsic;

    return
    {
        { "abort",                              { I::Abort,                             4, 0 } },
        { "abs",                                { I::Abs,                               1, 1 } },
        { "acos",                               { I::ACos,                              1, 1 } },
        { "all",                                { I::All,                               1, 1 } },
        { "AllMemoryBarrier",                   { I::AllMemoryBarrier,                  5, 0 } },
        { "AllMemoryBarrierWithGroupSync",      { I::AllMemoryBarrierWithGroupSync,     5, 0 } },
        { "any",                                { I::Any,                               1, 1 } },
        { "asdouble",                           { I::AsDouble,                          5, 0 } },
        { "asfloat",                            { I::AsFloat,                           4, 0 } },
        { "asin",                               { I::ASin,                              1, 1 } },
        { "asint",                              { I::AsInt,                             4, 0 } },
        { "asuint",                             { I::AsUInt,                            5, 0 } }, // AsUInt_2: 4.0
        { "atan",                               { I::ATan,                              1, 1 } },
        { "atan2",                              { I::ATan2,                             1, 1 } },
        { "ceil",                               { I::Ceil,                              1, 1 } },
        { "CheckAccessFullyMapped",             { I::CheckAccessFullyMapped,            5, 0 } },
        { "clamp",                              { I::Clamp,                             1, 1 } },
        { "clip",                               { I::Clip,                              1, 1 } },
        { "cos",                                { I::Cos,                               1, 1 } },
        { "cosh",                               { I::CosH,                              1, 1 } },
        { "countbits",                          { I::CountBits,                         5, 0 } },
        { "cross",                              { I::Cross,                             1, 1 } },
        { "D3DCOLORtoUBYTE4",                   { I::D3DCOLORtoUBYTE4,                  1, 1 } },
        { "ddx",                                { I::DDX,                               2, 1 } },
        { "ddx_coarse",                         { I::DDXCoarse,                         5, 0 } },
        { "ddx_fine",                           { I::DDXFine,                           5, 0 } },
        { "ddy",                                { I::DDY,                               2, 1 } },
        { "ddy_coarse",                         { I::DDYCoarse,                         5, 0 } },
        { "ddy_fine",                           { I::DDYFine,                           5, 0 } },
        { "degrees",                            { I::Degrees,                           1, 1 } },
        { "determinant",                        { I::Determinant,                       1, 1 } },
        { "DeviceMemoryBarrier",                { I::DeviceMemoryBarrier,               5, 0 } },
        { "DeviceMemoryBarrierWithGroupSync",   { I::DeviceMemoryBarrierWithGroupSync,  5, 0 } },
        { "distance",                           { I::Distance,                          1, 1 } },
        { "dot",                                { I::Dot,                               1, 0 } },
        { "dst",                                { I::Dst,                               5, 0 } },
        { "errorf",                             { I::ErrorF,                            4, 0 } },
        { "EvaluateAttributeAtCentroid",        { I::EvaluateAttributeAtCentroid,       5, 0 } },
        { "EvaluateAttributeAtSample",          { I::EvaluateAttributeAtSample,         5, 0 } },
        { "EvaluateAttributeSnapped",           { I::EvaluateAttributeSnapped,          5, 0 } },
        { "exp",                                { I::Exp,                               1, 1 } },
        { "exp2",                               { I::Exp2,                              1, 1 } },
        { "f16tof32",                           { I::F16toF32,                          5, 0 } },
        { "f32tof16",                           { I::F32toF16,                          5, 0 } },
        { "faceforward",                        { I::FaceForward,                       1, 1 } },
        { "firstbithigh",                       { I::FirstBitHigh,                      5, 0 } },
        { "firstbitlow",                        { I::FirstBitLow,                       5, 0 } },
        { "floor",                              { I::Floor,                             1, 1 } },
        { "fma",                                { I::FMA,                               5, 0 } },
        { "fmod",                               { I::FMod,                              1, 1 } },
        { "frac",                               { I::Frac,                              1, 1 } },
        { "frexp",                              { I::FrExp,                             2, 1 } },
        { "fwidth",                             { I::FWidth,                            2, 1 } },
        { "GetRenderTargetSampleCount",         { I::GetRenderTargetSampleCount,        4, 0 } },
        { "GetRenderTargetSamplePosition",      { I::GetRenderTargetSamplePosition,     4, 0 } },
        { "GroupMemoryBarrier",                 { I::GroupMemoryBarrier,                5, 0 } },
        { "GroupMemoryBarrierWithGroupSync",    { I::GroupMemoryBarrierWithGroupSync,   5, 0 } },
        { "InterlockedAdd",                     { I::InterlockedAdd,                    5, 0 } },
        { "InterlockedAnd",                     { I::InterlockedAnd,                    5, 0 } },
        { "InterlockedCompareExchange",         { I::InterlockedCompareExchange,        5, 0 } },
        { "InterlockedCompareStore",            { I::InterlockedCompareStore,           5, 0 } },
        { "InterlockedExchange",                { I::InterlockedExchange,               5, 0 } },
        { "InterlockedMax",                     { I::InterlockedMax,                    5, 0 } },
        { "InterlockedMin",                     { I::InterlockedMin,                    5, 0 } },
        { "InterlockedOr",                      { I::InterlockedOr,                     5, 0 } },
        { "InterlockedXor",                     { I::InterlockedXor,                    5, 0 } },
        { "isfinite",                           { I::IsFinite,                          1, 1 } },
        { "isinf",                              { I::IsInf,                             1, 1 } },
        { "isnan",                              { I::IsNaN,                             1, 1 } },
        { "ldexp",                              { I::LdExp,                             1, 1 } },
        { "length",                             { I::Length,                            1, 1 } },
        { "lerp",                               { I::Lerp,                              1, 1 } },
        { "lit",                                { I::Lit,                               1, 1 } },
        { "log",                                { I::Log,                               1, 1 } },
        { "log10",                              { I::Log10,                             1, 1 } },
        { "log2",                               { I::Log2,                              1, 1 } },
        { "mad",                                { I::MAD,                               5, 0 } },
        { "max",                                { I::Max,                               1, 1 } },
        { "min",                                { I::Min,                               1, 1 } },
        { "modf",                               { I::ModF,                              1, 1 } },
        { "msad4",                              { I::MSAD4,                             5, 0 } },
        { "mul",                                { I::Mul,                               1, 0 } },
        { "noise",                              { I::Noise,                             1, 1 } },
        { "normalize",                          { I::Normalize,                         1, 1 } },
        { "pow",                                { I::Pow,                               1, 1 } },
        { "printf",                             { I::PrintF,                            4, 0 } },
        { "Process2DQuadTessFactorsAvg",        { I::Process2DQuadTessFactorsAvg,       5, 0 } },
        { "Process2DQuadTessFactorsMax",        { I::Process2DQuadTessFactorsMax,       5, 0 } },
        { "Process2DQuadTessFactorsMin",        { I::Process2DQuadTessFactorsMin,       5, 0 } },
        { "ProcessIsolineTessFactors",          { I::ProcessIsolineTessFactors,         5, 0 } },
        { "ProcessQuadTessFactorsAvg",          { I::ProcessQuadTessFactorsAvg,         5, 0 } },
        { "ProcessQuadTessFactorsMax",          { I::ProcessQuadTessFactorsMax,         5, 0 } },
        { "ProcessQuadTessFactorsMin",          { I::ProcessQuadTessFactorsMin,         5, 0 } },
        { "ProcessTriTessFactorsAvg",           { I::ProcessTriTessFactorsAvg,          5, 0 } },
        { "ProcessTriTessFactorsMax",           { I::ProcessTriTessFactorsMax,          5, 0 } },
        { "ProcessTriTessFactorsMin",           { I::ProcessTriTessFactorsMin,          5, 0 } },
        { "radians",                            { I::Radians,                           1, 0 } },
        { "rcp",                                { I::Rcp,                               5, 0 } },
        { "reflect",                            { I::Reflect,                           1, 0 } },
        { "refract",                            { I::Refract,                           1, 1 } },
        { "reversebits",                        { I::ReverseBits,                       5, 0 } },
        { "round",                              { I::Round,                             1, 1 } },
        { "rsqrt",                              { I::RSqrt,                             1, 1 } },
        { "saturate",                           { I::Saturate,                          1, 0 } },
        { "sign",                               { I::Sign,                              1, 1 } },
        { "sin",                                { I::Sin,                               1, 1 } },
        { "sincos",                             { I::SinCos,                            1, 1 } },
        { "sinh",                               { I::SinH,                              1, 1 } },
        { "smoothstep",                         { I::SmoothStep,                        1, 1 } },
        { "sqrt",                               { I::Sqrt,                              1, 1 } },
        { "step",                               { I::Step,                              1, 1 } },
        { "tan",                                { I::Tan,                               1, 1 } },
        { "tanh",                               { I::TanH,                              1, 1 } },
        { "tex1D",                              { I::Tex1D,                             1, 0 } }, // Tex1D_2: 2.1
        { "tex1Dbias",                          { I::Tex1DBias,                         2, 1 } },
        { "tex1Dgrad",                          { I::Tex1DGrad,                         2, 1 } },
        { "tex1Dlod",                           { I::Tex1DLod,                          3, 1 } },
        { "tex1Dproj",                          { I::Tex1DProj,                         2, 1 } },
        { "tex2D",                              { I::Tex2D,                             1, 1 } }, // Tex2D_2: 2.1
        { "tex2Dbias",                          { I::Tex2DBias,                         2, 1 } },
        { "tex2Dgrad",                          { I::Tex2DGrad,                         2, 1 } },
        { "tex2Dlod",                           { I::Tex2DLod,                          3, 0 } },
        { "tex2Dproj",                          { I::Tex2DProj,                         2, 1 } },
        { "tex3D",                              { I::Tex3D,                             1, 1 } }, // Tex3D_2: 2.1
        { "tex3Dbias",                          { I::Tex3DBias,                         2, 1 } },
        { "tex3Dgrad",                          { I::Tex3DGrad,                         2, 1 } },
        { "tex3Dlod",                           { I::Tex3DLod,                          3, 1 } },
        { "tex3Dproj",                          { I::Tex3DProj,                         2, 1 } },
        { "texCUBE",                            { I::TexCube,                           1, 1 } }, // TexCube_2: 2.1
        { "texCUBEbias",                        { I::TexCubeBias,                       2, 1 } },
        { "texCUBEgrad",                        { I::TexCubeGrad,                       2, 1 } },
        { "texCUBElod",                         { I::TexCubeLod,                        3, 1 } },
        { "texCUBEproj",                        { I::TexCubeProj,                       2, 1 } },
        { "transpose",                          { I::Transpose,                         1, 0 } },
        { "trunc",                              { I::Trunc,                             1, 0 } },
    };
}

const HLSLIntrinsicsMap& HLSLIntrinsics()
{
    static const HLSLIntrinsicsMap intrinsicMap = GenerateIntrinsicMap();
    return intrinsicMap;
}


} // /namespace Xsc



// ================================================================================