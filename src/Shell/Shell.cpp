/*
 * Shell.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "Shell.h"
#include "CommandFactory.h"
#include "Helper.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <cmath>

#ifdef _WIN32
#include <conio.h>
#endif


namespace Xsc
{

namespace Util
{


#ifdef XSC_ENABLE_EASTER_EGGS

static void PrintBackdoorEasterEgg(std::ostream& output)
{
    std::cout << "here is your backdoor :-)" << std::endl;
    std::cout << " _____ " << std::endl;
    std::cout << "| ___ |" << std::endl;
    std::cout << "||___||" << std::endl;
    std::cout << "|   ~o|" << std::endl;
    std::cout << "|     |" << std::endl;
    std::cout << "|_____|" << std::endl;
    std::cout << "-------" << std::endl;
}

#endif

Shell* Shell::instance_ = nullptr;

Shell::Shell(std::ostream& output) :
    output{ output }
{
    Shell::instance_ = this;
}

Shell::~Shell()
{
    Shell::instance_ = nullptr;
}

Shell* Shell::Instance()
{
    return instance_;
}

void Shell::ExecuteCommandLine(CommandLine& cmdLine)
{
    if (cmdLine.ReachedEnd())
    {
        /* Print help */
        if (auto cmd = CommandFactory::Instance().Get("--help"))
            cmd->Run(cmdLine, state_);
        return;
    }

    try
    {
        /* Pares all arguments from command line */
        while (!cmdLine.ReachedEnd())
        {
            /* Get next command */
            auto cmdName = cmdLine.Accept();

            #ifdef XSC_ENABLE_EASTER_EGGS

            if (cmdName == "--backdoor")
            {
                PrintBackdoorEasterEgg(output);
                continue;
            }

            #endif

            Command::Identifier cmdIdent;
            auto cmd = CommandFactory::Instance().Get(cmdName, &cmdIdent);

            if (cmd)
            {
                /* Check if value is included within the command name */
                if (cmdIdent.includesValue)
                {
                    if (cmdName.size() > cmdIdent.name.size())
                        cmdLine.Insert(cmdName.substr(cmdIdent.name.size()));
                    else
                        throw std::invalid_argument("missing value in command '" + cmdIdent.name + "'");
                }

                /* Run command */
                cmd->Run(cmdLine, state_);
            }
            else
            {
                /* Compile specified shader file */
                Compile(cmdName);

                /* Reset output filename and entry point */
                state_.outputFilename.clear();
                state_.inputDesc.entryPoint.clear();
            }
        }
    }
    catch (const std::exception& e)
    {
        /* Print error message */
        std::cerr << e.what() << std::endl;
    }
}

void Shell::WaitForUser()
{
    /* Wait for user input (if enabled) */
    #ifdef _WIN32
    if (state_.pauseApp)
    {
        std::cout << "press any key to continue ...";
        int i = _getch();
        std::cout << std::endl;
    }
    #endif
}


/*
 * ======= Private: =======
 */

static std::string ExtractFilename(const std::string& filename)
{
    auto pos = filename.find_last_of('.');
    return (pos == std::string::npos ? filename : filename.substr(0, pos));
}

static std::string TargetToExtension(const ShaderTarget shaderTarget)
{
    switch (shaderTarget)
    {
        case ShaderTarget::Undefined:                       break;
        case ShaderTarget::VertexShader:                    return "vert";
        case ShaderTarget::TessellationControlShader:       return "tesc";
        case ShaderTarget::TessellationEvaluationShader:    return "tese";
        case ShaderTarget::GeometryShader:                  return "geom";
        case ShaderTarget::FragmentShader:                  return "frag";
        case ShaderTarget::ComputeShader:                   return "comp";
    }
    return "glsl";
}

std::string Shell::GetDefaultOutputFilename(const std::string& filename)
{
    return (ExtractFilename(filename) + "." + state_.inputDesc.entryPoint + "." + TargetToExtension(state_.inputDesc.shaderTarget));
}

void Shell::Compile(const std::string& filename)
{
    const auto  defaultOutputFilename   = GetDefaultOutputFilename(filename);
    auto        outputFilename          = state_.outputFilename;

    if (outputFilename.empty())
        outputFilename = defaultOutputFilename;
    else
        Replace(outputFilename, "*", defaultOutputFilename);

    try
    {
        /* Add pre-defined macros at the top of the input stream */
        auto inputStream = std::make_shared<std::stringstream>();
        
        for (const auto& macro : state_.predefinedMacros)
        {
            *inputStream << "#define " << macro.ident;
            if (!macro.value.empty())
                *inputStream << ' ' << macro.value;
            *inputStream << std::endl;
        }

        /* Open input stream */
        state_.inputDesc.filename = filename;

        std::ifstream inputFile(filename);
        if (!inputFile.good())
            throw std::runtime_error("failed to read file: \"" + filename + "\"");

        *inputStream << inputFile.rdbuf();

        std::stringstream outputStream;

        /* Initialize input and output descriptors */
        state_.inputDesc.sourceCode  = inputStream;
        state_.outputDesc.sourceCode = &outputStream;

        /* Final setup before compilation */
        StdLog          log;
        IncludeHandler  includeHandler;
        Statistics      stats;
        
        includeHandler.searchPaths = state_.searchPaths;
        state_.inputDesc.includeHandler = &includeHandler;

        if (state_.showStats)
            state_.outputDesc.statistics = &stats;

        /* Show compilation/validation status */
        if (state_.outputDesc.options.validateOnly)
            output << "validate \"" << filename << '\"' << std::endl;
        else
            output << "compile \"" << filename << "\" to \"" << outputFilename << '\"' << std::endl;

        /* Compile shader file */
        auto result = CompileShader(state_.inputDesc, state_.outputDesc, &log);

        /* Print all reports to the log output */
        log.PrintAll(state_.verbose, state_.outputDesc.options.warnings);

        if (result)
        {
            if (!state_.outputDesc.options.validateOnly)
            {
                output << "compilation successful" << std::endl;

                /* Write result to output stream only on success */
                std::ofstream outputFile(outputFilename);
                if (outputFile.good())
                    outputFile << outputStream.rdbuf();
                else
                    throw std::runtime_error("failed to write file: \"" + filename + "\"");
            }
            else
                output << "validation successful" << std::endl;
        }
        else
        {
            if (state_.outputDesc.options.validateOnly)
                output << "validation failed" << std::endl;
            else
                output << "compilation failed" << std::endl;
        }

        /* Show output statistics (if enabled) */
        if (state_.showStats)
            ShowStats(stats);
    }
    catch (const std::exception& err)
    {
        /* Print error message */
        output << err.what() << std::endl;
    }
}

