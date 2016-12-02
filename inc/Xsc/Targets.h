/*
 * Targets.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_TARGETS_H
#define XSC_TARGETS_H


#include "Export.h"
#include <string>


namespace Xsc
{


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
    HLSL3 = 3, //!< HLSL Shader Model 3.0 (DirectX 9).
    HLSL4 = 4, //!< HLSL Shader Model 4.0 (DirectX 10).
    HLSL5 = 5, //!< HLSL Shader Model 5.0 (DirectX 11).
};

//! Output shader version enumeration.
enum class OutputShaderVersion
{
    GLSL110 = 110,  //!< GLSL 1.10 (OpenGL 2.0).
    GLSL120 = 120,  //!< GLSL 1.20 (OpenGL 2.1).
    GLSL130 = 130,  //!< GLSL 1.30 (OpenGL 3.0).
    GLSL140 = 140,  //!< GLSL 1.40 (OpenGL 3.1).
    GLSL150 = 150,  //!< GLSL 1.50 (OpenGL 3.2).
    GLSL330 = 330,  //!< GLSL 3.30 (OpenGL 3.3).
    GLSL400 = 400,  //!< GLSL 4.00 (OpenGL 4.0).
    GLSL410 = 410,  //!< GLSL 4.10 (OpenGL 4.1).
    GLSL420 = 420,  //!< GLSL 4.20 (OpenGL 4.2).
    GLSL430 = 430,  //!< GLSL 4.30 (OpenGL 4.3).
    GLSL440 = 440,  //!< GLSL 4.40 (OpenGL 4.4).
    GLSL450 = 450,  //!< GLSL 4.50 (OpenGL 4.5).
    GLSL    = ~0,   //!< Auto-detect minimal required GLSL version.
};


//! Returns the specified shader target as string.
XSC_EXPORT std::string TargetToString(const ShaderTarget target);

//! Returns the specified shader input version as string.
XSC_EXPORT std::string ShaderVersionToString(const InputShaderVersion shaderVersion);

//! Returns the specified shader output version as string.
XSC_EXPORT std::string ShaderVersionToString(const OutputShaderVersion shaderVersion);


} // /namespace Xsc


#endif



// ================================================================================