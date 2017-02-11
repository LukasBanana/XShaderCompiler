/*
 * ASTEnums.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_AST_ENUMS_H
#define XSC_AST_ENUMS_H


#include <Xsc/Reflection.h>
#include <string>
#include <vector>
#include <set>


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

bool IsLogicalOp(const BinaryOp o);
bool IsBitwiseOp(const BinaryOp o);
bool IsBooleanOp(const BinaryOp o);


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

bool IsLogicalOp(const UnaryOp o);
bool IsBitwiseOp(const UnaryOp o);

// Returns true if the specified unary operator is only for l-values (e.g. ++x or --x).
bool IsLValueOp(const UnaryOp o);


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

// Returns true if the specified data type is a half-precision real type (i.e. half, half2, half4x4 etc.).
bool IsHalfRealType(const DataType t);

// Returns true if the specified data type is a double-precision real type (i.e. double, double2, double4x4 etc.).
bool IsDoubleRealType(const DataType t);

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


/* ----- PrimitiveType Enum ----- */

// Primitive type enumeration.
enum class PrimitiveType
{
    Undefined,

    Point,          // Point
    Line,           // Line
    LineAdj,        // Line adjacency
    Triangle,       // Triangle
    TriangleAdj,    // Triangle adjacency
};


/* ----- StorageClass Enum ----- */

// Variable storage class enumeration.
enum class StorageClass
{
    Undefined,

    Extern,
    Precise,
    Shared,
    GroupShared,
    Static,
    Volatile,
};


/* ----- InterpModifier Enum ----- */

// Variable interpolation modifier enumeration.
enum class InterpModifier
{
    Undefined,

    NoInterpolation,
    Linear,
    Centroid,
    NoPerspective,
    Sample,
};


/* ----- StorageClass Enum ----- */

// Variable type modifier enumeration.
enum class TypeModifier
{
    Undefined,

    Const,
    RowMajor,
    ColumnMajor,

    SNorm,
    UNorm,
};


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

    /* --- Storage Buffers --- */
    Buffer,
    StructuredBuffer,
    ByteAddressBuffer,

    RWBuffer,
    RWStructuredBuffer,
    RWByteAddressBuffer,
    AppendStructuredBuffer,
    ConsumeStructuredBuffer,

    /* --- Textures --- */
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

    /* --- Patches --- */
    InputPatch,
    OutputPatch,

    /* --- Streams --- */
    PointStream,
    LineStream,
    TriangleStream,
};

// Returns true if the specified buffer type is a storage buffer type (e.g. BufferType::Buffer, or BufferType::RWStructuredBuffer).
bool IsStorageBufferType(const BufferType t);

// Returns true if the specified buffer type is a RW (read/write) buffer type.
bool IsRWBufferType(const BufferType t);

// Returns true if the specified buffer type is a texture buffer.
bool IsTextureBufferType(const BufferType t);

// Returns true if the specified buffer type is a multi-sampled texture buffer (i.e. Texture2DMS or Texture2DMSArray).
bool IsTextureMSBufferType(const BufferType t);

// Returns true if the specified buffer type is an input or output patch.
bool IsPatchBufferType(const BufferType t);

// Returns true if the specified buffer type is either a point-, line-, or triangle stream.
bool IsStreamBufferType(const BufferType t);


/* ----- SamplerType Enum ----- */

// Sampler type enumeration.
enum class SamplerType
{
    Undefined,

    Sampler1D,              // 'sampler1D' in D3D9
    Sampler2D,              // 'sampler2D' in D3D9
    Sampler3D,              // 'sampler3D' in D3D9
    SamplerCube,            // 'samplerCUBE' in D3D9
    SamplerState,           // 'SamplerState' in D3D10+, or 'sampler_state' in D3D9
    SamplerComparisonState, // 'SamplerComparisonState' in D3D10+
};

// Returns true if the specified sampler type is sampler state (i.e. SamplerState or SamplerComparisonState).
bool IsSamplerStateType(const SamplerType t);


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


/* ----- AttributeType Enum ----- */

// Attribute type enumeration (TODO: incomplete list).
enum class AttributeType
{
    Undefined,

    Branch,
    Call,
    Flatten,
    IfAll,
    IfAny,
    Isolate,
    Loop,
    MaxExports,
    MaxInstructionCount,
    MaxTempReg,
    NoExpressionOptimizations,
    Predicate,
    PredicateBlock,
    ReduceTempRegUsage,
    RemoveUnusedInputs,
    SampReg,
    Unroll,
    Unused,
    Xps,

    Domain,
    EarlyDepthStencil,
    Instance,
    MaxTessFactor,
    MaxVertexCount,
    NumThreads,
    OutputControlPoints,
    OutputTopology,
    Partitioning,
    PatchSize,
    PatchConstantFunc,
};

// Returns true if the specified attribute is supported since shader model 3.
bool IsShaderModel3AttributeType(const AttributeType t);

// Returns true if the specified attribute is supported since shader model 5.
bool IsShaderModel5AttributeType(const AttributeType t);


/* ----- AttributeValue Enum ----- */

// Value enumeration of required attributes (e.g. domain types for tess.-control shader).
enum class AttributeValue
{
    Undefined,

    DomainTri,
    DomainQuad,
    DomainIsoline,

    OutputTopologyPoint,
    OutputTopologyLine,
    OutputTopologyTriangleCW,
    OutputTopologyTriangleCCW,

    PartitioningInteger,
    PartitioningPow2,
    PartitioningFractionalEven,
    PartitioningFractionalOdd,
};

// Returns true if the specified attribute value belongs to the 'domain' attribute.
bool IsAttributeValueDomain(const AttributeValue t);

