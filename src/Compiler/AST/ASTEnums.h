/*
 * ASTEnums.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_AST_ENUMS_H
#define XSC_AST_ENUMS_H


#include <string>


namespace Xsc
{


class Token;

/* ----- AssignOp Enum ----- */

// Assignment operator enumeration:
// =, +=, -=, *=, /=, %=, <<=, >>=, |= , &=, ^=
enum class AssignOp
{
    Undefined,

    Set,    // =
    Add,    // +=
    Sub,    // -=
    Mul,    // *=
    Div,    // /=
    Mod,    // %=
    LShift, // <<=
    RShift, // >>=
    Or,     // |=
    And,    // &=
    Xor,    // ^=
};

std::string AssignOpToString(const AssignOp o);
AssignOp StringToAssignOp(const std::string& s);

bool IsBitwiseOp(const AssignOp o);


/* ----- BinaryOp Enum ----- */

// Binary operator enumeration:
// &&, ||, |, ^, &, <<, >>, +, -, *, /, %, ==, !=, <, >, <=, >=
enum class BinaryOp
{
    Undefined,

    LogicalAnd,     // &&
    LogicalOr,      // ||
    Or,             // |
    Xor,            // ^
    And,            // &
    LShift,         // <<
    RShift,         // >>
    Add,            // +
    Sub,            // -
    Mul,            // *
    Div,            // /
    Mod,            // %
    Equal,          // ==
    NotEqual,       // !=
    Less,           // <
    Greater,        // >
    LessEqual,      // <=
    GreaterEqual,   // >=
};

std::string BinaryOpToString(const BinaryOp o);
BinaryOp StringToBinaryOp(const std::string& s);

bool IsBitwiseOp(const BinaryOp o);


/* ----- UnaryOp Enum ----- */

// Unary operator enumeration:
// !, ~, +, -, ++, --
enum class UnaryOp
{
    Undefined,

    LogicalNot, // Logical not (e.g. !x)
    Not,        // Bitwise not (e.g. ~x)
    Nop,        // No-op (e.g. +x is equal to x)
    Negate,     // Negate (e.g. -x)
    Inc,        // Increment (e.g. ++x)
    Dec,        // Decrement (e.g. --x)
};

std::string UnaryOpToString(const UnaryOp o);
UnaryOp StringToUnaryOp(const std::string& s);

bool IsBitwiseOp(const UnaryOp o);


/* ----- CtrlTransfer Enum ----- */

// Control transfer enumeration:
// break, continue, discard
enum class CtrlTransfer
{
    Undefined,

    Break,
    Continue,
    Discard,
};

std::string CtrlTransformToString(const CtrlTransfer ct);
CtrlTransfer StringToCtrlTransfer(const std::string& s);


/* ----- DataType Enum ----- */

// Base data type enumeration.
enum class DataType
{
    Undefined,

    // String types,
    String,

    // Scalar types
    Bool,
    Int,
    UInt,
    Half,
    Float,
    Double,
    
    // Vector types
    Bool2,
    Bool3,
    Bool4,
    Int2,
    Int3,
    Int4,
    UInt2,
    UInt3,
    UInt4,
    Half2,
    Half3,
    Half4,
    Float2,
    Float3,
    Float4,
    Double2,
    Double3,
    Double4,

    // Matrix types
    Bool2x2,
    Bool2x3,
    Bool2x4,
    Bool3x2,
    Bool3x3,
    Bool3x4,
    Bool4x2,
    Bool4x3,
    Bool4x4,
    Int2x2,
    Int2x3,
    Int2x4,
    Int3x2,
    Int3x3,
    Int3x4,
    Int4x2,
    Int4x3,
    Int4x4,
    UInt2x2,
    UInt2x3,
    UInt2x4,
    UInt3x2,
    UInt3x3,
    UInt3x4,
    UInt4x2,
    UInt4x3,
    UInt4x4,
    Half2x2,
    Half2x3,
    Half2x4,
    Half3x2,
    Half3x3,
    Half3x4,
    Half4x2,
    Half4x3,
    Half4x4,
    Float2x2,
    Float2x3,
    Float2x4,
    Float3x2,
    Float3x3,
    Float3x4,
    Float4x2,
    Float4x3,
    Float4x4,
    Double2x2,
    Double2x3,
    Double2x4,
    Double3x2,
    Double3x3,
    Double3x4,
    Double4x2,
    Double4x3,
    Double4x4,
};

