/*
 * Generator.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "Generator.h"
#include "AST.h"
#include "ReportIdents.h"
#include <sstream>
#include <ctime>
#include <chrono>
#include <iomanip>


namespace Xsc
{


Generator::Generator(Log* log) :
    reportHandler_{ R_CodeGeneration, log }
{
}

bool Generator::GenerateCode(
    Program& program, const ShaderInput& inputDesc, const ShaderOutput& outputDesc, Log* log)
{
    /* Store parameters */
    writer_.SetIndent(outputDesc.formatting.indent);

    shaderTarget_               = inputDesc.shaderTarget;
    warnings_                   = inputDesc.warnings;
    allowBlanks_                = outputDesc.formatting.blanks;
    allowLineSeparation_        = outputDesc.formatting.lineSeparation;
    writer_.newLineOpenScope    = outputDesc.formatting.newLineOpenScope;
    program_                    = &program;

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

    return (!reportHandler_.HasErros());
}


/*
 * ======= Private: =======
 */

void Generator::Error(const std::string& msg, const AST* ast, bool breakWithExpection)
{
    reportHandler_.Error(breakWithExpection, msg, program_->sourceCode.get(), (ast ? ast->area : SourceArea::ignore));
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

void Generator::BeginSep()
{
    if (allowLineSeparation_)
        writer_.BeginSeparation();
}

void Generator::EndSep()
{
    if (allowLineSeparation_)
        writer_.EndSeparation();
}

void Generator::Separator()
{
    writer_.Separator();
}

void Generator::WriteScopeOpen(bool compact, bool endWithSemicolon, bool useBraces)
{
    writer_.BeginScope(compact, endWithSemicolon, useBraces);
}

void Generator::WriteScopeClose()
{
    writer_.EndScope();
}

void Generator::WriteScopeContinue()
{
    writer_.ContinueScope();
}

bool Generator::IsOpenLine() const
{
    return writer_.IsOpenLine();
}

void Generator::Write(const std::string& text)
{
    FlushWritePrefixes();
    writer_.Write(text);
}

void Generator::WriteLn(const std::string& text)
{
    FlushWritePrefixes();
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

void Generator::PushWritePrefix(const std::string& text)
{
    writePrefixStack_.push_back({ text, false });
}

void Generator::PopWritePrefix()
{
    if (writePrefixStack_.empty())
        throw std::underflow_error(R_WritePrefixStackUnderflow);
    else
        writePrefixStack_.pop_back();
}

bool Generator::TopWritePrefix() const
{
    return (writePrefixStack_.empty() ? false : writePrefixStack_.back().written);
}

//private
void Generator::FlushWritePrefixes()
{
    /* Write prefixes from first to last */
    for (auto& prefix : writePrefixStack_)
    {
        if (!prefix.written)
        {
            writer_.Write(prefix.text);
            prefix.written = true;
        }
    }
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

bool Generator::WarnEnabled(unsigned int flags) const
{
    return warnings_(flags);
}

bool Generator::IsVertexShader() const
{
    return (shaderTarget_ == ShaderTarget::VertexShader);
}

bool Generator::IsTessControlShader() const
{
    return (shaderTarget_ == ShaderTarget::TessellationControlShader);
}

bool Generator::IsTessEvaluationShader() const
{
    return (shaderTarget_ == ShaderTarget::TessellationEvaluationShader);
}

bool Generator::IsGeometryShader() const
{
    return (shaderTarget_ == ShaderTarget::GeometryShader);
}

bool Generator::IsFragmentShader() const
{
    return (shaderTarget_ == ShaderTarget::FragmentShader);
}

bool Generator::IsComputeShader() const
{
    return (shaderTarget_ == ShaderTarget::ComputeShader);
}


} // /namespace Xsc



// ================================================================================