// Returns true if the specified attribute value belongs to the 'outputtopology' attribute.
bool IsAttributeValueOutputTopology(const AttributeValue t);

// Returns true if the specified attribute value belongs to the 'partitioning' attribute.
bool IsAttributeValuePartitioning(const AttributeValue t);

bool IsAttributeValueTrianglePartitioning(const AttributeValue t);


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
    AsUInt_1,
    AsUInt_3,
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
    Tex1D_2,
    Tex1D_4,
    Tex1DBias,
    Tex1DGrad,
    Tex1DLod,
    Tex1DProj,
    Tex2D_2,
    Tex2D_4,
    Tex2DBias,
    Tex2DGrad,
    Tex2DLod,
    Tex2DProj,
    Tex3D_2,
    Tex3D_4,
    Tex3DBias,
    Tex3DGrad,
    Tex3DLod,
    Tex3DProj,
    TexCube_2,
    TexCube_4,
    TexCubeBias,
    TexCubeGrad,
    TexCubeLod,
    TexCubeProj,
    Transpose,
    Trunc,

    Texture_GetDimensions,
    Texture_Load_1,             // Load(int[1,2,3,4] Location)
    Texture_Load_2,             // Load(int[1,2,3,4] Location, int SampleIndex)
    Texture_Load_3,             // Load(int[1,2,3,4] Location, int SampleIndex, int Offset)
    //Texture_Gather_3,           // Gather(SamplerState S, float[1,2,3,4] Location, int[1,2,3] Offset)
    //Texture_Gather_4,           // Gather(SamplerState S, float[1,2,3,4] Location, int[1,2,3] Offset, out uint Status)
    Texture_Sample_2,           // Sample(SamplerState S, float[1,2,3,4] Location)
    Texture_Sample_3,           // Sample(SamplerState S, float[1,2,3,4] Location, int[1,2,3] Offset)
    Texture_Sample_4,           // Sample(SamplerState S, float[1,2,3,4] Location, int[1,2,3] Offset, float Clamp)
    Texture_Sample_5,           // Sample(SamplerState S, float[1,2,3,4] Location, int[1,2,3] Offset, float Clamp, out uint Status)
    Texture_SampleBias_3,
    Texture_SampleBias_4,
    Texture_SampleBias_5,
    Texture_SampleBias_6,
    Texture_SampleCmp_3,
    Texture_SampleCmp_4,
    Texture_SampleCmp_5,
    Texture_SampleCmp_6,
    Texture_SampleGrad_4,
    Texture_SampleGrad_5,
    Texture_SampleGrad_6,
    Texture_SampleGrad_7,
    Texture_SampleLevel_3,      // SampleLevel(SamplerState S, float[1,2,3,4] Location, float LOD)
    Texture_SampleLevel_4,      // SampleLevel(SamplerState S, float[1,2,3,4] Location, float LOD, int[1,2,3] Offset)
    Texture_SampleLevel_5,      // SampleLevel(SamplerState S, float[1,2,3,4] Location, float LOD, int[1,2,3] Offset, out uint Status)

    StreamOutput_Append,        // Append(StreamDataType)
    StreamOutput_RestartStrip,  // RestartStrip()
};

// Container structure for all kinds of intrinsic call usages (can be used as std::map<Intrinsic, IntrinsicUsage>
struct IntrinsicUsage
{
    struct ArgumentList
    {
        std::vector<DataType> argTypes;

        inline bool operator < (const ArgumentList& rhs) const
        {
            return (argTypes < rhs.argTypes);
        }
    };

    // Set of all argument lists that where used for an intrinsic.
    std::set<ArgumentList> argLists;
};

// Returns true if the specified intrinsic is a global intrinsic.
bool IsGlobalIntrinsic(const Intrinsic t);

// Returns true if the speciifed intrinsic belongs to a texture object.
bool IsTextureIntrinsic(const Intrinsic t);

// Returns true if the speciifed intrinsic belongs to a stream-output object.
bool IsStreamOutputIntrinsic(const Intrinsic t);


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
    DomainLocation,         // SV_DomainLocation, gl_TessCoord
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

        IndexedSemantic(Semantic semantic, int index = 0);
        IndexedSemantic(const std::string& userDefined, int index = 0);

        inline operator Semantic() const
        {
            return semantic_;
        }

        // Returns true if this the semantic is not undefined (Semantic::Undefined).
        bool IsValid() const;

        // see Xsc::IsSystemSemantic
        bool IsSystemValue() const;

        // see Xsc::IsUserSemantic
        bool IsUserDefined() const;

        std::string ToString() const;

        // Returns the semantic index.
        inline int Index() const
        {
            return index_;
        }

    private:

        Semantic    semantic_   = Semantic::Undefined;
        int         index_      = 0;
        std::string userDefined_;

};

// Returns true if the specified semantic is a system value semantic.
bool IsSystemSemantic(const Semantic t);

// Returns true if the specified semantic is a user defined semantic.
bool IsUserSemantic(const Semantic t);

// Returns the specified semantic as string.
std::string SemanticToString(const Semantic t);


/* ----- Reflection::Filter Enum ----- */

std::string FilterToString(const Reflection::Filter t);
Reflection::Filter StringToFilter(const std::string& s);


/* ----- Reflection::TextureAddressMode Enum ----- */

std::string TexAddressModeToString(const Reflection::TextureAddressMode t);
Reflection::TextureAddressMode StringToTexAddressMode(const std::string& s);


/* ----- Reflection::ComparisonFunc Enum ----- */

std::string CompareFuncToString(const Reflection::ComparisonFunc t);
Reflection::ComparisonFunc StringToCompareFunc(const std::string& s);


} // /namespace Xsc


#endif



// ================================================================================