// Returns a descriptive string of the specified data type.
std::string DataTypeToString(const DataType t, bool useTemplateSyntax = false);

// Returns true if the specified data type is a scalar type.
bool IsScalarType(const DataType t);

// Returns true if the specified data type is a vector type.
bool IsVectorType(const DataType t);

// Returns true if the specified data type is a matrix type.
bool IsMatrixType(const DataType t);

// Returns true if the specified data type is a boolean type (i.e. bool, and all vectors and matrices of these).
bool IsBooleanType(const DataType t);

// Returns true if the specified data type is a real type (i.e. half, float, double, and all vectors and matrices of these).
bool IsRealType(const DataType t);

// Returns true if the specified data type is an integral type (i.e. int, uint, and all vectors and matrices of these).
bool IsIntegralType(const DataType t);

// Returns true if the specified data type is an integer type (i.e. int, and all vectors and matrices of these).
bool IsIntType(const DataType t);

// Returns true if the specified data type is an unsigned-integer type (i.e. uint, and all vectors and matrices of these).
bool IsUIntType(const DataType t);

/*
Returns the dimension of the specified data type interpreted as vector type.
Values range from 1 to 4, but 0 for matrix types).
*/
int VectorTypeDim(const DataType t);

/*
Returns the dimensions MxN of the specified data type interpreted as matrix type.
Values range from 1x1 to 4x4, but 1x1 to 4x1 for vector and scalar types).
*/
std::pair<int, int> MatrixTypeDim(const DataType t);

// Returns the base data type for the specified type or DataType::Undefined on failure.
DataType BaseDataType(const DataType t);

// Returns the vector data type for the specified type and vector size.
DataType VectorDataType(const DataType baseDataType, int vectorSize);

// Returns the matrix data type for the specified type, rows, and columns.
DataType MatrixDataType(const DataType baseDataType, int rows, int columns);

// Returns the data type for the specified swizzle operator or throws and std::invalid_argument on failure.
DataType SubscriptDataType(const DataType dataType, const std::string& subscript);

// Returns the data type for the specified literal token (BoolLiteral, IntLiteral, FloatLiteral, and StringLiteral).
DataType TokenToDataType(const Token& tkn);

// Returns the data type as non-double (i.e. replaces doubles by floats).
DataType DoubleToFloatDataType(const DataType dataType);


/* ----- StorageClass Enum ----- */

/*
Variable storage class enumeration.
This also contains the interpolation modifier for simplicity.
*/
enum class StorageClass
{
    Undefined,

    // Storage classes
    Extern,
    Precise,
    Shared,
    GroupShared,
    Static,
    Uniform,
    Volatile,

    // Interpolation modifiers
    NoInterpolation,
    Linear,
    Centroid,
    NoPerspective,
    Sample,
};

// Returns true if the specified storage class is actually an interpolation modifier.
bool IsInterpolationModifier(const StorageClass s);


/* ----- UniformBufferType Enum ----- */

enum class UniformBufferType
{
    Undefined,

    ConstantBuffer, // Constant buffer ("cbuffer" in HLSL).
    TextureBuffer,  // Texture buffer ("tbuffer" in HLSL).
};


/* ----- TextureType Enum ----- */

// Buffer (and texture) object type enumeration.
enum class BufferType
{
    Undefined,

    Buffer,
    StucturedBuffer,
    ByteAddressBuffer,

    RWBuffer,
    RWStucturedBuffer,
    RWByteAddressBuffer,
    AppendStructuredBuffer,
    ConsumeStructuredBuffer,

    RWTexture1D,
    RWTexture1DArray,
    RWTexture2D,
    RWTexture2DArray,
    RWTexture3D,

    Texture1D,
    Texture1DArray,
    Texture2D,
    Texture2DArray,
    Texture3D,
    TextureCube,
    TextureCubeArray,
    Texture2DMS,
    Texture2DMSArray,

    GenericTexture,             // Texture of unspecified dimension (used in DX9 effect files: "texture" keyword).
};

// Returns true if the specified buffer type is a storage buffer type (e.g. BufferType::Buffer, or BufferType::RWStructuredBuffer).
bool IsStorageBufferType(const BufferType t);

// Returns true if the specified buffer type is a RW (read/write) buffer type.
bool IsRWBufferType(const BufferType t);

// Returns true if the specified buffer type is a texture buffer.
bool IsTextureBufferType(const BufferType t);


/* ----- SamplerType Enum ----- */

