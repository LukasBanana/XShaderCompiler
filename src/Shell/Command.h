/*
 * Command.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_COMMAND_H
#define XSC_COMMAND_H


#include "HelpPrinter.h"
#include "CommandLine.h"
#include "ShellState.h"
#include <string>


namespace Xsc
{

namespace Util
{


// Interface for a shell command.
class Command
{

    public:

        struct Identifier
        {
            Identifier() = default;

            inline Identifier(const std::string& name, bool includesValue = false) :
                name          { name          },
                includesValue { includesValue }
            {
            }

            // Command identifier name.
            std::string name;

            // Specifies whether the command identifier name includes a value (e.g. "-D<IDENT>").
            bool        includesValue   = false;
        };

        virtual ~Command();

        // Returns a list of identifiers for this command.
        virtual std::vector<Identifier> Idents() const = 0;

        // Returns the information for an help entry.
        virtual HelpDescriptor Help() const = 0;

        // Runs this command with the arguments from the specified command line.
        virtual void Run(CommandLine& cmdLine, ShellState& state) = 0;

};


#define DECL_SHELL_COMMAND(NAME)                                        \
    class NAME : public Command                                         \
    {                                                                   \
        public:                                                         \
            std::vector<Identifier> Idents() const override;            \
            HelpDescriptor Help() const override;                       \
            void Run(CommandLine& cmdLine, ShellState& state) override; \
    }

DECL_SHELL_COMMAND( EntryCommand                 );
DECL_SHELL_COMMAND( SecndEntryCommand            );
DECL_SHELL_COMMAND( TargetCommand                );
DECL_SHELL_COMMAND( VersionInCommand             );
DECL_SHELL_COMMAND( VersionOutCommand            );
DECL_SHELL_COMMAND( OutputCommand                );
DECL_SHELL_COMMAND( IncludePathCommand           );
DECL_SHELL_COMMAND( WarnCommand                  );
DECL_SHELL_COMMAND( ShowASTCommand               );
DECL_SHELL_COMMAND( ShowTimesCommand             );
DECL_SHELL_COMMAND( ReflectCommand               );
DECL_SHELL_COMMAND( PPOnlyCommand                );
DECL_SHELL_COMMAND( MacroCommand                 );
DECL_SHELL_COMMAND( SemanticCommand              );
DECL_SHELL_COMMAND( PackUniformsCommand          );
DECL_SHELL_COMMAND( PauseCommand                 );
DECL_SHELL_COMMAND( PresettingCommand            );
DECL_SHELL_COMMAND( VersionCommand               );
DECL_SHELL_COMMAND( HelpCommand                  );
DECL_SHELL_COMMAND( VerboseCommand               );
DECL_SHELL_COMMAND( ColorCommand                 );
DECL_SHELL_COMMAND( OptimizeCommand              );
DECL_SHELL_COMMAND( ExtensionCommand             );
DECL_SHELL_COMMAND( EnumExtensionCommand         );
DECL_SHELL_COMMAND( BindingCommand               );
DECL_SHELL_COMMAND( CommentCommand               );
DECL_SHELL_COMMAND( WrapperCommand               );
DECL_SHELL_COMMAND( UnrollInitializerCommand     );
DECL_SHELL_COMMAND( ObfuscateCommand             );
DECL_SHELL_COMMAND( RowMajorAlignmentCommand     );
DECL_SHELL_COMMAND( AutoBindingCommand           );
DECL_SHELL_COMMAND( AutoBindingStartSlotCommand  );
DECL_SHELL_COMMAND( FormattingCommand            );
DECL_SHELL_COMMAND( IndentCommand                );
DECL_SHELL_COMMAND( PrefixCommand                );
DECL_SHELL_COMMAND( NameManglingCommand          );
DECL_SHELL_COMMAND( SeparateShadersCommand       );
DECL_SHELL_COMMAND( SeparateSamplersCommand      );
DECL_SHELL_COMMAND( DisassembleCommand           );
DECL_SHELL_COMMAND( DisassembleExtCommand        );

#ifdef XSC_ENABLE_LANGUAGE_EXT

DECL_SHELL_COMMAND( LanguageExtensionCommand     );

#endif

#undef DECL_SHELL_COMMAND


} // /namespace Util

} // /namespace Xsc


#endif



// ================================================================================
