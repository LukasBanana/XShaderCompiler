/*
 * Shell.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "Shell.h"
#include "CommandFactory.h"
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

Shell::Shell(std::ostream& output) :
    output{ output }
{
}

void Shell::ExecuteCommandLine(CommandLine& cmdLine)
{
    if (cmdLine.ReachedEnd())
    {
        std::cout << "no input : enter \"xsc help\"" << std::endl;
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
                cmd->Run(cmdLine, state);
            }
            else
            {
                /* Compile specified shader file */
                Compile(cmdName);

                /* Reset output filename and entry point */
                state.outputFilename.clear();
                state.inputDesc.entryPoint.clear();
            }
        }
    }
    catch (const std::exception& e)
    {
        /* Print error message */
        std::cerr << e.what() << std::endl;
    }

    /* Wait for user input (if enabled) */
    #ifdef _WIN32
    if (state.pauseApp)
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

void Shell::Compile(const std::string& filename)
{
    auto outputFilename = state.outputFilename;

    if (outputFilename.empty())
    {
        /* Set default output filename */
        outputFilename = ExtractFilename(filename);
        if (!state.inputDesc.entryPoint.empty())
            outputFilename += "." + state.inputDesc.entryPoint;
        outputFilename += ".glsl";
    }

    try
    {
        /* Add pre-defined macros at the top of the input stream */
        auto inputStream = std::make_shared<std::stringstream>();
        
        for (const auto& macro : state.predefinedMacros)
        {
            *inputStream << "#define " << macro.ident;
            if (!macro.value.empty())
                *inputStream << ' ' << macro.value;
            *inputStream << std::endl;
        }

        /* Open input stream */
        state.inputDesc.filename = filename;

        std::ifstream inputFile(filename);
        if (!inputFile.good())
            throw std::runtime_error("failed to read file: \"" + filename + "\"");

        *inputStream << inputFile.rdbuf();

        /* Open output stream */
        std::ofstream outputStream(outputFilename);
        if (!outputStream.good())
            throw std::runtime_error("failed to write file: \"" + filename + "\"");

        /* Initialize input and output descriptors */
        state.inputDesc.sourceCode  = inputStream;
        state.outputDesc.sourceCode = &outputStream;

        /* Final setup before compilation */
        StdLog          log;
        IncludeHandler  includeHandler;
        Statistics      stats;
        
        includeHandler.searchPaths = state.searchPaths;
        state.inputDesc.includeHandler = &includeHandler;

        if (state.dumpStats)
            state.outputDesc.statistics = &stats;

        /* Compile shader file */
        output << "compile " << filename << " to " << outputFilename << std::endl;

        auto result = CompileShader(state.inputDesc, state.outputDesc, &log);

        log.PrintAll(state.verbose);

        if (result)
        {
            output << "translation successful" << std::endl;

            /* Show output statistics (if enabled) */
            if (state.dumpStats)
                ShowStats(stats);
        }
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
    ShowStatsFor(stats.textures, "texture bindings");
    ShowStatsFor(stats.constantBuffers, "constant buffer bindings");
    ShowStatsFor(stats.fragmentTargets, "fragment target bindings");
}

void Shell::ShowStatsFor(const std::vector<Statistics::Binding>& objects, const std::string& title)
{
    const std::string indent("  ");
    output << indent << title << ':' << std::endl;

    if (!objects.empty())
    {
        /* Determine offset for right-aligned location index */
        int maxLocation = 0;
        for (const auto& obj : objects)
            maxLocation = std::max(maxLocation, obj.location);

        auto NumDigits = [](int n) -> std::size_t
        {
            return (n > 0u ? static_cast<std::size_t>(std::log10(n)) + 1u : 1u);
        };

        std::size_t maxLocationLen = NumDigits(maxLocation);

        /* Print binding points */
        for (const auto& obj : objects)
        {
            if (obj.location >= 0)
                output << indent << indent << std::string(maxLocationLen - NumDigits(obj.location), ' ') << obj.location << ": " << obj.ident << std::endl;
            else
                output << indent << indent << std::string(maxLocationLen, ' ') << "  " << obj.ident << std::endl;
        }
    }
    else
        output << indent << indent << "< none >" << std::endl;
}


} // /namespace Util

} // /namespace Xsc



// ================================================================================