/*
 * Reflection.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_REFLECTION_H
#define XSC_REFLECTION_H


#include "Export.h"
#include <limits>
#include <string>
#include <vector>
#include <ostream>


namespace Xsc
{

//! Shader code reflection namespace
namespace Reflection
{


/* ===== Public enumerations ===== */

/**
\brief Sampler filter enumeration (compatible to `D3D11_FILTER`).
\see https://docs.microsoft.com/en-us/windows/desktop/api/d3d11/ne-d3d11-d3d11_filter
*/
enum class Filter
{
    MinMagMipPoint                          = 0,
    MinMagPointMipLinear                    = 0x1,
    MinPointMagLinearMipPoint               = 0x4,
    MinPointMagMipLinear                    = 0x5,
    MinLinearMagMipPoint                    = 0x10,
    MinLinearMagPointMipLinear              = 0x11,
    MinMagLinearMipPoint                    = 0x14,
    MinMagMipLinear                         = 0x15,
    Anisotropic                             = 0x55,
    ComparisonMinMagMipPoint                = 0x80,
    ComparisonMinMagPointMipLinear          = 0x81,
    ComparisonMinPointMagLinearMipPoint     = 0x84,
    ComparisonMinPointMagMipLinear          = 0x85,
    ComparisonMinLinearMagMipPoint          = 0x90,
    ComparisonMinLinearMagPointMipLinear    = 0x91,
    ComparisonMinMagLinearMipPoint          = 0x94,
    ComparisonMinMagMipLinear               = 0x95,
    ComparisonAnisotropic                   = 0xd5,
    MinimumMinMagMipPoint                   = 0x100,
    MinimumMinMagPointMipLinear             = 0x101,
    MinimumMinPointMagLinearMipPoint        = 0x104,
    MinimumMinPointMagMipLinear             = 0x105,
    MinimumMinLinearMagMipPoint             = 0x110,
    MinimumMinLinearMagPointMipLinear       = 0x111,
    MinimumMinMagLinearMipPoint             = 0x114,
    MinimumMinMagMipLinear                  = 0x115,
    MinimumAnisotropic                      = 0x155,
    MaximumMinMagMipPoint                   = 0x180,
    MaximumMinMagPointMipLinear             = 0x181,
    MaximumMinPointMagLinearMipPoint        = 0x184,
    MaximumMinPointMagMipLinear             = 0x185,
    MaximumMinLinearMagMipPoint             = 0x190,
    MaximumMinLinearMagPointMipLinear       = 0x191,
    MaximumMinMagLinearMipPoint             = 0x194,
    MaximumMinMagMipLinear                  = 0x195,
    MaximumAnisotropic                      = 0x1d5,
};

/**
\brief Texture address mode enumeration (compatible to `D3D11_TEXTURE_ADDRESS_MODE`).
\see https://docs.microsoft.com/en-us/windows/desktop/api/d3d11/ne-d3d11-d3d11_texture_address_mode
*/
enum class TextureAddressMode
{
    Wrap        = 1,
    Mirror      = 2,
    Clamp       = 3,
    Border      = 4,
    MirrorOnce  = 5,
};

/**
\brief Sample comparison function enumeration (compatible to `D3D11_COMPARISON_FUNC`).
\see https://docs.microsoft.com/en-us/windows/desktop/api/d3d11/ne-d3d11-d3d11_comparison_func
*/
enum class ComparisonFunc
{
    Never           = 1,
    Less            = 2,
    Equal           = 3,
    LessEqual       = 4,
    Greater         = 5,
    NotEqual        = 6,
    GreaterEqual    = 7,
    Always          = 8,
};

/**
\brief Data field type enumeration.
\see Field::type
*/
enum class FieldType
{
    Undefined,  //!< Undefined field type.
    Bool,       //!< Boolean type.
    Int,        //!< Signed integer type.
    UInt,       //!< Unsigned integer type.
    Half,       //!< Half-precision floating-point type.
    Float,      //!< Single-precision floating-point type.
    Double,     //!< Double-precision floating-point type.
    Record,     //!< Record (also called structure, struct, or compound data) type.
};

/**
\brief Resource type enumeration.
\see Resource::type
*/
enum class ResourceType
{
    Undefined,                  //!< Undfined resource type.

