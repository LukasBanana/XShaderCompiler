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
        StdLog                      log;
        IncludeHandler              includeHandler;
        Reflection::ReflectionData  reflectionData;
        
        includeHandler.searchPaths = state_.searchPaths;
        state_.inputDesc.includeHandler = &includeHandler;

        /* Show compilation/validation status */
        if (state_.outputDesc.options.validateOnly)
            output << "validate \"" << filename << '\"' << std::endl;
        else
            output << "compile \"" << filename << "\" to \"" << outputFilename << '\"' << std::endl;

        /* Compile shader file */
        auto result = CompileShader(
            state_.inputDesc,
            state_.outputDesc,
            &log,
            (state_.showReflection ? &reflectionData : nullptr)
        );

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
        if (state_.showReflection)
            PrintReflection(output, reflectionData);
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
