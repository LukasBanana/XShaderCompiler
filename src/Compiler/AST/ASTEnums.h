/*
 * ASTEnums.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2019 by Lukas Hermanns)
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

// Converts the specified assignment operator to an equivalent binary operator, or returns BinaryOp::Undefined if no conversion is possible.
BinaryOp AssignOpToBinaryOp(const AssignOp op);

// Returns true if the specified binary operator is a logical operator (&&, ||).
bool IsLogicalOp(const BinaryOp o);

// Returns true if the specified binary operator is a bitwise operator (|, ^, &, <<, >>).
bool IsBitwiseOp(const BinaryOp o);

// Returns true if the specified binary operator is a comparison operator (==, !=, <, >, <=, >=).
bool IsCompareOp(const BinaryOp o);

// Returns true if the specified binary operator is a boolean operator, i.e. either logical or compare operator.
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

    PostInc,    // Post increment (e.g. x++)
    PostDec,    // Post decrement (e.g. x--)
};

std::string UnaryOpToString(const UnaryOp o, bool postUnaryExpr);
UnaryOp StringToUnaryOp(const std::string& s, bool postUnaryExpr);

bool IsLogicalOp(const UnaryOp o);
bool IsBitwiseOp(const UnaryOp o);

// Returns true if the specified unary operator is a post increment or decremenet operator (i.e. UnaryOp::PostInc or UnaryOp::PostDec).
bool IsPostUnaryOp(const UnaryOp o);

// Returns true if the specified unary operator is a write operation (i.e. ++x, --x, x++, and x--).
bool IsWriteOp(const UnaryOp o);

// Returns true if an expression with the specified unary operator can be used as l-value (i.e. ++x and --x).
bool IsLvalueOp(const UnaryOp o);


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

    /* --- String types --- */
                // HLSL         GLSL         Metal
                // -----------  -----------  -----------
    String,     // string       n/a          n/a

    /* --- Scalar types --- */
                // HLSL         GLSL         Metal
                // -----------  -----------  -----------
    Bool,       // bool         bool         bool
    Int,        // int          int          int
    UInt,       // uint         uint         uint
    Half,       // half         n/a          half
    Float,      // float        float        float
    Double,     // double       double       n/a

    /* --- Vector types --- */
                // HLSL         GLSL         Metal
                // -----------  -----------  -----------
    Bool2,      // bool2        bvec2        bool2
    Bool3,      // bool3        bvec3        bool3
    Bool4,      // bool4        bvec4        bool4
    Int2,       // int2         ivec2        int2
    Int3,       // int3         ivec3        int3
    Int4,       // int4         ivec4        int4
    UInt2,      // uint2        uvec2        uint2
    UInt3,      // uint3        uvec3        uint3
    UInt4,      // uint4        uvec4        uint4
    Half2,      // half2        n/a          half2
    Half3,      // half3        n/a          half3
    Half4,      // half4        n/a          half4
    Float2,     // float2       vec2         float2
    Float3,     // float3       vec3         float3
    Float4,     // float4       vec4         float4
    Double2,    // double2      dvec2        n/a
    Double3,    // double3      dvec3        n/a
    Double4,    // double4      dvec4        n/a

    /* --- Matrix types --- */
                // HLSL         GLSL         Metal
                // -----------  -----------  -----------
    Bool2x2,    // bool2x2      n/a          bool2x2
    Bool2x3,    // bool2x3      n/a          bool2x3
    Bool2x4,    // bool2x4      n/a          bool2x4
    Bool3x2,    // bool3x2      n/a          bool3x2
    Bool3x3,    // bool3x3      n/a          bool3x3
    Bool3x4,    // bool3x4      n/a          bool3x4
    Bool4x2,    // bool4x2      n/a          bool4x2
    Bool4x3,    // bool4x3      n/a          bool4x3
    Bool4x4,    // bool4x4      n/a          bool4x4
    Int2x2,     // int2x2       n/a          int2x2
    Int2x3,     // int2x3       n/a          int2x3
    Int2x4,     // int2x4       n/a          int2x4
    Int3x2,     // int3x2       n/a          int3x3
    Int3x3,     // int3x3       n/a          int3x3
    Int3x4,     // int3x4       n/a          int3x4
    Int4x2,     // int4x2       n/a          int4x2
    Int4x3,     // int4x3       n/a          int4x3
    Int4x4,     // int4x4       n/a          int4x4
    UInt2x2,    // uint2x2      n/a          uint2x2
    UInt2x3,    // uint2x3      n/a          uint2x3
    UInt2x4,    // uint2x4      n/a          uint2x4
    UInt3x2,    // uint3x2      n/a          uint3x2
    UInt3x3,    // uint3x3      n/a          uint3x3
    UInt3x4,    // uint3x4      n/a          uint3x4
    UInt4x2,    // uint4x2      n/a          uint4x2
    UInt4x3,    // uint4x3      n/a          uint4x3
    UInt4x4,    // uint4x4      n/a          uint4x4
    Half2x2,    // half2x2      n/a          half2x2
    Half2x3,    // half2x3      n/a          half2x3
    Half2x4,    // half2x4      n/a          half2x4
    Half3x2,    // half3x2      n/a          half3x2
    Half3x3,    // half3x3      n/a          half3x3
    Half3x4,    // half3x4      n/a          half3x4
    Half4x2,    // half4x2      n/a          half4x2
    Half4x3,    // half4x3      n/a          half4x3
    Half4x4,    // half4x4      n/a          half4x4
    Float2x2,   // float2x2     mat2         float2x2
    Float2x3,   // float2x3     mat2x3       float2x3
    Float2x4,   // float2x4     mat2x4       float2x4
    Float3x2,   // float3x2     mat3x2       float3x2
    Float3x3,   // float3x3     mat3         float3x3
    Float3x4,   // float3x4     mat3x4       float3x4
    Float4x2,   // float4x2     mat4x2       float4x2
    Float4x3,   // float4x3     mat4x3       float4x3
    Float4x4,   // float4x4     mat4         float4x4
    Double2x2,  // double2x2    dmat2        n/a
    Double2x3,  // double2x3    dmat2x3      n/a
    Double2x4,  // double2x4    dmat2x4      n/a
    Double3x2,  // double3x2    dmat3x2      n/a
    Double3x3,  // double3x3    dmat3        n/a
    Double3x4,  // double3x4    dmat3x4      n/a
    Double4x2,  // double4x2    dmat4x2      n/a
    Double4x3,  // double4x3    dmat4x3      n/a
    Double4x4,  // double4x4    dmat4        n/a
};

