/*
 * Command.cpp
 * 
 * This file is part of the "HLSL Translator" (Copyright (c) 2014 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "Command.h"
#include "CommandFactory.h"
#include "Shell.h"
#include <HT/ConsoleManip.h>
#include <map>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <set>


namespace HTLib
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
    return { { "-entry" } };
}

HelpDescriptor EntryCommand::Help() const
{
    return { "-entry ENTRY", "HLSL shader entry point" };
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
    return { { "-target" } };
}

HelpDescriptor TargetCommand::Help() const
{
    return
    {
        "-target TARGET", "Shader target; valid values:",
        "vertex, fragment, geometry, tess-control, tess-evaluation, compute"
    };
}

void TargetCommand::Run(CommandLine& cmdLine, ShellState& state)
{
    state.inputDesc.shaderTarget = MapStringToType<ShaderTarget>(
        cmdLine.Accept(),
        {
          //{ "common",          ShaderTarget::CommonShader             },
            { "vertex",          ShaderTarget::GLSLVertexShader         },
            { "fragment",        ShaderTarget::GLSLFragmentShader       },
            { "geometry",        ShaderTarget::GLSLGeometryShader       },
            { "tess-control",    ShaderTarget::GLSLTessControlShader    },
            { "tess-evaluation", ShaderTarget::GLSLTessEvaluationShader },
            { "compute",         ShaderTarget::GLSLComputeShader        },
        },
        "invalid shader target"
    );
}


/*
 * ShaderInCommand class
 */

std::vector<Command::Identifier> ShaderInCommand::Idents() const
{
    return { { "-shaderin" } };
}

HelpDescriptor ShaderInCommand::Help() const
{
    return { "-shaderin VERSION", "Input shader version; default is HLSL5; valid values:", "HLSL3, HLSL4, HLSL5" };
}

void ShaderInCommand::Run(CommandLine& cmdLine, ShellState& state)
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

std::vector<Command::Identifier> ShaderOutCommand::Idents() const
{
    return { { "-shaderout" } };
}

HelpDescriptor ShaderOutCommand::Help() const
{
    return
    {
        "-shaderout VERSION", "GLSL version; default is GLSL330; valid values:",
        "GLSL110, GLSL120, GLSL130, GLSL140, GLSL150, GLSL330,\n" \
        "GLSL400, GLSL410, GLSL420, GLSL430, GLSL440, GLSL450"
    };
}

void ShaderOutCommand::Run(CommandLine& cmdLine, ShellState& state)
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
        },
        "invalid output shader version"
    );
}


/*
 * IndentCommand class
 */

std::vector<Command::Identifier> IndentCommand::Idents() const
{
    return { { "-indent" } };
}

HelpDescriptor IndentCommand::Help() const
{
    return { "-indent INDENT", "Code indentation string; by default 4 spaces" };
}

void IndentCommand::Run(CommandLine& cmdLine, ShellState& state)
{
    state.outputDesc.options.indent = cmdLine.Accept();
}


/*
 * PrefixCommand class
 */

std::vector<Command::Identifier> PrefixCommand::Idents() const
{
    return { { "-prefix" } };
}

HelpDescriptor PrefixCommand::Help() const
{
    return { "-prefix PREFIX", "Prefix for local variables (use \"<none>\" to disable); by default '_'" };
}

void PrefixCommand::Run(CommandLine& cmdLine, ShellState& state)
{
    auto prefix = cmdLine.Accept();
    state.outputDesc.options.prefix = (prefix == "<none>" ? "" : prefix);
}


/*
 * OutputCommand class
 */

std::vector<Command::Identifier> OutputCommand::Idents() const
{
    return { { "-output" } };
}

HelpDescriptor OutputCommand::Help() const
{
    return { "-output FILE", "Shader output file; default is '<FILE>.<ENTRY>.glsl'" };
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
    return { { "-warn" } };
}

HelpDescriptor WarnCommand::Help() const
{
    return { "-warn [on|off]", "Enables/disables all warnings; by default off" };
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
    return { { "-blanks" } };
}

HelpDescriptor BlanksCommand::Help() const
{
    return { "-blanks [on|off]", "Enables/disables generation of blank lines between declarations; by default on" };
}

void BlanksCommand::Run(CommandLine& cmdLine, ShellState& state)
{
    state.outputDesc.options.blanks = cmdLine.AcceptBoolean(true);
}

//~~~~~~~~~~~~~~~
/*
 * LineMarksCommand class
 */

std::vector<Command::Identifier> LineMarksCommand::Idents() const
{
    return { { "-line-marks" } };
}

HelpDescriptor LineMarksCommand::Help() const
{
    return { "-line-marks [on|off]", "Enables/disables generation of line marks (e.g. '#line 30'); by default off" };
}

