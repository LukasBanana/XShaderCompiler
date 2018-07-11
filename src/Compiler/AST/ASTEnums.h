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

    /* --- String types --- */
                // HLSL         GLSL
                // -----------  ---------------
    String,     // string       n/a

    /* --- Scalar types --- */
                // HLSL         GLSL
                // -----------  -----------
    Bool,       // bool         bool
    Int,        // int          int
    UInt,       // uint         uint
    Half,       // half         float
    Float,      // float        float
    Double,     // double       double

    /* --- Vector types --- */
                // HLSL         GLSL
                // -----------  -----------
    Bool2,      // bool2        bvec2
    Bool3,      // bool3        bvec3
    Bool4,      // bool4        bvec4
    Int2,       // int2         ivec2
    Int3,       // int3         ivec3
    Int4,       // int4         ivec4
    UInt2,      // uint2        uvec2
    UInt3,      // uint3        uvec3
    UInt4,      // uint4        uvec4
    Half2,      // half2        vec2
    Half3,      // half3        vec3
    Half4,      // half4        vec4
    Float2,     // float2       vec2
    Float3,     // float3       vec3
    Float4,     // float4       vec4
    Double2,    // double2      dvec2
    Double3,    // double3      dvec3
    Double4,    // double4      dvec4

    /* --- Matrix types --- */
                // HLSL         GLSL
                // -----------  -----------
    Bool2x2,    // bool2x2      n/a
    Bool2x3,    // bool2x3      n/a
    Bool2x4,    // bool2x4      n/a
    Bool3x2,    // bool3x2      n/a
    Bool3x3,    // bool3x3      n/a
    Bool3x4,    // bool3x4      n/a
    Bool4x2,    // bool4x2      n/a
    Bool4x3,    // bool4x3      n/a
    Bool4x4,    // bool4x4      n/a
    Int2x2,     // int2x2       n/a
    Int2x3,     // int2x3       n/a
    Int2x4,     // int2x4       n/a
    Int3x2,     // int3x2       n/a
    Int3x3,     // int3x3       n/a
    Int3x4,     // int3x4       n/a
    Int4x2,     // int4x2       n/a
    Int4x3,     // int4x3       n/a
    Int4x4,     // int4x4       n/a
    UInt2x2,    // uint2x2      n/a
    UInt2x3,    // uint2x3      n/a
    UInt2x4,    // uint2x4      n/a
    UInt3x2,    // uint3x2      n/a
    UInt3x3,    // uint3x3      n/a
    UInt3x4,    // uint3x4      n/a
    UInt4x2,    // uint4x2      n/a
    UInt4x3,    // uint4x3      n/a
    UInt4x4,    // uint4x4      n/a
    Half2x2,    // half2x2      mat2
    Half2x3,    // half2x3      mat2x3
    Half2x4,    // half2x4      mat2x4
    Half3x2,    // half3x2      mat3x2
    Half3x3,    // half3x3      mat3
    Half3x4,    // half3x4      mat3x4
    Half4x2,    // half4x2      mat4x2
    Half4x3,    // half4x3      mat4x3
    Half4x4,    // half4x4      mat4
    Float2x2,   // float2x2     mat2
    Float2x3,   // float2x3     mat2x3
    Float2x4,   // float2x4     mat2x4
    Float3x2,   // float3x2     mat3x2
    Float3x3,   // float3x3     mat3
    Float3x4,   // float3x4     mat3x4
    Float4x2,   // float4x2     mat4x2
    Float4x3,   // float4x3     mat4x3
    Float4x4,   // float4x4     mat4
    Double2x2,  // double2x2    dmat2
    Double2x3,  // double2x3    dmat2x3
    Double2x4,  // double2x4    dmat2x4
    Double3x2,  // double3x2    dmat3x2
    Double3x3,  // double3x3    dmat3
    Double3x4,  // double3x4    dmat3x4
    Double4x2,  // double4x2    dmat4x2
    Double4x3,  // double4x3    dmat4x3
    Double4x4,  // double4x4    dmat4
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
DataType SubscriptDataType(const DataType dataType, const std::string& subscript, std::vector<std::pair<int, int>>* indices = nullptr);

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

                        // HLSL             GLSL
                        // ---------------  ---------------
    Centroid,           // centroid         centroid
    Linear,             // linear           smooth
    NoInterpolation,    // nointerpolation  flat
    NoPerspective,      // noperspective    noperspective
    Sample,             // sample           sample
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
                            // HLSL3            HLSL4+                  GLSL
                            // ---------------  ----------------------  ----------------------
    Sampler1D,              // sampler1D        n/a                     sampler1D
    Sampler2D,              // sampler2D        n/a                     sampler2D
    Sampler3D,              // sampler3D        n/a                     sampler3D
    SamplerCube,            // samplerCUBE      n/a                     samplerCube
    Sampler2DRect,          // n/a              n/a                     sampler2DRect
    Sampler1DArray,         // n/a              n/a                     sampler1DArray
    Sampler2DArray,         // n/a              n/a                     sampler2DArray
    SamplerCubeArray,       // n/a              n/a                     samplerCubeArray
    SamplerBuffer,          // n/a              n/a                     samplerBuffer
    Sampler2DMS,            // n/a              n/a                     sampler2DMS
    Sampler2DMSArray,       // n/a              n/a                     sampler2DMSArray
    Sampler1DShadow,        // sampler1DShadow  n/a                     sampler1DShadow
    Sampler2DShadow,        // sampler2DShadow  n/a                     sampler2DShadow
    SamplerCubeShadow,      // n/a              n/a                     samplerCubeShadow
    Sampler2DRectShadow,    // n/a              n/a                     sampler2DRectShadow
    Sampler1DArrayShadow,   // n/a              n/a                     sampler1DArrayShadow
    Sampler2DArrayShadow,   // n/a              n/a                     sampler2DArrayShadow
    SamplerCubeArrayShadow, // n/a              n/a                     samplerCubeArrayShadow

    /* --- Sampler states --- */
                            // HLSL3            HLSL4+                  GLSL
                            // ---------------  ----------------------  ----------------------
    SamplerState,           // sampler_state    SamplerState            sampler
    SamplerComparisonState, // sampler_state    SamplerComparisonState  samplerShadow
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
                                        // HLSL                                 GLSL
                                        // -----------------------------------  ------------------------
    Abort,                              // abort                                n/a
    Abs,                                // abs                                  abs
    ACos,                               // acos                                 acos
    All,                                // all                                  all
    AllMemoryBarrier,                   // AllMemoryBarrier                     memoryBarrier
    AllMemoryBarrierWithGroupSync,      // AllMemoryBarrierWithGroupSync        n/a
    Any,                                // any                                  any
    AsDouble,                           // asdouble                             uint64BitsToDouble
    AsFloat,                            // asfloat                              uintBitsToFloat
    ASin,                               // asin                                 asin
    AsInt,                              // asint                                floatBitsToInt
    AsUInt_1,                           // asuint                               floatBitsToUint
    AsUInt_3,                           // asuint                               n/a
    ATan,                               // atan                                 atan
    ATan2,                              // atan2                                atan
    Ceil,                               // ceil                                 cail
    CheckAccessFullyMapped,             // CheckAccessFullyMapped               n/a
    Clamp,                              // clamp                                clamp
    Clip,                               // clip                                 n/a
    Cos,                                // cos                                  cos
    CosH,                               // cosh                                 cosh
    CountBits,                          // countbits                            bitCount
    Cross,                              // cross                                cross
    D3DCOLORtoUBYTE4,                   // D3DCOLORtoUBYTE4                     n/a
    DDX,                                // ddx                                  dFdx
    DDXCoarse,                          // ddx_coarse                           dFdxCoarse
    DDXFine,                            // ddx_fine                             dFdxFine
    DDY,                                // ddy                                  dFdy
    DDYCoarse,                          // ddy_coarse                           dFdyCoarse
    DDYFine,                            // ddy_fine                             dFdyFine
    Degrees,                            // degrees                              degrees
    Determinant,                        // determinant                          determinant
    DeviceMemoryBarrier,                // DeviceMemoryBarrier                  n/a
    DeviceMemoryBarrierWithGroupSync,   // DeviceMemoryBarrierWithGroupSync     n/a
    Distance,                           // distance                             distance
    Dot,                                // dot                                  dot
    Dst,                                // dst                                  n/a
    Equal,                              // n/a                                  equal
    ErrorF,                             // errorf                               n/a
    EvaluateAttributeAtCentroid,        // EvaluateAttributeAtCentroid          interpolateAtCentroid
    EvaluateAttributeAtSample,          // EvaluateAttributeAtSample            interpolateAtSample
    EvaluateAttributeSnapped,           // EvaluateAttributeSnapped             interpolateAtOffset
    Exp,                                // exp                                  exp
    Exp2,                               // exp2                                 exp2
    F16toF32,                           // f16tof32                             n/a
    F32toF16,                           // f32tof16                             n/a
    FaceForward,                        // faceforward                          faceforward
    FirstBitHigh,                       // firstbithigh                         findMSB
    FirstBitLow,                        // firstbitlow                          findLSB
    Floor,                              // floor                                floor
    FMA,                                // fma                                  fma
    FMod,                               // fmod                                 mod
    Frac,                               // frac                                 fract
    FrExp,                              // frexp                                frexp
    FWidth,                             // fwidth                               fwidth
    GetRenderTargetSampleCount,         // GetRenderTargetSampleCount           n/a
    GetRenderTargetSamplePosition,      // GetRenderTargetSamplePosition        n/a
    GreaterThan,                        // n/a                                  greaterThan
    GreaterThanEqual,                   // n/a                                  greaterThanEqual
    GroupMemoryBarrier,                 // GroupMemoryBarrier                   groupMemoryBarrier
    GroupMemoryBarrierWithGroupSync,    // GroupMemoryBarrierWithGroupSync      n/a
    InterlockedAdd,                     // InterlockedAdd                       atomicAdd
    InterlockedAnd,                     // InterlockedAnd                       atomicAnd
    InterlockedCompareExchange,         // InterlockedCompareExchange           atomicCompSwap
    InterlockedCompareStore,            // InterlockedCompareStore              n/a
    InterlockedExchange,                // InterlockedExchange                  atomicExchange
    InterlockedMax,                     // InterlockedMax                       atomicMax
    InterlockedMin,                     // InterlockedMin                       atomicMin
    InterlockedOr,                      // InterlockedOr                        atomicOr
    InterlockedXor,                     // InterlockedXor                       atomicXor
    IsFinite,                           // isfinite                             n/a
    IsInf,                              // isinf                                isinf
    IsNaN,                              // isnan                                isnan
    LdExp,                              // ldexp                                ldexp
    Length,                             // length                               length
    Lerp,                               // lerp                                 mix
    LessThan,                           // n/a                                  lessThan
    LessThanEqual,                      // n/a                                  lessThanEqual
    Lit,                                // lit                                  n/a
    Log,                                // log                                  log
    Log10,                              // log10                                n/a
    Log2,                               // log2                                 log2
    MAD,                                // mad                                  fma
    Max,                                // max                                  max
    Min,                                // min                                  min
    ModF,                               // modf                                 modf
    MSAD4,                              // msad4                                n/a
    Mul,                                // mul                                  n/a
    Normalize,                          // normalize                            normalize
    NotEqual,                           // n/a                                  notEqual
    Not,                                // n/a                                  not
    Pow,                                // pow                                  pow
    PrintF,                             // printf                               n/a
    Process2DQuadTessFactorsAvg,        // Process2DQuadTessFactorsAvg          n/a
    Process2DQuadTessFactorsMax,        // Process2DQuadTessFactorsMax          n/a
    Process2DQuadTessFactorsMin,        // Process2DQuadTessFactorsMin          n/a
    ProcessIsolineTessFactors,          // ProcessIsolineTessFactors            n/a
    ProcessQuadTessFactorsAvg,          // ProcessQuadTessFactorsAvg            n/a
    ProcessQuadTessFactorsMax,          // ProcessQuadTessFactorsMax            n/a
    ProcessQuadTessFactorsMin,          // ProcessQuadTessFactorsMin            n/a
    ProcessTriTessFactorsAvg,           // ProcessTriTessFactorsAvg             n/a
    ProcessTriTessFactorsMax,           // ProcessTriTessFactorsMax             n/a
    ProcessTriTessFactorsMin,           // ProcessTriTessFactorsMin             n/a
    Radians,                            // radians                              radians
    Rcp,                                // rcp                                  n/a
    Reflect,                            // reflect                              reflect
    Refract,                            // refract                              refract
    ReverseBits,                        // reversebits                          n/a
    Round,                              // round                                round
    RSqrt,                              // rsqrt                                inversesqrt
    Saturate,                           // saturate                             n/a
    Sign,                               // sign                                 sign
    Sin,                                // sin                                  sin
    SinCos,                             // sincos                               n/a
    SinH,                               // sinh                                 sinh
    SmoothStep,                         // smoothstep                           smoothstep
    Sqrt,                               // sqrt                                 sqrt
    Step,                               // step                                 step
    Tan,                                // tan                                  tan
    TanH,                               // tanh                                 tanh
    Transpose,                          // transpose                            transpose
    Trunc,                              // trunc                                trunc

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

    /* --- GLSL image intrinsics --- */
    Image_Load,                 // GLSL only
    Image_Store,                // GLSL only
    Image_AtomicAdd,            // GLSL only
    Image_AtomicAnd,            // GLSL only
    Image_AtomicOr,             // GLSL only
    Image_AtomicXor,            // GLSL only
    Image_AtomicMin,            // GLSL only
    Image_AtomicMax,            // GLSL only
    Image_AtomicCompSwap,       // GLSL only
    Image_AtomicExchange        // GLSL only
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

                            // HLSL3     HLSL4+                     GLSL
                            // --------  -------------------------  ---------------------
    ClipDistance,           // n/a       SV_ClipDistance            gl_ClipDistance
    CullDistance,           // n/a       SV_CullDistance            gl_CullDistance (if ARB_cull_distance is present)
    Coverage,               // n/a       SV_Coverage                gl_SampleMask
    Depth,                  // DEPTH     SV_Depth                   gl_FragDepth
    DepthGreaterEqual,      // n/a       SV_DepthGreaterEqual       layout(depth_greater) out float gl_FragDepth
    DepthLessEqual,         // n/a       SV_DepthLessEqual          layout(depth_less) out float gl_FragDepth
    DispatchThreadID,       // n/a       SV_DispatchThreadID        gl_GlobalInvocationID
    DomainLocation,         // n/a       SV_DomainLocation          gl_TessCoord
    FragCoord,              // VPOS      SV_Position                gl_FragCoord
    GroupID,                // n/a       SV_GroupID                 gl_WorkGroupID
    GroupIndex,             // n/a       SV_GroupIndex              gl_LocalInvocationIndex
    GroupThreadID,          // n/a       SV_GroupThreadID           gl_LocalInvocationID
    GSInstanceID,           // n/a       SV_GSInstanceID            gl_InvocationID
  //HelperInvocation,       // n/a       n/a                        gl_HelperInvocation
    InnerCoverage,          // n/a       SV_InnerCoverage           gl_SampleMaskIn
    InsideTessFactor,       // n/a       SV_InsideTessFactor[2]     gl_TessLevelInner[2]
    InstanceID,             // n/a       SV_InstanceID              gl_InstanceID (OpenGL)/ gl_InstanceIndex (Vulkan)
    IsFrontFace,            // VFACE     SV_IsFrontFace             gl_FrontFacing
    OutputControlPointID,   // n/a       SV_OutputControlPointID    gl_InvocationID
    PointSize,              // PSIZE     n/a                        gl_PointSize
    PrimitiveID,            // n/a       SV_PrimitiveID             gl_PrimitiveID
    RenderTargetArrayIndex, // n/a       SV_RenderTargetArrayIndex  gl_Layer
    SampleIndex,            // n/a       SV_SampleIndex             gl_SampleID
    StencilRef,             // n/a       SV_StencilRef              gl_FragStencilRef (if ARB_shader_stencil_export is present)
    Target,                 // COLOR     SV_Target                  gl_FragData
    TessFactor,             // n/a       SV_TessFactor[4]           gl_TessLevelOuter[4]
    VertexID,               // n/a       SV_VertexID                gl_VertexID (OpenGL)/ gl_VertexIndex (Vulkan)
    VertexPosition,         // POSITION  SV_Position                gl_Position
    ViewportArrayIndex,     // n/a       SV_ViewportArrayIndex      gl_ViewportIndex
  //NumWorkGroups,          // n/a       n/a                        gl_NumWorkGroups
  //WorkGroupSize,          // n/a       n/a                        gl_WorkGroupSize
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


} // /namespace Xsc


#endif



// ================================================================================