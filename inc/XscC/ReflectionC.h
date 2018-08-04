/*
 * ReflectionC.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_REFLECTION_C_H
#define XSC_REFLECTION_C_H


#include <Xsc/Export.h>
#include <stdio.h>


#ifdef __cplusplus
extern "C" {
#endif


/* ===== Public enumerations ===== */

//! Sampler filter enumeration (D3D11_FILTER).
enum XscFilter
{
    XscEFilterMinMagMipPoint                        = 0,
    XscEFilterMinMagPointMipLinear                  = 0x1,
    XscEFilterMinPointMagLinearMipPoint             = 0x4,
    XscEFilterMinPointMagMipLinear                  = 0x5,
    XscEFilterMinLinearMagMipPoint                  = 0x10,
    XscEFilterMinLinearMagPointMipLinear            = 0x11,
    XscEFilterMinMagLinearMipPoint                  = 0x14,
    XscEFilterMinMagMipLinear                       = 0x15,
    XscEFilterAnisotropic                           = 0x55,
    XscEFilterComparisonMinMagMipPoint              = 0x80,
    XscEFilterComparisonMinMagPointMipLinear        = 0x81,
    XscEFilterComparisonMinPointMagLinearMipPoint   = 0x84,
    XscEFilterComparisonMinPointMagMipLinear        = 0x85,
    XscEFilterComparisonMinLinearMagMipPoint        = 0x90,
    XscEFilterComparisonMinLinearMagPointMipLinear  = 0x91,
    XscEFilterComparisonMinMagLinearMipPoint        = 0x94,
    XscEFilterComparisonMinMagMipLinear             = 0x95,
    XscEFilterComparisonAnisotropic                 = 0xd5,
    XscEFilterMinimumMinMagMipPoint                 = 0x100,
    XscEFilterMinimumMinMagPointMipLinear           = 0x101,
    XscEFilterMinimumMinPointMagLinearMipPoint      = 0x104,
    XscEFilterMinimumMinPointMagMipLinear           = 0x105,
    XscEFilterMinimumMinLinearMagMipPoint           = 0x110,
    XscEFilterMinimumMinLinearMagPointMipLinear     = 0x111,
    XscEFilterMinimumMinMagLinearMipPoint           = 0x114,
    XscEFilterMinimumMinMagMipLinear                = 0x115,
    XscEFilterMinimumAnisotropic                    = 0x155,
    XscEFilterMaximumMinMagMipPoint                 = 0x180,
    XscEFilterMaximumMinMagPointMipLinear           = 0x181,
    XscEFilterMaximumMinPointMagLinearMipPoint      = 0x184,
    XscEFilterMaximumMinPointMagMipLinear           = 0x185,
    XscEFilterMaximumMinLinearMagMipPoint           = 0x190,
    XscEFilterMaximumMinLinearMagPointMipLinear     = 0x191,
    XscEFilterMaximumMinMagLinearMipPoint           = 0x194,
    XscEFilterMaximumMinMagMipLinear                = 0x195,
    XscEFilterMaximumAnisotropic                    = 0x1d5,
};

//! Texture address mode enumeration (D3D11_TEXTURE_ADDRESS_MODE).
enum XscTextureAddressMode
{
    XscEAddressWrap         = 1,
    XscEAddressMirror       = 2,
    XscEAddressClamp        = 3,
    XscEAddressBorder       = 4,
    XscEAddressMirrorOnce   = 5,
};

//! Sample comparison function enumeration (D3D11_COMPARISON_FUNC).
enum XscComparisonFunc
{
    XscEComparisonNever         = 1,
    XscEComparisonLess          = 2,
    XscEComparisonEqual         = 3,
    XscEComparisonLessEqual     = 4,
    XscEComparisonGreater       = 5,
    XscEComparisonNotEqual      = 6,
    XscEComparisonGreaterEqual  = 7,
    XscEComparisonAlways        = 8,
};

