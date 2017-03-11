/*
 * XscCSharp.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <Xsc/Xsc.h>
#include <vcclr.h>

#using <System.dll>
#using <System.Core.dll>


using namespace System;


public ref class XscCompiler
{

    public:

        //! Shader target enumeration.
        enum class ShaderTarget
        {
            Undefined,                      //!< Undefined shader target.

            VertexShader,                   //!< Vertex shader.
            TessellationControlShader,      //!< Tessellation-control (also Hull-) shader.
            TessellationEvaluationShader,   //!< Tessellation-evaluation (also Domain-) shader.
            GeometryShader,                 //!< Geometry shader.
            FragmentShader,                 //!< Fragment (also Pixel-) shader.
            ComputeShader,                  //!< Compute shader.
        };

        //! Input shader version enumeration.
        enum class InputShaderVersion
        {
            HLSL3   = 3,            //!< HLSL Shader Model 3.0 (DirectX 9).
            HLSL4   = 4,            //!< HLSL Shader Model 4.0 (DirectX 10).
            HLSL5   = 5,            //!< HLSL Shader Model 5.0 (DirectX 11).

            GLSL    = 0x0000ffff,   //!< GLSL (OpenGL).
            ESSL    = 0x0001ffff,   //!< GLSL (OpenGL ES).
            VKSL    = 0x0002ffff,   //!< GLSL (Vulkan).
        };

        //! Output shader version enumeration.
        enum class OutputShaderVersion
        {
            GLSL110 = 110,                  //!< GLSL 1.10 (OpenGL 2.0).
            GLSL120 = 120,                  //!< GLSL 1.20 (OpenGL 2.1).
            GLSL130 = 130,                  //!< GLSL 1.30 (OpenGL 3.0).
            GLSL140 = 140,                  //!< GLSL 1.40 (OpenGL 3.1).
            GLSL150 = 150,                  //!< GLSL 1.50 (OpenGL 3.2).
            GLSL330 = 330,                  //!< GLSL 3.30 (OpenGL 3.3).
            GLSL400 = 400,                  //!< GLSL 4.00 (OpenGL 4.0).
            GLSL410 = 410,                  //!< GLSL 4.10 (OpenGL 4.1).
            GLSL420 = 420,                  //!< GLSL 4.20 (OpenGL 4.2).
            GLSL430 = 430,                  //!< GLSL 4.30 (OpenGL 4.3).
            GLSL440 = 440,                  //!< GLSL 4.40 (OpenGL 4.4).
            GLSL450 = 450,                  //!< GLSL 4.50 (OpenGL 4.5).
            GLSL    = 0x0000ffff,           //!< Auto-detect minimal required GLSL version (for OpenGL 2+).

            ESSL100 = (0x00010000 + 100),   //!< ESSL 1.00 (OpenGL ES 2.0). \note Currently not supported!
            ESSL300 = (0x00010000 + 300),   //!< ESSL 3.00 (OpenGL ES 3.0). \note Currently not supported!
            ESSL310 = (0x00010000 + 310),   //!< ESSL 3.10 (OpenGL ES 3.1). \note Currently not supported!
            ESSL320 = (0x00010000 + 320),   //!< ESSL 3.20 (OpenGL ES 3.2). \note Currently not supported!
            ESSL    = 0x0001ffff,           //!< Auto-detect minimum required ESSL version (for OpenGL ES 2+). \note Currently not supported!

            VKSL450 = (0x00020000 + 450),   //!< VKSL 4.50 (Vulkan 1.0).
            VKSL    = 0x0002ffff,           //!< Auto-detect minimum required VKSL version (for Vulkan/SPIR-V).
        };

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
        enum class Comparison
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
        ref class SamplerState
        {

            public:

                SamplerState()
                {
                    TextureFilter   = Filter::MinMagMipLinear;
                    AddressU        = TextureAddressMode::Clamp;
                    AddressV        = TextureAddressMode::Clamp;
                    AddressW        = TextureAddressMode::Clamp;
                    MipLODBias      = 0.0f;
                    MaxAnisotropy   = 1u;
                    ComparisonFunc  = Comparison::Never;
                    BorderColor     = gcnew array<float> { 0.0f, 0.0f, 0.0f, 0.0f };
                    MinLOD          = -std::numeric_limits<float>::max();
                    MaxLOD          = std::numeric_limits<float>::max();
                }

                property Filter             TextureFilter;
                property TextureAddressMode AddressU;
                property TextureAddressMode AddressV;
                property TextureAddressMode AddressW;
                property float              MipLODBias;
                property unsigned int       MaxAnisotropy;
                property Comparison         ComparisonFunc;
                property array<float>^      BorderColor;
                property float              MinLOD;
                property float              MaxLOD;

        };

        XscCompiler()
        {
        }

        ~XscCompiler()
        {
        }

        /// Returns the compiler version.
        property String ^ Version
        {
            String ^ get()
            {
                return gcnew String(XSC_VERSION_STRING);
            }
        }

        /// Returns a dictionary of all supported GLSL extensions with their minimum required version number.
        property Collections::Generic::Dictionary<String^, int>^ GLSLExtensionEnumeration
        {
            Collections::Generic::Dictionary<String^, int>^ get()
            {
                auto dict = gcnew Collections::Generic::Dictionary<String^, int>();

                for (const auto it : Xsc::GetGLSLExtensionEnumeration())
                    dict->Add(gcnew String(it.first.c_str()), it.second);

                return dict;
            }
        }


    private:



};





// ================================================================================