// Container structure for all kinds of matrix subscript usages.
struct MatrixSubscriptUsage
{
    MatrixSubscriptUsage() = default;
    MatrixSubscriptUsage(const DataType dataTypeIn, const std::string& subscript);

    // Strict-weak-order (SWP) comparison.
    bool operator < (const MatrixSubscriptUsage& rhs) const;

    // Returns the indices to a unique string.
    std::string IndicesToString() const;

    std::vector<std::pair<int, int>>    indices;
    DataType                            dataTypeIn  = DataType::Undefined;
    DataType                            dataTypeOut = DataType::Undefined;
};

// Returns a descriptive string of the specified data type.
std::string DataTypeToString(const DataType t, bool useTemplateSyntax = false);

// Returns the size (in bytes) of the specified data type, or 0 if the data type is invalid, undefined, or equal to DataType::String.
unsigned int DataTypeSize(const DataType t);

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

// Returns true if the specified data type is a single-precision real type (i.e. float, float2, float4x4 etc.).
bool IsSingleRealType(const DataType t);

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
DataType SubscriptDataType(const DataType dataType, const std::string& subscript, std::vector<std::pair<int, int>>* indices = nullptr);

// Returns the data type for the specified literal token (BoolLiteral, IntLiteral, FloatLiteral, and StringLiteral).
DataType TokenToDataType(const Token& tkn);

// Returns the data type as non-double (i.e. replaces doubles by floats).
DataType DoubleToFloatDataType(const DataType dataType);

// Returns the remaining size (in bytes) of a vector slot with the specified alignment.
unsigned int RemainingVectorSize(unsigned int vectorSize, unsigned int alignment = 16u);

// Accumulates the vector size for the specified data type (with a 16 byte boundary), and returns true on success.
bool AccumAlignedVectorSize(const DataType dataType, unsigned int& size, unsigned int& padding, unsigned int* offset = nullptr);


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

// Variable storage class enumeration (also Metal address space, e.g. "constant", "device").
enum class StorageClass
{
    Undefined,

                    // HLSL         GLSL         Metal
                    // -----------  -----------  -----------
    Extern,         // extern       n/a          extern
    Precise,        // precise      n/a          n/a
    Shared,         // shared       ???          ???
    GroupShared,    // groupshared  shared       threadgroup
    Static,         // static       n/a          static
    Volatile,       // volatile     n/a          volatile

    // Metal only
    Constant,       // n/a          n/a          constant
    Device,         // n/a          n/a          device
};


/* ----- InterpModifier Enum ----- */

// Variable interpolation modifier enumeration.
enum class InterpModifier
{
    Undefined,

                        // HLSL             GLSL             Metal
                        // ---------------  ---------------  ---------------
    Centroid,           // centroid         centroid         centroid_perspective
    Linear,             // linear           smooth           center_perspective
    NoInterpolation,    // nointerpolation  flat             flat
    NoPerspective,      // noperspective    noperspective    center_no_perspective
    Sample,             // sample           sample           sample_perspective
};


/* ----- TypeModifier Enum ----- */

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

    ConstantBuffer, // Constant buffer ("cbuffer" in HLSL; "uniform" in GLSL).
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

    GenericTexture,                 // Texture of unspecified dimension (used in DX9 effect files: "texture" keyword).

    /* --- Patches --- */
    InputPatch,
    OutputPatch,

    /* --- Streams --- */
    PointStream,
    LineStream,
    TriangleStream,

    /* --- Generic Buffers --- */
    GenericBuffer,                  // GLSL "buffer"
};

// Converts the specified BufferType enumeration entry into a string.
std::string BufferTypeToString(const BufferType t);

// Returns true if the specified buffer type is a storage buffer type (i.e. gets converted to a 'buffer' block in GLSL).
bool IsStorageBufferType(const BufferType t);

// Returns true if the specified buffer type is a RW (read/write) buffer type (for storage buffers and textures).
bool IsRWBufferType(const BufferType t);

// Returns true if the specified buffer type is a texture buffer.
bool IsTextureBufferType(const BufferType t);

// Returns true if the specified buffer type is a multi-sampled texture buffer (i.e. Texture2DMS or Texture2DMSArray).
bool IsTextureMSBufferType(const BufferType t);

// Returns true if the specified buffer type is an image buffer (i.e. gets converted to an 'imageBuffer' in GLSL).
bool IsImageBufferType(const BufferType t);

// Returns true if the specified buffer type is a RW (read/write) texture buffer type (represented in GLSL with 'image...').
bool IsRWImageBufferType(const BufferType t);

// Returns true if the specified buffer type is an input or output patch.
bool IsPatchBufferType(const BufferType t);

// Returns true if the specified buffer type is either a point-, line-, or triangle stream.
bool IsStreamBufferType(const BufferType t);

// Returns the texture dimension of the specified buffer type in the range [1, 4] or 0 if the type is not a texture type.
int GetBufferTypeTextureDim(const BufferType t);


/* ----- SamplerType Enum ----- */

// Sampler type enumeration.
enum class SamplerType
{
    Undefined,