/**
\brief Resource type enumeration.
\see XscResource::type
*/
enum XscResourceType
{
    XscEResourceUndefined,                  //!< Undfined resource type.

    XscEResourceTexture1D,                  //!< 1D texture: `Texture1D` in HLSL, `texture1D` in GLSL (Vulkan only).
    XscEResourceTexture2D,                  //!< 2D texture: `Texture2D` in HLSL, `texture2D` in GLSL (Vulkan only).
    XscEResourceTexture3D,                  //!< 3D texture: `Texture3D` in HLSL, `texture3D` in GLSL (Vulkan only).
    XscEResourceTextureCube,                //!< Cube texture: `TextureCube` in HLSL, `textureCube` in GLSL (Vulkan only).
    XscEResourceTexture1DArray,             //!< 1D array texture: `Texture1DArray` in HLSL, `texture1DArray` in GLSL (Vulkan only).
    XscEResourceTexture2DArray,             //!< 2D array texture: `Texture2DArray` in HLSL, `texture2DArray` in GLSL (Vulkan only).
    XscEResourceTextureCubeArray,           //!< Cube array texture: `TextureCubeArray` in HLSL, `textureCubeArray` in GLSL (Vulkan only).
    XscEResourceTexture2DMS,                //!< 2D multi-sampled texture: `Texture2DMS` in HLSL, `texture2DMS` in GLSL (Vulkan only).
    XscEResourceTexture2DMSArray,           //!< 2D multi-sampled array texture: `Texture2DMSArray` in HLSL, `texture2DMSArray` in GLSL (Vulkan only).

    XscEResourceRWTexture1D,                //!< 1D read-write texture: `RWTexture1D` in HLSL, `image1D` in GLSL.
    XscEResourceRWTexture2D,                //!< 2D read-write texture: `RWTexture2D` in HLSL, `image2D` in GLSL.
    XscEResourceRWTexture3D,                //!< 3D read-write texture: `RWTexture3D` in HLSL, `image3D` in GLSL.
    XscEResourceRWTextureCube,              //!< Cube read-write texture: `RWTextureCube` in HLSL, `imageCube` in GLSL.
    XscEResourceRWTexture1DArray,           //!< 1D array read-write texture: `RWTexture1DArray` in HLSL, `image1DArray` in GLSL.
    XscEResourceRWTexture2DArray,           //!< 2D array read-write texture: `RWTexture2DArray` in HLSL, `image2DArray` in GLSL.
    XscEResourceRWTextureCubeArray,         //!< Cube array read-write texture: `RWTextureCubeArray` in HLSL, `imageCubeArray` in GLSL.
    XscEResourceRWTexture2DMS,              //!< 2D multi-sampled read-write texture: `RWTexture2DMS` in HLSL, `image2DMS` in GLSL.
    XscEResourceRWTexture2DMSArray,         //!< 2D multi-sampled array read-write texture: `RWTexture2DMSArray` in HLSL, `image2DMSArray` in GLSL.

    XscEResourceSampler1D,                  //!< Combined 1D texture-sampler: `sampler1D` in GLSL.
    XscEResourceSampler2D,                  //!< Combined 2D texture-sampler: `sampler2D` in GLSL.
    XscEResourceSampler3D,                  //!< Combined 3D texture-sampler: `sampler3D` in GLSL.
    XscEResourceSamplerCube,                //!< Combined Cube texture-sampler: `samplerCube` in GLSL.
    XscEResourceSampler1DArray,             //!< Combined 1D array texture-sampler: `sampler1DArray` in GLSL.
    XscEResourceSampler2DArray,             //!< Combined 2D array texture-sampler: `sampler2DArray` in GLSL.
    XscEResourceSamplerCubeArray,           //!< Combined Cube array texture-sampler: `samplerCubeArray` in GLSL.
    XscEResourceSampler2DMS,                //!< Combined 2D multi-sampled texture-sampler: `sampler2DMS` in GLSL.
    XscEResourceSampler2DMSArray,           //!< Combined 2D multi-sampled array texture-sampler: `sampler2DMSArray` in GLSL.
    XscEResourceSampler2DRect,              //!< Combined 2D texture-sampler with unnormalized texture coordinates: `sampler2DRect` in GLSL.

