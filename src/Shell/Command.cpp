/*
 * Command.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "Command.h"
#include "CommandFactory.h"
#include "Shell.h"
#include "Helper.h"
#include <Xsc/Targets.h>
#include <Xsc/ConsoleManip.h>
#include <map>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <set>


namespace Xsc
{

namespace Util
{


/*
 * Internal functions
 */

template <typename T>
T MapStringToType(const std::string& search, const std::map<std::string, T>& mappingList, const std::string& errorMsg)
{
    auto it = mappingList.find(search);
    if (it == mappingList.end())
        throw std::invalid_argument(errorMsg + " '" + search + "'");
    return it->second;
}


/*
 * Command class
 */

Command::~Command()
{
    // dummy
}


/*
 * EntryCommand class
 */

std::vector<Command::Identifier> EntryCommand::Idents() const
{
    return { { "-E" }, { "--entry" } };
}

HelpDescriptor EntryCommand::Help() const
{
    return
    {
        "-E, --entry ENTRY",
        "Shader entry point; default=main",
        HelpCategory::Main
    };
}

void EntryCommand::Run(CommandLine& cmdLine, ShellState& state)
{
    state.inputDesc.entryPoint = cmdLine.Accept();
}


/*
 * SecndEntryCommand class
 */

std::vector<Command::Identifier> SecndEntryCommand::Idents() const
{
    return { { "-E2" }, { "--entry2" } };
}

HelpDescriptor SecndEntryCommand::Help() const
{
    return
    {
        "-E2, --entry2 ENTRY",
        "Secondary shader entry point",
        HelpCategory::Main
    };
}

void SecndEntryCommand::Run(CommandLine& cmdLine, ShellState& state)
{
    state.inputDesc.secondaryEntryPoint = cmdLine.Accept();
}


/*
 * TargetCommand class
 */

std::vector<Command::Identifier> TargetCommand::Idents() const
{
    return { { "-T" }, { "--target" } };
}

HelpDescriptor TargetCommand::Help() const
{
    return
    {
        "-T, --target TARGET", "Input shader target; valid targets:",
        "vert, tesc, tese, geom, frag, comp",
        HelpCategory::Main
    };
}

void TargetCommand::Run(CommandLine& cmdLine, ShellState& state)
{
    state.inputDesc.shaderTarget = MapStringToType<ShaderTarget>(
        cmdLine.Accept(),
        {
            { "vert", ShaderTarget::VertexShader                 },
            { "tesc", ShaderTarget::TessellationControlShader    },
            { "tese", ShaderTarget::TessellationEvaluationShader },
            { "geom", ShaderTarget::GeometryShader               },
            { "frag", ShaderTarget::FragmentShader               },
            { "comp", ShaderTarget::ComputeShader                },
        },
        "invalid shader target"
    );
}


/*
 * VersionInCommand class
 */

std::vector<Command::Identifier> VersionInCommand::Idents() const
{
    return { { "-Vin" }, { "--version-in" } };
}

HelpDescriptor VersionInCommand::Help() const
{
    return
    {
        "-Vin, --version-in VERSION",
        "Input shader version; default=HLSL5; valid versions:",
        "Cg, HLSL3, HLSL4, HLSL5, HLSL6, GLSL, ESSL, VKSL",
        HelpCategory::Main
    };
}

void VersionInCommand::Run(CommandLine& cmdLine, ShellState& state)
{
    state.inputDesc.shaderVersion = MapStringToType<InputShaderVersion>(
        cmdLine.Accept(),
        {
            { "Cg",    InputShaderVersion::Cg    },
            { "HLSL3", InputShaderVersion::HLSL3 },
            { "HLSL4", InputShaderVersion::HLSL4 },
            { "HLSL5", InputShaderVersion::HLSL5 },
            { "HLSL6", InputShaderVersion::HLSL6 },
            { "GLSL",  InputShaderVersion::GLSL  },
            { "ESSL",  InputShaderVersion::ESSL  },
            { "VKSL",  InputShaderVersion::VKSL  },
        },
        "invalid input shader version"
    );
}


/*
 * VersionOutCommand class
 */

std::vector<Command::Identifier> VersionOutCommand::Idents() const
{
    return { { "-Vout" }, { "--version-out" } };
}

HelpDescriptor VersionOutCommand::Help() const
{
    return
    {
        "-Vout, --version-out VERSION",
        "Shader output version; default=GLSL; valid versions:",
        (
            "GLSL[110, 120, 130, 140, 150, 330, 400, 410, 420, 430, 440, 450],\n"   \
            "ESSL[100, 300, 310, 320],\n"                                           \
            "VKSL[450]"
        ),
        HelpCategory::Main
    };
}

