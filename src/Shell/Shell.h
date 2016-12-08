/*
 * Shell.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_SHELL_H
#define XSC_SHELL_H


#include <Xsc/IndentHandler.h>
#include <Xsc/Reflection.h>
#include "ShellState.h"
#include "CommandLine.h"
#include <ostream>


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

        void ExecuteCommandLine(CommandLine& cmdLine);

        void WaitForUser();

        std::ostream& output;

    private:

        std::string GetDefaultOutputFilename(const std::string& filename);

        void Compile(const std::string& filename);

        void ShowReflection(const Reflection::ReflectionData& reflectionData);
        void ShowReflectionFor(const std::vector<Reflection::BindingSlot>& objects, const std::string& title);
        void ShowReflectionFor(const std::vector<std::string>& idents, const std::string& title);
        void ShowReflectionFor(const std::map<std::string, Reflection::SamplerState>& samplerStates, const std::string& title);

        ShellState      state_;
        IndentHandler   indentHandler_;

        static Shell*   instance_;

};


} // /namespace Util

} // /namespace Xsc


#endif



// ================================================================================