    Texture1D,                  //!< 1D texture: `Texture1D` in HLSL, `texture1D` in GLSL (Vulkan only).
    Texture2D,                  //!< 2D texture: `Texture2D` in HLSL, `texture2D` in GLSL (Vulkan only).
    Texture3D,                  //!< 3D texture: `Texture3D` in HLSL, `texture3D` in GLSL (Vulkan only).
    TextureCube,                //!< Cube texture: `TextureCube` in HLSL, `textureCube` in GLSL (Vulkan only).
    Texture1DArray,             //!< 1D array texture: `Texture1DArray` in HLSL, `texture1DArray` in GLSL (Vulkan only).
    Texture2DArray,             //!< 2D array texture: `Texture2DArray` in HLSL, `texture2DArray` in GLSL (Vulkan only).
    TextureCubeArray,           //!< Cube array texture: `TextureCubeArray` in HLSL, `textureCubeArray` in GLSL (Vulkan only).
    Texture2DMS,                //!< 2D multi-sampled texture: `Texture2DMS` in HLSL, `texture2DMS` in GLSL (Vulkan only).
    Texture2DMSArray,           //!< 2D multi-sampled array texture: `Texture2DMSArray` in HLSL, `texture2DMSArray` in GLSL (Vulkan only).

    RWTexture1D,                //!< 1D read-write texture: `RWTexture1D` in HLSL, `image1D` in GLSL.
    RWTexture2D,                //!< 2D read-write texture: `RWTexture2D` in HLSL, `image2D` in GLSL.
    RWTexture3D,                //!< 3D read-write texture: `RWTexture3D` in HLSL, `image3D` in GLSL.
    RWTextureCube,              //!< Cube read-write texture: `RWTextureCube` in HLSL, `imageCube` in GLSL.
    RWTexture1DArray,           //!< 1D array read-write texture: `RWTexture1DArray` in HLSL, `image1DArray` in GLSL.
    RWTexture2DArray,           //!< 2D array read-write texture: `RWTexture2DArray` in HLSL, `image2DArray` in GLSL.
    RWTextureCubeArray,         //!< Cube array read-write texture: `RWTextureCubeArray` in HLSL, `imageCubeArray` in GLSL.
    RWTexture2DMS,              //!< 2D multi-sampled read-write texture: `RWTexture2DMS` in HLSL, `image2DMS` in GLSL.
    RWTexture2DMSArray,         //!< 2D multi-sampled array read-write texture: `RWTexture2DMSArray` in HLSL, `image2DMSArray` in GLSL.

    Sampler1D,                  //!< Combined 1D texture-sampler: `sampler1D` in GLSL.
    Sampler2D,                  //!< Combined 2D texture-sampler: `sampler2D` in GLSL.
    Sampler3D,                  //!< Combined 3D texture-sampler: `sampler3D` in GLSL.
    SamplerCube,                //!< Combined Cube texture-sampler: `samplerCube` in GLSL.
    Sampler1DArray,             //!< Combined 1D array texture-sampler: `sampler1DArray` in GLSL.
    Sampler2DArray,             //!< Combined 2D array texture-sampler: `sampler2DArray` in GLSL.
    SamplerCubeArray,           //!< Combined Cube array texture-sampler: `samplerCubeArray` in GLSL.
    Sampler2DMS,                //!< Combined 2D multi-sampled texture-sampler: `sampler2DMS` in GLSL.
    Sampler2DMSArray,           //!< Combined 2D multi-sampled array texture-sampler: `sampler2DMSArray` in GLSL.
    Sampler2DRect,              //!< Combined 2D texture-sampler with unnormalized texture coordinates: `sampler2DRect` in GLSL.

    Buffer,                     //!< Vector buffer: `Buffer` in HLSL, `samplerBuffer` in GLSL.
    ByteAddressBuffer,          //!< Byte addressable buffer: `ByteAddressBuffer` in HLSL, `samplerBuffer` in GLSL.
    StructuredBuffer,           //!< Structured buffer: `StructuredBuffer` in HLSL, `buffer` in GLSL and VKSL.
    AppendStructuredBuffer,     //!< Append structured buffer: `AppendStructuredBuffer` in HLSL, `buffer` in GLSL.
    ConsumeStructuredBuffer,    //!< Consume structured buffer: `ConsumeStructuredBuffer` in HLSL, `buffer` in GLSL.

    RWBuffer,                   //!< Vector read-write buffer: `RWBuffer` in HLSL, `imageBuffer` in GLSL.
    RWByteAddressBuffer,        //!< Byte addressable read-write buffer: `RWByteAddressBuffer` in HLSL, `imageBuffer` in GLSL.
    RWStructuredBuffer,         //!< Structured read-write buffer: `RWStructuredBuffer` in HLSL, `buffer` in GLSL and VKSL.

