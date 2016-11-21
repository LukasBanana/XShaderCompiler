/*
 * Xsc.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <Xsc/Xsc.h>
#include "PreProcessor.h"
#include "HLSLParser.h"
#include "HLSLAnalyzer.h"
#include "GLSLGenerator.h"
#include "Optimizer.h"
#include "ASTPrinter.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <array>


namespace Xsc
{


XSC_EXPORT std::string TargetToString(const ShaderTarget target)
{
    switch (target)
    {
        case ShaderTarget::VertexShader:
            return "Vertex Shader";
        case ShaderTarget::FragmentShader:
            return "Fragment Shader";
        case ShaderTarget::GeometryShader:
            return "Geometry Shader";
        case ShaderTarget::TessellationControlShader:
            return "Tessellation-Control Shader";
        case ShaderTarget::TessellationEvaluationShader:
            return "Tessellation-Evaluation Shader";
        case ShaderTarget::ComputeShader:
            return "Compute Shader";
    }
    return "";
}

using Time      = std::chrono::high_resolution_clock;
using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;

static bool CompileShaderPrimary(
    const ShaderInput& inputDesc, const ShaderOutput& outputDesc, Log* log, std::array<TimePoint, 6>& timePoints)
{
    auto SubmitError = [log](const char* msg)
    {
        if (log)
            log->SumitReport(Report(Report::Types::Error, msg));
        return false;
    };

    /* Validate arguments */
    if (!inputDesc.sourceCode)
        throw std::invalid_argument("input stream must not be null");
    if (!outputDesc.sourceCode)
        throw std::invalid_argument("output stream must not be null");

    /* Pre-process input code */
    timePoints[0] = Time::now();

    std::unique_ptr<IncludeHandler> stdIncludeHandler;
    if (!inputDesc.includeHandler)
        stdIncludeHandler = std::unique_ptr<IncludeHandler>(new IncludeHandler());

    PreProcessor preProcessor(
        (inputDesc.includeHandler != nullptr ? *inputDesc.includeHandler : *stdIncludeHandler),
        log
    );

    auto processedInput = preProcessor.Process(
        std::make_shared<SourceCode>(inputDesc.sourceCode),
        inputDesc.filename
    );

    if (outputDesc.statistics)
        outputDesc.statistics->macros = preProcessor.ListDefinedMacroIdents();

    if (!processedInput)
        return SubmitError("preprocessing input code failed");

    if (outputDesc.options.preprocessOnly)
    {
        (*outputDesc.sourceCode) << processedInput->rdbuf();
        return true;
    }

    /* Parse HLSL input code */
    timePoints[1] = Time::now();

    HLSLParser parser(log);
    auto program = parser.ParseSource(std::make_shared<SourceCode>(std::move(processedInput)));

    if (!program)
        return SubmitError("parsing input code failed");

    /* Small context analysis */
    timePoints[2] = Time::now();

    HLSLAnalyzer analyzer(log);
    auto analyzerResult = analyzer.DecorateAST(*program, inputDesc, outputDesc);

    /* Print debug output */
    if (outputDesc.options.dumpAST && log)
    {
        ASTPrinter printer;
        printer.DumpAST(program.get(), *log);
    }

    if (!analyzerResult)
        return SubmitError("analyzing input code failed");

    /* Optimize AST */
    timePoints[3] = Time::now();

    if (outputDesc.options.optimize)
    {
        Optimizer optimizer;
        optimizer.Optimize(*program);
    }

    /* Generate GLSL output code */
    timePoints[4] = Time::now();

    GLSLGenerator generator(log);
    if (!generator.GenerateCode(*program, inputDesc, outputDesc, log))
        return SubmitError("generating output code failed");

    timePoints[5] = Time::now();

    return true;
}

XSC_EXPORT bool CompileShader(const ShaderInput& inputDesc, const ShaderOutput& outputDesc, Log* log)
{
    std::array<TimePoint, 6> timePoints;

    /* Compile shader with primary function */
    auto result = CompileShaderPrimary(inputDesc, outputDesc, log, timePoints);

    /* Sort statistics */
    if (outputDesc.statistics)
    {
        auto SortStats = [](std::vector<Statistics::Binding>& objects)
        {
            std::sort(
                objects.begin(), objects.end(),
                [](const Statistics::Binding& lhs, const Statistics::Binding& rhs)
                {
                    return (lhs.location < rhs.location);
                }
            );
        };

        SortStats(outputDesc.statistics->textures);
        SortStats(outputDesc.statistics->constantBuffers);
        SortStats(outputDesc.statistics->fragmentTargets);
    }

    /* Dump timings */
    if (outputDesc.options.dumpTimes && log)
    {
        auto PrintTimePoint = [log](const std::string& processName, const TimePoint startTime, const TimePoint endTime)
        {
            auto duration = (endTime > startTime ? std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::duration<float>(endTime - startTime)).count() : 0ll);
            log->SumitReport(Report(Report::Types::Info, "timing " + processName + std::to_string(duration) + " ms"));
        };

        PrintTimePoint("pre-processing:   ", timePoints[0], timePoints[1]);
        PrintTimePoint("parsing:          ", timePoints[1], timePoints[2]);
        PrintTimePoint("context analysis: ", timePoints[2], timePoints[3]);
        PrintTimePoint("optimization:     ", timePoints[3], timePoints[4]);
        PrintTimePoint("code generation:  ", timePoints[4], timePoints[5]);
    }

    return result;
}


} // /namespace Xsc



// ================================================================================