void VersionOutCommand::Run(CommandLine& cmdLine, ShellState& state)
{
    state.outputDesc.shaderVersion = MapStringToType<OutputShaderVersion>(
        cmdLine.Accept(),
        {
            { "GLSL110", OutputShaderVersion::GLSL110 },
            { "GLSL120", OutputShaderVersion::GLSL120 },
            { "GLSL130", OutputShaderVersion::GLSL130 },
            { "GLSL140", OutputShaderVersion::GLSL140 },
            { "GLSL150", OutputShaderVersion::GLSL150 },
            { "GLSL330", OutputShaderVersion::GLSL330 },
            { "GLSL400", OutputShaderVersion::GLSL400 },
            { "GLSL410", OutputShaderVersion::GLSL410 },
            { "GLSL420", OutputShaderVersion::GLSL420 },
            { "GLSL430", OutputShaderVersion::GLSL430 },
            { "GLSL440", OutputShaderVersion::GLSL440 },
            { "GLSL450", OutputShaderVersion::GLSL450 },
            { "GLSL",    OutputShaderVersion::GLSL    },

            { "ESSL100", OutputShaderVersion::ESSL100 },
            { "ESSL300", OutputShaderVersion::ESSL300 },
            { "ESSL310", OutputShaderVersion::ESSL310 },
            { "ESSL320", OutputShaderVersion::ESSL320 },
            { "ESSL",    OutputShaderVersion::ESSL    },

            { "VKSL450", OutputShaderVersion::VKSL450 },
            { "VKSL",    OutputShaderVersion::VKSL    },
        },
        "invalid output shader version"
    );
}


/*
 * OutputCommand class
 */

std::vector<Command::Identifier> OutputCommand::Idents() const
{
    return { { "-o", }, { "--output" } };
}

HelpDescriptor OutputCommand::Help() const
{
    return
    {
        "-o, --output FILE",
        "Shader output file (use '*' for default); default='<FILE>.<ENTRY>.<TARGET>'",
        HelpCategory::Main
    };
}

void OutputCommand::Run(CommandLine& cmdLine, ShellState& state)
{
    state.outputFilename = cmdLine.Accept();
}


/*
 * IncludePathCommand class
 */

std::vector<Command::Identifier> IncludePathCommand::Idents() const
{
    return { { "-I" }, { "--include" } };
}

HelpDescriptor IncludePathCommand::Help() const
{
    return
    {
        "-I, --include PATH",
        "Adds PATH to the search include paths",
        HelpCategory::Main
    };
}

void IncludePathCommand::Run(CommandLine& cmdLine, ShellState& state)
{
    state.searchPaths.push_back(cmdLine.Accept());
}


/*
 * WarnCommand class
 */

std::vector<Command::Identifier> WarnCommand::Idents() const
{
    return { { "-W", true } };
}

HelpDescriptor WarnCommand::Help() const
{
    return
    {
        "-W<TYPE> [" + CommandLine::GetBooleanOption() + "]",
        "Enables/disables the specified warning type; default=" + CommandLine::GetBooleanFalse() + "; valid types:",
        (
            "all           => all kinds of warnings\n"               \
            "basic         => warn for basic issues\n"               \
            "decl-shadow   => warn for declaration shadowing\n"      \
            "empty-body    => warn for empty statement body\n"       \
            "extension     => warn for required extensions\n"        \
            "implicit-cast => warn for implicit type casts\n"        \
            "preprocessor  => warn for pre-processor issues\n"       \
            "reflect       => warn for issues in code reflection\n"  \
            "syntax        => warn for syntactic issues\n"           \
            "unlocated-obj => warn for unlocated optional objects\n" \
            "unused-vars   => warn for unused variables"
        ),
    };
}

void WarnCommand::Run(CommandLine& cmdLine, ShellState& state)
{
    const auto flags = MapStringToType<unsigned int>(
        cmdLine.Accept(),
        {
            { "all",           Warnings::All                     },
            { "basic",         Warnings::Basic                   },
            { "decl-shadow",   Warnings::DeclarationShadowing    },
            { "empty-body",    Warnings::EmptyStatementBody      },
            { "extension",     Warnings::RequiredExtensions      },
            { "implicit-cast", Warnings::ImplicitTypeConversions },
            { "preprocessor",  Warnings::PreProcessor            },
            { "reflect",       Warnings::CodeReflection          },
            { "syntax",        Warnings::Syntax                  },
            { "unlocated-obj", Warnings::UnlocatedObjects        },
            { "unused-vars",   Warnings::UnusedVariables         },
        },
        "invalid warning type"
    );

    if (cmdLine.AcceptBoolean(true))
        state.inputDesc.warnings |= flags;
    else
        state.inputDesc.warnings &= (~flags);
}