    XscEResourceBuffer,                     //!< Vector buffer: `Buffer` in HLSL, `samplerBuffer` in GLSL.
    XscEResourceByteAddressBuffer,          //!< Byte addressable buffer: `ByteAddressBuffer` in HLSL, `samplerBuffer` in GLSL.
    XscEResourceStructuredBuffer,           //!< Structured buffer: `StructuredBuffer` in HLSL, `buffer` in GLSL and VKSL.
    XscEResourceAppendStructuredBuffer,     //!< Append structured buffer: `AppendStructuredBuffer` in HLSL, `buffer` in GLSL.
    XscEResourceConsumeStructuredBuffer,    //!< Consume structured buffer: `ConsumeStructuredBuffer` in HLSL, `buffer` in GLSL.

    XscEResourceRWBuffer,                   //!< Vector read-write buffer: `RWBuffer` in HLSL, `imageBuffer` in GLSL.
    XscEResourceRWByteAddressBuffer,        //!< Byte addressable read-write buffer: `RWByteAddressBuffer` in HLSL, `imageBuffer` in GLSL.
    XscEResourceRWStructuredBuffer,         //!< Structured read-write buffer: `RWStructuredBuffer` in HLSL, `buffer` in GLSL and VKSL.

    XscEResourceConstantBuffer,             //!< Constant buffer: `cbuffer` in HLSL, `uniform` in GLSL.
    XscEResourceTextureBuffer,              //!< Texture buffer: `tbuffer` in HLSL, `samplerBuffer` in GLSL.
    XscEResourceSamplerState,               //!< Sampler state: `SamplerState` in HLSL, `sampler` in GLSL (Vulkan only).
    XscEResourceSamplerComparisonState,     //!< Sampler comparison state: `SamplerComparisonState` in HLSL, `sampler` in GLSL (Vulkan only).
};


/* ===== Public structures ===== */

/**
\brief Static sampler state descriptor structure (D3D11_SAMPLER_DESC).
\remarks All members and enumerations have the same values like the one in the "D3D11_SAMPLER_DESC" structure respectively.
Thus, they can all be statically casted from and to the original D3D11 values.
\see XscStaticSamplerState::desc
\see https://msdn.microsoft.com/en-us/library/windows/desktop/ff476207(v=vs.85).aspx
*/
struct XscSamplerStateDesc
{
    enum XscFilter              filter;
    enum XscTextureAddressMode  addressU;
    enum XscTextureAddressMode  addressV;
    enum XscTextureAddressMode  addressW;
    float                       mipLODBias;
    unsigned int                maxAnisotropy;
    enum XscComparisonFunc      comparisonFunc;
    float                       borderColor[4];
    float                       minLOD;
    float                       maxLOD;
};

//! Binding slot of textures, constant buffers, and fragment targets.
struct XscAttribute
{
    //! Name of the attribute.
    const char* name;

    //! Zero-based attribute slot number. If this is -1, the binding slot was not specified. By default -1.
    int         slot;
};

/**
\brief Resource reflection structure for textures, combined texture samplers, and buffers.
\see XscReflectionData::resources
*/
struct XscResource
{
    //! Resource type. By default ResourceType::Undefined.
    enum XscResourceType    type;

    //! Name of the resource.
    const char*             name;

    //! Zero-based binding slot number. If this is -1, the binding slot was not specified. By default -1.
    int                     slot;
};

/**
\brief Constant buffer reflection structure.
\see XscReflectionData::constantBuffers
*/
struct XscConstantBuffer
{
    //! Resource type. By default ResourceType::Undefined.
    enum XscResourceType    type;

