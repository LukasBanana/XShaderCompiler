/*
 * ShellState.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_SHELL_STATE_H
#define XSC_SHELL_STATE_H


#include <Xsc/Xsc.h>


namespace Xsc
{

namespace Util
{


struct CompileStatus
{
    // Number of compilations that succeeded
    std::size_t numSucceeded    = 0;

    // Number of compilations that failed
    std::size_t numFailed       = 0;
};

struct PredefinedMacro
{
    // Macro identifier
    std::string ident;

    // Optional macro body
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
    bool                            verbose             = false;

    // Pause application after everything is done.
    bool                            pauseApp            = false;

    // Show code reflection output after compilation.
    bool                            showReflection      = false;

    // Show extended code reflection (including all unreferenced objects).
    bool                            showReflectionExt   = false;

    // True, if any meaningful action has been performed (e.g. printed version or compiled any files).
    bool                            actionPerformed     = false;

    // Writes the compiled shader to the standard output instead of a file.
    bool                            writeStdout         = false;

    // Reads the source shader from the standard input instead of a file.
    bool                            readStdin           = false;

    // Status of the compilation results.
    CompileStatus                   compileStatus;
};


} // /namespace Util

} // /namespace Xsc


#endif



// ================================================================================