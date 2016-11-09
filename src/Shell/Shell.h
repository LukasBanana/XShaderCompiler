/*
 * Shell.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_SHELL_H
#define XSC_SHELL_H


#include "ShellState.h"
#include "CommandLine.h"
#include <ostream>


namespace Xsc
{

namespace Util
{


class Shell
{
    
    public:
        
        Shell(std::ostream& output);

        void ExecuteCommandLine(CommandLine& cmdLine);

        std::ostream& output;

    private:

        void Compile(const std::string& filename);

        ShellState state;

};


} // /namespace Util

} // /namespace Xsc


#endif



// ================================================================================