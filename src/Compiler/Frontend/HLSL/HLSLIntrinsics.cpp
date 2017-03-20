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
#include "ReportIdents.h"


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
      //{ "",                                 { T::Equal,                            0, 0 } }, // GLSL only
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
      //{ "",                                 { T::GreaterThan,                      0, 0 } }, // GLSL only
      //{ "",                                 { T::GreaterThanEqual,                 0, 0 } }, // GLSL only
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
      //{ "",                                 { T::LessThan,                         0, 0 } }, // GLSL only
      //{ "",                                 { T::LessThanEqual,                    0, 0 } }, // GLSL only
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
        { "normalize",                        { T::Normalize,                        1, 1 } },
      //{ ""                                  { T::NotEqual,                         0, 0 } }, // GLSL only
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


/* ----- IntrinsicSignature class ----- */

enum class IntrinsicReturnType
{
    Void,
    
    Bool,
    Int,
    Int2,
    Int3,
    Int4,
    UInt,
    UInt2,
    UInt3,
    UInt4,
    Float,
    Float2,
    Float3,
    Float4,
    Double,

    GenericArg0,    // Get return type from first argument (index 0)
    GenericArg1,    // Get return type from second argument (index 1)
    GenericArg2,    // Get return type from thrid argument (index 2)
};

static DataType IntrinsicReturnTypeToDataType(const IntrinsicReturnType t)
{
    switch (t)
    {
        case IntrinsicReturnType::Bool:     return DataType::Bool;
        case IntrinsicReturnType::Int:      return DataType::Int;
        case IntrinsicReturnType::Int2:     return DataType::Int2;
        case IntrinsicReturnType::Int3:     return DataType::Int3;
        case IntrinsicReturnType::Int4:     return DataType::Int4;
        case IntrinsicReturnType::UInt:     return DataType::UInt;
        case IntrinsicReturnType::UInt2:    return DataType::UInt2;
        case IntrinsicReturnType::UInt3:    return DataType::UInt3;
        case IntrinsicReturnType::UInt4:    return DataType::UInt4;
        case IntrinsicReturnType::Float:    return DataType::Float;
        case IntrinsicReturnType::Float2:   return DataType::Float2;
        case IntrinsicReturnType::Float3:   return DataType::Float3;
        case IntrinsicReturnType::Float4:   return DataType::Float4;
        case IntrinsicReturnType::Double:   return DataType::Double;
        default:                            return DataType::Undefined;
    }
}

static std::size_t IntrinsicReturnTypeToArgIndex(const IntrinsicReturnType t)
{
    switch (t)
    {
        case IntrinsicReturnType::GenericArg0:  return 0;
        case IntrinsicReturnType::GenericArg1:  return 1;
        case IntrinsicReturnType::GenericArg2:  return 2;
        default:                                return ~0;
    }
}

static TypeDenoterPtr DeriveCommonTypeDenoter(std::size_t majorArgIndex, const std::vector<ExprPtr>& args)
{
    if (majorArgIndex < args.size())
    {
        /* Find common type denoter for all arguments */
        TypeDenoterPtr commonTypeDenoter = args[majorArgIndex]->GetTypeDenoter()->Get();

        for (std::size_t i = 0, n = args.size(); i < n; ++i)
        {
            if (i != majorArgIndex)
            {
                commonTypeDenoter = TypeDenoter::FindCommonTypeDenoter(
                    commonTypeDenoter,
                    args[i]->GetTypeDenoter()->Get()
                );
            }
        }

        return commonTypeDenoter;
    }
    return nullptr;
}

struct IntrinsicSignature
{
    IntrinsicSignature(int numArgs = 0);
    IntrinsicSignature(int numArgsMin, int numArgsMax);
    IntrinsicSignature(IntrinsicReturnType returnType, int numArgs = 0);

    TypeDenoterPtr GetTypeDenoterWithArgs(const std::vector<ExprPtr>& args) const;