    ConstantBuffer,             //!< Constant buffer: `cbuffer` in HLSL, `uniform` in GLSL.
    TextureBuffer,              //!< Texture buffer: `tbuffer` in HLSL, `samplerBuffer` in GLSL.
    SamplerState,               //!< Sampler state: `SamplerState` in HLSL, `sampler` in GLSL (Vulkan only).
    SamplerComparisonState,     //!< Sampler comparison state: `SamplerComparisonState` in HLSL, `sampler` in GLSL (Vulkan only).
};


/* ===== Public structures ===== */

/**
\brief Static sampler state descriptor structure (D3D11_SAMPLER_DESC).
\remarks All members and enumerations have the same values like the one in the "D3D11_SAMPLER_DESC" structure respectively.
Thus, they can all be statically casted from and to the original D3D11 values.
\see StaticSamplerState::desc
\see https://msdn.microsoft.com/en-us/library/windows/desktop/ff476207(v=vs.85).aspx
*/
struct SamplerStateDesc
{
    Filter              filter          = Filter::MinMagMipLinear;
    TextureAddressMode  addressU        = TextureAddressMode::Clamp;
    TextureAddressMode  addressV        = TextureAddressMode::Clamp;
    TextureAddressMode  addressW        = TextureAddressMode::Clamp;
    float               mipLODBias      = 0.0f;
    unsigned int        maxAnisotropy   = 1u;
    ComparisonFunc      comparisonFunc  = ComparisonFunc::Never;
    float               borderColor[4]  = { 0.0f, 0.0f, 0.0f, 0.0f };
    float               minLOD          = -std::numeric_limits<float>::max();
    float               maxLOD          = std::numeric_limits<float>::max();
};

/**
\brief Input/output attribute and uniform reflection structure.
\see ReflectionData::inputAttributes
\see ReflectionData::outputAttributes
*/
struct Attribute
{
    //! Default constructor.
    Attribute() = default;

    //! Constructor to initialize all members.
    inline Attribute(const std::string& name, int slot) :
        name { name },
        slot { slot }
    {
    }

    //! Specifies whether this attribute is referenced in the output shader unit. By default false.
    bool        referenced  = false;

    //! Name of the attribute.
    std::string name;

    //! Zero-based attribute slot number. If this is -1, the binding slot was not specified. By default -1.
    int         slot        = -1;
};

/**
\brief Resource reflection structure for textures, combined texture samplers, and buffers.
\see ReflectionData::resources
*/
struct Resource
{
    //! Specifies whether this resource is referenced in the output shader unit. By default false.
    bool            referenced  = false;

    //! Resource type. By default ResourceType::Undefined.
    ResourceType    type        = ResourceType::Undefined;

    //! Name of the resource.
    std::string     name;

    //! Zero-based binding slot number. If this is -1, the binding slot was not specified. By default -1.
    int             slot        = -1;
};

/**
\brief A field denotes a data member of a record or constant buffer.
\see Record::fields
\see ConstantBuffer::fields
*/
struct Field
{
    //! Specifies whether this field is referenced in the output shader unit. By default false.
    bool                        referenced      = false;

    //! Name of the field (also called a data member or variable).
    std::string                 name;

    //! Base type of the field. Note that this does not include the dimension of a vector type. By default FieldType::Undefined.
    FieldType                   type            = FieldType::Undefined;

    /**
    \brief Number of vector dimensions. This is typically used in combination with the 'type' member. By default 1.
    \remarks If this is 1, the field denotes a scalar type. If this is 0, the field denotes a record type.
    */
    unsigned int                dimensions      = 1;

    //! Index to the global record type. If this is -1, the field does not denote a record type. By default -1.
    int                         typeRecordIndex = -1;

    //! Size (in bytes) of the field. If this is 0xFFFFFFFF, the field size could not be determined. By default 0.
    unsigned int                size            = 0;

    //! Local offset (in bytes) within the containing record or constant buffer.
    unsigned int                offset          = 0;

    //! Number of array elements. If this container is empty, the field does not denote an array.
    std::vector<unsigned int>   arrayElements;
};

/**
\brief A record denotes a data structure declared in a shader.
\see ReflectionData::records
*/
struct Record
{
    //! Specifies whether this record is referenced in the output shader unit. By default false.
    bool                referenced      = false;

    //! Name of the record (also called a structure, struct, or compound data).
    std::string         name;

    //! Optional index to the global base record type from which this record inherits. If this is -1, this record does not inherit from any other record type. By default -1.
    int                 baseRecordIndex = -1;

    //! Collection of all fields within this record.
    std::vector<Field>  fields;

    //! Size (in bytes) of the record. If this is 0xFFFFFFFF, the record size could not be determined. By default 0.
    unsigned int        size            = 0;