/*
 * ShowASTCommand class
 */

std::vector<Command::Identifier> ShowASTCommand::Idents() const
{
    return { { "--show-ast" } };
}

HelpDescriptor ShowASTCommand::Help() const
{
    return
    {
        "--show-ast [" + CommandLine::GetBooleanOption() + "]",
        "Enables/disables debug output for the AST (Abstract Syntax Tree); default=" + CommandLine::GetBooleanFalse()
    };
}

void ShowASTCommand::Run(CommandLine& cmdLine, ShellState& state)
{
    state.outputDesc.options.showAST = cmdLine.AcceptBoolean(true);
}


/*
 * ShowTimesCommand class
 */

std::vector<Command::Identifier> ShowTimesCommand::Idents() const
{
    return { { "--show-times" } };
}

HelpDescriptor ShowTimesCommand::Help() const
{
    return
    {
        "--show-times [" + CommandLine::GetBooleanOption() + "]",
        "Enables/disables debug output for timings of each compilation step; default=" + CommandLine::GetBooleanFalse()
    };
}

void ShowTimesCommand::Run(CommandLine& cmdLine, ShellState& state)
{
    state.outputDesc.options.showTimes = cmdLine.AcceptBoolean(true);
}


/*
 * ReflectCommand class
 */

std::vector<Command::Identifier> ReflectCommand::Idents() const
{
    return { { "--reflect" } };
}

HelpDescriptor ReflectCommand::Help() const
{
    return
    {
        "--reflect [" + CommandLine::GetBooleanOption() + "]",
        "Enables/disables code reflection output; default=" + CommandLine::GetBooleanFalse()
    };
}

void ReflectCommand::Run(CommandLine& cmdLine, ShellState& state)
{
    state.showReflection = cmdLine.AcceptBoolean(true);
}


/*
 * PPOnlyCommand class
 */

std::vector<Command::Identifier> PPOnlyCommand::Idents() const
{
    return { { "-PP" }, { "--preprocess-only" } };
}

HelpDescriptor PPOnlyCommand::Help() const
{
    return
    {
        "-PP, --preprocess-only [" + CommandLine::GetBooleanOption() + "]",
        "Enables/disables to only preprocess source code; default=" + CommandLine::GetBooleanFalse()
    };
}

void PPOnlyCommand::Run(CommandLine& cmdLine, ShellState& state)
{
    state.outputDesc.options.preprocessOnly = cmdLine.AcceptBoolean(true);
}


/*
 * MacroCommand class
 */

std::vector<Command::Identifier> MacroCommand::Idents() const
{
    return { { "-D", true } };
}

HelpDescriptor MacroCommand::Help() const
{
    return
    {
        "-D<IDENT>, -D<IDENT>=VALUE",
        "Adds the identifier <IDENT> to the pre-defined macros with an optional VALUE"
    };
}

void MacroCommand::Run(CommandLine& cmdLine, ShellState& state)
{
    auto arg = cmdLine.Accept();

    PredefinedMacro macro;

    auto pos = arg.find('=');
    if (pos != std::string::npos && pos + 1 < arg.size())
    {
        macro.ident = arg.substr(0, pos);
        macro.value = arg.substr(pos + 1);
    }
    else
        macro.ident = arg;

    state.predefinedMacros.push_back(macro);
}


/*
 * SemanticCommand class
 */

std::vector<Command::Identifier> SemanticCommand::Idents() const
{
    return { { "-S", true } };
}

HelpDescriptor SemanticCommand::Help() const
{
    return
    {
        "-S<IDENT>=VALUE",
        "Adds the vertex semantic <IDENT> binding to VALUE (Requires -EB)"
    };
}

void SemanticCommand::Run(CommandLine& cmdLine, ShellState& state)
{
    auto arg = cmdLine.Accept();

    auto pos = arg.find('=');
    if (pos != std::string::npos && pos + 1 < arg.size())
    {
        /* Get semantic name and location index */
        auto ident = arg.substr(0, pos);
        auto value = arg.substr(pos + 1);
        auto locIndex = std::stoi(value);
        state.outputDesc.vertexSemantics.push_back({ ident, locIndex });
    }
    else
        throw std::runtime_error("vertex attribute value expected for \"" + arg + "\"");
}


/*
 * PauseCommand class
 */

