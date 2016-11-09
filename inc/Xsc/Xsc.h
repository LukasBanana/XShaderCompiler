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
#include <istream>
#include <ostream>
#include <memory>


namespace Xsc
{


//! Structure for additional translation options.
struct Options
{
    //! Indentation string for code generation. By default std::string(4, ' ').
    std::string indent          = "    ";
    
    /**
    Prefix string for all local variables. By default "_".
    \remarks This prefix is used because GLSL does not allow interface blocks as
    input for vertex shaders or output for fragment shaders.
    Thus some identifiers of local variables may overlap with input variables.
    This prefix is added to all local function variables.
    \todo Remove this option and handle this workaround in an autmatic manner.
    */
    std::string prefix          = "_";

    //! True if warnings are allowed. By default false.
    bool        warnings        = false;

    //! True if blanks are allowed. By default true.
    bool        blanks          = true;

    //! True if line marks are allowed. By default false.
    bool        lineMarks       = false;

    //! If true, the abstract syntax tree (AST) will be printed as debug output. By default false.
    bool        dumpAST         = false;

    //! If true, only the preprocessed source code will be written out.
    bool        preprocessOnly  = false;

    //! If true, (almost) all comments are kept in the output code. By default true.
    bool        keepComments    = true;
};

//! Shader input descriptor structure.
struct ShaderInput
{
    //! Specifies the input stream. This must be valid HLSL code.
    std::shared_ptr<std::istream>   sourceCode;

    //! Specifies the input shader version (e.g. InputShaderVersion::HLSL5 for "HLSL 5"). By default InputShaderVersion::HLSL5.
    InputShaderVersion              shaderVersion       = InputShaderVersion::HLSL5;

    //! Specifies the HLSL shader entry point. If may also be empty.
    std::string                     entryPoint;
    
    //! Specifies the target shader (Vertex, Fragment etc.).
    ShaderTarget                    shaderTarget;

    /**
    \brief Optional pointer to the implementation of the "IncludeHandler" interface. By default null.
    \remarks If this is null, the default include handler will be used, which will include files with the STL input file streams.
    */
    IncludeHandler*                 includeHandler      = nullptr;
};

//! Shader output descriptor structure.
struct ShaderOutput
{
    //! Specifies the output stream. This will contain the output GLSL code. This must not be null!
    std::ostream*                   sourceCode          = nullptr;

    //! Specifies the output shader version (e.g. for "GLSL 1.20" use 'OutputShaderVersion::GLSL120'). By default OutputShaderVersion::GLSL330.
    OutputShaderVersion             shaderVersion       = OutputShaderVersion::GLSL330;

    //! Additional options to configure the code generation.
    Options                         options;
};

/**
\brief Cross compiles the shader code from the specified input stream into the specified output shader code.
\param[in] inputDesc Input shader code descriptor.
\param[in] outputDesc Output shader code descriptor.
\param[in] log Optional pointer to an output log. Inherit from the "Log" class interface.
\return True if the code has been translated successfully.
\see ShaderInput
\see ShaderOutput
\see Log
*/
HTLIB_EXPORT bool CompileShader(
    const ShaderInput& inputDesc,
    const ShaderOutput& outputDesc,
    Log* log = nullptr
);


} // /namespace Xsc


#endif



// ================================================================================