    /* --- Samplers --- */
                            // HLSL3            HLSL4+                  GLSL                    Metal
                            // ---------------  ----------------------  ----------------------  -----------------
    Sampler1D,              // sampler1D        n/a                     sampler1D               texture1d
    Sampler2D,              // sampler2D        n/a                     sampler2D               texture2d
    Sampler3D,              // sampler3D        n/a                     sampler3D               texture3d
    SamplerCube,            // samplerCUBE      n/a                     samplerCube             texturecube
    Sampler2DRect,          // n/a              n/a                     sampler2DRect           texture2d
    Sampler1DArray,         // n/a              n/a                     sampler1DArray          texture1d_array
    Sampler2DArray,         // n/a              n/a                     sampler2DArray          texture2d_array
    SamplerCubeArray,       // n/a              n/a                     samplerCubeArray        texturecube_array
    SamplerBuffer,          // n/a              n/a                     samplerBuffer           texture_buffer
    Sampler2DMS,            // n/a              n/a                     sampler2DMS             texture2d_ms
    Sampler2DMSArray,       // n/a              n/a                     sampler2DMSArray        n/a
    Sampler1DShadow,        // sampler1DShadow  n/a                     sampler1DShadow         n/a
    Sampler2DShadow,        // sampler2DShadow  n/a                     sampler2DShadow         depth2d
    SamplerCubeShadow,      // n/a              n/a                     samplerCubeShadow       depthcube
    Sampler2DRectShadow,    // n/a              n/a                     sampler2DRectShadow     depth2d
    Sampler1DArrayShadow,   // n/a              n/a                     sampler1DArrayShadow    n/a
    Sampler2DArrayShadow,   // n/a              n/a                     sampler2DArrayShadow    depth2d_array
    SamplerCubeArrayShadow, // n/a              n/a                     samplerCubeArrayShadow  depthcube_array

    /* --- Sampler states --- */
                            // HLSL3            HLSL4+                  GLSL                    Metal
                            // ---------------  ----------------------  ----------------------  -----------------
    SamplerState,           // sampler_state    SamplerState            sampler                 sampler
    SamplerComparisonState, // sampler_state    SamplerComparisonState  samplerShadow           sampler
};

// Returns true if the specified sampler type is sampler state (i.e. SamplerState or SamplerComparisonState).
bool IsSamplerStateType(const SamplerType t);

// Returns true if the specified sampler type is a shadow sampler (e.g. Sampler1DShadow).
bool IsSamplerTypeShadow(const SamplerType t);

// Returns true if the specified sampler type is an array sampler (e.g. Sampler1DArray).
bool IsSamplerTypeArray(const SamplerType t);

// Returns the texture dimension of the specified sampler type in the range [1, 4] or 0 if the type is not a texture sampler (e.g. a sampler state).
int GetSamplerTypeTextureDim(const SamplerType t);

// Maps a texture type to an appropriate sampler type.
SamplerType TextureTypeToSamplerType(const BufferType t);

// Converts a non-shadow sampler variant into a shadow one, if possible.
SamplerType SamplerTypeToShadowSamplerType(const SamplerType t);


/* ----- ImageLayoutFormat Enum ----- */

// Image layout format enumeration.
enum class ImageLayoutFormat
{
    Undefined,

    /* --- Float formats --- */
    F32X4,          // rgba32f
    F32X2,          // rg32f
    F32X1,          // r32f
    F16X4,          // rgba16f
    F16X2,          // rg16f
    F16X1,          // r16f
    F11R11G10B,     // r11f_g11f_b10f

    /* --- Unsigned normalized formats --- */
    UN32X4,         // rgba16
    UN16X2,         // rg16
    UN16X1,         // r16
    UN10R10G10B2A,  // rgb10_a2
    UN8X4,          // rgba8
    UN8X2,          // rg8
    UN8X1,          // r8

    /* --- Signed normalized formats --- */
    SN16X4,         // rgba16_snorm
    SN16X2,         // rg16_snorm
    SN16X1,         // r16_snorm
    SN8X4,          // rgba8_snorm
    SN8X2,          // rg8_snorm
    SN8X1,          // r8_snorm

    /* --- Signed integer formats --- */
    I32X4,          // rgba32i
    I32X2,          // rg32i
    I32X1,          // r32i
    I16X4,          // rgba16i
    I16X2,          // rg16i
    I16X1,          // r16i
    I8X4,           // rgba8i
    I8X2,           // rg8i
    I8X1,           // r8i

    /* --- Unsigned integer formats --- */
    UI32X4,         // rgba32ui
    UI32X2,         // rg32ui
    UI32X1,         // r32ui
    UI16X4,         // rgba16ui
    UI16X2,         // rg16ui
    UI16X1,         // r16ui
    UI10R10G10B2A,  // rgb10_a2ui
    UI8X4,          // rgba8ui
    UI8X2,          // rg8ui
    UI8X1,          // r8ui
};

// Returns the base type of a single component in the specified image layout format.
DataType GetImageLayoutFormatBaseType(const ImageLayoutFormat format);