std::vector<Command::Identifier> PauseCommand::Idents() const
{
    return { { "--pause" } };
}

HelpDescriptor PauseCommand::Help() const
{
    return
    {
        "--pause",
        "Waits for user input after the translation process"
    };
}

void PauseCommand::Run(CommandLine& cmdLine, ShellState& state)
{
    state.pauseApp = true;
}


/*
 * PresettingCommand class
 */

std::vector<Command::Identifier> PresettingCommand::Idents() const
{
    return { { "-PS" }, { "--presetting" } };
}

HelpDescriptor PresettingCommand::Help() const
{
    return
    {
        "-PS, --presetting FILE",
        "Parse further arguments from the presetting file"
    };
}

void PresettingCommand::Run(CommandLine& cmdLine, ShellState& state)
{
    static std::set<std::string> pressetingFilenames;

    auto filename = cmdLine.Accept();

    /* Check if this presetting file has already been processed */
    if (pressetingFilenames.find(filename) != pressetingFilenames.end())
        throw std::runtime_error("loop in presetting files detected");

    pressetingFilenames.insert(filename);

    /* Read arguments from file */
    std::ifstream file(filename);
    if (!file.good())
        throw std::runtime_error("failed to read file: \"" + filename + "\"");

    struct Presetting
    {
        std::string title;
        CommandLine cmdLine;
    };

    auto RunPresetting = [&](Presetting& preset) -> bool
    {
        std::cout << "run presetting: \"" << preset.title << "\"" << std::endl;
        auto shell = Shell::Instance();
        
        bool result = false;

        shell->PushState();
        {
            shell->ExecuteCommandLine(preset.cmdLine);

            if (!shell->GetLastOutputFilename().empty())
            {
                #ifdef XSC_ENABLE_POST_VALIDATION
            
                /* Build command to run glslangValidator for the previous output file */
                std::string cmd = "glslangValidator ";
                
                if (IsLanguageVKSL(shell->GetState().outputDesc.shaderVersion))
                    cmd += "-V ";

                cmd += '\"';
                cmd += shell->GetLastOutputFilename();
                cmd += '\"';

                /* Print command to standard output */
                if (shell->GetState().verbose)
                    std::cout << cmd << std::endl;

                /* Run command */
                system(cmd.c_str());
                
                #endif

                result = true;
            }
        }
        shell->PopState();

        state.actionPerformed = true;

        return result;
    };

    std::vector<Presetting> presettings;

    static const char commentChar = '#';

    while (!file.eof())
    {
        /* Get presetting title */
        Presetting preset;
        std::getline(file, preset.title);

        if (!preset.title.empty() && preset.title.front() != commentChar)
        {
            /* Get presetting arguments */
            std::string line;
            std::getline(file, line);

            if (!line.empty() && line.front() != commentChar)
            {
                /* Parse command line arguments for current presetting */
                preset.cmdLine = CommandLine(line);

                /* Add presetting to the selection */
                if (!preset.cmdLine.ReachedEnd())
                    presettings.emplace_back(std::move(preset));
            }
        }
    }

    if (presettings.size() > 1)
    {
        std::size_t idx = ~0;

        while (idx > presettings.size())
        {
            /* Returns the number of digits in the specified integral number */
            auto NumDigits = [](std::size_t n)
            {
                std::size_t d = 0;
                do
                {
                    n /= 10;
                    ++d;
                }
                while (n != 0);
                return d;
            };

            /* Returns the indentation for the specified number with the current number of presettings */
            auto Indent = [&](std::size_t n)
            {
                return std::string(NumDigits(presettings.size()) - NumDigits(n), ' ');
            };

            /* Let user choose between one of the presettings */
            std::cout << "choose presetting:" << std::endl;
            std::cout << "  " << Indent(0) << "0.) ALL" << std::endl;

            for (std::size_t i = 0; i < presettings.size(); ++i)
                std::cout << "  " << Indent(i+1) << (i+1) << ".) " << presettings[i].title << std::endl;

            std::cin >> idx;
        }

        if (idx == 0)
        {
            /* Run all presettings */
            std::size_t numFailed = 0;

            for (auto& preset : presettings)
            {
                if (!RunPresetting(preset))
                    ++numFailed;
            }

            /* Print information of success or failure */
            if (numFailed == 0)
            {
                ConsoleManip::ScopedColor color(ConsoleManip::ColorFlags::Green | ConsoleManip::ColorFlags::Intens);
                std::cout << "all presettings processed successfully" << std::endl;
            }
            else
            {
                ConsoleManip::ScopedColor color(ConsoleManip::ColorFlags::Red | ConsoleManip::ColorFlags::Intens);
                std::cout << numFailed << " of " << presettings.size() << " presetting(s) failed" << std::endl;
            }
        }
        else if (idx > 0)
        {
            /* Run selected presetting */
            RunPresetting(presettings[idx - 1]);
        }
    }
    else if (presettings.size() == 1)
    {
        /* First single presetting */
        RunPresetting(presettings.front());
    }
    else
    {
        /* Tell user that no presettings are defined */
        std::cout << "no presettings defined" << std::endl;
    }

    state.actionPerformed = true;
}


