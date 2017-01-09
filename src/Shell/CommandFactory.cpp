/*
 * CommandFactory.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "CommandFactory.h"
#include <tuple>


namespace Xsc
{

namespace Util
{


const CommandFactory& CommandFactory::Instance()
{
    static const CommandFactory instance;
    return instance;
}

Command* CommandFactory::Get(const std::string& name, Command::Identifier* cmdIdent) const
{
    for (const auto& cmd : commands_)
    {
        for (const auto& ident : cmd->Idents())
        {
            auto identLen = ident.name.size();
            if ( ( !ident.includesValue && name == ident.name ) ||
                 ( ident.includesValue && name.size() >= identLen && name.substr(0, identLen) == ident.name ) )
            {
                if (cmdIdent)
                    *cmdIdent = ident;
                return cmd.get();
            }
        }
    }
    return nullptr;
}


/*
 * ======= Private: =======
 */

CommandFactory::CommandFactory()
{
    MakeStandardCommands
    <
        EntryCommand,
        SecndEntryCommand,
        TargetCommand,
        VersionInCommand,
        VersionOutCommand,
        PrefixCommand,
        OutputCommand,
        WarnCommand,
        ShowASTCommand,
        ShowTimesCommand,
        ReflectCommand,
        PPOnlyCommand,
        MacroCommand,
        PauseCommand,
        PresettingCommand,
        VersionCommand,
        HelpCommand,
        IncludePathCommand,
        VerboseCommand,
        OptimizeCommand,
        ExtensionCommand,
        ValidateCommand,
        BindingCommand,
        CommentCommand,
        WrapperCommand,
        UnrollInitializerCommand,
        ObfuscateCommand,

        FormatBlanksCommand,
        FormatLineMarksCommand,
        FormatIndentCommand,
        FormatLineSeparationCommand,
        FormatCompactWrappersCommand,
        FormatBracedScopeCommand,
        FormatNewLineScopeCommand
    >();
}

template <typename T, typename... Args>
void CommandFactory::MakeCommand(Args&&... args)
{
    auto cmd = std::unique_ptr<T>(new T(std::forward<Args>(args)...));
    helpPrinter_.AppendCommandHelp(*cmd);
    commands_.emplace_back(std::move(cmd));
}

template <typename... Commands>
void CommandFactory::MakeStandardCommands()
{
    MakeStandardCommandsFirst<Commands...>();
    MakeStandardCommandsNext<Commands...>();
}

// No declaration for template specialization (not allowed with GCC)
template <>
void CommandFactory::MakeStandardCommands()
{
    // do nothing
}

template <typename FirstCommand, typename... NextCommands>
void CommandFactory::MakeStandardCommandsFirst()
{
    MakeCommand<FirstCommand>();
}

template <typename FirstCommand, typename... NextCommands>
void CommandFactory::MakeStandardCommandsNext()
{
    MakeStandardCommands<NextCommands...>();
}


} // /namespace Util

} // /namespace Xsc



// ================================================================================