    IntrinsicReturnType returnType = IntrinsicReturnType::Void;
    int                 numArgsMin = 0;
    int                 numArgsMax = 0;
};

IntrinsicSignature::IntrinsicSignature(int numArgs) :
    numArgsMin { numArgs },
    numArgsMax { numArgs }
{
}

IntrinsicSignature::IntrinsicSignature(int numArgsMin, int numArgsMax) :
    numArgsMin { numArgsMin },
    numArgsMax { numArgsMax }
{
}

IntrinsicSignature::IntrinsicSignature(IntrinsicReturnType returnType, int numArgs) :
    returnType { returnType },
    numArgsMin { numArgs    },
    numArgsMax { numArgs    }
{
}

TypeDenoterPtr IntrinsicSignature::GetTypeDenoterWithArgs(const std::vector<ExprPtr>& args) const
{
    /* Validate number of arguments */
    if (numArgsMin >= 0)
    {
        auto numMin = static_cast<std::size_t>(numArgsMin);
        auto numMax = static_cast<std::size_t>(numArgsMax);

        if (args.size() < numMin || args.size() > numMax)
        {
            RuntimeErr(
                R_InvalidIntrinsicArgCount(
                    "",
                    (numMin < numMax ? std::to_string(numMin) + "-" + std::to_string(numMax) : std::to_string(numMin)),
                    args.size()
                )
            );
        }
    }

    if (returnType != IntrinsicReturnType::Void)
    {
        /* Return fixed base type denoter */
        const auto returnTypeFixed = IntrinsicReturnTypeToDataType(returnType);
        if (returnTypeFixed != DataType::Undefined)
            return std::make_shared<BaseTypeDenoter>(returnTypeFixed);

        /* Take type denoter from argument */
        const auto returnTypeByArgIndex = IntrinsicReturnTypeToArgIndex(returnType);
        if (returnTypeByArgIndex < args.size())
            return DeriveCommonTypeDenoter(returnTypeByArgIndex, args);
    }

    /* Return default void type denoter */
    return std::make_shared<VoidTypeDenoter>();
}