/*
 * VersionCommand class
 */

std::vector<Command::Identifier> VersionCommand::Idents() const
{
    return { { "--version" } };
}

HelpDescriptor VersionCommand::Help() const
{
    return
    {
        "--version",
        "Prints the version information"
    };
}

void VersionCommand::Run(CommandLine& cmdLine, ShellState& state)
{
    ConsoleManip::ScopedColor highlight(ConsoleManip::ColorFlags::Green | ConsoleManip::ColorFlags::Blue);
    std::cout << "XShaderCompiler ( Version " << XSC_VERSION_STRING << " )" << std::endl;
    std::cout << "Copyright (c) 2014-2017 by Lukas Hermanns" << std::endl;
    std::cout << "3-Clause BSD License" << std::endl;
    state.actionPerformed = true;
}


/*
 * HelpCommand class
 */

std::vector<Command::Identifier> HelpCommand::Idents() const
{
    return { { "--help" } };
}

HelpDescriptor HelpCommand::Help() const
{
    return
    {
        "--help",
        "Prints the help reference"
    };
}

void HelpCommand::Run(CommandLine& cmdLine, ShellState& state)
{
    CommandFactory::Instance().GetHelpPrinter().PrintHelpReference(std::cout, 2, false, true);
    state.actionPerformed = true;
}


/*
 * VerboseCommand class
 */

std::vector<Command::Identifier> VerboseCommand::Idents() const
{
    return { { "-V" }, { "--verbose" } };
}

HelpDescriptor VerboseCommand::Help() const
{
    return
    {
        "-V, --verbose [" + CommandLine::GetBooleanOption() + "]",
        "Enables/disables more output for compiler reports; default=" + CommandLine::GetBooleanTrue()
    };
}

void VerboseCommand::Run(CommandLine& cmdLine, ShellState& state)
{
    state.verbose = cmdLine.AcceptBoolean(true);
}


/*
 * ColorCommand class
 */

std::vector<Command::Identifier> ColorCommand::Idents() const
{
    return { { "--color" } };
}

HelpDescriptor ColorCommand::Help() const
{
    return
    {
        "--color [" + CommandLine::GetBooleanOption() + "]",
        "Enables/disables color highlighting for shell output; default=" + CommandLine::GetBooleanTrue()
    };
}

void ColorCommand::Run(CommandLine& cmdLine, ShellState& state)
{
    ConsoleManip::Enable(cmdLine.AcceptBoolean(true));
}


/*
 * OptimizeCommand class
 */

std::vector<Command::Identifier> OptimizeCommand::Idents() const
{
    return { { "-O" }, { "--optimize" } };
}

HelpDescriptor OptimizeCommand::Help() const
{
    return
    {
        "-O, --optimize [" + CommandLine::GetBooleanOption() + "]",
        "Enables/disables optimization; default=" + CommandLine::GetBooleanFalse()
    };
}

void OptimizeCommand::Run(CommandLine& cmdLine, ShellState& state)
{
    state.outputDesc.options.optimize = cmdLine.AcceptBoolean(true);
}


/*
 * ExtensionCommand class
 */

std::vector<Command::Identifier> ExtensionCommand::Idents() const
{
    return { { "--extension" } };
}

HelpDescriptor ExtensionCommand::Help() const
{
    return
    {
        "--extension [" + CommandLine::GetBooleanOption() + "]",
        "Enables/disables shader extension output; default=" + CommandLine::GetBooleanFalse()
    };
}

void ExtensionCommand::Run(CommandLine& cmdLine, ShellState& state)
{
    state.outputDesc.options.allowExtensions = cmdLine.AcceptBoolean(true);
}


/*
 * EnumExtensionCommand class
 */

std::vector<Command::Identifier> EnumExtensionCommand::Idents() const
{
    return { { "--enum-extensions" } };
}

HelpDescriptor EnumExtensionCommand::Help() const
{
    return
    {
        "--enum-extensions",
        "Enumerates all supported GLSL extensions"
    };
}

