/*
 * Shell.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_SHELL_H
#define XSC_SHELL_H


#include <Xsc/IndentHandler.h>
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

        void ShowStats(const Statistics& stats);
        void ShowStatsFor(const std::vector<Statistics::Binding>& objects, const std::string& title);

        ShellState      state_;
        IndentHandler   indentHandler_;

};


} // /namespace Util

} // /namespace Xsc


#endif



// ================================================================================