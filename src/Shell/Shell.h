/*
 * Shell.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_SHELL_H
#define XSC_SHELL_H


#include <Xsc/IndentHandler.h>
#include <Xsc/Reflection.h>
#include "ShellState.h"
#include "CommandLine.h"
#include <ostream>
#include <stack>


namespace Xsc
{

namespace Util
{


// The shell is the main class of the command line tool for the compiler.
class Shell
{
    
    public:
        
        Shell(std::ostream& output);
        ~Shell();

        static Shell* Instance();

        // Executes the specified command line, and return true if any action has been performed.
        bool ExecuteCommandLine(CommandLine& cmdLine, bool& succeeded, bool enableBriefHelp = true);

        void WaitForUser();

        void PushState();
        void PopState();

        // Returns the current shell state.
        inline const ShellState& GetState() const
        {
            return state_;
        }

        // Returns the previously compiled output filename or an empty string, if the previous compilation has failed.
        inline const std::string& GetLastOutputFilename() const
        {
            return lastOutputFilename_;
        }

        std::ostream& output;

    private:

        std::string GetDefaultOutputFilename(const std::string& filename) const;

        bool Compile(const std::string& filename);

        ShellState              state_;
        std::stack<ShellState>  stateStack_;

        std::string             lastOutputFilename_;

        static Shell*           instance_;

};


} // /namespace Util

} // /namespace Xsc


#endif



// ================================================================================