void EnumExtensionCommand::Run(CommandLine& cmdLine, ShellState& state)
{
    for (const auto& it : GetGLSLExtensionEnumeration())
        std::cout << it.first << std::endl;
    state.actionPerformed = true;
}


/*
 * ValidateCommand class
 */

std::vector<Command::Identifier> ValidateCommand::Idents() const
{
    return { { "-Valid" }, { "--validate-only" } };
}

HelpDescriptor ValidateCommand::Help() const
{
    return
    {
        "-Valid, --validate-only [" + CommandLine::GetBooleanOption() + "]",
        "Enables/disables to only validate source code; default=" + CommandLine::GetBooleanFalse()
    };
}

void ValidateCommand::Run(CommandLine& cmdLine, ShellState& state)
{
    state.outputDesc.options.validateOnly = cmdLine.AcceptBoolean(true);
}


/*
 * BindingCommand class
 */

std::vector<Command::Identifier> BindingCommand::Idents() const
{
    return { { "-EB" }, { "--explicit-bind" } };
}

HelpDescriptor BindingCommand::Help() const
{
    return
    {
        "-EB, --explicit-bind [" + CommandLine::GetBooleanOption() + "]",
        "Enables/disables explicit binding slots; default=" + CommandLine::GetBooleanFalse()
    };
}

void BindingCommand::Run(CommandLine& cmdLine, ShellState& state)
{
    state.outputDesc.options.explicitBinding = cmdLine.AcceptBoolean(true);
}

/*
 * AutoBindingCommand class
 */

std::vector<Command::Identifier> AutoBindingCommand::Idents() const
{
    return { { "-AB" }, { "--auto-bind" } };
}

HelpDescriptor AutoBindingCommand::Help() const
{
    return
    {
        "-AB, --auto-bind [" + CommandLine::GetBooleanOption() + "]",
        "Enables/disables automatic binding slot generation (implies -EB); default=" + CommandLine::GetBooleanFalse()
    };
}

void AutoBindingCommand::Run(CommandLine& cmdLine, ShellState& state)
{
    state.outputDesc.options.autoBinding = cmdLine.AcceptBoolean(true);
}

/*
 * AutoBindingStartSlotCommand class
 */

std::vector<Command::Identifier> AutoBindingStartSlotCommand::Idents() const
{
    return { { "-AB-slot" }, { "--auto-bind-slot" } };
}

HelpDescriptor AutoBindingStartSlotCommand::Help() const
{
    return
    {
        "-AB-slot, --auto-bind-slot OFFSET",
        "Sets the start slot index for automatic binding slot generation; default=0"
    };
}

void AutoBindingStartSlotCommand::Run(CommandLine& cmdLine, ShellState& state)
{
    state.outputDesc.options.autoBindingStartSlot = std::stoi(cmdLine.Accept());
}

/*
 * CommentCommand class
 */

std::vector<Command::Identifier> CommentCommand::Idents() const
{
    return { { "--comments" } };
}

HelpDescriptor CommentCommand::Help() const
{
    return
    {
        "--comments [" + CommandLine::GetBooleanOption() + "]",
        "Enables/disables commentary preservation; default=" + CommandLine::GetBooleanFalse()
    };
}

void CommentCommand::Run(CommandLine& cmdLine, ShellState& state)
{
    state.outputDesc.options.preserveComments = cmdLine.AcceptBoolean(true);
}


/*
 * WrapperCommand class
 */

std::vector<Command::Identifier> WrapperCommand::Idents() const
{
    return { { "--wrapper" } };
}

HelpDescriptor WrapperCommand::Help() const
{
    return
    {
        "--wrapper [" + CommandLine::GetBooleanOption() + "]",
        "Enables/disables the preference for intrinsic wrappers; default=" + CommandLine::GetBooleanFalse()
    };
}

void WrapperCommand::Run(CommandLine& cmdLine, ShellState& state)
{
    state.outputDesc.options.preferWrappers = cmdLine.AcceptBoolean(true);
}


/*
 * UnrollInitializerCommand class
 */

std::vector<Command::Identifier> UnrollInitializerCommand::Idents() const
{
    return { { "-Uinit" }, { "--unroll-initializer" } };
}

HelpDescriptor UnrollInitializerCommand::Help() const
{
    return
    {
        "-Uinit, --unroll-initializer [" + CommandLine::GetBooleanOption() + "]",
        "Enables/disables unrolling of array initializers; default=" + CommandLine::GetBooleanFalse()
    };
}

