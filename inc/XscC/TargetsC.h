/*
 * TargetsC.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_TARGETS_C_H
#define XSC_TARGETS_C_H


#include <Xsc/Export.h>
#include <stdbool.h>
#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif


//! Shader target enumeration.
enum XscShaderTarget
{
    XscUndefinedShader,                 //!< Undefined shader target.

    XscVertexShader,                   //!< Vertex shader.
    XscTessellationControlShader,      //!< Tessellation-control (also Hull-) shader.
    XscTessellationEvaluationShader,   //!< Tessellation-evaluation (also Domain-) shader.
    XscGeometryShader,                 //!< Geometry shader.
    XscFragmentShader,                 //!< Fragment (also Pixel-) shader.
    XscComputeShader,                  //!< Compute shader.
};

//! Input/output shader version enumeration.
enum XscShaderVersion
{
    XscHLSL3    = 3,
    XscHLSL4    = 4,
    XscHLSL5    = 5,

    XscGLSL110  = 110,                 //!< GLSL 1.10 (OpenGL 2.0).
    XscGLSL120  = 120,                 //!< GLSL 1.20 (OpenGL 2.1).
    XscGLSL130  = 130,                 //!< GLSL 1.30 (OpenGL 3.0).
    XscGLSL140  = 140,                 //!< GLSL 1.40 (OpenGL 3.1).
    XscGLSL150  = 150,                 //!< GLSL 1.50 (OpenGL 3.2).
    XscGLSL330  = 330,                 //!< GLSL 3.30 (OpenGL 3.3).
    XscGLSL400  = 400,                 //!< GLSL 4.00 (OpenGL 4.0).
    XscGLSL410  = 410,                 //!< GLSL 4.10 (OpenGL 4.1).
    XscGLSL420  = 420,                 //!< GLSL 4.20 (OpenGL 4.2).
    XscGLSL430  = 430,                 //!< GLSL 4.30 (OpenGL 4.3).
    XscGLSL440  = 440,                 //!< GLSL 4.40 (OpenGL 4.4).
    XscGLSL450  = 450,                 //!< GLSL 4.50 (OpenGL 4.5).
    XscGLSL     = 0x0000ffff,          //!< Auto-detect minimal required GLSL version (for OpenGL 2+).

    XscESSL100  = (0x00010000 + 100),  //!< ESSL 1.00 (OpenGL ES 2.0). \note Currently not supported!
    XscESSL300  = (0x00010000 + 300),  //!< ESSL 3.00 (OpenGL ES 3.0). \note Currently not supported!
    XscESSL310  = (0x00010000 + 310),  //!< ESSL 3.10 (OpenGL ES 3.1). \note Currently not supported!
    XscESSL320  = (0x00010000 + 320),  //!< ESSL 3.20 (OpenGL ES 3.2). \note Currently not supported!
    XscESSL     = 0x0001ffff,          //!< Auto-detect minimum required ESSL version (for OpenGL ES 2+). \note Currently not supported!

    XscVKSL450  = (0x00020000 + 450),  //!< VKSL 4.50 (Vulkan 1.0).
    XscVKSL     = 0x0002ffff,           //!< Auto-detect minimum required VKSL version (for Vulkan/SPIR-V).
};


//! Returns the specified shader target as string.
XSC_EXPORT void XscShaderTargetToString(const enum XscShaderTarget target, char* str, size_t maxSize);

//! Returns the specified shader input version as string.
XSC_EXPORT void XscInputShaderVersionToString(const enum XscShaderVersion shaderVersion, char* str, size_t maxSize);

//! Returns the specified shader output version as string.
XSC_EXPORT void XscOutputShaderVersionToString(const enum XscShaderVersion shaderVersion, char* str, size_t maxSize);

//! Returns true if the shader input version specifies HLSL (for DirectX).
XSC_EXPORT bool XscIsInputLanguageHLSL(const enum XscShaderVersion shaderVersion);

//! Returns true if the shader input version specifies GLSL (for OpenGL, OpenGL ES, and Vulkan).
XSC_EXPORT bool XscIsInputLanguageGLSL(const enum XscShaderVersion shaderVersion);

//! Returns true if the shader output version specifies GLSL (for OpenGL 2+).
XSC_EXPORT bool XscIsOutputLanguageGLSL(const enum XscShaderVersion shaderVersion);

//! Returns true if the shader output version specifies ESSL (for OpenGL ES 2+).
XSC_EXPORT bool XscIsOutputLanguageESSL(const enum XscShaderVersion shaderVersion);

//! Returns true if the shader output version specifies VKSL (for Vulkan).
XSC_EXPORT bool XscIsOutputLanguageVKSL(const enum XscShaderVersion shaderVersion);

/**
\brief Returns the enumeration of all supported GLSL extensions as a map of extension name and version number.
\param[in] iterator Specifies the iterator. This must be NULL, to get the first element, or the value previously returned by this function.
\param[out] extension Specifies the output string of the extension name.
\param[in] maxSize Specifies the maximal size of the extension name string including the null terminator.
\param[out] version Specifies the output extension version.
\remarks Here is a usage example:
\code
char extension[256];
int version;

// Get first extension
void* iterator = XscGetGLSLExtensionEnumeration(NULL, extension, 256, &version);

while (iterator != NULL)
{
    // Print extension name and version
    printf("%s ( %d )\n", extension, version);
    
    // Get next extension
    iterator = XscGetGLSLExtensionEnumeration(iterator, extension, 256, &version);
}
\endcode
\note This can NOT be used in a multi-threaded environment!
*/
XSC_EXPORT void* XscGetGLSLExtensionEnumeration(void* iterator, char* extension, size_t maxSize, int* version);


#ifdef __cplusplus
} // /extern "C"
#endif


#endif



// ================================================================================