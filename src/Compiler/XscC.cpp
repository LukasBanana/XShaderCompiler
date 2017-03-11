/*
 * Xsc.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <Xsc/Xsc.h>
#include <XscC/XscC.h>
#include <string.h>


#ifdef __cplusplus
extern "C" {
#endif


/*
 * Public functions
 */

static void InitializeFormatting(struct XscFormatting* s)
{
}

static void InitializeOptions(struct XscOptions* s)
{
}

static void InitializeNameMangling(struct XscNameMangling* s)
{
}

static void InitializeShaderInput(struct XscShaderInput* s)
{
    //s->filename         = NULL;
    //s->shaderVersion    = ;
}

static void InitializeShaderOutput(struct XscShaderOutput* s)
{
}

XSC_EXPORT void XscInitialize(struct XscShaderInput* inputDesc, struct XscShaderOutput* outputDesc)
{
    if (inputDesc != NULL)
        InitializeShaderInput(inputDesc);
    if (outputDesc != NULL)
        InitializeShaderOutput(outputDesc);
}

XSC_EXPORT bool XscCompileShader(
    const struct XscShaderInput* inputDesc,
    const struct XscShaderOutput* outputDesc/*,
    XscLog* log,
    XscReflectionData* reflectionData*/)
{

    return false;
}

static void CopyStringC(const std::string& src, char* dest, size_t maxSize)
{
    if (dest != NULL)
    {
        if (src.size() < maxSize)
            strncpy(dest, src.c_str(), maxSize);
        else
            memset(dest, 0, maxSize);
    }
}

XSC_EXPORT void XscShaderTargetToString(const enum XscShaderTarget target, char* str, size_t maxSize)
{
    CopyStringC(Xsc::ToString(static_cast<Xsc::ShaderTarget>(target)), str, maxSize);
}

XSC_EXPORT void XscInputShaderVersionToString(const enum XscShaderVersion shaderVersion, char* str, size_t maxSize)
{
    CopyStringC(Xsc::ToString(static_cast<Xsc::InputShaderVersion>(shaderVersion)), str, maxSize);
}

XSC_EXPORT void XscOutputShaderVersionToString(const enum XscShaderVersion shaderVersion, char* str, size_t maxSize)
{
    CopyStringC(Xsc::ToString(static_cast<Xsc::OutputShaderVersion>(shaderVersion)), str, maxSize);
}

XSC_EXPORT bool XscIsInputLanguageHLSL(const enum XscShaderVersion shaderVersion)
{
    return Xsc::IsLanguageHLSL(static_cast<Xsc::InputShaderVersion>(shaderVersion));
}

XSC_EXPORT bool XscIsInputLanguageGLSL(const enum XscShaderVersion shaderVersion)
{
    return Xsc::IsLanguageGLSL(static_cast<Xsc::InputShaderVersion>(shaderVersion));
}

XSC_EXPORT bool XscIsOutputLanguageGLSL(const enum XscShaderVersion shaderVersion)
{
    return Xsc::IsLanguageGLSL(static_cast<Xsc::OutputShaderVersion>(shaderVersion));
}

XSC_EXPORT bool XscIsOutputLanguageESSL(const enum XscShaderVersion shaderVersion)
{
    return Xsc::IsLanguageESSL(static_cast<Xsc::OutputShaderVersion>(shaderVersion));
}

XSC_EXPORT bool XscIsOutputLanguageVKSL(const enum XscShaderVersion shaderVersion)
{
    return Xsc::IsLanguageVKSL(static_cast<Xsc::OutputShaderVersion>(shaderVersion));
}

using GLSLExtensionEnumIterator = std::map<std::string, int>::const_iterator;

XSC_EXPORT void* XscGetGLSLExtensionEnumeration(void* iterator, char* extension, size_t maxSize, int* version)
{
    const auto& extMap = Xsc::GetGLSLExtensionEnumeration();

    /* Get GLSL extension enumeration iterator */
    static GLSLExtensionEnumIterator mapIt;

    if (iterator)
        mapIt = *reinterpret_cast<GLSLExtensionEnumIterator*>(iterator);
    else
        mapIt = extMap.begin();

    if (mapIt != extMap.end())
    {
        /* Return enumeration entry */
        CopyStringC(mapIt->first.c_str(), extension, maxSize);

        if (version)
            *version = mapIt->second;

        /* Move to next element for the next call */
        ++mapIt;

        /* Return reference to iterator */
        return reinterpret_cast<void*>(&mapIt);
    }

    return NULL;
}


#ifdef __cplusplus
} // /extern "C"
#endif



// ================================================================================