    //! Size (in bytes) of the padding that is added to the record. By default 0.
    unsigned int        padding         = 0;
};

/**
\brief Constant buffer reflection structure.
\see ReflectionData::constantBuffers
*/
struct ConstantBuffer
{
    //! Specifies whether this constant buffer is referenced in the output shader unit. By default false.
    bool                referenced  = false;

    //! Resource type. By default ResourceType::Undefined.
    ResourceType        type        = ResourceType::Undefined;

    //! Name of the constant buffer.
    std::string         name;

    //! Zero-based binding slot number. If this is -1, the binding slot was not specified. By default -1.
    int                 slot        = -1;

    //! Collection of all fields within this constant buffer.
    std::vector<Field>  fields;

    //! Size (in bytes) of the constant buffer with a 16-byte alignment. If this is 0xFFFFFFFF, the buffer size could not be determined. By default 0.
    unsigned int        size        = 0;

    //! Size (in bytes) of the padding that is added to the constant buffer. By default 0.
    unsigned int        padding     = 0;
};

/**
\brief Sampler state reflection structure.
\see ReflectionData::samplerStates
*/
struct SamplerState
{
    //! Resource type. By default ResourceType::Undefined.
    ResourceType    type        = ResourceType::Undefined;

    //! Name of the sampler state.
    std::string     name;

    //! Zero-based binding slot number. If this is -1, the binding slot was not specified. By default -1.
    int             slot        = -1;

    //! Specifies whether this record is referenced in the output shader unit. By default false.
    bool            referenced  = false;
};

/**
\brief Static sampler state reflection structure.
\see ReflectionData::staticSamplerStates
*/
struct StaticSamplerState
{
    //! Resource type. By default ResourceType::Undefined.
    ResourceType        type    = ResourceType::Undefined;

    //! Name of the static sampler state.
    std::string         name;

    //! Descriptor of the sampler state.
    SamplerStateDesc    desc;
};

/**
\brief Number of threads within each work group of a compute shader.
\see ReflectionData::numThreads
*/
struct NumThreads
{
    //! Number of shader compute threads in X dimension.
    int x = 0;

    //! Number of shader compute threads in Y dimension.
    int y = 0;

    //! Number of shader compute threads in Z dimension.
    int z = 0;
};

//! Structure for shader output statistics (e.g. texture/buffer binding points).
struct ReflectionData
{
    //! All defined macros after pre-processing.
    std::vector<std::string>        macros;

    //! All records declared both globally and within constant buffers (also called structure, struct, or compound data).
    std::vector<Record>             records;

    //! Shader input attributes.
    std::vector<Attribute>          inputAttributes;

    //! Shader output attributes.
    std::vector<Attribute>          outputAttributes;

    //! Single shader uniforms.
    std::vector<Attribute>          uniforms;

    //! Texture and buffer resources.
    std::vector<Resource>           resources;

    //! Constant buffers.
    std::vector<ConstantBuffer>     constantBuffers;

    //! Dynamic sampler states.
    std::vector<SamplerState>       samplerStates;

    //! Static sampler states.
    std::vector<StaticSamplerState> staticSamplerStates;

    //! Number of local threads in a compute shader.
    NumThreads                      numThreads;
};


} // /namespace Reflection


/* ===== Public functions ===== */

//! Returns the string representation of the specified <Filter> type.
XSC_EXPORT std::string ToString(const Reflection::Filter t);

//! Returns the string representation of the specified <TextureAddressMode> type.
XSC_EXPORT std::string ToString(const Reflection::TextureAddressMode t);

//! Returns the string representation of the specified <ComparisonFunc> type.
XSC_EXPORT std::string ToString(const Reflection::ComparisonFunc t);

//! Returns the string representation of the specified <Reflection::ResourceType> type.
XSC_EXPORT std::string ToString(const Reflection::ResourceType t);

/**
\brief Prints the reflection data into the output stream in a human readable format.
\param[out] stream Specifies the output stream to which the reflection will be printed.
This can be <code>std::cout</code> to print to the standard output for instance, or an instance of <code>std::stringstream</code> to convert the result into a string.
\param[in] reflectionData Specifies the input reflection data that can be obtained by the \c CompileShader function.
\param[in] referencedOnly Specifies whether or not to print only resources that are referenced in the output shader unit. By default false.
\see CompileShader
*/
XSC_EXPORT void PrintReflection(std::ostream& stream, const Reflection::ReflectionData& reflectionData, bool referencedOnly = false);


} // /namespace Xsc


#endif



// ================================================================================