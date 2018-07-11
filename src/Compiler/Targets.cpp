/*
 * Targets.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <Xsc/Targets.h>
#include "GLSLExtensions.h"


namespace Xsc
{


XSC_EXPORT std::string ToString(const ShaderTarget target)
{
    switch (target)
    {
        case ShaderTarget::Undefined:                       return "Undefined";
        case ShaderTarget::VertexShader:                    return "Vertex Shader";
        case ShaderTarget::FragmentShader:                  return "Fragment Shader";
        case ShaderTarget::GeometryShader:                  return "Geometry Shader";
        case ShaderTarget::TessellationControlShader:       return "Tessellation-Control Shader";
        case ShaderTarget::TessellationEvaluationShader:    return "Tessellation-Evaluation Shader";
        case ShaderTarget::ComputeShader:                   return "Compute Shader";
    }
    return "";
}

XSC_EXPORT std::string ToString(const InputShaderVersion shaderVersion)
{
    switch (shaderVersion)
    {
        case InputShaderVersion::Cg:    return "Cg";

        case InputShaderVersion::HLSL3: return "HLSL 3.0";
        case InputShaderVersion::HLSL4: return "HLSL 4.0";
        case InputShaderVersion::HLSL5: return "HLSL 5.0";
        case InputShaderVersion::HLSL6: return "HLSL 6.0";

        case InputShaderVersion::GLSL:  return "GLSL";
        case InputShaderVersion::ESSL:  return "ESSL";
        case InputShaderVersion::VKSL:  return "VKSL";
    }
    return "";
}

XSC_EXPORT std::string ToString(const OutputShaderVersion shaderVersion)
{
    switch (shaderVersion)
    {
        case OutputShaderVersion::GLSL110:  return "GLSL 1.10";
        case OutputShaderVersion::GLSL120:  return "GLSL 1.20";
        case OutputShaderVersion::GLSL130:  return "GLSL 1.30";
        case OutputShaderVersion::GLSL140:  return "GLSL 1.40";
        case OutputShaderVersion::GLSL150:  return "GLSL 1.50";
        case OutputShaderVersion::GLSL330:  return "GLSL 3.30";
        case OutputShaderVersion::GLSL400:  return "GLSL 4.00";
        case OutputShaderVersion::GLSL410:  return "GLSL 4.10";
        case OutputShaderVersion::GLSL420:  return "GLSL 4.20";
        case OutputShaderVersion::GLSL430:  return "GLSL 4.30";
        case OutputShaderVersion::GLSL440:  return "GLSL 4.40";
        case OutputShaderVersion::GLSL450:  return "GLSL 4.50";
        case OutputShaderVersion::GLSL460:  return "GLSL 4.60";
        case OutputShaderVersion::GLSL:     return "GLSL";

        case OutputShaderVersion::ESSL100:  return "ESSL 1.00";
        case OutputShaderVersion::ESSL300:  return "ESSL 3.00";
        case OutputShaderVersion::ESSL310:  return "ESSL 3.10";
        case OutputShaderVersion::ESSL320:  return "ESSL 3.20";
        case OutputShaderVersion::ESSL:     return "ESSL";

        case OutputShaderVersion::VKSL450:  return "VKSL 4.50";
        case OutputShaderVersion::VKSL:     return "VKSL";
    }
    return "";
}

XSC_EXPORT std::string ToString(const IntermediateLanguage language)
{
    switch (language)
    {
        case IntermediateLanguage::SPIRV: return "SPIR-V";
    }
    return "";
}

XSC_EXPORT bool IsLanguageHLSL(const InputShaderVersion shaderVersion)
{
    return (shaderVersion >= InputShaderVersion::Cg && shaderVersion <= InputShaderVersion::HLSL6);
}

XSC_EXPORT bool IsLanguageGLSL(const InputShaderVersion shaderVersion)
{
    return (shaderVersion >= InputShaderVersion::GLSL && shaderVersion <= InputShaderVersion::VKSL);
}

XSC_EXPORT bool IsLanguageGLSL(const OutputShaderVersion shaderVersion)
{
    return ((shaderVersion >= OutputShaderVersion::GLSL110 && shaderVersion <= OutputShaderVersion::GLSL460) || shaderVersion == OutputShaderVersion::GLSL);
}

XSC_EXPORT bool IsLanguageESSL(const OutputShaderVersion shaderVersion)
{
    return ((shaderVersion >= OutputShaderVersion::ESSL100 && shaderVersion <= OutputShaderVersion::ESSL320) || shaderVersion == OutputShaderVersion::ESSL);
}

XSC_EXPORT bool IsLanguageVKSL(const OutputShaderVersion shaderVersion)
{
    return (shaderVersion == OutputShaderVersion::VKSL450 || shaderVersion == OutputShaderVersion::VKSL);
}

XSC_EXPORT const std::map<std::string, int>& GetGLSLExtensionEnumeration()
{
    return GetGLSLExtensionVersionMap();
}


} // /namespace Xsc



// ================================================================================
