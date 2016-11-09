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
    ShaderInput                     inputDesc;
    ShaderOutput                    outputDesc;
    std::string                     outputFilename;
    std::vector<PredefinedMacro>    predefinedMacros;
    std::vector<std::string>        searchPaths;
    bool                            pauseApp            = false;
};


} // /namespace Util

} // /namespace Xsc


#endif



// ================================================================================