static std::map<Intrinsic, IntrinsicSignature> GenerateIntrinsicSignatureMap()
{
    using T = Intrinsic;
    using Ret = IntrinsicReturnType;

    return
    {
        { T::Abort,                            {                        } },
        { T::Abs,                              { Ret::GenericArg0, 1    } },
        { T::ACos,                             { Ret::GenericArg0, 1    } },
        { T::All,                              { Ret::Bool,        1    } },
        { T::AllMemoryBarrier,                 {                        } },
        { T::AllMemoryBarrierWithGroupSync,    {                        } },
        { T::Any,                              { Ret::Bool,        1    } },
        { T::AsDouble,                         { Ret::Double,      2    } },
        { T::AsFloat,                          { Ret::GenericArg0, 1    } },
        { T::ASin,                             { Ret::GenericArg0, 1    } },
        { T::AsInt,                            { Ret::GenericArg0, 1    } },
        { T::AsUInt_1,                         { Ret::GenericArg0, 1    } },
        { T::AsUInt_3,                         {                   3    } },
        { T::ATan,                             { Ret::GenericArg0, 1    } },
        { T::ATan2,                            { Ret::GenericArg1, 2    } },
        { T::Ceil,                             { Ret::GenericArg0, 1    } },
        { T::CheckAccessFullyMapped,           { Ret::Bool,        1    } },
        { T::Clamp,                            { Ret::GenericArg0, 3    } },
        { T::Clip,                             {                   1    } },
        { T::Cos,                              { Ret::GenericArg0, 1    } },
        { T::CosH,                             { Ret::GenericArg0, 1    } },
        { T::CountBits,                        { Ret::UInt,        1    } },
        { T::Cross,                            { Ret::Float3,      2    } },
        { T::D3DCOLORtoUBYTE4,                 { Ret::Int4,        1    } },
        { T::DDX,                              { Ret::GenericArg0, 1    } },
        { T::DDXCoarse,                        { Ret::GenericArg0, 1    } },
        { T::DDXFine,                          { Ret::GenericArg0, 1    } },
        { T::DDY,                              { Ret::GenericArg0, 1    } },
        { T::DDYCoarse,                        { Ret::GenericArg0, 1    } },
        { T::DDYFine,                          { Ret::GenericArg0, 1    } },
        { T::Degrees,                          { Ret::GenericArg0, 1    } },
        { T::Determinant,                      { Ret::Float,       1    } },
        { T::DeviceMemoryBarrier,              {                        } },
        { T::DeviceMemoryBarrierWithGroupSync, {                        } },
        { T::Distance,                         { Ret::Float,       2    } },
        { T::Dot,                              { Ret::Float,       2    } }, // float or int with size of input
        { T::Dst,                              { Ret::GenericArg0, 2    } },
        { T::ErrorF,                           {                  -1    } },
        { T::Equal,                            { Ret::Bool,        2    } }, // GLSL only
        { T::EvaluateAttributeAtCentroid,      { Ret::GenericArg0, 1    } },
        { T::EvaluateAttributeAtSample,        { Ret::GenericArg0, 2    } },
        { T::EvaluateAttributeSnapped,         { Ret::GenericArg0, 2    } },
        { T::Exp,                              { Ret::GenericArg0, 1    } },
        { T::Exp2,                             { Ret::GenericArg0, 1    } },
        { T::F16toF32,                         { Ret::Float,       1    } },
        { T::F32toF16,                         { Ret::UInt,        1    } },
        { T::FaceForward,                      { Ret::GenericArg0, 3    } },
        { T::FirstBitHigh,                     { Ret::Int,         1    } },
        { T::FirstBitLow,                      { Ret::Int,         1    } },
        { T::Floor,                            { Ret::GenericArg0, 1    } },
        { T::FMA,                              { Ret::GenericArg0, 3    } },
        { T::FMod,                             { Ret::GenericArg0, 2    } },
        { T::Frac,                             { Ret::GenericArg0, 1    } },
        { T::FrExp,                            { Ret::GenericArg0, 2    } },
        { T::FWidth,                           { Ret::GenericArg0, 1    } },
        { T::GetRenderTargetSampleCount,       { Ret::UInt              } },
        { T::GetRenderTargetSamplePosition,    { Ret::Float2,      1    } },
        { T::GreaterThan,                      { Ret::Bool,        2    } }, // GLSL only
        { T::GreaterThanEqual,                 { Ret::Bool,        2    } }, // GLSL only
        { T::GroupMemoryBarrier,               {                        } },
        { T::GroupMemoryBarrierWithGroupSync,  {                        } },
        { T::InterlockedAdd,                   {                   2, 3 } },
        { T::InterlockedAnd,                   {                   2, 3 } },
        { T::InterlockedCompareExchange,       {                   4    } },
        { T::InterlockedCompareStore,          {                   3    } },
        { T::InterlockedExchange,              {                   3    } },
        { T::InterlockedMax,                   {                   2, 3 } },
        { T::InterlockedMin,                   {                   2, 3 } },
        { T::InterlockedOr,                    {                   2, 3 } },
        { T::InterlockedXor,                   {                   2, 3 } },
        { T::IsFinite,                         { Ret::GenericArg0, 1    } }, // bool with size as input
        { T::IsInf,                            { Ret::GenericArg0, 1    } }, // bool with size as input
        { T::IsNaN,                            { Ret::GenericArg0, 1    } }, // bool with size as input
        { T::LdExp,                            { Ret::GenericArg0, 2    } }, // float with size as input
        { T::Length,                           { Ret::Float,       1    } },
        { T::Lerp,                             { Ret::GenericArg0, 3    } },
        { T::LessThan,                         { Ret::Bool,        2    } }, // GLSL only
        { T::LessThanEqual,                    { Ret::Bool,        2    } }, // GLSL only
        { T::Lit,                              { Ret::GenericArg0, 3    } },
        { T::Log,                              { Ret::GenericArg0, 1    } },
        { T::Log10,                            { Ret::GenericArg0, 1    } },
        { T::Log2,                             { Ret::GenericArg0, 1    } },
        { T::MAD,                              { Ret::GenericArg0, 3    } },
        { T::Max,                              { Ret::GenericArg0, 2    } },
        { T::Min,                              { Ret::GenericArg0, 2    } },
        { T::ModF,                             { Ret::GenericArg0, 2    } },
        { T::MSAD4,                            { Ret::UInt4,       3    } },
      //{ T::Mul,                              {                        } }, // special case
        { T::Normalize,                        { Ret::GenericArg0, 1    } },
        { T::NotEqual,                         { Ret::Bool,        2    } }, // GLSL only
        { T::Pow,                              { Ret::GenericArg0, 2    } },
        { T::PrintF,                           {                  -1    } },
        { T::Process2DQuadTessFactorsAvg,      {                   5    } },
        { T::Process2DQuadTessFactorsMax,      {                   5    } },
        { T::Process2DQuadTessFactorsMin,      {                   5    } },
        { T::ProcessIsolineTessFactors,        {                   4    } },
        { T::ProcessQuadTessFactorsAvg,        {                   5    } },
        { T::ProcessQuadTessFactorsMax,        {                   5    } },
        { T::ProcessQuadTessFactorsMin,        {                   5    } },
        { T::ProcessTriTessFactorsAvg,         {                   5    } },
        { T::ProcessTriTessFactorsMax,         {                   5    } },
        { T::ProcessTriTessFactorsMin,         {                   5    } },
        { T::Radians,                          { Ret::GenericArg0, 1    } },
        { T::Rcp,                              { Ret::GenericArg0, 1    } },
        { T::Reflect,                          { Ret::GenericArg0, 2    } },
        { T::Refract,                          { Ret::GenericArg0, 3    } },
        { T::ReverseBits,                      { Ret::UInt,        1    } },
        { T::Round,                            { Ret::GenericArg0, 1    } },
        { T::RSqrt,                            { Ret::GenericArg0, 1    } },
        { T::Saturate,                         { Ret::GenericArg0, 1    } },
        { T::Sign,                             { Ret::GenericArg0, 1    } },
        { T::Sin,                              { Ret::GenericArg0, 1    } },
        { T::SinCos,                           {                   3    } },
        { T::SinH,                             { Ret::GenericArg0, 1    } },
        { T::SmoothStep,                       { Ret::GenericArg2, 3    } },
        { T::Sqrt,                             { Ret::GenericArg0, 1    } },
        { T::Step,                             { Ret::GenericArg0, 2    } },
        { T::Tan,                              { Ret::GenericArg0, 1    } },
        { T::TanH,                             { Ret::GenericArg0, 1    } },
        { T::Tex1D_2,                          { Ret::Float4,      2    } },
        { T::Tex1D_4,                          { Ret::Float4,      4    } },
        { T::Tex1DBias,                        { Ret::Float4,      2    } },
        { T::Tex1DGrad,                        { Ret::Float4,      4    } },
        { T::Tex1DLod,                         { Ret::Float4,      2    } },
        { T::Tex1DProj,                        { Ret::Float4,      2    } },
        { T::Tex2D_2,                          { Ret::Float4,      2    } },
        { T::Tex2D_4,                          { Ret::Float4,      4    } },
        { T::Tex2DBias,                        { Ret::Float4,      2    } },
        { T::Tex2DGrad,                        { Ret::Float4,      4    } },
        { T::Tex2DLod,                         { Ret::Float4,      2    } },
        { T::Tex2DProj,                        { Ret::Float4,      2    } },
        { T::Tex3D_2,                          { Ret::Float4,      2    } },
        { T::Tex3D_4,                          { Ret::Float4,      4    } },
        { T::Tex3DBias,                        { Ret::Float4,      2    } },
        { T::Tex3DGrad,                        { Ret::Float4,      4    } },
        { T::Tex3DLod,                         { Ret::Float4,      2    } },
        { T::Tex3DProj,                        { Ret::Float4,      2    } },
        { T::TexCube_2,                        { Ret::Float4,      2    } },
        { T::TexCube_4,                        { Ret::Float4,      4    } },
        { T::TexCubeBias,                      { Ret::Float4,      2    } },
        { T::TexCubeGrad,                      { Ret::Float4,      4    } },
        { T::TexCubeLod,                       { Ret::Float4,      2    } },
        { T::TexCubeProj,                      { Ret::Float4,      2    } },
      //{ T::Transpose,                        {                   } }, // special case
        { T::Trunc,                            { Ret::GenericArg0, 1    } },

        { T::Texture_GetDimensions,            {                   3    } },
        { T::Texture_Load_1,                   { Ret::Float4,      1    } },
        { T::Texture_Load_2,                   { Ret::Float4,      2    } },
        { T::Texture_Load_3,                   { Ret::Float4,      3    } },
        { T::Texture_Sample_2,                 { Ret::Float4,      2    } },
        { T::Texture_Sample_3,                 { Ret::Float4,      3    } },
        { T::Texture_Sample_4,                 { Ret::Float4,      4    } },
        { T::Texture_Sample_5,                 { Ret::Float4,      5    } },
        { T::Texture_SampleBias_3,             { Ret::Float4,      3    } },
        { T::Texture_SampleBias_4,             { Ret::Float4,      4    } },
        { T::Texture_SampleBias_5,             { Ret::Float4,      5    } },
        { T::Texture_SampleBias_6,             { Ret::Float4,      6    } },
        { T::Texture_SampleCmp_3,              { Ret::Float4,      3    } },
        { T::Texture_SampleCmp_4,              { Ret::Float4,      4    } },
        { T::Texture_SampleCmp_5,              { Ret::Float4,      5    } },
        { T::Texture_SampleCmp_6,              { Ret::Float4,      6    } },
        { T::Texture_SampleGrad_4,             { Ret::Float4,      4    } },
        { T::Texture_SampleGrad_5,             { Ret::Float4,      5    } },
        { T::Texture_SampleGrad_6,             { Ret::Float4,      6    } },
        { T::Texture_SampleGrad_7,             { Ret::Float4,      7    } },
        { T::Texture_SampleLevel_3,            { Ret::Float4,      3    } },
        { T::Texture_SampleLevel_4,            { Ret::Float4,      4    } },
        { T::Texture_SampleLevel_5,            { Ret::Float4,      5    } },
        { T::Texture_QueryLod,                 { Ret::Float,       2    } },
        { T::Texture_QueryLodUnclamped,        { Ret::Float,       2    } },

        { T::StreamOutput_Append,              {                   1    } },
        { T::StreamOutput_RestartStrip,        {                        } },

        { T::Image_Load,                       { Ret::Float4,      2    } },
        { T::Image_Store,                      {                   3    } },

        { T::Image_AtomicAdd,                  {                   2, 3 } },
        { T::Image_AtomicAnd,                  {                   2, 3 } },
        { T::Image_AtomicCompSwap,             {                   4    } },
        { T::Image_AtomicExchange,             {                   3    } },
        { T::Image_AtomicMax,                  {                   2, 3 } },
        { T::Image_AtomicMin,                  {                   2, 3 } },
        { T::Image_AtomicOr,                   {                   2, 3 } },
        { T::Image_AtomicXor,                  {                   2, 3 } },
    };
}

