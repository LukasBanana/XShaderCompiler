/*
 * SamplerState.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_SAMPLER_STATE_H
#define XSC_SAMPLER_STATE_H


#include "Export.h"
#include <limits>


namespace Xsc
{


/**
\brief Static sampler state descriptor structure (D3D11_SAMPLER_DESC).
\see https://msdn.microsoft.com/en-us/library/windows/desktop/ff476207(v=vs.85).aspx
*/
struct SamplerState
{
    //! Sampler filter enumeration (D3D11_FILTER).
    enum class Filter
    {
        MinMagMipPoint,
        MinMagPointMipLinear,
        MinPointMagLinearMipPoint,
        MinPointMagMipLinear,
        MinLinearMagMipPoint,
        MinMagLinearMipPoint,
        MinMagMipLinear,
        Anisotropic,
        ComparisonMinMagMipPoint,
        ComparisonMinMagPointMipLinear,
        ComparisonMinPointMagLinearMipPoint,
        ComparisonMinPointMagMipLinear,
        ComparisonMinLinearMagMipPoint,
        ComparisonMinMagLinearMipPoint,
        ComparisonMinMagMipLinear,
        ComparisonAnisotropic,
        Text1Bit,
    };

    //! Texture address mode enumeration (D3D11_TEXTURE_ADDRESS_MODE).
    enum class TextureAddressMode
    {
        Wrap,
        Mirror,
        Clamp,
        Border,
        MirrorOnce,
    };

    //! Sample comparison function enumeration (D3D11_COMPARISON_FUNC).
    enum class ComparisonFunc
    {
        Never,
        Less,
        Equal,
        LessEqual,
        Greater,
        NotEqual,
        GreaterEqual,
        Always,
    };

    Filter              filter          = Filter::MinMagMipLinear;
    TextureAddressMode  addressU        = TextureAddressMode::Clamp;
    TextureAddressMode  addressV        = TextureAddressMode::Clamp;
    TextureAddressMode  addressW        = TextureAddressMode::Clamp;
    float               mipLODBias      = 0.0f;
    unsigned int        maxAnisotropy   = 16;
    ComparisonFunc      comparisonFunc  = ComparisonFunc::Never;
    float               borderColor[4]  = { 0.0f, 0.0f, 0.0f, 0.0f };
    float               minLOD          = -std::numeric_limits<float>::max();
    float               maxLOD          = std::numeric_limits<float>::max();
};


} // /namespace Xsc


#endif



// ================================================================================