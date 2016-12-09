/*
 * Generator.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "Generator.h"
#include "AST.h"
#include <sstream>
#include <ctime>
#include <chrono>
#include <iomanip>


namespace Xsc
{


Generator::Generator(Log* log) :
    reportHandler_{ "code generation", log }
{
}

bool Generator::GenerateCode(
    Program& program, const ShaderInput& inputDesc, const ShaderOutput& outputDesc, Log* log)
{
    /* Store parameters */
    writer_.SetIndent(outputDesc.formatting.indent);

    allowBlanks_    = outputDesc.formatting.blanks;
    program_        = &program;

    try
    {
        writer_.OutputStream(*outputDesc.sourceCode);
        GenerateCodePrimary(program, inputDesc, outputDesc);
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
    reportHandler_.Error(true, msg, program_->sourceCode.get(), (ast ? ast->area : SourceArea::ignore));
}

void Generator::ErrorInvalidNumArgs(const std::string& functionName, const AST* ast)
{
    Error("invalid number of arguments for " + functionName, ast);
}

void Generator::Warning(const std::string& msg, const AST* ast)
{
    reportHandler_.Warning(false, msg, program_->sourceCode.get(), (ast ? ast->area : SourceArea::ignore));
}

void Generator::BeginLn()
{
    writer_.BeginLine();
}

void Generator::EndLn()
{
    writer_.EndLine();
}

bool Generator::IsOpenLine() const
{
    return writer_.IsOpenLine();
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
    auto currentTime    = std::chrono::system_clock::now();
    auto date           = std::chrono::system_clock::to_time_t(currentTime);

    std::stringstream s;
    s << std::put_time(std::localtime(&date), "%d/%m/%Y %H:%M:%S");

    return s.str();
}


} // /namespace Xsc



// ================================================================================