// Sampler type enumeration.
enum class SamplerType
{
    Undefined,

    Sampler,                // 'sampler' in D3D9
    Sampler1D,              // 'sampler1D' in D3D9
    Sampler2D,              // 'sampler2D' in D3D9
    Sampler3D,              // 'sampler3D' in D3D9
    SamplerCube,            // 'samplerCUBE' in D3D9
    Sampler_State,          // 'sampler_state' in D3D9, e.g. "sampler texSampler = sampler_state { Texture = <texObject>; MinFilter = LINEAR; };"
    SamplerState,           // 'SamplerState' in D3D10+, e.g. "SamplerState texSampler { Filter = MIN_MAG_MIP_LINEAR; };"
    SamplerComparisonState, // 'SamplerComparisonState' in D3D10+
};

// Returns true if the specified sampler type is a type from D3D9.
bool IsD3D9SamplerType(const SamplerType t);

// Returns true if the specified sampler type is a type from D3D10+.
bool IsD3D10SamplerType(const SamplerType t);


/* ----- RegisterType Enum ----- */

// Register type enumeration.
enum class RegisterType
{
    Undefined,

    ConstantBuffer,         // 'b'-register
    TextureBuffer,          // 't'-register
    BufferOffset,           // 'c'-register
    Sampler,                // 's'-register
    UnorderedAccessView,    // 'u'-register
};

// Returns the register type for the specified register character (e.g. 'b' for ConstantBuffer).
RegisterType CharToRegisterType(char c);

// Returns the respective register character for the specified register type (e.g. 'b' for ConstantBuffer).
char RegisterTypeToChar(const RegisterType t);

// Returns a descriptive string for the specified register type.
std::string RegisterTypeToString(const RegisterType t);


/* ----- Intrinsic Enum ----- */

/*
Intrinsics function enumeration (currently only HLSL intrinsics).
see https://msdn.microsoft.com/en-us/library/windows/desktop/ff471376(v=vs.85).aspx
*/
enum class Intrinsic
{
    Undefined,

    Abort,
    Abs,
    ACos,
    All,
    AllMemoryBarrier,
    AllMemoryBarrierWithGroupSync,
    Any,
    AsDouble,
    AsFloat,
    ASin,
    AsInt,
    AsUInt,
    AsUInt_2,
    ATan,
    ATan2,
    Ceil,
    CheckAccessFullyMapped,
    Clamp,
    Clip,
    Cos,
    CosH,
    CountBits,
    Cross,
    D3DCOLORtoUBYTE4,
    DDX,
    DDXCoarse,
    DDXFine,
    DDY,
    DDYCoarse,
    DDYFine,
    Degrees,
    Determinant,
    DeviceMemoryBarrier,
    DeviceMemoryBarrierWithGroupSync,
    Distance,
    Dot,
    Dst,
    ErrorF,
    EvaluateAttributeAtCentroid,
    EvaluateAttributeAtSample,
    EvaluateAttributeSnapped,
    Exp,
    Exp2,
    F16toF32,
    F32toF16,
    FaceForward,
    FirstBitHigh,
    FirstBitLow,
    Floor,
    FMA,
    FMod,
    Frac,
    FrExp,
    FWidth,
    GetRenderTargetSampleCount,
    GetRenderTargetSamplePosition,
    GroupMemoryBarrier,
    GroupMemoryBarrierWithGroupSync,
    InterlockedAdd,
    InterlockedAnd,
    InterlockedCompareExchange,
    InterlockedCompareStore,
    InterlockedExchange,
    InterlockedMax,
    InterlockedMin,
    InterlockedOr,
    InterlockedXor,
    IsFinite,
    IsInf,
    IsNaN,
    LdExp,
    Length,
    Lerp,
    Lit,
    Log,
    Log10,
    Log2,
    MAD,
    Max,
    Min,
    ModF,
    MSAD4,
    Mul,
    Noise,
    Normalize,
    Pow,
    PrintF,
    Process2DQuadTessFactorsAvg,
    Process2DQuadTessFactorsMax,
    Process2DQuadTessFactorsMin,
    ProcessIsolineTessFactors,
    ProcessQuadTessFactorsAvg,
    ProcessQuadTessFactorsMax,
    ProcessQuadTessFactorsMin,
    ProcessTriTessFactorsAvg,
    ProcessTriTessFactorsMax,
    ProcessTriTessFactorsMin,
    Radians,
    Rcp,
    Reflect,
    Refract,
    ReverseBits,
    Round,
    RSqrt,
    Saturate,
    Sign,
    Sin,
    SinCos,
    SinH,
    SmoothStep,
    Sqrt,
    Step,
    Tan,
    TanH,
    Tex1D,
    Tex1D_2,
    Tex1DBias,
    Tex1DGrad,
    Tex1DLod,
    Tex1DProj,
    Tex2D,
    Tex2D_2,
    Tex2DBias,
    Tex2DGrad,
    Tex2DLod,
    Tex2DProj,
    Tex3D,
    Tex3D_2,
    Tex3DBias,
    Tex3DGrad,
    Tex3DLod,
    Tex3DProj,
    TexCube,
    TexCube_2,
    TexCubeBias,
    TexCubeGrad,
    TexCubeLod,
    TexCubeProj,
    Transpose,
    Trunc,
};


