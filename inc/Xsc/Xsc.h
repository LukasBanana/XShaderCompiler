/*
 * Xsc.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_XSC_H
#define XSC_XSC_H


#include "Export.h"
#include "Log.h"
#include "IncludeHandler.h"
#include "Targets.h"
#include "Version.h"
#include "Reflection.h"

#include <string>
#include <vector>
#include <map>
#include <istream>
#include <ostream>
#include <memory>


/**
\mainpage
Welcome to the XShaderCompiler, Version 0.02 Alpha

Here is a quick start example:
\code
#include <Xsc/Xsc.h>
#include <fstream>

int main()
{
    // Open input and output streams
    auto inputStream = std::make_shared<std::ifstream>("Example.hlsl");
    std::ofstream outputStream("Example.VS.vert");

    // Initialize shader input descriptor structure
    Xsc::ShaderInput inputDesc;
    {
        inputDesc.sourceCode     = inputStream;
        inputDesc.shaderVersion  = Xsc::InputShaderVersion::HLSL5;
        inputDesc.entryPoint     = "VS";
        inputDesc.shaderTarget   = Xsc::ShaderTarget::VertexShader;
    }

    // Initialize shader output descriptor structure
    Xsc::ShaderOutput outputDesc;
    {
        outputDesc.sourceCode    = &outputStream;
        outputDesc.shaderVersion = Xsc::OutputShaderVersion::GLSL330;
    }

    // Compile HLSL code into GLSL
    Xsc::StdLog log;
    bool result = Xsc::CompileShader(inputDesc, outputDesc, &log);

    // Show compilation status
    if (result)
        std::cout << "Compilation successful" << std::endl;
    else
        std::cerr << "Compilation failed" << std::endl;

    return 0;
}
\endcode
*/


//! Main XShaderCompiler namespace
namespace Xsc
{


//! Formatting descriptor structure for the output shader.
struct Formatting
{
    //! Indentation string for code generation. By default std::string(4, ' ').
    std::string indent              = "    ";

    //! If true, blank lines are allowed. By default true.
    bool        blanks              = true;

    //! If true, line marks are allowed. By default false.
    bool        lineMarks           = false;

    //! If true, wrapper functions for special intrinsics are written in a compact formatting (i.e. all in one line). By default true.
    bool        compactWrappers     = true;

    //! If true, scopes are always written in braces. By default false.
    bool        alwaysBracedScopes  = false;

    //! If true, the '{'-braces for an open scope gets its own line. If false, braces are written like in Java coding conventions. By default true.
    bool        newLineOpenScope    = true;

    //! If true, auto-formatting of line separation is allowed. By default true.
    bool        lineSeparation      = true;
};

//! Structure for additional translation options.
struct Options
{
    //! True if warnings are allowed. By default false.
    bool warnings                   = false;

    //! If true, little code optimizations are performed. By default false.
    bool optimize                   = false;

    //! If true, only the preprocessed source code will be written out. By default false.
    bool preprocessOnly             = false;

    //! If true, the source code is only validated, but no output code will be generated. By default false.
    bool validateOnly               = false;

    //! If true, the shader output may contain GLSL extensions, if the target shader version is too low. By default false.
    bool allowExtensions            = false;

    //! If true, explicit binding slots are enabled. By default false.
    bool explicitBinding            = false;

    //! If true, commentaries are preserved for each statement. By default false.
    bool preserveComments           = false;

    //! If true, intrinsics are prefered to be implemented as wrappers (instead of inlining). By default false.
    bool preferWrappers             = false;

    //! If true, array initializations will be unrolled. By default false.
    bool unrollArrayInitializers    = false;

    //! If true, code obfuscation is performed. By default false.
    bool obfuscation                = false;

    //! If true, the AST (Abstract Syntax Tree) will be written to the log output. By default false.
    bool showAST                    = false;

    //! If true, the timings of the different compilation processes are written to the log output. By default false.
    bool showTimes                  = false;
};

//! Shader input descriptor structure.
struct ShaderInput
{
    //! Specifies the filename of the input shader code. This is an optional attribute, and only a hint to the compiler.
    std::string                     filename;

    //! Specifies the input stream. This must be valid HLSL code.
    std::shared_ptr<std::istream>   sourceCode;

    //! Specifies the input shader version (e.g. InputShaderVersion::HLSL5 for "HLSL 5"). By default InputShaderVersion::HLSL5.
    InputShaderVersion              shaderVersion   = InputShaderVersion::HLSL5;
    
    //! Specifies the target shader (Vertex, Fragment etc.). By default ShaderTarget::Undefined.
    ShaderTarget                    shaderTarget    = ShaderTarget::Undefined;

    //! Specifies the HLSL shader entry point.
    std::string                     entryPoint;

    /**
    \brief Specifies the secondary HLSL shader entry point.
    \remarks This is only used for a Tessellation-Control Shader (alias Hull Shader) entry point,
    when a Tessellation-Control Shader (alias Domain Shader) is the output target.
    This is required to translate all Tessellation-Control attributes (i.e. "partitioning" and "outputtopology")
    to the Tessellation-Evaluation output shader. If this is empty, the default values for these attributes are used.
    */
    std::string                     secondaryEntryPoint;

    /**
    \brief Optional pointer to the implementation of the "IncludeHandler" interface. By default null.
    \remarks If this is null, the default include handler will be used, which will include files with the STL input file streams.
    */
    IncludeHandler*                 includeHandler  = nullptr;
};

//! Shader output descriptor structure.
struct ShaderOutput
{
    //! Specifies the filename of the output shader code. This is an optional attribute, and only a hint to the compiler.
    std::string         filename;

    //! Specifies the output stream. This will contain the output GLSL code. This must not be null when passed to the "CompileShader" function!
    std::ostream*       sourceCode          = nullptr;

    //! Specifies the output shader version. By default OutputShaderVersion::GLSL (to auto-detect minimum required version).
    OutputShaderVersion shaderVersion       = OutputShaderVersion::GLSL;
    
    //! Prefix string for name mangling. By default "xsc_".
    std::string         nameManglingPrefix  = "xsc_";

    //! Output code formatting descriptor.
    Formatting          formatting;

    //! Additional options to configure the code generation.
    Options             options;
};

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
XSC_EXPORT bool CompileShader(
    const ShaderInput&          inputDesc,
    const ShaderOutput&         outputDesc,
    Log*                        log             = nullptr,
    Reflection::ReflectionData* reflectionData  = nullptr
);


} // /namespace Xsc


#endif



// ================================================================================