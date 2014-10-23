/*
 * Translator.h
 * 
 * This file is part of the "HLSL Translator" (Copyright (c) 2014 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __HT_TRANSLATOR_H__
#define __HT_TRANSLATOR_H__


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

    //! True if no blanks are allowed. By default false.
    bool        noBlanks    = false;

    //! True if no line marks are allowed. By default false.
    bool        noLineMarks = false;
};

//! Interface for handling new include streams.
class _HT_EXPORT_ IncludeHandler
{
    
    public:
        
        virtual ~IncludeHandler()
        {
        }

        virtual std::shared_ptr<std::istream> Include(const std::string& includeName) = 0;

};


/**
Translates the specifies HLSL code into GLSL code.
\param[in,out] input Specifies the input stream. This must be valid HLSL code.
\param[in,out] output Specifies the output stream. This will contain the output GLSL code.
\param[in] shaderTarget Specifies the target shader.
\param[in] inputShaderVersion Specifies the input shader version (e.g. for "HLSL 5" use 'InputShaderVersions::HLSL5').
\param[in] outputShaderVersion Specifies the output shader version (e.g. for "GLSL 1.20" use 'OutputShaderVersions::GLSL120').
\param[in] includeHandler Optional pointer to the implementation of the "IncludeHandler" interface.
This will be used when en "#include" directive occurs. If such a directive occurs
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


} // /namespace HLSLTrans


#endif



// ================================================================================