/*
 * Shell.h
 * 
 * This file is part of the "HLSL Translator" (Copyright (c) 2014 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef HTLIB_SHELL_H
#define HTLIB_SHELL_H


#include "ShellState.h"
#include "CommandLine.h"
#include <ostream>


namespace HTLib
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

        void Translate(const std::string& filename);

        ShellState state;

};


} // /namespace Util

} // /namespace HTLib


#endif



// ================================================================================