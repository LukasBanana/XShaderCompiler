/*
 * ShellState.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_SHELL_STATE_H
#define XSC_SHELL_STATE_H


#include <Xsc/Xsc.h>


namespace Xsc
{

namespace Util
{


struct PredefinedMacro
{
    std::string ident;
    std::string value;
};

struct ShellState
{
    // Shader input descriptor.
    ShaderInput                     inputDesc;

    // Shader output descriptor.
    ShaderOutput                    outputDesc;

    // Output filename (hint).
    std::string                     outputFilename;

    // Predefined macros for the preprocessor
    std::vector<PredefinedMacro>    predefinedMacros;

    // Include search paths for the preprocessor.
    std::vector<std::string>        searchPaths;

    // Print line marks for compiler reports.
    bool                            verbose             = true;

    // Pause application after everything is done.
    bool                            pauseApp            = false;

    // Show code reflection output after compilation.
    bool                            showReflection           = false;
};


} // /namespace Util

} // /namespace Xsc


#endif



// ================================================================================