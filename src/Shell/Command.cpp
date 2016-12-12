/*
 * Command.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "Command.h"
#include "CommandFactory.h"
#include "Shell.h"
#include "Helper.h"
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
    return { "-E, --entry ENTRY", "Shader entry point" };
}

void EntryCommand::Run(CommandLine& cmdLine, ShellState& state)
{
    state.inputDesc.entryPoint = cmdLine.Accept();
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
        "-T, --target TARGET", "Shader target; valid values:",
        "vert, tesc, tese, geom, frag, comp"
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
    return { "-Vin, --version-in VERSION", "Input shader version; default=HLSL5; valid values:", "HLSL3, HLSL4, HLSL5" };
}

void VersionInCommand::Run(CommandLine& cmdLine, ShellState& state)
{
    state.inputDesc.shaderVersion = MapStringToType<InputShaderVersion>(
        cmdLine.Accept(),
        {
            { "HLSL3", InputShaderVersion::HLSL3 },
            { "HLSL4", InputShaderVersion::HLSL4 },
            { "HLSL5", InputShaderVersion::HLSL5 },
        },
        "invalid input shader version"
    );
}


/*
 * ShaderOutCommand class
 */

std::vector<Command::Identifier> VersionOutCommand::Idents() const
{
    return { { "-Vout" }, { "--version-out" } };
}

HelpDescriptor VersionOutCommand::Help() const
{
    return
    {
        "-Vout, --version-out VERSION", "Shader output version; default=GLSL; valid values:",
        (
            "GLSL[110, 120, 130, 140, 150, 330, 400, 410, 420, 430, 440, 450],\n" \
            "ESSL[100, 300, 310, 320],\n" \
            "VKSL[450]"
        )
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
 * IndentCommand class
 */

std::vector<Command::Identifier> IndentCommand::Idents() const
{
    return { { "--indent" } };
}

HelpDescriptor IndentCommand::Help() const
{
    return { "--indent INDENT", "Code indentation string (use '\\t' for tabs); default='    '" };
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
    return { { "--prefix" } };
}

HelpDescriptor PrefixCommand::Help() const
{
    return { "--prefix PREFIX", "Prefix for name-mangling of variables with reserved names; default='xsc_'" };
}

void PrefixCommand::Run(CommandLine& cmdLine, ShellState& state)
{
    state.outputDesc.formatting.prefix = cmdLine.Accept();
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
    return { "-o, --output FILE", "Shader output file (use '*' for default); default='<FILE>.<ENTRY>.<TARGET>'" };
}

void OutputCommand::Run(CommandLine& cmdLine, ShellState& state)
{
    state.outputFilename = cmdLine.Accept();
}


/*
 * WarnCommand class
 */

std::vector<Command::Identifier> WarnCommand::Idents() const
{
    return { { "-W" }, { "--warnings" } };
}

HelpDescriptor WarnCommand::Help() const
{
    return
    {
        "-W, --warnings [" + CommandLine::GetBooleanOption() + "]",
        "Enables/disables all warnings; default=" + CommandLine::GetBooleanFalse()
    };
}

void WarnCommand::Run(CommandLine& cmdLine, ShellState& state)
{
    state.outputDesc.options.warnings = cmdLine.AcceptBoolean(true);
}


/*
 * BlanksCommand class
 */

std::vector<Command::Identifier> BlanksCommand::Idents() const
{
    return { { "--blanks" } };
}

HelpDescriptor BlanksCommand::Help() const
{
    return
    {
        "--blanks [" + CommandLine::GetBooleanOption() + "]",
        "Enables/disables generation of blank lines between declarations; default=" + CommandLine::GetBooleanTrue()
    };
}

void BlanksCommand::Run(CommandLine& cmdLine, ShellState& state)
{
    state.outputDesc.formatting.blanks = cmdLine.AcceptBoolean(true);
}


/*
 * LineMarksCommand class
 */

std::vector<Command::Identifier> LineMarksCommand::Idents() const
{
    return { { "--line-marks" } };
}

HelpDescriptor LineMarksCommand::Help() const
{
    return
    {
        "--line-marks [" + CommandLine::GetBooleanOption() + "]",
        "Enables/disables generation of line marks (e.g. '#line 30'); default=" + CommandLine::GetBooleanFalse()
    };
}

void LineMarksCommand::Run(CommandLine& cmdLine, ShellState& state)
{
    state.outputDesc.formatting.lineMarks = cmdLine.AcceptBoolean(true);
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
 * PauseCommand class
 */

std::vector<Command::Identifier> PauseCommand::Idents() const
{
    return { { "--pause" } };
}

HelpDescriptor PauseCommand::Help() const
{
    return { "--pause", "Waits for user input after the translation process" };
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
    return { "-PS, --presetting FILE", "Parse further arguments from the presetting file" };
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

    auto RunPresetting = [&](Presetting& preset)
    {
        std::cout << "run presetting: \"" << preset.title << "\"" << std::endl;
        auto shell = Shell::Instance();
        shell->PushState();
        shell->ExecuteCommandLine(preset.cmdLine);
        shell->PopState();
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
            /* Let user choose between one of the presettings */
            std::cout << "choose presetting:" << std::endl;
            std::cout << "  0.) ALL" << std::endl;

            for (std::size_t i = 0; i < presettings.size(); ++i)
                std::cout << "  " << (i+1) << ".) " << presettings[i].title << std::endl;

            std::cin >> idx;
        }

        if (idx == 0)
        {
            /* Run all presettings */
            for (auto& preset : presettings)
                RunPresetting(preset);
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
    return { "--version", "Prints the version information" };
}

void VersionCommand::Run(CommandLine& cmdLine, ShellState& state)
{
    ConsoleManip::ScopedColor highlight(std::cout, ConsoleManip::ColorFlags::Green | ConsoleManip::ColorFlags::Blue);
    std::cout << "XShaderCompiler ( Version " << XSC_VERSION_STRING << " )" << std::endl;
    std::cout << "Copyright (c) 2014-2016 by Lukas Hermanns" << std::endl;
    std::cout << "3-Clause BSD License" << std::endl;
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
    return { "--help", "Prints this help reference" };
}

void HelpCommand::Run(CommandLine& cmdLine, ShellState& state)
{
    std::cout << "Usage:" << std::endl;
    std::cout << "  xsc (OPTION+ FILE)+" << std::endl;
    std::cout << "Options:" << std::endl;

    CommandFactory::Instance().GetHelpPrinter().PrintAll(std::cout, 2);

    std::cout << "Example:" << std::endl;
    std::cout << "  xsc -E VS -T vert Example.hlsl -E PS -T frag Example.hlsl" << std::endl;
    std::cout << "   -> Output files: 'Example.VS.vert', and 'Example.PS.frag'" << std::endl;
}


/*
 * IncludePathCommand class
 */

std::vector<Command::Identifier> IncludePathCommand::Idents() const
{
    return { { "-I" }, { "--include-path" } };
}

HelpDescriptor IncludePathCommand::Help() const
{
    return { "-I, --include-path PATH", "Adds PATH to the search include paths" };
}

void IncludePathCommand::Run(CommandLine& cmdLine, ShellState& state)
{
    state.searchPaths.push_back(cmdLine.Accept());
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


} // /namespace Util

} // /namespace Xsc



// ================================================================================
