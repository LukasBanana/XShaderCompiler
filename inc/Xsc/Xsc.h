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

#include <string>
#include <vector>
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
    std::ofstream outputStream("Example.vertex.glsl");

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
    std::string indent      = "    ";
    
    /**
    \brief Prefix string for name mangling. By default "xsc_".
    \remarks This prefix is used because GLSL does not allow interface blocks as
    input for vertex shaders or output for fragment shaders.
    Thus some identifiers of local variables may overlap with input variables.
    This prefix is added to all local function variables.
    */
    std::string prefix      = "xsc_";

    //! True if blanks are allowed. By default true.
    bool        blanks      = true;

    //! True if line marks are allowed. By default false.
    bool        lineMarks   = false;
};

//! Structure for additional translation options.
struct Options
{
    //! True if warnings are allowed. By default false.
    bool warnings           = false;

    //! If true, little code optimizations are performed. By default false.
    bool optimize           = false;

    //! If true, only the preprocessed source code will be written out. By default false.
    bool preprocessOnly     = false;

    //! If true, the source code is only validated, but no output code will be generated. By default false.
    bool validateOnly       = false;

    //! If true, the shader output may contain GLSL extensions, if the target shader version is too low. By default false.
    bool allowExtensions    = false;
    
    //! If true, explicit binding slots are enabled. By default false.
    bool explicitBinding    = false;

    //! If true, commentaries are preserved for each statement. By default false.
    bool preserveComments   = false;

    //! If true, intrinsics are prefered to be implemented as wrappers (instead of inlining). By default false.
    bool preferWrappers     = false;

    //! If true, the AST (Abstract Syntax Tree) will be written to the log output. By default false.
    bool showAST            = false;

    //! If true, the timings of the different compilation processes are written to the log output. By default false.
    bool showTimes          = false;
};

//! Structure for shader output statistics (e.g. texture/buffer binding points).
struct Statistics
{
    struct Binding
    {
        //! Identifier of the binding point.
        std::string ident;

        //! Zero based binding point or location. If this is -1, the location has not been set explicitly.
        int         location;
    };

    //! All defined macros after pre-processing.
    std::vector<std::string>    macros;

    //! Texture bindings.
    std::vector<Binding>        textures;

    //! Constant buffer bindings.
    std::vector<Binding>        constantBuffers;

    //! Fragment shader output targets.
    std::vector<Binding>        fragmentTargets;
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

    //! Specifies the HLSL shader entry point.
    std::string                     entryPoint;
    
    //! Specifies the target shader (Vertex, Fragment etc.). By default ShaderTarget::Undefined.
    ShaderTarget                    shaderTarget    = ShaderTarget::Undefined;

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
    std::ostream*       sourceCode      = nullptr;

    //! Specifies the output shader version. By default OutputShaderVersion::GLSL (to auto-detect minimum required version).
    OutputShaderVersion shaderVersion   = OutputShaderVersion::GLSL;

    //! Output code formatting descriptor.
    Formatting          formatting;

    //! Additional options to configure the code generation.
    Options             options;

    //! Optional output statistics. By default null.
    Statistics*         statistics      = nullptr;
};

/**
\brief Cross compiles the shader code from the specified input stream into the specified output shader code.
\param[in] inputDesc Input shader code descriptor.
\param[in] outputDesc Output shader code descriptor.
\param[in] log Optional pointer to an output log. Inherit from the "Log" class interface.
\return True if the code has been translated successfully.
\throw std::invalid_argument If either the input or output streams are null.
\see ShaderInput
\see ShaderOutput
\see Log
*/
XSC_EXPORT bool CompileShader(
    const ShaderInput& inputDesc,
    const ShaderOutput& outputDesc,
    Log* log = nullptr
);


} // /namespace Xsc


#endif



// ================================================================================