static const auto g_intrinsicSignatureMap = GenerateIntrinsicSignatureMap();


/* ----- HLSLIntrinsicAdept class ----- */

HLSLIntrinsicAdept::HLSLIntrinsicAdept()
{
    /* Initialize intrinsic identifiers */
    for (const auto& it : HLSLIntrinsicAdept::GetIntrinsicMap())
        SetIntrinsicIdent(it.second.intrinsic, it.first);

    /* Fill remaining identifiers (for overloaded intrinsics) */
    FillOverloadedIntrinsicIdents();
}

TypeDenoterPtr HLSLIntrinsicAdept::GetIntrinsicReturnType(const Intrinsic intrinsic, const std::vector<ExprPtr>& args) const
{
    switch (intrinsic)
    {
        case Intrinsic::Mul:
            return DeriveReturnTypeMul(args);
        case Intrinsic::Transpose:
            return DeriveReturnTypeTranspose(args);
        case Intrinsic::Equal:
        case Intrinsic::NotEqual:
        case Intrinsic::LessThan:
        case Intrinsic::LessThanEqual:
        case Intrinsic::GreaterThan:
        case Intrinsic::GreaterThanEqual:
            return DeriveReturnTypeVectorCompare(args);
        default:
            return DeriveReturnType(intrinsic, args);
    }
}

