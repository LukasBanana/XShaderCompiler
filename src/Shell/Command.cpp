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
#include "ReportIdents.h"
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


using namespace ConsoleManip;

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

static void SetFlagsByBoolean(CommandLine& cmdLine, unsigned int& flagsOut, unsigned int flags)
{
    if (cmdLine.AcceptBoolean(true))
        flagsOut |= flags;
    else
        flagsOut &= (~flags);
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
        R_CmdHelpEntry,
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
        R_CmdHelpSecndEntry,
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
        "-T, --target TARGET", R_CmdHelpTarget,
        "vert, tesc, tese, geom, frag, comp",
        HelpCategory::Main
    };
}

void TargetCommand::Run(CommandLine& cmdLine, ShellState& state)
{
    const auto target = cmdLine.Accept();

    state.inputDesc.shaderTarget = MapStringToType<ShaderTarget>(
        target,
        {
            { "vert", ShaderTarget::VertexShader                 },
            { "tesc", ShaderTarget::TessellationControlShader    },
            { "tese", ShaderTarget::TessellationEvaluationShader },
            { "geom", ShaderTarget::GeometryShader               },
            { "frag", ShaderTarget::FragmentShader               },
            { "comp", ShaderTarget::ComputeShader                },
        },
        R_InvalidShaderTarget(target)
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
        R_CmdHelpVersionIn,
        "Cg, HLSL3, HLSL4, HLSL5, HLSL6, GLSL, ESSL, VKSL",
        HelpCategory::Main
    };
}

void VersionInCommand::Run(CommandLine& cmdLine, ShellState& state)
{
    const auto version = cmdLine.Accept();

    state.inputDesc.shaderVersion = MapStringToType<InputShaderVersion>(
        version,
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
        R_InvalidShaderVersionIn(version)
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
        R_CmdHelpVersionOut,
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
    const auto version = cmdLine.Accept();

    state.outputDesc.shaderVersion = MapStringToType<OutputShaderVersion>(
        version,
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
        R_InvalidShaderVersionOut(version)
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
        R_CmdHelpOutput,
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
        R_CmdHelpIncludePath,
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
        R_CmdHelpWarn(CommandLine::GetBooleanFalse()),
        R_CmdHelpDetailsWarn
    };
}

void WarnCommand::Run(CommandLine& cmdLine, ShellState& state)
{
    const auto type = cmdLine.Accept();

    const auto flags = MapStringToType<unsigned int>(
        type,
        {
            { "all",           Warnings::All                     },
            { "basic",         Warnings::Basic                   },
            { "decl-shadow",   Warnings::DeclarationShadowing    },
            { "empty-body",    Warnings::EmptyStatementBody      },
            { "extension",     Warnings::RequiredExtensions      },
            { "implicit-cast", Warnings::ImplicitTypeConversions },
            { "index-bound",   Warnings::IndexBoundary           },
            { "preprocessor",  Warnings::PreProcessor            },
            { "reflect",       Warnings::CodeReflection          },
            { "syntax",        Warnings::Syntax                  },
            { "unlocated-obj", Warnings::UnlocatedObjects        },
            { "unused-vars",   Warnings::UnusedVariables         },
        },
        R_InvalidWarningType(type)
    );

    SetFlagsByBoolean(cmdLine, state.inputDesc.warnings, flags);
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
        R_CmdHelpShowAST(CommandLine::GetBooleanFalse())
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
        R_CmdHelpShowTimes(CommandLine::GetBooleanFalse())
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
        R_CmdHelpReflect(CommandLine::GetBooleanFalse())
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
        R_CmdHelpPPOnly(CommandLine::GetBooleanFalse())
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
        R_CmdHelpMacro
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
        R_CmdHelpSemantic
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
        throw std::runtime_error(R_VertexAttribValueExpectedFor(arg));
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
        R_CmdHelpPause
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
        R_CmdHelpPresetting
    };
}

void PresettingCommand::Run(CommandLine& cmdLine, ShellState& state)
{
    static std::set<std::string> pressetingFilenames;

    auto filename = cmdLine.Accept();

    /* Check if this presetting file has already been processed */
    if (pressetingFilenames.find(filename) != pressetingFilenames.end())
        throw std::runtime_error(R_LoopInPresettingFiles);

    pressetingFilenames.insert(filename);

    /* Read arguments from file */
    std::ifstream file(filename);
    if (!file.good())
        throw std::runtime_error(R_FailedToReadFile(filename));

    struct Presetting
    {
        std::string title;
        CommandLine cmdLine;
    };

    auto RunPresetting = [&](Presetting& preset) -> bool
    {
        std::cout << R_RunPresetting(preset.title) << std::endl;
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
            std::cout << R_ChoosePresetting() << ':' << std::endl;
            std::cout << "  " << Indent(0) << "0.) " << R_ChooseAllPresettings() << std::endl;

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
                ScopedColor scopedColor(ColorFlags::Green | ColorFlags::Intens);
                std::cout << R_PresettingsSucceeded() << std::endl;
            }
            else
            {
                ScopedColor scopedColor(ColorFlags::Red | ColorFlags::Intens);
                std::cout << R_PresettingsFailed(numFailed, presettings.size()) << std::endl;
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
        std::cout << R_NoPresettingsDefined() << std::endl;
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
        R_CmdHelpVersion
    };
}