// Returns the image layout format for the specified data type or ImageLayoutFormat::Undefined.
ImageLayoutFormat DataTypeToImageLayoutFormat(const DataType t);


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

    /* --- HLSL 3 attributes --- */
    Branch,                     // [branch]
    Call,                       // [call]
    Flatten,                    // [flatten]
    IfAll,                      // [ifAll]
    IfAny,                      // [ifAny]
    Isolate,                    // [isolate]
    Loop,                       // [loop]
    MaxExports,                 // [maxexports(N)]
    MaxInstructionCount,        // [maxInstructionCount(N)]
    MaxTempReg,                 // [maxtempreg(N)]
    NoExpressionOptimizations,  // [noExpressionOptimizations]
    Predicate,                  // [predicate]
    PredicateBlock,             // [predicateBlock]
    ReduceTempRegUsage,         // [reduceTempRegUsage(N)]
    RemoveUnusedInputs,         // [removeUnusedInputs]
    SampReg,                    // [sampreg(N, M)]
    Unroll,                     // [unroll(MaxIterationCount)]
    Unused,                     // [unused]
    Xps,                        // [xps]

    /* --- HLSL 4+ attributes --- */
    Domain,                     // [domain(STRING)]
    EarlyDepthStencil,          // [earlydepthstencil]
    Instance,                   // [instance(N)]
    MaxTessFactor,              // [maxtessfactor(X)]
    MaxVertexCount,             // [maxvertexcount(N)]
    NumThreads,                 // [numthreads(X, Y, Z)]
    OutputControlPoints,        // [outputcontrolpoints(N)]
    OutputTopology,             // [outputtopology(STRING)]
    Partitioning,               // [partitioning(STRING)]
    PatchSize,                  // [patchsize(N)]
    PatchConstantFunc,          // [patchconstantfunc(STRING)]

    /* --- GLSL Layout Attributes --- */
    Align,                      // layout(align = <EXPR>)
    Binding,                    // layout(binding = <EXPR>)
    CW,                         // layout(cw)
    CCW,                        // layout(ccw)
    ColumnMajor,                // layout(column_major)
    Component,                  // layout(component = <EXPR>)
    DepthAny,                   // layout(depth_any)
    DepthGreater,               // layout(depth_greater)
    DepthLess,                  // layout(depth_less)
    DepthUnchanged,             // layout(depth_unchanged)
    EarlyFragmentTests,         // layout(early_fragment_tests)
    EqualSpacing,               // layout(equal_spacing)
    FractionalEvenSpacing,      // layout(fractional_even_spacing)
    FractionalOddSpacing,       // layout(fractional_odd_spacing)
    Index,                      // layout(index = <EXPR>)
    Invocations,                // layout(invocations = <EXPR>)
    Isolines,                   // layout(isolines)
    Lines,                      // layout(lines)
    LinesAdjacency,             // layout(lines_adjacency)
    LineStrip,                  // layout(line_strip)
    LocalSizeX,                 // layout(local_size_x = <EXPR>)
    LocalSizeY,                 // layout(local_size_y = <EXPR>)
    LocalSizeZ,                 // layout(local_size_z = <EXPR>)
    Location,                   // layout(location = <EXPR>)
    MaxVertices,                // layout(max_vertices = <EXPR>)
    OriginUpperLeft,            // layout(origin_upper_left)
    Offset,                     // layout(offset = <EXPR>)
    Packed,                     // layout(packed)
    PixelCenterInteger,         // layout(pixel_center_integer)
    Points,                     // layout(points)
    PointMode,                  // layout(point_mode)
    Quads,                      // layout(quads)
    RowMajor,                   // layout(row_major)
    Shared,                     // layout(shared)
    Std140,                     // layout(std140)
    Std430,                     // layout(std430)
    Stream,                     // layout(stream = <EXPR>)
    Triangles,                  // layout(triangles)
    TrianglesAdjacency,         // layout(triangles_adjacency)
    TriangleStrip,              // layout(triangle_strip)
    Vertices,                   // layout(vertices = <EXPR>)
    XfbBuffer,                  // layout(xfb_buffer = <EXPR>)
    XfbOffset,                  // layout(xfb_offset = <EXPR>)
    XfbStride,                  // layout(xfb_stride = <EXPR>)

    #ifdef XSC_ENABLE_LANGUAGE_EXT

    /* --- Extended Attributes --- */
    Space,                      // Extended attribute to specify a vector space.
    Layout,                     // Extended attribute to specify a layout format.

    #endif
};

// Returns true if the specified attribute is supported since shader model 3.
bool IsShaderModel3AttributeType(const AttributeType t);

// Returns true if the specified attribute is supported since shader model 5.
bool IsShaderModel5AttributeType(const AttributeType t);

// Returns true if the specified attribute is supported only in HLSL, e.g. [ATTRIBUTE].
bool IsHLSLAttributeType(const AttributeType t);

// Returns true if the specified attribute is supported only in GLSL, e.g. layout(ATTRIBUTE).
bool IsGLSLAttributeType(const AttributeType t);

#ifdef XSC_ENABLE_LANGUAGE_EXT

// Returns true if the specified attribute is an extended attribute, i.e. AttributeType::Space, AttributeType::Layout.
bool IsExtAttributeType(const AttributeType t);

#endif


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

// Returns true if the specified attribute value is a triangle 'partitioning' attribute, i.e. either OutputTopologyTriangleCW or OutputTopologyTriangleCCW.
bool IsAttributeValueTrianglePartitioning(const AttributeValue t);


/* ----- Intrinsic Enum ----- */

/*
Intrinsics function enumeration (currently only HLSL intrinsics).
see https://msdn.microsoft.com/en-us/library/windows/desktop/ff471376(v=vs.85).aspx
*/
enum class Intrinsic
{
    Undefined,