std::vector<TypeDenoterPtr> HLSLIntrinsicAdept::GetIntrinsicParameterTypes(const Intrinsic intrinsic, const std::vector<ExprPtr>& args) const
{
    std::vector<TypeDenoterPtr> paramTypeDenoters;

    switch (intrinsic)
    {
        case Intrinsic::Mul:
            DeriveParameterTypesMul(paramTypeDenoters, args);
            break;
        case Intrinsic::Transpose:
            DeriveParameterTypesTranspose(paramTypeDenoters, args);
            break;
        default:
            DeriveParameterTypes(paramTypeDenoters, intrinsic, args);
            break;
    }

    return paramTypeDenoters;
}

std::vector<std::size_t> HLSLIntrinsicAdept::GetIntrinsicOutputParameterIndices(const Intrinsic intrinsic) const
{
    switch (intrinsic)
    {
        // asuint(double value, out uint lowbits, out uint highbits)
        case Intrinsic::AsUInt_3:
            return { 1, 2 };

        // InterlockedAdd(R dest, T value, out T original_value)
        case Intrinsic::InterlockedAdd:
        case Intrinsic::InterlockedAnd:
        case Intrinsic::InterlockedExchange:
        case Intrinsic::InterlockedMax:
        case Intrinsic::InterlockedMin:
        case Intrinsic::InterlockedOr:
        case Intrinsic::InterlockedXor:
            return { 2 };

        // sincos(x, out s, out c)
        case Intrinsic::SinCos:
            return { 1, 2 };

        default:
            break;
    }
    return {};
}

