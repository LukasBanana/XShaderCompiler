/*
 * Shell.cpp
 * 
 * This file is part of the "HLSL Translator" (Copyright (c) 2014 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "Shell.h"
#include "CommandFactory.h"
#include <iostream>
#include <sstream>
#include <fstream>

#ifdef _WIN32
#include <conio.h>
#endif


namespace Xsc
{

namespace Util
{


Shell::Shell(std::ostream& output) :
    output{ output }
{
}

void Shell::ExecuteCommandLine(CommandLine& cmdLine)
{
    if (cmdLine.ReachedEnd())
    {
        std::cout << "no input : enter \"HTLibCmd help\"" << std::endl;
        return;
    }

    try
    {
        /* Pares all arguments from command line */
        while (!cmdLine.ReachedEnd())
        {
            /* Get next command */
            auto cmdName = cmdLine.Accept();

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
                /* Translate specified shader file */
                Translate(cmdName);

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

void Shell::Translate(const std::string& filename)
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

        /* Translate HLSL file into GLSL */
        StdLog log;

        output << "translate from " << filename << " to " << outputFilename << std::endl;

        auto result = TranslateHLSLtoGLSL(state.inputDesc, state.outputDesc, &log);

        log.PrintAll();

        if (result)
            output << "translation successful" << std::endl;
    }
    catch (const std::exception& err)
    {
        /* Print error message */
        output << err.what() << std::endl;
    }
}


} // /namespace Util

} // /namespace Xsc



// ================================================================================