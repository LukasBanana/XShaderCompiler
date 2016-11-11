/*
 * Generator.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "Generator.h"
#include "AST.h"
#include <ctime>
#include <chrono>


namespace Xsc
{


bool Generator::GenerateCode(
    Program& program, const ShaderInput& inputDesc, const ShaderOutput& outputDesc, Log* log)
{
    /* Store parameters */
    writer_.SetIndent(outputDesc.options.indent);
    allowBlanks_ = outputDesc.options.blanks;

    try
    {
        writer_.OutputStream(*outputDesc.sourceCode);
        GeneratePrimaryCode(program, inputDesc, outputDesc);
    }
    catch (const Report& err)
    {
        if (log)
            log->SumitReport(err);
        return false;
    }

    return true;
}


/*
 * ======= Private: =======
 */

void Generator::Error(const std::string& msg, const AST* ast)
{
    std::string fullMsg = "code generation error";

    if (ast)
        fullMsg += (" (" + ast->area.pos.ToString() + ") : " + msg);
    else
        fullMsg += (" : " + msg);

    throw Report(Report::Types::Error, fullMsg);
}

void Generator::ErrorInvalidNumArgs(const std::string& functionName, const AST* ast)
{
    Error("invalid number of arguments for " + functionName, ast);
}

void Generator::BeginLn()
{
    writer_.BeginLine();
}

void Generator::EndLn()
{
    writer_.EndLine();
}

void Generator::Write(const std::string& text)
{
    writer_.Write(text);
}

void Generator::WriteLn(const std::string& text)
{
    writer_.WriteLine(text);
}

void Generator::IncIndent()
{
    writer_.IncIndent();
}

void Generator::DecIndent()
{
    writer_.DecIndent();
}

void Generator::PushOptions(const CodeWriter::Options& options)
{
    writer_.PushOptions(options);
}

void Generator::PopOptions()
{
    writer_.PopOptions();
}

void Generator::Blank()
{
    if (allowBlanks_)
        WriteLn("");
}

std::string Generator::TimePoint() const
{
    /* Determine current time point */
    const auto currentTime  = std::chrono::system_clock::now();
    const auto duration     = std::chrono::duration_cast<std::chrono::seconds>(currentTime.time_since_epoch());
    const auto date         = static_cast<std::time_t>(duration.count());
    
    /* Get time point as string */
    auto timePoint = std::string(std::ctime(&date));

    /* Remove new-line character at the end */
    if (!timePoint.empty() && timePoint.back() == '\n')
        timePoint.resize(timePoint.size() - 1);

    return timePoint;
}


} // /namespace Xsc



// ================================================================================