    /* --- Common global intrinsics --- */
                                        // HLSL                              GLSL                   Metal
                                        // --------------------------------  ---------------------  -------------------------
    Abort,                              // abort                             n/a                    n/a
    Abs,                                // abs                               abs                    abs
    ACos,                               // acos                              acos                   acos
    All,                                // all                               all                    n/a
    AllMemoryBarrier,                   // AllMemoryBarrier                  memoryBarrier          ?
    AllMemoryBarrierWithGroupSync,      // AllMemoryBarrierWithGroupSync     n/a                    ?
    Any,                                // any                               any                    n/a
    AsDouble,                           // asdouble                          uint64BitsToDouble     n/a
    AsFloat,                            // asfloat                           uintBitsToFloat        as_type<float>
    ASin,                               // asin                              asin                   asin
    AsInt,                              // asint                             floatBitsToInt         as_type<int>
    AsUInt_1,                           // asuint                            floatBitsToUint        as_type<uint>
    AsUInt_3,                           // asuint                            n/a                    ?
    ATan,                               // atan                              atan                   atan
    ATan2,                              // atan2                             atan                   atan2
    Ceil,                               // ceil                              ceil                   ceil
    CheckAccessFullyMapped,             // CheckAccessFullyMapped            n/a                    n/a
    Clamp,                              // clamp                             clamp                  clamp
    Clip,                               // clip                              n/a                    n/a
    Cos,                                // cos                               cos                    cos
    CosH,                               // cosh                              cosh                   cosh
    CountBits,                          // countbits                         bitCount               ?
    Cross,                              // cross                             cross                  cross
    D3DCOLORtoUBYTE4,                   // D3DCOLORtoUBYTE4                  n/a                    n/a
    DDX,                                // ddx                               dFdx                   dfdx
    DDXCoarse,                          // ddx_coarse                        dFdxCoarse             n/a
    DDXFine,                            // ddx_fine                          dFdxFine               n/a
    DDY,                                // ddy                               dFdy                   dfdy
    DDYCoarse,                          // ddy_coarse                        dFdyCoarse             n/a
    DDYFine,                            // ddy_fine                          dFdyFine               n/a
    Degrees,                            // degrees                           degrees                n/a
    Determinant,                        // determinant                       determinant            determinant
    DeviceMemoryBarrier,                // DeviceMemoryBarrier               n/a                    ?
    DeviceMemoryBarrierWithGroupSync,   // DeviceMemoryBarrierWithGroupSync  n/a                    ?
    Distance,                           // distance                          distance               distance
    Dot,                                // dot                               dot                    dot
    Dst,                                // dst                               n/a                    n/a
    Equal,                              // n/a                               equal                  n/a
    ErrorF,                             // errorf                            n/a                    n/a
    EvaluateAttributeAtCentroid,        // EvaluateAttributeAtCentroid       interpolateAtCentroid  ?
    EvaluateAttributeAtSample,          // EvaluateAttributeAtSample         interpolateAtSample    ?
    EvaluateAttributeSnapped,           // EvaluateAttributeSnapped          interpolateAtOffset    ?
    Exp,                                // exp                               exp                    exp
    Exp2,                               // exp2                              exp2                   exp2
    F16toF32,                           // f16tof32                          n/a                    as_type<float>
    F32toF16,                           // f32tof16                          n/a                    as_type<half>
    FaceForward,                        // faceforward                       faceforward            faceforward
    FirstBitHigh,                       // firstbithigh                      findMSB                ?
    FirstBitLow,                        // firstbitlow                       findLSB                ?
    Floor,                              // floor                             floor                  floor
    FMA,                                // fma                               fma                    fma
    FMod,                               // fmod                              mod                    fmod
    Frac,                               // frac                              fract                  fract
    FrExp,                              // frexp                             frexp                  frexp
    FWidth,                             // fwidth                            fwidth                 fwidth
    GetRenderTargetSampleCount,         // GetRenderTargetSampleCount        n/a                    n/a
    GetRenderTargetSamplePosition,      // GetRenderTargetSamplePosition     n/a                    n/a
    GreaterThan,                        // n/a                               greaterThan            n/a
    GreaterThanEqual,                   // n/a                               greaterThanEqual       n/a
    GroupMemoryBarrier,                 // GroupMemoryBarrier                groupMemoryBarrier     ?
    GroupMemoryBarrierWithGroupSync,    // GroupMemoryBarrierWithGroupSync   n/a                    ?
    InterlockedAdd,                     // InterlockedAdd                    atomicAdd              atomic_fetch_add_explicit
    InterlockedAnd,                     // InterlockedAnd                    atomicAnd              atomic_fetch_and_explicit
    InterlockedCompareExchange,         // InterlockedCompareExchange        atomicCompSwap         ?
    InterlockedCompareStore,            // InterlockedCompareStore           n/a                    ?
    InterlockedExchange,                // InterlockedExchange               atomicExchange         atomic_exchange_explicit
    InterlockedMax,                     // InterlockedMax                    atomicMax              atomic_fetch_max_explicit
    InterlockedMin,                     // InterlockedMin                    atomicMin              atomic_fetch_min_explicit
    InterlockedOr,                      // InterlockedOr                     atomicOr               atomic_fetch_or_explicit
    InterlockedXor,                     // InterlockedXor                    atomicXor              atomic_fetch_xor_explicit
    IsFinite,                           // isfinite                          n/a                    isfinite
    IsInf,                              // isinf                             isinf                  isinf
    IsNaN,                              // isnan                             isnan                  isnan
    LdExp,                              // ldexp                             ldexp                  ldexp
    Length,                             // length                            length                 length
    Lerp,                               // lerp                              mix                    mix
    LessThan,                           // n/a                               lessThan               n/a
    LessThanEqual,                      // n/a                               lessThanEqual          n/a
    Lit,                                // lit                               n/a                    n/a
    Log,                                // log                               log                    log
    Log10,                              // log10                             n/a                    log10
    Log2,                               // log2                              log2                   log2
    MAD,                                // mad                               n/a                    n/a
    Max,                                // max                               max                    max
    Min,                                // min                               min                    min
    ModF,                               // modf                              modf                   modf
    MSAD4,                              // msad4                             n/a                    n/a
    Mul,                                // mul                               n/a                    n/a
    Normalize,                          // normalize                         normalize              normalize
    NotEqual,                           // n/a                               notEqual               n/a
    Not,                                // n/a                               not                    n/a
    Pow,                                // pow                               pow                    pow
    PrintF,                             // printf                            n/a                    n/a
    Process2DQuadTessFactorsAvg,        // Process2DQuadTessFactorsAvg       n/a                    n/a
    Process2DQuadTessFactorsMax,        // Process2DQuadTessFactorsMax       n/a                    n/a
    Process2DQuadTessFactorsMin,        // Process2DQuadTessFactorsMin       n/a                    n/a
    ProcessIsolineTessFactors,          // ProcessIsolineTessFactors         n/a                    n/a
    ProcessQuadTessFactorsAvg,          // ProcessQuadTessFactorsAvg         n/a                    n/a
    ProcessQuadTessFactorsMax,          // ProcessQuadTessFactorsMax         n/a                    n/a
    ProcessQuadTessFactorsMin,          // ProcessQuadTessFactorsMin         n/a                    n/a
    ProcessTriTessFactorsAvg,           // ProcessTriTessFactorsAvg          n/a                    n/a
    ProcessTriTessFactorsMax,           // ProcessTriTessFactorsMax          n/a                    n/a
    ProcessTriTessFactorsMin,           // ProcessTriTessFactorsMin          n/a                    n/a
    Radians,                            // radians                           radians                n/a
    Rcp,                                // rcp                               n/a                    n/a
    Reflect,                            // reflect                           reflect                reflect
    Refract,                            // refract                           refract                refract
    ReverseBits,                        // reversebits                       n/a                    ?
    Round,                              // round                             round                  round
    RSqrt,                              // rsqrt                             inversesqrt            rsqrt
    Saturate,                           // saturate                          n/a                    saturate
    Sign,                               // sign                              sign                   sign
    Sin,                                // sin                               sin                    sin
    SinCos,                             // sincos                            n/a                    sincos
    SinH,                               // sinh                              sinh                   sinh
    SmoothStep,                         // smoothstep                        smoothstep             smoothstep
    Sqrt,                               // sqrt                              sqrt                   sqrt
    Step,                               // step                              step                   step
    Tan,                                // tan                               tan                    tan
    TanH,                               // tanh                              tanh                   tanh
    Transpose,                          // transpose                         transpose              transpose
    Trunc,                              // trunc                             trunc                  trunc

