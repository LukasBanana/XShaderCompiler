/*
 * Targets.h
 * 
 * This file is part of the "HLSL Translator" (Copyright (c) 2014 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef HTLIB_TARGETS_H
#define HTLIB_TARGETS_H


namespace HTLib
{


//! Shader target enumeration.
enum class ShaderTarget
{
    /* --- Special types --- */
    CommonShader = 0,           //< Common shader (used as include file).

    /* --- GLSL shader types --- */
    GLSLVertexShader,           //< GLSL vertex shader.
    GLSLGeometryShader,         //< GLSL geometry shader.
    GLSLTessControlShader,      //< GLSL tessellation control (also hull-) shader.
    GLSLTessEvaluationShader,   //< GLSL tessellation evaluation (also domain-) shader.
    GLSLFragmentShader,         //< GLSL fragment (also pixel-) shader.
    GLSLComputeShader,          //< GLSL compute shader.
};

//! Input shader version enumeration.
enum class InputShaderVersions
{
    HLSL3 = 3, //< HLSL Shader Model 3 (DirectX 9).
    HLSL4 = 4, //< HLSL Shader Model 4 (DirectX 10).
    HLSL5 = 5, //< HLSL Shader Model 5 (DirectX 11).
};

//! Output shader version enumeration.
enum class OutputShaderVersions
{
    GLSL110 = 110, //< GLSL 1.10 (OpenGL 2.0).
    GLSL120 = 120, //< GLSL 1.20 (OpenGL 2.1).
    GLSL130 = 130, //< GLSL 1.30 (OpenGL 3.0).
    GLSL140 = 140, //< GLSL 1.40 (OpenGL 3.1).
    GLSL150 = 150, //< GLSL 1.50 (OpenGL 3.2).
    GLSL330 = 330, //< GLSL 3.30 (OpenGL 3.3).
    GLSL400 = 400, //< GLSL 4.00 (OpenGL 4.0).
    GLSL410 = 410, //< GLSL 4.10 (OpenGL 4.1).
    GLSL420 = 420, //< GLSL 4.20 (OpenGL 4.2).
    GLSL430 = 430, //< GLSL 4.30 (OpenGL 4.3).
    GLSL440 = 440, //< GLSL 4.40 (OpenGL 4.4).
    GLSL450 = 450, //< GLSL 4.50 (OpenGL 4.5).
};


} // /namespace HTLib


#endif



// ================================================================================