void LineMarksCommand::Run(CommandLine& cmdLine, ShellState& state)
{
    state.outputDesc.options.lineMarks = cmdLine.AcceptBoolean(true);
}


/*
 * DumpASTCommand class
 */

std::vector<Command::Identifier> DumpASTCommand::Idents() const
{
    return { { "-dump-ast" } };
}

HelpDescriptor DumpASTCommand::Help() const
{
    return { "-dump-ast [on|off]", "Enables/disables debug output for the abstract syntax tree (AST); by default off" };
}

void DumpASTCommand::Run(CommandLine& cmdLine, ShellState& state)
{
    state.outputDesc.options.dumpAST = cmdLine.AcceptBoolean(true);
}


/*
 * PPOnlyCommand class
 */

std::vector<Command::Identifier> PPOnlyCommand::Idents() const
{
    return { { "-pponly" } };
}

HelpDescriptor PPOnlyCommand::Help() const
{
    return { "-pponly [on|off]", "Enables/disables to only preprocess source code; by default off" };
}

void PPOnlyCommand::Run(CommandLine& cmdLine, ShellState& state)
{
    state.outputDesc.options.preprocessOnly = cmdLine.AcceptBoolean(true);
}


/*
 * CommentsCommand class
 */

std::vector<Command::Identifier> CommentsCommand::Idents() const
{
    return { { "-comments" } };
}

HelpDescriptor CommentsCommand::Help() const
{
    return { "-comments [on|off]", "Enables/disables commentaries output kept from the sources; by default on" };
}

void CommentsCommand::Run(CommandLine& cmdLine, ShellState& state)
{
    state.outputDesc.options.keepComments = cmdLine.AcceptBoolean(true);
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
    return { "-D<IDENT>, -D<IDENT>=VALUE", "Adds the identifier <IDENT> to the pre-defined macros with an optional VALUE" };
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
    return { { "--presetting" } };
}

HelpDescriptor PresettingCommand::Help() const
{
    return { "--presetting FILE", "Parse further arguments from the presetting file" };
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
        std::string                 title;
        std::vector<std::string>    args;
    };

    auto RunPresetting = [&](const Presetting& preset)
    {
        std::cout << "run presetting: \"" << preset.title << "\"" << std::endl;

        CommandLine subCmdLine(preset.args);
        Shell subShell(std::cout);
        subShell.ExecuteCommandLine(subCmdLine);
    };

    std::vector<Presetting> presettings;

    while (!file.eof())
    {
        /* Get presetting title */
        Presetting preset;
        std::getline(file, preset.title);

        if (!preset.title.empty())
        {
            /* Get presetting arguments */
            std::string line;
            std::getline(file, line);

            std::stringstream lineStream;
            lineStream << line;

            while (!lineStream.eof())
            {
                std::string arg;
                lineStream >> arg;
                preset.args.push_back(arg);
            }

            if (!preset.args.empty())
                presettings.emplace_back(std::move(preset));
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
            for (const auto& preset : presettings)
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
    return { { "--version" }, { "-v" } };
}

HelpDescriptor VersionCommand::Help() const
{
    return { "--version, -v", "Prints the version information" };
}

void VersionCommand::Run(CommandLine& cmdLine, ShellState& state)
{
    ConsoleManip::ScopedColor highlight(std::cout, ConsoleManip::ColorFlags::Green | ConsoleManip::ColorFlags::Blue);
    std::cout << "HLSL Translator ( Version " << HTLIB_VERSION_STRING << " )" << std::endl;
    std::cout << "Copyright (c) 2014-2016 by Lukas Hermanns" << std::endl;
    std::cout << "3-Clause BSD License" << std::endl;
}


/*
 * HelpCommand class
 */

std::vector<Command::Identifier> HelpCommand::Idents() const
{
    return { { "--help" }, { "help" }, { "-h" } };
}

HelpDescriptor HelpCommand::Help() const
{
    return { "--help, help, -h", "Prints this help reference" };
}

void HelpCommand::Run(CommandLine& cmdLine, ShellState& state)
{
    std::cout << "Usage:" << std::endl;
    std::cout << "  HTLibCmd (OPTION+ FILE)+" << std::endl;
    std::cout << "Options:" << std::endl;

    CommandFactory::Instance().GetHelpPrinter().PrintAll(std::cout, 2);

    std::cout << "Example:" << std::endl;
    std::cout << "  HTLibCmd -entry VS -target vertex Example.hlsl -entry PS -target fragment Example.hlsl" << std::endl;
    std::cout << "   --> Example.vertex.glsl; Example.fragment.glsl" << std::endl;
}


} // /namespace Util

} // /namespace HTLib



// ================================================================================