/* ----- Semantic Enum ----- */

// Semantic enumeration (vertex input is omitted).
enum class Semantic
{
    Undefined,

    UserDefined,            // User defined HLSL semantic

    ClipDistance,           // SV_ClipDistance, gl_ClipDistance
    CullDistance,           // SV_CullDistance, gl_CullDistance (if ARB_cull_distance is present)
    Coverage,               // SV_Coverage, gl_SampleMask
    Depth,                  // DEPTH (D3D9), SV_Depth (D3D10+), gl_FragDepth
    DepthGreaterEqual,      // SV_DepthGreaterEqual, layout(depth_greater) out float gl_FragDepth;
    DepthLessEqual,         // SV_DepthLessEqual, layout(depth_less) out float gl_FragDepth;
    DispatchThreadID,       // SV_DispatchThreadID, gl_GlobalInvocationID
    DomainLocation,         // SV_DomainLocation, gl_TessCord
    GroupID,                // SV_GroupID, gl_WorkGroupID
    GroupIndex,             // SV_GroupIndex, N/A
    GroupThreadID,          // SV_GroupThreadID, gl_LocalInvocationID
    GSInstanceID,           // SV_GSInstanceID, gl_InvocationID
    InnerCoverage,          // SV_InnerCoverage, gl_SampleMaskIn
    InsideTessFactor,       // SV_InsideTessFactor, gl_TessLevelInner
    InstanceID,             // SV_InstanceID, gl_InstanceID (GLSL), gl_InstanceIndex (Vulkan)
    IsFrontFace,            // VFACE (D3D9), SV_IsFrontFace (D3D10+), gl_FrontFacing
    OutputControlPointID,   // SV_OutputControlPointID, gl_InvocationID
    Position,               // VPOS (D3D9), SV_Position (D3D10+), gl_FragCoord
    PrimitiveID,            // SV_PrimitiveID, gl_PrimitiveID
    RenderTargetArrayIndex, // SV_RenderTargetArrayIndex, gl_Layer
    SampleIndex,            // SV_SampleIndex, gl_SampleID
    StencilRef,             // SV_StencilRef, gl_FragStencilRef (if ARB_shader_stencil_export is present)
    Target,                 // COLOR (D3D9), SV_Target (D3D10+), gl_FragData
    TessFactor,             // SV_TessFactor, gl_TessLevelOuter
    VertexID,               // SV_VertexID, gl_VertexID (GLSL), gl_VertexIndex (Vulkan)
    VertexPosition,         // POSITION (D3D9), SV_Position (D3D10+), gl_Position
    ViewportArrayIndex,     // SV_ViewportArrayIndex, gl_ViewportIndex
};

// Indexed semantic type with 'Semantic' enum, integral index, and implicit conversion from and to 'Semantic' enum.
class IndexedSemantic
{

    public:

        IndexedSemantic() = default;
        IndexedSemantic(const IndexedSemantic&) = default;
        IndexedSemantic& operator = (const IndexedSemantic&) = default;

        inline IndexedSemantic(Semantic semantic, int index = 0) :
            semantic_   { semantic },
            index_      { index    }
        {
        }

        inline operator Semantic() const
        {
            return semantic_;
        }

        // Returns the semantic index.
        inline int Index() const
        {
            return index_;
        }

    private:

        Semantic    semantic_   = Semantic::Undefined;
        int         index_      = 0;

};

// Returns true if the specified semantic is a system value semantic.
bool IsSystemSemantic(const Semantic t);

// Returns true if the specified semantic is a user defined semantic.
bool IsUserSemantic(const Semantic t);


} // /namespace Xsc


#endif



// ================================================================================