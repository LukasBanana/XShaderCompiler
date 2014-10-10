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

#include <string>
#include <istream>
#include <ostream>


namespace HTLib
{


//! Structure for additional translation options.
struct Options
{
    std::string indent = "    "; //!< Indentation string. By default std::string(4, ' ').
};

//! Interface for handling new include streams.
class _HT_EXPORT_ IncludeHandler
{
    
    public:
        
        virtual ~IncludeHandler()
        {
        }

        virtual void Include(const std::string& includeName, std::istream& includedInputStream) = 0;

};


/**
Translates the specifies HLSL code into GLSL code.
\param[in,out] input Specifies the input stream. This must be valid HLSL code.
\param[in,out] output Specifies the output stream. This will be GLSL code.
\param[in] shaderTarget Specifies the target shader.
\param[in] shaderVersion Specifies the output shader version (e.g. for "GLSL 1.20" use ShaderVersions::GLSL120).
\param[in] includeHandler Optional pointer to the implementation of the "IncludeHandler" interface.
This will be used when en "#include" directive occurs. If such a directive occurs
and this parameter is null, the code generation will fail! By default null.
\param[in] options Additional options to configure the code generation.
\param[in] log Optional pointer to an output log. Inherit from the "Logger" class interface.
\return True if the code has been translated correctly.
\note This translator does not make any context analysis!
Therefore wrong HLSL code will be translated into wrong GLSL code.
\see IncludeHandler
\see Options
*/
_HT_EXPORT_ bool TranslateHLSLtoGLSL(
    std::istream& input,
    std::ostream& output,
    const std::string& entryPoint,
    const ShaderTargets shaderTarget,
    const ShaderVersions shaderVersion,
    IncludeHandler* includeHandler = nullptr,
    const Options& options = {},
    Logger* log = nullptr
);


} // /namespace HLSLTrans


#endif



// ================================================================================