    /* --- HLSL 3 texture intrinsics --- */
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

    /* --- HLSL 4+ class intrinsics --- */
    Texture_GetDimensions,
    Texture_QueryLod,           // CalculateLevelOfDetail(SamplerState S, float[1,2,3] Location)
    Texture_QueryLodUnclamped,  // CalculateLevelOfDetailUnclamped(SamplerState S, float[1,2,3] Location)

    Texture_Load_1,             // Load(int[1,2,3,4] Location)
    Texture_Load_2,             // Load(int[1,2,3,4] Location, int SampleIndex)
    Texture_Load_3,             // Load(int[1,2,3,4] Location, int SampleIndex, int Offset)

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
    Texture_SampleCmpLevelZero_3,
    Texture_SampleCmpLevelZero_4,
    Texture_SampleCmpLevelZero_5,
    Texture_SampleGrad_4,
    Texture_SampleGrad_5,
    Texture_SampleGrad_6,
    Texture_SampleGrad_7,
    Texture_SampleLevel_3,      // SampleLevel(SamplerState S, float[1,2,3,4] Location, float LOD)
    Texture_SampleLevel_4,      // SampleLevel(SamplerState S, float[1,2,3,4] Location, float LOD, int[1,2,3] Offset)
    Texture_SampleLevel_5,      // SampleLevel(SamplerState S, float[1,2,3,4] Location, float LOD, int[1,2,3] Offset, out uint Status)

    Texture_Gather_2,           // Gather(SamplerState S, float[2,3,4] Location)
    Texture_GatherRed_2,        // GatherRed(SamplerState S, float[2,3,4] Location)
    Texture_GatherGreen_2,      // GatherGreen(SamplerState S, float[2,3,4] Location)
    Texture_GatherBlue_2,       // GatherBlue(SamplerState S, float[2,3,4] Location)
    Texture_GatherAlpha_2,      // GatherAlpha(SamplerState S, float[2,3,4] Location)

    Texture_Gather_3,           // Gather(SamplerState S, float[2,3] Location, int2 Offset)
    Texture_Gather_4,           // Gather(SamplerState S, float[2,3] Location, int2 Offset, out uint Status)
    Texture_GatherRed_3,        // GatherRed(SamplerState S, float[2,3] Location, int2 Offset)
    Texture_GatherRed_4,        // GatherRed(SamplerState S, float[2,3] Location, int2 Offset, out uint Status)
    Texture_GatherGreen_3,      // GatherGreen(SamplerState S, float[2,3] Location, int2 Offset)
    Texture_GatherGreen_4,      // GatherGreen(SamplerState S, float[2,3] Location, int2 Offset, out uint Status)
    Texture_GatherBlue_3,       // GatherBlue(SamplerState S, float[2,3] Location, int2 Offset)
    Texture_GatherBlue_4,       // GatherBlue(SamplerState S, float[2,3] Location, int2 Offset, out uint Status)
    Texture_GatherAlpha_3,      // GatherAlpha(SamplerState S, float[2,3] Location, int2 Offset)
    Texture_GatherAlpha_4,      // GatherAlpha(SamplerState S, float[2,3] Location, int2 Offset, out uint Status)

    Texture_GatherRed_6,        // GatherRed(SamplerState S, float[2,3] Location, int2 Offset1, int2 Offset2, int2 Offset3, int2 Offset4)
    Texture_GatherRed_7,        // GatherRed(SamplerState S, float[2,3] Location, int2 Offset1, int2 Offset2, int2 Offset3, int2 Offset4, out uint Status)
    Texture_GatherGreen_6,      // GatherGreen(SamplerState S, float[2,3] Location, int2 Offset1, int2 Offset2, int2 Offset3, int2 Offset4)
    Texture_GatherGreen_7,      // GatherGreen(SamplerState S, float[2,3] Location, int2 Offset1, int2 Offset2, int2 Offset3, int2 Offset4, out uint Status)
    Texture_GatherBlue_6,       // GatherBlue(SamplerState S, float[2,3] Location, int2 Offset1, int2 Offset2, int2 Offset3, int2 Offset4)
    Texture_GatherBlue_7,       // GatherBlue(SamplerState S, float[2,3] Location, int2 Offset1, int2 Offset2, int2 Offset3, int2 Offset4, out uint Status)
    Texture_GatherAlpha_6,      // GatherAlpha(SamplerState S, float[2,3] Location, int2 Offset1, int2 Offset2, int2 Offset3, int2 Offset4)
    Texture_GatherAlpha_7,      // GatherAlpha(SamplerState S, float[2,3] Location, int2 Offset1, int2 Offset2, int2 Offset3, int2 Offset4, out uint Status)

    Texture_GatherCmp_3,        // GatherCmp(SamplerComparisonState S, float[2,3,4] Location, float CompareValue)
    Texture_GatherCmpRed_3,     // GatherCmpRed(SamplerComparisonState S, float[2,3,4] Location, float CompareValue)
    Texture_GatherCmpGreen_3,   // GatherCmpGreen(SamplerComparisonState S, float[2,3,4] Location, float CompareValue)
    Texture_GatherCmpBlue_3,    // GatherCmpBlue(SamplerComparisonState S, float[2,3,4] Location, float CompareValue)
    Texture_GatherCmpAlpha_3,   // GatherCmpAlpha(SamplerComparisonState S, float[2,3,4] Location, float CompareValue)