void UnrollInitializerCommand::Run(CommandLine& cmdLine, ShellState& state)
{
    state.outputDesc.options.unrollArrayInitializers = cmdLine.AcceptBoolean(true);
}


/*
 * ObfuscateCommand class
 */

std::vector<Command::Identifier> ObfuscateCommand::Idents() const
{
    return { { "--obfuscate" } };
}

HelpDescriptor ObfuscateCommand::Help() const
{
    return
    {
        "--obfuscate [" + CommandLine::GetBooleanOption() + "]",
        "Enables/disables code obfuscation; default=" + CommandLine::GetBooleanFalse()
    };
}

void ObfuscateCommand::Run(CommandLine& cmdLine, ShellState& state)
{
    state.outputDesc.options.obfuscate = cmdLine.AcceptBoolean(true);
}


/*
 * RowMajorAlignmentCommand class
 */

std::vector<Command::Identifier> RowMajorAlignmentCommand::Idents() const
{
    return { { "--row-major" } };
}

HelpDescriptor RowMajorAlignmentCommand::Help() const
{
    return
    {
        "--row-major [" + CommandLine::GetBooleanOption() + "]",
        "Enables/disables row major packing alignment for matrices; default=" + CommandLine::GetBooleanFalse()
    };
}

void RowMajorAlignmentCommand::Run(CommandLine& cmdLine, ShellState& state)
{
    state.outputDesc.options.rowMajorAlignment = cmdLine.AcceptBoolean(true);
}


/*
 * FormattingCommand class
 */

std::vector<Command::Identifier> FormattingCommand::Idents() const
{
    return { { "-F", true } };
}

HelpDescriptor FormattingCommand::Help() const
{
    return
    {
        "-F<TYPE> [" + CommandLine::GetBooleanOption() + "]",
        "Enables/disables the specified formatting option; valid types:",
        (
            "blanks        => blank lines between declarations; default=" + CommandLine::GetBooleanTrue() + "\n"    \
            "force-braces  => force braces for scopes; default=" + CommandLine::GetBooleanFalse() + "\n"            \
            "compact       => write compact wrapper functions; default=" + CommandLine::GetBooleanFalse() + "\n"    \
            "line-marks    => line marks (e.g. '#line 30'); default=" + CommandLine::GetBooleanFalse() + "\n"       \
            "line-sep      => separate lines in columns; default=" + CommandLine::GetBooleanTrue() + "\n"           \
            "newline-scope => write open braces at new lines; default=" + CommandLine::GetBooleanTrue()
        )
    };
}

void FormattingCommand::Run(CommandLine& cmdLine, ShellState& state)
{
    state.outputDesc.formatting.blanks = cmdLine.AcceptBoolean(true);

    auto type = cmdLine.Accept();

    if (type == "blanks")
        state.outputDesc.formatting.blanks = cmdLine.AcceptBoolean(true);
    else if (type == "force-braces")
        state.outputDesc.formatting.alwaysBracedScopes = cmdLine.AcceptBoolean(true);
    else if (type == "compact")
        state.outputDesc.formatting.compactWrappers = cmdLine.AcceptBoolean(true);
    else if (type == "line-marks")
        state.outputDesc.formatting.lineMarks = cmdLine.AcceptBoolean(true);
    else if (type == "line-sep")
        state.outputDesc.formatting.lineSeparation = cmdLine.AcceptBoolean(true);
    else if (type == "newline-scope")
        state.outputDesc.formatting.newLineOpenScope = cmdLine.AcceptBoolean(true);
    else
        throw std::invalid_argument("invalid formatting type '" + type + "'");
}


/*
 * IndentCommand class
 */

std::vector<Command::Identifier> IndentCommand::Idents() const
{
    return { { "--indent" } };
}

HelpDescriptor IndentCommand::Help() const
{
    return
    {
        "--indent INDENT",
        "Code indentation string (use '\\t' for tabs); default='    '"
    };
}

void IndentCommand::Run(CommandLine& cmdLine, ShellState& state)
{
    state.outputDesc.formatting.indent = cmdLine.Accept();
    Replace(state.outputDesc.formatting.indent, "\\t", "\t");
}


/*
 * PrefixCommand class
 */

std::vector<Command::Identifier> PrefixCommand::Idents() const
{
    return { { "-P", true } };
}

HelpDescriptor PrefixCommand::Help() const
{
    return
    {
        "-P<TYPE> VALUE",
        "Prefix for the specified name-mangling type; valid types:",
        (
            "in        => input variables; default='xsv_'\n"   \
            "namespace => namespace objects; default='xsn_'\n" \
            "out       => output variables; default='xsv_'\n"  \
            "reserved  => reserved words; default='xsr_'\n"    \
            "temp      => temporary variables; default='xst_'"
        )
    };
}