    //! Name of the constant buffer.
    const char*             name;

    //! Zero-based binding slot number. If this is -1, the binding slot was not specified. By default -1.
    int                     slot;

    //! Size (in bytes) of the constant buffer with a 16-byte alignment. If this is 0xFFFFFFFF, the buffer size could not be determined. By default 0.
    unsigned int            size;

    //! Size (in bytes) of the padding that is added to the constant buffer. By default 0.
    unsigned int            padding;
};

/**
\brief Sampler state reflection structure.
\see XscReflectionData::samplerStates
*/
struct XscSamplerState
{
    //! Resource type. By default ResourceType::Undefined.
    enum XscResourceType    type;

    //! Name of the sampler state.
    const char*             name;

    //! Zero-based binding slot number. If this is -1, the binding slot was not specified. By default -1.
    int                     slot;
};

/**
\brief Static sampler state reflection structure.
\see XscReflectionData::staticSamplerStates
*/
struct XscStaticSamplerState
{
    //! Resource type. By default ResourceType::Undefined.
    enum XscResourceType        type;

    //! Name of the static sampler state.
    const char*                 name;

    //! Descriptor of the sampler state.
    struct XscSamplerStateDesc  desc;
};

/**
\brief Number of threads within each work group of a compute shader.
\see XscReflectionData::numThreads
*/
struct XscNumThreads
{
    //! Number of shader compute threads in X dimension.
    int x;

    //! Number of shader compute threads in Y dimension.
    int y;

    //! Number of shader compute threads in Z dimension.
    int z;
};

//! Structure for shader output statistics (e.g. texture/buffer binding points).
struct XscReflectionData
{
    //! All defined macros after pre-processing.
    const char**                        macros;

    //! Number of elements in 'macros'.
    size_t                              macrosCount;

    //! Shader input attributes.
    const struct XscAttribute*          inputAttributes;

    //! Number of elements in 'inputAttributes'.
    size_t                              inputAttributesCount;

    //! Shader output attributes.
    const struct XscAttribute*          outputAttributes;

    //! Number of elements in 'outputAttributes'.
    size_t                              outputAttributesCount;

    //! Single shader uniforms.
    const struct XscAttribute*          uniforms;

    //! Number of elements in 'uniforms'.
    size_t                              uniformsCount;

    //! Texture bindings.
    const struct XscResource*           resources;

    //! Number of elements in 'textures'.
    size_t                              resourcesCount;

    //! Constant buffer bindings.
    const struct XscConstantBuffer*     constantBuffers;

    //! Number of elements in 'constantBuffers'.
    size_t                              constantBufferCounts;

    //! Static sampler states (identifier, states).
    const struct XscSamplerState*       samplerStates;

    //! Number of elements in 'samplerStates'.
    size_t                              samplerStatesCount;

    //! Static sampler states (identifier, states).
    const struct XscStaticSamplerState* staticSamplerStates;

    //! Number of elements in 'samplerStates'.
    size_t                              staticSamplerStatesCount;

    //! 'numthreads' attribute of a compute shader.
    struct XscNumThreads                numThreads;
};


/* ===== Public functions ===== */

//! Returns the string representation of the specified <XscFilter> type.
XSC_EXPORT void XscFilterToString(const enum XscFilter t, char* str, size_t maxSize);

//! Returns the string representation of the specified <XscTextureAddressMode> type.
XSC_EXPORT void XscTextureAddressModeToString(const enum XscTextureAddressMode t, char* str, size_t maxSize);

//! Returns the string representation of the specified <XscComparisonFunc> type.
XSC_EXPORT void XscComparisonFuncToString(const enum XscComparisonFunc t, char* str, size_t maxSize);

//! Returns the string representation of the specified <XscResourceType> type.
XSC_EXPORT void XscResourceTypeToString(const enum XscResourceType t, char* str, size_t maxSize);


#ifdef __cplusplus
} // /extern "C"
#endif


#endif



// ================================================================================