    Texture_GatherCmp_4,        // GatherCmp(SamplerComparisonState S, float[2,3] Location, float CompareValue, int2 Offset)
    Texture_GatherCmp_5,        // GatherCmp(SamplerComparisonState S, float[2,3] Location, float CompareValue, int2 Offset, out uint Status)
    Texture_GatherCmpRed_4,     // GatherCmpRed(SamplerComparisonState S, float[2,3] Location, float CompareValue, int2 Offset)
    Texture_GatherCmpRed_5,     // GatherCmpRed(SamplerComparisonState S, float[2,3] Location, float CompareValue, int2 Offset, out uint Status)
    Texture_GatherCmpGreen_4,   // GatherCmpGreen(SamplerComparisonState S, float[2,3] Location, float CompareValue, int2 Offset)
    Texture_GatherCmpGreen_5,   // GatherCmpGreen(SamplerComparisonState S, float[2,3] Location, float CompareValue, int2 Offset, out uint Status)
    Texture_GatherCmpBlue_4,    // GatherCmpBlue(SamplerComparisonState S, float[2,3] Location, float CompareValue, int2 Offset)
    Texture_GatherCmpBlue_5,    // GatherCmpBlue(SamplerComparisonState S, float[2,3] Location, float CompareValue, int2 Offset, out uint Status)
    Texture_GatherCmpAlpha_4,   // GatherCmpAlpha(SamplerComparisonState S, float[2,3] Location, float CompareValue, int2 Offset)
    Texture_GatherCmpAlpha_5,   // GatherCmpAlpha(SamplerComparisonState S, float[2,3] Location, float CompareValue, int2 Offset, out uint Status)

    Texture_GatherCmpRed_7,     // GatherCmpRed(SamplerComparisonState S, float[2,3] Location, float CompareValue, int2 Offset1, int2 Offset2, int2 Offset3, int2 Offset4)
    Texture_GatherCmpRed_8,     // GatherCmpRed(SamplerComparisonState S, float[2,3] Location, float CompareValue, int2 Offset1, int2 Offset2, int2 Offset3, int2 Offset4, out uint Status)
    Texture_GatherCmpGreen_7,   // GatherCmpGreen(SamplerComparisonState S, float[2,3] Location, float CompareValue, int2 Offset1, int2 Offset2, int2 Offset3, int2 Offset4)
    Texture_GatherCmpGreen_8,   // GatherCmpGreen(SamplerComparisonState S, float[2,3] Location, float CompareValue, int2 Offset1, int2 Offset2, int2 Offset3, int2 Offset4, out uint Status)
    Texture_GatherCmpBlue_7,    // GatherCmpBlue(SamplerComparisonState S, float[2,3] Location, float CompareValue, int2 Offset1, int2 Offset2, int2 Offset3, int2 Offset4)
    Texture_GatherCmpBlue_8,    // GatherCmpBlue(SamplerComparisonState S, float[2,3] Location, float CompareValue, int2 Offset1, int2 Offset2, int2 Offset3, int2 Offset4, out uint Status)
    Texture_GatherCmpAlpha_7,   // GatherCmpAlpha(SamplerComparisonState S, float[2,3] Location, float CompareValue, int2 Offset1, int2 Offset2, int2 Offset3, int2 Offset4)
    Texture_GatherCmpAlpha_8,   // GatherCmpAlpha(SamplerComparisonState S, float[2,3] Location, float CompareValue, int2 Offset1, int2 Offset2, int2 Offset3, int2 Offset4, out uint Status)

    StreamOutput_Append,        // Append(StreamDataType)
    StreamOutput_RestartStrip,  // RestartStrip()

    /* --- GLSL only intrinsics --- */
    Image_Load,                 // imageLoad(gimage Image, T Location)
    Image_Store,                // imageStore(gimage Image, T Location, G Data)
    Image_AtomicAdd,            // atomicAdd(inout T Memory, T Data)
    Image_AtomicAnd,            // atomicAnd(inout T Memory, T Data)
    Image_AtomicOr,             // atomicOr(inout T Memory, T Data)
    Image_AtomicXor,            // atomicXor(inout T Memory, T Data)
    Image_AtomicMin,            // atomicMin(inout T Memory, T Data)
    Image_AtomicMax,            // atomicMax(inout T Memory, T Data)
    Image_AtomicCompSwap,       // atomicCompSwap(inout T Memory, T compare, T Data)
    Image_AtomicExchange,       // atomicExchange(inout T Memory, T Data)

    PackHalf2x16,               // packHalf2x16(vec2 Vec)
};

// Container structure for all kinds of intrinsic call usages (can be used as std::map<Intrinsic, IntrinsicUsage>).
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

// Returns true if the specified intrinsic belongs to a texture object.
bool IsTextureIntrinsic(const Intrinsic t);

// Returns true if the specified intrinsic is a texture gather intrinsic.
bool IsTextureGatherIntrisic(const Intrinsic t);

// Returns true if the specified intrinsic is a texture sample intrinsic.
bool IsTextureSampleIntrinsic(const Intrinsic t);

// Returns true if the specified intrinsic is a texture sample or gather intrisic, with a compare operation.
bool IsTextureCompareIntrinsic(const Intrinsic t);

// Returns true if the specified intrinsic is a texture sample compare intrinsic that only samples the first mip level.
bool IsTextureCompareLevelZeroIntrinsic(const Intrinsic t);

// Returns true if the specified intrinsic is a texture load intrisic (e.g. Texture_Load1).
bool IsTextureLoadIntrinsic(const Intrinsic t);

// Returns true if the specified intrinsic belongs to a stream-output object.
bool IsStreamOutputIntrinsic(const Intrinsic t);

// Returns true if the specified intrinsic is an image load/store intrinsic.
bool IsImageIntrinsic(const Intrinsic t);

// Returns true if the specified intrinsic in an interlocked intrinsic (e.g. Intrinsic::InterlockedAdd).
bool IsInterlockedIntristic(const Intrinsic t);