const HLSLIntrinsicsMap& HLSLIntrinsicAdept::GetIntrinsicMap()
{
    static const HLSLIntrinsicsMap intrinsicMap = GenerateIntrinsicMap();
    return intrinsicMap;
}


/*
 * ======= Private: =======
 */

TypeDenoterPtr HLSLIntrinsicAdept::DeriveReturnType(const Intrinsic intrinsic, const std::vector<ExprPtr>& args) const
{
    /* Get type denoter from intrinsic signature map */
    auto it = g_intrinsicSignatureMap.find(intrinsic);
    if (it != g_intrinsicSignatureMap.end())
        return it->second.GetTypeDenoterWithArgs(args);
    else
        RuntimeErr(R_FailedToDeriveIntrinsicType(GetIntrinsicIdent(intrinsic)));
}

TypeDenoterPtr HLSLIntrinsicAdept::DeriveReturnTypeMul(const std::vector<ExprPtr>& args) const
{
    /* Validate number of arguments */
    if (args.size() != 2)
        RuntimeErr(R_InvalidIntrinsicArgCount("mul"));

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
            return std::make_shared<BaseTypeDenoter>(baseDataType0); // scalar
        }

        if (type1->IsMatrix())
        {
            auto dataType1      = static_cast<BaseTypeDenoter&>(*type1).dataType;
            auto baseDataType1  = BaseDataType(dataType1);
            auto matrixTypeDim1 = MatrixTypeDim(dataType1);
            return std::make_shared<BaseTypeDenoter>(VectorDataType(baseDataType1, matrixTypeDim1.second));
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
            return std::make_shared<BaseTypeDenoter>(VectorDataType(baseDataType0, matrixTypeDim0.first));
        }

        if (type1->IsMatrix())
        {
            auto dataType0      = static_cast<BaseTypeDenoter&>(*type0).dataType;
            auto baseDataType0  = BaseDataType(dataType0);
            auto matrixTypeDim0 = MatrixTypeDim(dataType0);
            auto dataType1      = static_cast<BaseTypeDenoter&>(*type1).dataType;
            auto matrixTypeDim1 = MatrixTypeDim(dataType1);
            return std::make_shared<BaseTypeDenoter>(MatrixDataType(baseDataType0, matrixTypeDim0.first, matrixTypeDim1.second));
        }
    }

    RuntimeErr(R_InvalidIntrinsicArgs("mul"));
}

