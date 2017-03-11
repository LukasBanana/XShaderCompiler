/*
 * XscC.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_XSC_C_H
#define XSC_XSC_C_H


#include <Xsc/Export.h>
#include <Xsc/Version.h>
#include "TargetsC.h"
#include <stdbool.h>


#ifdef __cplusplus
extern "C" {
#endif


//! Formatting descriptor structure for the output shader.
struct XscFormatting
{
    //! Indentation string for code generation. By default 4 spaces.
    const char* indent;

    //! If true, blank lines are allowed. By default true.
    bool        blanks;

    //! If true, line marks are allowed. By default false.
    bool        lineMarks;

    //! If true, wrapper functions for special intrinsics are written in a compact formatting (i.e. all in one line). By default false.
    bool        compactWrappers;

    //! If true, scopes are always written in braces. By default false.
    bool        alwaysBracedScopes;

    //! If true, the '{'-braces for an open scope gets its own line. If false, braces are written like in Java coding conventions. By default true.
    bool        newLineOpenScope;

    //! If true, auto-formatting of line separation is allowed. By default true.
    bool        lineSeparation;
};

//! Structure for additional translation options.
struct XscOptions
{
    //! True if warnings are allowed. By default false.
    bool warnings;

    //! If true, little code optimizations are performed. By default false.
    bool optimize;

    //! If true, only the preprocessed source code will be written out. By default false.
    bool preprocessOnly;

    //! If true, the source code is only validated, but no output code will be generated. By default false.
    bool validateOnly;

    //! If true, the shader output may contain GLSL extensions, if the target shader version is too low. By default false.
    bool allowExtensions;

    //! If true, explicit binding slots are enabled. By default false.
    bool explicitBinding;

    //! If true, commentaries are preserved for each statement. By default false.
    bool preserveComments;

    //! If true, intrinsics are prefered to be implemented as wrappers (instead of inlining). By default false.
    bool preferWrappers;

    //! If true, array initializations will be unrolled. By default false.
    bool unrollArrayInitializers;

    //! If true, matrices have row-major alignment. Otherwise the matrices have column-major alignment. By default false.
    bool rowMajorAlignment;

    //! If true, code obfuscation is performed. By default false.
    bool obfuscate;

    //! If true, the AST (Abstract Syntax Tree) will be written to the log output. By default false.
    bool showAST;

    //! If true, the timings of the different compilation processes are written to the log output. By default false.
    bool showTimes;
};

//! Name mangling descriptor structure for shader input/output variables (also referred to as "varyings"), temporary variables, and reserved keywords.
struct XscNameMangling
{
    //! Name mangling prefix for shader input variables. By default "xsv_".
    const char* inputPrefix;

    //! Name mangling prefix for shader output variables. By default "xsv_".
    const char* outputPrefix;

    /**
    \brief Name mangling prefix for reserved words (such as "texture", "main", "sin" etc.). By default "xsr_".
    \remarks This must not be equal to any of the other prefixes and it must not be empty.
    */
    const char* reservedWordPrefix;

    /**
    \brief Name mangling prefix for temporary variables. By default "xst_".
    \remarks This must not be equal to any of the other prefixes and it must not be empty.
    */
    const char* temporaryPrefix;

    /**
    If true, shader input/output variables are always renamed to their semantics,
    even for vertex input and fragment output. Otherwise, their original identifiers are used.
    */
    const bool* useAlwaysSemantics;
};

//! Shader input descriptor structure.
struct XscShaderInput
{
    //! Specifies the filename of the input shader code. This is an optional attribute, and only a hint to the compiler. By default NULL.
    const char*                     filename;

    //! Specifies the input stream. This must be valid HLSL code.
    //std::shared_ptr<std::istream>   sourceCode;

    //! Specifies the input shader version (e.g. XscHLSL5 for "HLSL 5"). By default XscHLSL5.
    enum XscShaderVersion           shaderVersion;
    
    //! Specifies the target shader (Vertex, Fragment etc.). By default XscUndefinedShader.
    enum XscShaderTarget            shaderTarget;

    //! Specifies the HLSL shader entry point. By default NULL.
    const char*                     entryPoint;

    /**
    \brief Specifies the secondary HLSL shader entry point.
    \remarks This is only used for a Tessellation-Control Shader (alias Hull Shader) entry point,
    when a Tessellation-Control Shader (alias Domain Shader) is the output target.
    This is required to translate all Tessellation-Control attributes (i.e. "partitioning" and "outputtopology")
    to the Tessellation-Evaluation output shader. If this is empty, the default values for these attributes are used.
    */
    const char*                     secondaryEntryPoint;

    /**
    \brief Optional pointer to the implementation of the "IncludeHandler" interface. By default null.
    \remarks If this is null, the default include handler will be used, which will include files with the STL input file streams.
    */
    //struct XscIncludeHandler*       includeHandler  = nullptr;
};

//! Vertex shader semantic (or rather attribute) layout structure.
struct XscVertexSemantic
{
    const char* semantic;
    int         location;
};

//! Shader output descriptor structure.
struct XscShaderOutput
{
    //! Specifies the filename of the output shader code. This is an optional attribute, and only a hint to the compiler.
    const char*                     filename;

    //! Specifies the output stream. This will contain the output GLSL code. This must not be null when passed to the "CompileShader" function!
    //std::ostream*                   sourceCode;

    //! Specifies the output shader version. By default XscGLSL (to auto-detect minimum required version).
    enum XscShaderVersion           shaderVersion;

    //! Optional list of vertex semantic layouts, to bind a vertex attribute (semantic name) to a location index (only used when 'explicitBinding' is true). By default NULL.
    const struct XscVertexSemantic* vertexSemantics;

    //! Number of elements the 'vertexSemantics' member points to. By default 0.
    size_t                          vertexSemanticsCount;

    //! Additional options to configure the code generation.
    struct XscOptions               options;

    //! Output code formatting descriptor.
    struct XscFormatting            formatting;
    
    //! Specifies the options for name mangling.
    struct XscNameMangling          nameMangling;
};

/**
\brief Initializes the specified descriptor structures to their default values.
\param[out] inputDesc Input shader code descriptor. If NULL, this structure is not initialized.
\param[out] outputDesc Output shader code descriptor. If NULL, this structure is not initialized.
*/
XSC_EXPORT void XscInitialize(struct XscShaderInput* inputDesc, struct XscShaderOutput* outputDesc);

/**
\brief Cross compiles the shader code from the specified input stream into the specified output shader code.
\param[in] inputDesc Input shader code descriptor.
\param[in] outputDesc Output shader code descriptor.
\param[in] log Optional pointer to an output log. Inherit from the "Log" class interface. By default null.
\param[out] reflectionData Optional pointer to a code reflection data structure. By default null.
\return True if the code has been translated successfully.
\throw std::invalid_argument If either the input or output streams are null.
\see ShaderInput
\see ShaderOutput
\see Log
\see ReflectionData
*/
XSC_EXPORT bool XscCompileShader(
    const struct XscShaderInput*    inputDesc,
    const struct XscShaderOutput*   outputDesc/*,
    XscLog*                         log,
    XscReflectionData*              reflectionData*/
);


#ifdef __cplusplus
} // /extern "C"
#endif


#endif



// ================================================================================