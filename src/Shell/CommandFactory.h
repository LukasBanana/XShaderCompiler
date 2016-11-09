/*
 * CommandFactory.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_COMMAND_FACTOR_H
#define XSC_COMMAND_FACTOR_H


#include "Command.h"
#include "HelpPrinter.h"
#include <memory>
#include <vector>


namespace Xsc
{

namespace Util
{


// Command factor (also 'command allocator') singleton class.
class CommandFactory
{

    public:

        CommandFactory(const CommandFactory&) = delete;
        CommandFactory& operator = (const CommandFactory&) = delete;

        // Returns the instance of this command factory singleton.
        static const CommandFactory& Instance();

        // Returns a pointer to the command with the specified name or null if there is no such command.
        Command* Get(const std::string& name, Command::Identifier* cmdIdent = nullptr) const;

        // Returns the help printer with the help entries for all commands.
        inline const HelpPrinter& GetHelpPrinter() const
        {
            return helpPrinter_;
        }

    private:

        CommandFactory();

        template <typename T, typename... Args>
        void MakeCommand(Args&&... args);

        std::vector<std::unique_ptr<Command>>   commands_;

        HelpPrinter                             helpPrinter_;

};


} // /namespace Util

} // /namespace Xsc


#endif



// ================================================================================