TypeDenoterPtr HLSLIntrinsicAdept::DeriveReturnTypeTranspose(const std::vector<ExprPtr>& args) const
{
    /* Validate number of arguments */
    if (args.size() != 1)
        RuntimeErr(R_InvalidIntrinsicArgCount("transpose"));

    auto type0 = args[0]->GetTypeDenoter()->Get();

    if (type0->IsMatrix())
    {
        /* Convert MxN matrix type to NxM matrix type */
        auto dataType0      = static_cast<BaseTypeDenoter&>(*type0).dataType;
        auto baseDataType0  = BaseDataType(dataType0);
        auto matrixTypeDim0 = MatrixTypeDim(dataType0);
        return std::make_shared<BaseTypeDenoter>(MatrixDataType(baseDataType0, matrixTypeDim0.second, matrixTypeDim0.first));
    }

    RuntimeErr(R_InvalidIntrinsicArgs("transpose"));
}

TypeDenoterPtr HLSLIntrinsicAdept::DeriveReturnTypeVectorCompare(const std::vector<ExprPtr>& args) const
{
    /* Validate number of arguments */
    if (args.size() != 2)
        RuntimeErr(R_InvalidIntrinsicArgCount("vector-compare"));

    auto type0 = args[0]->GetTypeDenoter()->Get();

    if (auto baseType0 = type0->As<BaseTypeDenoter>())
    {
        const auto vecTypeSize = VectorTypeDim(baseType0->dataType);
        return std::make_shared<BaseTypeDenoter>(VectorDataType(DataType::Bool, vecTypeSize));
    }

    return type0;
}

/*
TODO:
This is a temporary solution to derive parameter types of intrinsic.
Currently all global intrinsics use a common type denoter for all parameters.
*/
void HLSLIntrinsicAdept::DeriveParameterTypes(std::vector<TypeDenoterPtr>& paramTypeDenoters, const Intrinsic intrinsic, const std::vector<ExprPtr>& args) const
{
    /* Get type denoter from intrinsic signature map */
    auto it = g_intrinsicSignatureMap.find(intrinsic);
    if (it != g_intrinsicSignatureMap.end())
    {
        if (!args.empty() && IsGlobalIntrinsic(intrinsic))
        {
            /* Find common type denoter for all arguments */
            auto commonTypeDenoter = DeriveCommonTypeDenoter(0, args);

            /* Add parameter type denoter */
            paramTypeDenoters.resize(args.size());
            for (std::size_t i = 0, n = args.size(); i < n; ++i)
                paramTypeDenoters[i] = commonTypeDenoter;
        }
    }
    else
        RuntimeErr(R_FailedToDeriveIntrinsicParamType(GetIntrinsicIdent(intrinsic)));
}

void HLSLIntrinsicAdept::DeriveParameterTypesMul(std::vector<TypeDenoterPtr>& paramTypeDenoters, const std::vector<ExprPtr>& args) const
{
    //TODO...
}

void HLSLIntrinsicAdept::DeriveParameterTypesTranspose(std::vector<TypeDenoterPtr>& paramTypeDenoters, const std::vector<ExprPtr>& args) const
{
    //TODO...
}


} // /namespace Xsc



// ================================================================================