void PrefixCommand::Run(CommandLine& cmdLine, ShellState& state)
{
    auto type = cmdLine.Accept();

    if (type == "in")
        state.outputDesc.nameMangling.inputPrefix = cmdLine.Accept();
    else if (type == "out")
        state.outputDesc.nameMangling.outputPrefix = cmdLine.Accept();
    else if (type == "reserved")
        state.outputDesc.nameMangling.reservedWordPrefix = cmdLine.Accept();
    else if (type == "temp")
        state.outputDesc.nameMangling.temporaryPrefix = cmdLine.Accept();
    else if (type == "namespace")
        state.outputDesc.nameMangling.namespacePrefix = cmdLine.Accept();
    else
        throw std::invalid_argument("invalid prefix type '" + type + "'");
}


/*
 * NameManglingCommand class
 */

std::vector<Command::Identifier> NameManglingCommand::Idents() const
{
    return { { "-N", true } };
}

HelpDescriptor NameManglingCommand::Help() const
{
    return
    {
        "-N<TYPE> [" + CommandLine::GetBooleanOption() + "]",
        "Enables/disables the specified name-mangling option; value types:",
        (
            "buffer-fields   => rename 'buffer' fields, not its identifier; default=" + CommandLine::GetBooleanFalse() + "\n" \
            "force-semantics => force semantics for input/output variables; default=" + CommandLine::GetBooleanFalse()
        )
    };
}

void NameManglingCommand::Run(CommandLine& cmdLine, ShellState& state)
{
    auto type = cmdLine.Accept();

    if (type == "buffer-fields")
        state.outputDesc.nameMangling.renameBufferFields = cmdLine.AcceptBoolean(true);
    else if (type == "force-semantics")
        state.outputDesc.nameMangling.useAlwaysSemantics = cmdLine.AcceptBoolean(true);
    else
        throw std::invalid_argument("invalid name-mangling type '" + type + "'");
}


/*
 * SeparateShadersCommand class
 */

std::vector<Command::Identifier> SeparateShadersCommand::Idents() const
{
    return { { "--separate-shaders" } };
}

HelpDescriptor SeparateShadersCommand::Help() const
{
    return
    {
        "--separate-shaders [" + CommandLine::GetBooleanOption() + "]",
        "Ensures compatibility to 'ARB_separate_shader_objects' extension; default=" + CommandLine::GetBooleanFalse()
    };
}

void SeparateShadersCommand::Run(CommandLine& cmdLine, ShellState& state)
{
    state.outputDesc.options.separateShaders = cmdLine.AcceptBoolean(true);
}


/*
 * SeparateSamplersCommand class
 */

std::vector<Command::Identifier> SeparateSamplersCommand::Idents() const
{
    return { { "--separate-samplers" } };
}

HelpDescriptor SeparateSamplersCommand::Help() const
{
    return
    {
        "--separate-samplers [" + CommandLine::GetBooleanOption() + "]",
        "Enables/disables generation of separate sampler state objects; default=" + CommandLine::GetBooleanTrue()
    };
}

void SeparateSamplersCommand::Run(CommandLine& cmdLine, ShellState& state)
{
    state.outputDesc.options.separateSamplers = cmdLine.AcceptBoolean(true);
}


#ifdef XSC_ENABLE_LANGUAGE_EXT

/*
 * LanguageExtensionCommand class
 */

std::vector<Command::Identifier> LanguageExtensionCommand::Idents() const
{
    return { { "-X", true } };
}

HelpDescriptor LanguageExtensionCommand::Help() const
{
    return
    {
        "-X<TYPE> [" + CommandLine::GetBooleanOption() + "]",
        "Enables/disables the specified language extension; default=" + CommandLine::GetBooleanFalse() + "; value types:",
        (
            "all         => enables all extensions\n"                                           \
            "attr-layout => extension for 'layout' attribute to specify image layout format"
        )
    };
}

void LanguageExtensionCommand::Run(CommandLine& cmdLine, ShellState& state)
{
    const auto flags = MapStringToType<unsigned int>(
        cmdLine.Accept(),
        {
            { "all",         Extensions::All             },
            { "attr-layout", Extensions::LayoutAttribute },
        },
        "invalid extension type"
    );

    if (cmdLine.AcceptBoolean(true))
        state.inputDesc.extensions |= flags;
    else
        state.inputDesc.extensions &= (~flags);
}

#endif


} // /namespace Util

} // /namespace Xsc



// ================================================================================
