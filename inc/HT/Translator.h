/*
 * Translator.h
 * 
 * This file is part of the "HLSL Translator" (Copyright (c) 2014 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef HTLIB_TRANSLATOR_H
#define HTLIB_TRANSLATOR_H


#include "HT/Export.h"
#include "HT/Logger.h"
#include "HT/Targets.h"
#include "HT/Version.h"

#include <string>
#include <istream>
#include <ostream>
#include <memory>


namespace HTLib
{


//! Structure for additional translation options.
struct Options
{
    //! Indentation string for code generation. By default std::string(4, ' ').
    std::string indent      = "    ";
    
    /**
    Prefix string for all local variables. By default "_".
    \remarks This prefix is used because GLSL does not allow interface blocks as
    input for vertex shaders or output for fragment shaders.
    Thus some identifiers of local variables may overlap with input variables.
    This prefix is added to all local function variables.
    */
    std::string prefix      = "_";

    //! True if warnings are allowed. By default false.
    bool        warnings    = false;

    //! True if blanks are allowed. By default true.
    bool        blanks      = true;

    //! True if line marks are allowed. By default false.
    bool        lineMarks   = false;

    //! If true, the abstract syntax tree (AST) will be printed as debug output. By default false.
    bool        dumpAST     = false;

    //! If true, (almost) all comments are kept in the output code. By default true.
    bool        keepComments    = true;
};

//! Interface for handling new include streams.
class _HT_EXPORT_ IncludeHandler
{
    
    public:
        
        virtual ~IncludeHandler()
        {
        }

        /**
        Inlcudes the specified file and/or modifies the filename.
        \param[in,out] includeName Specifies the include filename. This name can be modified.
        If the return value is not null, a modification has no effect.
        \return Shared pointer to the new input stream. This may also be null.
        In this case the '#include' directive will be modified with the output value of "includeName".
        \remarks A modification of "includeName" can be used to modify the following include directive:
        \code
        // HLSL Code:
        #include "CoreShader.hlsl"
        \endcode
        ... and transform it to:
        \code
        // GLSL Code:
        #include "CoreShader.glsl"
        \endcode
        */
        virtual std::shared_ptr<std::istream> Include(std::string& includeName) = 0;

};


/**
Translates the HLSL code from the specified input stream into GLSL code.
\param[in] input Specifies the input stream. This must be valid HLSL code.
\param[out] output Specifies the output stream. This will contain the output GLSL code.
\param[in] entryPoint Specifies the HLSL shader entry point. If may also be empty.
\param[in] shaderTarget Specifies the target shader (Vertex, Fragment etc.).
\param[in] inputShaderVersion Specifies the input shader version (e.g. for "HLSL 5" use 'InputShaderVersions::HLSL5').
\param[in] outputShaderVersion Specifies the output shader version (e.g. for "GLSL 1.20" use 'OutputShaderVersions::GLSL120').
\param[in] includeHandler Optional pointer to the implementation of the "IncludeHandler" interface.
This will be used when an "#include" directive occurs. If such a directive occurs
and this parameter is null, the code generation will fail! By default null.
\param[in] options Additional options to configure the code generation.
\param[in] log Optional pointer to an output log. Inherit from the "Logger" class interface.
\return True if the code has been translated correctly.
\note This translator makes a minimum of contextual analysis.
Therefore wrong HLSL code may be translated into wrong GLSL code!
\see InputShaderVersions
\see OutputShaderVersions
\see IncludeHandler
\see Options
\see Logger
*/
_HT_EXPORT_ bool TranslateHLSLtoGLSL(
    const std::shared_ptr<std::istream>&    input,
    std::ostream&                           output,
    const std::string&                      entryPoint,
    const ShaderTargets                     shaderTarget,
    const InputShaderVersions               inputShaderVersion,
    const OutputShaderVersions              outputShaderVersion,
    IncludeHandler*                         includeHandler = nullptr,
    const Options&                          options = {},
    Logger*                                 log = nullptr
);


} // /namespace HTLib


#endif



// ================================================================================