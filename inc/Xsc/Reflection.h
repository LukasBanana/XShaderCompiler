/*
 * Reflection.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_REFLECTION_H
#define XSC_REFLECTION_H


#include "Export.h"
#include <limits>
#include <string>
#include <map>
#include <vector>
#include <ostream>


namespace Xsc
{

//! Shader code reflection namespace
namespace Reflection
{


//! Sampler filter enumeration (D3D11_FILTER).
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

//! Texture address mode enumeration (D3D11_TEXTURE_ADDRESS_MODE).
enum class TextureAddressMode
{
    Wrap        = 1,
    Mirror      = 2,
    Clamp       = 3,
    Border      = 4,
    MirrorOnce  = 5,
};

//! Sample comparison function enumeration (D3D11_COMPARISON_FUNC).
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
\brief Static sampler state descriptor structure (D3D11_SAMPLER_DESC).
\remarks All members and enumerations have the same values like the one in the "D3D11_SAMPLER_DESC" structure respectively.
Thus, they can all be statically casted from and to the original D3D11 values.
\see https://msdn.microsoft.com/en-us/library/windows/desktop/ff476207(v=vs.85).aspx
*/
struct SamplerState
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

//! Binding slot of textures, constant buffers, and fragment targets.
struct BindingSlot
{
    //! Identifier of the binding point.
    std::string ident;

    //! Zero based binding point or location. If this is -1, the location has not been set explicitly.
    int         location;
};

//! Number of threads within each work group of a compute shader.
struct NumThreads
{
    int x = 0, y = 0, z = 0;
};

//! Structure for shader output statistics (e.g. texture/buffer binding points).
struct ReflectionData
{
    //! All defined macros after pre-processing.
    std::vector<std::string>            macros;

    //! Texture bindings.
    std::vector<BindingSlot>            textures;

    //! Constant buffer bindings.
    std::vector<BindingSlot>            constantBuffers;

    //! Shader input attributes.
    std::vector<BindingSlot>            inputAttributes;

    //! Shader output attributes.
    std::vector<BindingSlot>            outputAttributes;

    //! Static sampler states (identifier, states).
    std::map<std::string, SamplerState> samplerStates;

    //! 'numthreads' attribute of a compute shader.
    NumThreads                          numThreads;
};


} // /namespace Reflection


//! Returns the string representation of the specified 'SamplerState::Filter' type.
XSC_EXPORT std::string ToString(const Reflection::Filter t);

//! Returns the string representation of the specified 'SamplerState::TextureAddressMode' type.
XSC_EXPORT std::string ToString(const Reflection::TextureAddressMode t);

//! Returns the string representation of the specified 'SamplerState::ComparisonFunc' type.
XSC_EXPORT std::string ToString(const Reflection::ComparisonFunc t);

//! Prints the reflection data into the output stream in a human readable format.
XSC_EXPORT void PrintReflection(std::ostream& stream, const Reflection::ReflectionData& reflectionData);


} // /namespace Xsc


#endif



// ================================================================================