// Returns the respective intrinsic for the specified binary compare operator, or Intrinsic::Undefined if the operator is not a compare operator.
Intrinsic CompareOpToIntrinsic(const BinaryOp op);

/*
Returns the respecitve image atomic intrinsic for the specified interlocked intrinsic,
or the input intrinsic, if is is not an interlocked intrinsic (e.g. Intrinsic::InterlockedAdd to Intrinsic::Image_AtomicAdd).
*/
Intrinsic InterlockedToImageAtomicIntrinsic(const Intrinsic t);

// Returns the number of offset parameters accepted by the specified gather intrinsic.
int GetGatherIntrinsicOffsetParamCount(const Intrinsic t);

// Maps a texture gather intrinsic to a component index (e.g. red -> 0, green -> 1, etc.)
int GetGatherIntrinsicComponentIndex(const Intrinsic t);


/* ----- Semantic Enum ----- */

// Semantic enumeration (vertex input is omitted).
enum class Semantic
{
    Undefined,

    UserDefined,            // User defined HLSL semantic

                            // HLSL3     HLSL4+                     GLSL                                   Metal
                            // --------  -------------------------  -------------------------------------  ----------------------------
    ClipDistance,           // n/a       SV_ClipDistance            gl_ClipDistance                        clip_distance
    CullDistance,           // n/a       SV_CullDistance            gl_CullDistance                        n/a
    Coverage,               // n/a       SV_Coverage                gl_SampleMask                          sample_mask
    Depth,                  // DEPTH     SV_Depth                   gl_FragDepth                           depth(any)
    DepthGreaterEqual,      // n/a       SV_DepthGreaterEqual       gl_FragDepth w. layout(depth_greater)  depth(greater)
    DepthLessEqual,         // n/a       SV_DepthLessEqual          gl_FragDepth w. layout(depth_less)     depth(less)
    DispatchThreadID,       // n/a       SV_DispatchThreadID        gl_GlobalInvocationID                  thread_position_in_grid
    DomainLocation,         // n/a       SV_DomainLocation          gl_TessCoord                           n/a
    FragCoord,              // VPOS      SV_Position                gl_FragCoord                           position
    GroupID,                // n/a       SV_GroupID                 gl_WorkGroupID                         threadgroup_position_in_grid
    GroupIndex,             // n/a       SV_GroupIndex              gl_LocalInvocationIndex                n/a
    GroupThreadID,          // n/a       SV_GroupThreadID           gl_LocalInvocationID                   thread_index_in_threadgroup
    GSInstanceID,           // n/a       SV_GSInstanceID            gl_InvocationID                        n/a
  //HelperInvocation,       // n/a       n/a                        gl_HelperInvocation                    n/a
    InnerCoverage,          // n/a       SV_InnerCoverage           gl_SampleMaskIn                        n/a
    InsideTessFactor,       // n/a       SV_InsideTessFactor[2]     gl_TessLevelInner[2]                   n/a
    InstanceID,             // n/a       SV_InstanceID              gl_InstanceID/ gl_InstanceIndex        instance_id
    IsFrontFace,            // VFACE     SV_IsFrontFace             gl_FrontFacing                         front_facing
    OutputControlPointID,   // n/a       SV_OutputControlPointID    gl_InvocationID                        n/a
    PointSize,              // PSIZE     n/a                        gl_PointSize                           point_size
    PrimitiveID,            // n/a       SV_PrimitiveID             gl_PrimitiveID                         n/a
    RenderTargetArrayIndex, // n/a       SV_RenderTargetArrayIndex  gl_Layer                               render_target_array_index
    SampleIndex,            // n/a       SV_SampleIndex             gl_SampleID                            sample_id
    StencilRef,             // n/a       SV_StencilRef              gl_FragStencilRef                      n/a
    Target,                 // COLOR     SV_Target[N]               gl_FragData[N]                         color(N)
    TessFactor,             // n/a       SV_TessFactor[4]           gl_TessLevelOuter[4]                   n/a
    VertexID,               // n/a       SV_VertexID                gl_VertexID/ gl_VertexIndex            vertex_id
    VertexPosition,         // POSITION  SV_Position                gl_Position                            position
    ViewportArrayIndex,     // n/a       SV_ViewportArrayIndex      gl_ViewportIndex                       viewport_array_index
  //NumWorkGroups,          // n/a       n/a                        gl_NumWorkGroups                       threadgroups_per_grid
  //WorkGroupSize,          // n/a       n/a                        gl_WorkGroupSize                       threads_per_threadgroup
};

// Indexed semantic type with 'Semantic' enum, integral index, and implicit conversion from and to 'Semantic' enum.
class IndexedSemantic
{

    public:

        IndexedSemantic() = default;
        IndexedSemantic(const IndexedSemantic&) = default;
        IndexedSemantic& operator = (const IndexedSemantic&) = default;

        IndexedSemantic(Semantic semantic, int index = 0);
        IndexedSemantic(const std::string& userDefined);
        IndexedSemantic(const IndexedSemantic& rhs, int index);

        inline operator Semantic() const
        {
            return semantic_;
        }

        // Compares this semantic with the specified semantic for a strict-weak-order (SWO).
        bool operator < (const IndexedSemantic& rhs) const;

        // Returns true if this the semantic is not undefined (Semantic::Undefined).
        bool IsValid() const;

        // see Xsc::IsSystemSemantic
        bool IsSystemValue() const;

        // see Xsc::IsUserSemantic
        bool IsUserDefined() const;

        std::string ToString() const;

        // Resest this semantic to undefined.
        void Reset();

        // Restes the index of this semantic.
        void ResetIndex(int index);

        // Converts this system value semantic to a user defined semantic.
        void MakeUserDefined(const std::string& semanticName = "");

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


/* ----- Reflection::ResourceType Enum ----- */

std::string ResourceTypeToString(const Reflection::ResourceType t);

Reflection::ResourceType UniformBufferTypeToResourceType(const UniformBufferType t);
Reflection::ResourceType BufferTypeToResourceType(const BufferType t);
Reflection::ResourceType SamplerTypeToResourceType(const SamplerType t);


} // /namespace Xsc


#endif



// ================================================================================