void VersionCommand::Run(CommandLine& cmdLine, ShellState& state)
{
    /* Print version info in highlighted color */
    ScopedColor scopedColor(ColorFlags::Green | ColorFlags::Blue);

    /* Print version */
    std::cout << "XShaderCompiler ( ";
    {
        ScopedColor scopedColor(ColorFlags::Green | ColorFlags::Blue | ColorFlags::Intens);
        std::cout << "Version " << XSC_VERSION_STRING;
    }
    
    #ifdef XSC_ENABLE_LANGUAGE_EXT
    std::cout << "; ";
    {
        ScopedColor scopedColor(ColorFlags::Green | ColorFlags::Blue | ColorFlags::Intens);
        std::cout << "Ext";
    }
    #endif
    
    #ifdef _DEBUG
    std::cout << "; ";
    {
        ScopedColor scopedColor(ColorFlags::Green | ColorFlags::Blue | ColorFlags::Intens);
        std::cout << "DEBUG";
    }
    #endif

    std::cout << " )" << std::endl;
    
    /* Print copyright and license notice */
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
        R_CmdHelpHelp
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
        R_CmdHelpVerbose(CommandLine::GetBooleanTrue())
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
        R_CmdHelpColor(CommandLine::GetBooleanTrue())
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
        R_CmdHelpOptimize(CommandLine::GetBooleanFalse())
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
        R_CmdHelpExtension(CommandLine::GetBooleanFalse())
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
        R_CmdHelpEnumExtension
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
        R_CmdHelpValidate(CommandLine::GetBooleanFalse())
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
        R_CmdHelpBinding(CommandLine::GetBooleanFalse())
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
        R_CmdHelpAutoBinding(CommandLine::GetBooleanFalse())
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
        R_CmdHelpAutoBindingStartSlot
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
        R_CmdHelpComment(CommandLine::GetBooleanFalse())
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
        R_CmdHelpWrapper(CommandLine::GetBooleanFalse())
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
        R_CmdHelpUnrollInitializer(CommandLine::GetBooleanFalse())
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
        R_CmdHelpObfuscate(CommandLine::GetBooleanFalse())
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
        R_CmdHelpRowMajorAlignment(CommandLine::GetBooleanFalse())
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
        R_CmdHelpFormatting,
        R_CmdHelpDetailsFormatting(CommandLine::GetBooleanFalse(), CommandLine::GetBooleanTrue())
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
        throw std::invalid_argument(R_InvalidFormattingType(type));
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
        R_CmdHelpIndent
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
        R_CmdHelpPrefix,
        R_CmdHelpDetailsPrefix
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
        throw std::invalid_argument(R_InvalidPrefixType(type));
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
        R_CmdHelpNameMangling,
        R_CmdHelpDetailsNameMangling(CommandLine::GetBooleanFalse())
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
        throw std::invalid_argument(R_InvalidNameManglingType(type));
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
        R_CmdHelpSeparateShaders(CommandLine::GetBooleanFalse())
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
        R_CmdHelpSeparateSamplers(CommandLine::GetBooleanTrue())
    };
}

void SeparateSamplersCommand::Run(CommandLine& cmdLine, ShellState& state)
{
    state.outputDesc.options.separateSamplers = cmdLine.AcceptBoolean(true);
}


/*
 * DisassembleCommand class
 */

std::vector<Command::Identifier> DisassembleCommand::Idents() const
{
    return { { "-dasm" }, { "--disassemble" } };
}

HelpDescriptor DisassembleCommand::Help() const
{
    return
    {
        "-dasm, --disassemble FILE",
        R_CmdHelpDisassemble
    };
}

static void DisassembleCommandPrimary(CommandLine& cmdLine, ShellState& state, bool showNames)
{
    const auto filename = cmdLine.Accept();

    AssemblyDescriptor desc;
    desc.showNames = showNames;
    
    std::ifstream file(filename, std::ios::binary);
    DisassembleShader(file, std::cout, desc);

    state.actionPerformed = true;
}

void DisassembleCommand::Run(CommandLine& cmdLine, ShellState& state)
{
    DisassembleCommandPrimary(cmdLine, state, false);
}


/*
 * DisassembleExtCommand class
 */

std::vector<Command::Identifier> DisassembleExtCommand::Idents() const
{
    return { { "-dasmx" }, { "--disassemble-ext" } };
}

HelpDescriptor DisassembleExtCommand::Help() const
{
    return
    {
        "-dasmx, --disassemble-ext FILE",
        R_CmdHelpDisassembleExt
    };
}

void DisassembleExtCommand::Run(CommandLine& cmdLine, ShellState& state)
{
    DisassembleCommandPrimary(cmdLine, state, true);
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
        R_CmdHelpLanguageExtension(CommandLine::GetBooleanFalse()),
        R_CmdHelpDetailsLanguageExtension
    };
}

void LanguageExtensionCommand::Run(CommandLine& cmdLine, ShellState& state)
{
    const auto type = cmdLine.Accept();

    const auto flags = MapStringToType<unsigned int>(
        type,
        {
            { "all",         Extensions::All             },
            { "attr-layout", Extensions::LayoutAttribute },
            { "attr-space",  Extensions::SpaceAttribute  },
        },
        R_InvalidExtensionType(type)
    );

    SetFlagsByBoolean(cmdLine, state.inputDesc.extensions, flags);
}

#endif


} // /namespace Util

} // /namespace Xsc



// ================================================================================