void Shell::ShowStats(const Statistics& stats)
{
    output << "statistics:" << std::endl;
    indentHandler_.IncIndent();
    {
        ShowStatsFor( stats.macros,          "macros"                   );
        ShowStatsFor( stats.textures,        "texture bindings"         );
        ShowStatsFor( stats.constantBuffers, "constant buffer bindings" );
        ShowStatsFor( stats.fragmentTargets, "fragment target bindings" );
        ShowStatsFor( stats.samplerStates,   "sampler states"           );
    }
    indentHandler_.DecIndent();
}

void Shell::ShowStatsFor(const std::vector<Statistics::Binding>& objects, const std::string& title)
{
    output << indentHandler_.FullIndent() << title << ':' << std::endl;
    indentHandler_.IncIndent();
    {
        if (!objects.empty())
        {
            /* Determine offset for right-aligned location index */
            int maxLocation = -1;
            for (const auto& obj : objects)
                maxLocation = std::max(maxLocation, obj.location);

            std::size_t maxLocationLen = std::to_string(maxLocation).size();

            /* Print binding points */
            for (const auto& obj : objects)
            {
                output << indentHandler_.FullIndent();
                if (maxLocation >= 0)
                {
                    if (obj.location >= 0)
                        output << std::string(maxLocationLen - std::to_string(obj.location).size(), ' ') << obj.location << ": ";
                    else
                        output << std::string(maxLocationLen, ' ') << "  ";
                }
                output << obj.ident << std::endl;
            }
        }
        else
            output << indentHandler_.FullIndent() << "< none >" << std::endl;
    }
    indentHandler_.DecIndent();
}

void Shell::ShowStatsFor(const std::vector<std::string>& idents, const std::string& title)
{
    output << indentHandler_.FullIndent() << title << ':' << std::endl;
    indentHandler_.IncIndent();
    {
        if (!idents.empty())
        {
            for (const auto& i : idents)
                output << indentHandler_.FullIndent() << i << std::endl;
        }
        else
            output << indentHandler_.FullIndent() << "< none >" << std::endl;
    }
    indentHandler_.DecIndent();
}

void Shell::ShowStatsFor(const std::map<std::string, SamplerState>& samplerStates, const std::string& title)
{
    output << indentHandler_.FullIndent() << title << ':' << std::endl;
    indentHandler_.IncIndent();
    {
        if (!samplerStates.empty())
        {
            for (const auto& it : samplerStates)
            {
                output << indentHandler_.FullIndent() << it.first << std::endl;
                indentHandler_.IncIndent();
                {
                    const auto& smpl = it.second;
                    const auto& brdCol = smpl.borderColor;
                    output << indentHandler_.FullIndent() << "AddressU       = " << static_cast<int>(smpl.addressU) << std::endl;
                    output << indentHandler_.FullIndent() << "AddressV       = " << static_cast<int>(smpl.addressV) << std::endl;
                    output << indentHandler_.FullIndent() << "AddressW       = " << static_cast<int>(smpl.addressW) << std::endl;
                    output << indentHandler_.FullIndent() << "BorderColor    = { " << brdCol[0] << ", " << brdCol[1] << ", " << brdCol[2] << ", " << brdCol[3] << " }" << std::endl;
                    output << indentHandler_.FullIndent() << "ComparisonFunc = " << static_cast<int>(smpl.comparisonFunc) << std::endl;
                    output << indentHandler_.FullIndent() << "Filter         = " << static_cast<int>(smpl.filter) << std::endl;
                    output << indentHandler_.FullIndent() << "MaxAnisotropy  = " << smpl.maxAnisotropy << std::endl;
                    output << indentHandler_.FullIndent() << "MaxLOD         = " << smpl.maxLOD << std::endl;
                    output << indentHandler_.FullIndent() << "MinLOD         = " << smpl.minLOD << std::endl;
                    output << indentHandler_.FullIndent() << "MipLODBias     = " << smpl.mipLODBias << std::endl;
                }
                indentHandler_.DecIndent();
            }
        }
        else
            output << indentHandler_.FullIndent() << "< none >" << std::endl;
    }
    indentHandler_.DecIndent();
}


} // /namespace Util

} // /namespace Xsc



// ================================================================================
