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

void Shell::Compile(const std::string& filename)
{
    auto outputFilename = state_.outputFilename;

    if (outputFilename.empty())
    {
        /* Set default output filename */
        outputFilename = ExtractFilename(filename);
        if (!state_.inputDesc.entryPoint.empty())
            outputFilename += "." + state_.inputDesc.entryPoint;
        outputFilename += ".glsl";
    }

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

        /* Open output stream */
        std::ofstream outputStream(outputFilename);
        if (!outputStream.good())
            throw std::runtime_error("failed to write file: \"" + filename + "\"");

        /* Initialize input and output descriptors */
        state_.inputDesc.sourceCode  = inputStream;
        state_.outputDesc.sourceCode = &outputStream;

        /* Final setup before compilation */
        StdLog          log;
        IncludeHandler  includeHandler;
        Statistics      stats;
        
        includeHandler.searchPaths = state_.searchPaths;
        state_.inputDesc.includeHandler = &includeHandler;

        if (state_.dumpStats)
            state_.outputDesc.statistics = &stats;

        /* Compile shader file */
        output << "compile " << filename << " to " << outputFilename << std::endl;

        auto result = CompileShader(state_.inputDesc, state_.outputDesc, &log);

        log.PrintAll(state_.verbose);

        if (result)
        {
            output << "translation successful" << std::endl;

            /* Show output statistics (if enabled) */
            if (state_.dumpStats)
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
    indentHandler_.IncIndent();
    {
        ShowStatsFor(stats.textures, "texture bindings");
        ShowStatsFor(stats.constantBuffers, "constant buffer bindings");
        ShowStatsFor(stats.fragmentTargets, "fragment target bindings");
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
                    output << indentHandler_.FullIndent() << std::string(maxLocationLen - NumDigits(obj.location), ' ') << obj.location << ": " << obj.ident << std::endl;
                else
                    output << indentHandler_.FullIndent() << std::string(maxLocationLen, ' ') << "  " << obj.ident << std::endl;
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