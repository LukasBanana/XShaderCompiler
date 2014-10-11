/*
 * GLSLGenerator.cpp
 * 
 * This file is part of the "HLSL Translator" (Copyright (c) 2014 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLSLGenerator.h"

#include <ctime>
#include <chrono>


namespace HTLib
{


/*
 * Internal functions
 */

static std::string TimePoint()
{
    /* Determine current time point */
    const auto currentTime = std::chrono::high_resolution_clock::now();
    const auto duration = std::chrono::duration_cast<std::chrono::seconds>(currentTime.time_since_epoch());
    const auto date = duration.count();
    
    /* Get time point as string */
    auto timePoint = std::string(std::ctime(&date));

    /* Remove new-line character at the end */
    if (!timePoint.empty() && timePoint.back() == '\n')
        timePoint.resize(timePoint.size() - 1);

    return timePoint;
}

static std::string TargetToString(const ShaderTargets shaderTarget)
{
    switch (shaderTarget)
    {
        case ShaderTargets::GLSLVertexShader:
            return "Vertex";
        case ShaderTargets::GLSLFragmentShader:
            return "Fragment";
        case ShaderTargets::GLSLGeometryShader:
            return "Geometry";
        case ShaderTargets::GLSLTessControlShader:
            return "Tessellation Control";
        case ShaderTargets::GLSLTessEvaluationShader:
            return "Tessellation Evaluation";
        case ShaderTargets::GLSLComputeShader:
            return "Compute";
    }
    return "";
}


/*
 * GLSLGenerator class
 */

GLSLGenerator::GLSLGenerator(Logger* log, IncludeHandler* includeHandler, const Options& options) :
    writer_         { options.indent },
    includeHandler_ { includeHandler },
    log_            { log            }
{
}

bool GLSLGenerator::GenerateCode(
    std::ostream& output,
    const std::string& entryPoint,
    const ShaderTargets shaderTarget,
    const ShaderVersions shaderVersion)
{
    try
    {
        writer_.OutputStream(output);

        /* Write header */
        Comment("GLSL " + TargetToString(shaderTarget) + " Shader");
        Comment("Generated from HLSL Shader \"" + entryPoint + "\"");
        Comment(TimePoint());
        Version(static_cast<int>(shaderVersion));

        //...
    }
    catch (const std::exception& err)
    {
        if (log_)
            log_->Error(err.what());
        return false;
    }
    return true;
}


/*
 * ======= Private: =======
 */

void GLSLGenerator::BeginLn()
{
    writer_.BeginLine();
}

void GLSLGenerator::EndLn()
{
    writer_.EndLine();
}

void GLSLGenerator::Write(const std::string& text)
{
    writer_.Write(text);
}

void GLSLGenerator::WriteLn(const std::string& text)
{
    writer_.WriteLine(text);
}

void GLSLGenerator::IncTab()
{
    writer_.PushIndent();
}

void GLSLGenerator::DecTab()
{
    writer_.PopIndent();
}

void GLSLGenerator::Comment(const std::string& text)
{
    WriteLn("// " + text);
}

void GLSLGenerator::Version(int versionNumber)
{
    WriteLn("#version " + std::to_string(versionNumber));
}

void GLSLGenerator::Line(int lineNumber)
{
    WriteLn("#line " + std::to_string(lineNumber));
}

void GLSLGenerator::Line(const TokenPtr& tkn)
{
    Line(tkn->Pos().Row());
}

void GLSLGenerator::OpenScope()
{
    WriteLn("{");
    IncTab();
}

void GLSLGenerator::CloseScope()
{
    DecTab();
    WriteLn("}");
}



} // /namespace HTLib



// ================================================================================