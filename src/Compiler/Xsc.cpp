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
#include "ReflectionAnalyzer.h"
#include "ReflectionPrinter.h"
#include "ASTPrinter.h"
#include "ASTEnums.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <array>


namespace Xsc
{


/*
 * Internal functions
 */

using Time      = std::chrono::system_clock;
using TimePoint = std::chrono::time_point<std::chrono::system_clock>;

static bool CompileShaderPrimary(
    const ShaderInput& inputDesc, const ShaderOutput& outputDesc,
    Log* log, Reflection::ReflectionData* reflectionData,
    std::array<TimePoint, 6>& timePoints)
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

    if (reflectionData)
        reflectionData->macros = preProcessor.ListDefinedMacroIdents();

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

    /* Print AST */
    if (outputDesc.options.showAST && log)
    {
        ASTPrinter printer;
        printer.PrintAST(program.get(), *log);
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

    /* Reflect shader code */
    timePoints[5] = Time::now();

    if (reflectionData)
    {
        ReflectionAnalyzer reflectAnalyzer(log);
        reflectAnalyzer.Reflect(*program, inputDesc.shaderTarget, *reflectionData);
    }

    return true;
}


/*
 * Public functions
 */

XSC_EXPORT bool CompileShader(
    const ShaderInput& inputDesc, const ShaderOutput& outputDesc,
    Log* log, Reflection::ReflectionData* reflectionData)
{
    std::array<TimePoint, 6> timePoints;

    /* Make copy of output descriptor to support validation without output stream */
    std::stringstream dummyOutputStream;

    auto outputDescCopy = outputDesc;

    if (outputDescCopy.options.validateOnly)
        outputDescCopy.sourceCode = &dummyOutputStream;

    /* Compile shader with primary function */
    auto result = CompileShaderPrimary(inputDesc, outputDescCopy, log, reflectionData, timePoints);

    if (reflectionData)
    {
        /* Sort reflection */
        auto SortStats = [](std::vector<Reflection::BindingSlot>& objects)
        {
            std::sort(
                objects.begin(), objects.end(),
                [](const Reflection::BindingSlot& lhs, const Reflection::BindingSlot& rhs)
                {
                    return (lhs.location < rhs.location);
                }
            );
        };

        SortStats(reflectionData->textures);
        SortStats(reflectionData->constantBuffers);
        SortStats(reflectionData->inputAttributes);
        SortStats(reflectionData->outputAttributes);
    }

    /* Show timings */
    if (outputDescCopy.options.showTimes && log)
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

XSC_EXPORT std::string ToString(const ShaderTarget target)
{
    switch (target)
    {
        case ShaderTarget::Undefined:                       return "Undefined";
        case ShaderTarget::VertexShader:                    return "Vertex Shader";
        case ShaderTarget::FragmentShader:                  return "Fragment Shader";
        case ShaderTarget::GeometryShader:                  return "Geometry Shader";
        case ShaderTarget::TessellationControlShader:       return "Tessellation-Control Shader";
        case ShaderTarget::TessellationEvaluationShader:    return "Tessellation-Evaluation Shader";
        case ShaderTarget::ComputeShader:                   return "Compute Shader";
    }
    return "";
}

XSC_EXPORT std::string ToString(const InputShaderVersion shaderVersion)
{
    switch (shaderVersion)
    {
        case InputShaderVersion::HLSL3: return "HLSL 3.0";
        case InputShaderVersion::HLSL4: return "HLSL 4.0";
        case InputShaderVersion::HLSL5: return "HLSL 5.0";
    }
    return "";
}

XSC_EXPORT std::string ToString(const OutputShaderVersion shaderVersion)
{
    switch (shaderVersion)
    {
        case OutputShaderVersion::GLSL110:  return "GLSL 1.10";
        case OutputShaderVersion::GLSL120:  return "GLSL 1.20";
        case OutputShaderVersion::GLSL130:  return "GLSL 1.30";
        case OutputShaderVersion::GLSL140:  return "GLSL 1.40";
        case OutputShaderVersion::GLSL150:  return "GLSL 1.50";
        case OutputShaderVersion::GLSL330:  return "GLSL 3.30";
        case OutputShaderVersion::GLSL400:  return "GLSL 4.00";
        case OutputShaderVersion::GLSL410:  return "GLSL 4.10";
        case OutputShaderVersion::GLSL420:  return "GLSL 4.20";
        case OutputShaderVersion::GLSL430:  return "GLSL 4.30";
        case OutputShaderVersion::GLSL440:  return "GLSL 4.40";
        case OutputShaderVersion::GLSL450:  return "GLSL 4.50";
        case OutputShaderVersion::GLSL:     return "GLSL";
        
        case OutputShaderVersion::ESSL100:  return "ESSL 1.00";
        case OutputShaderVersion::ESSL300:  return "ESSL 3.00";
        case OutputShaderVersion::ESSL310:  return "ESSL 3.10";
        case OutputShaderVersion::ESSL320:  return "ESSL 3.20";
        case OutputShaderVersion::ESSL:     return "ESSL";
        
        case OutputShaderVersion::VKSL450:  return "VKSL 4.50";
        case OutputShaderVersion::VKSL:     return "VKSL";
    }
    return "";
}

XSC_EXPORT std::string ToString(const Reflection::Filter t)
{
    return FilterToString(t);
}

XSC_EXPORT std::string ToString(const Reflection::TextureAddressMode t)
{
    return TexAddressModeToString(t);
}

XSC_EXPORT std::string ToString(const Reflection::ComparisonFunc t)
{
    return CompareFuncToString(t);
}

XSC_EXPORT void PrintReflection(std::ostream& stream, const Reflection::ReflectionData& reflectionData)
{
    ReflectionPrinter printer(stream);
    printer.PrintReflection(reflectionData);
}

XSC_EXPORT bool IsLanguageGLSL(const OutputShaderVersion shaderVersion)
{
    return ((shaderVersion >= OutputShaderVersion::GLSL110 && shaderVersion <= OutputShaderVersion::GLSL450) || shaderVersion == OutputShaderVersion::GLSL);
}

XSC_EXPORT bool IsLanguageESSL(const OutputShaderVersion shaderVersion)
{
    return ((shaderVersion >= OutputShaderVersion::ESSL100 && shaderVersion <= OutputShaderVersion::ESSL320) || shaderVersion == OutputShaderVersion::ESSL);
}

XSC_EXPORT bool IsLanguageVKSL(const OutputShaderVersion shaderVersion)
{
    return (shaderVersion == OutputShaderVersion::VKSL450 || shaderVersion == OutputShaderVersion::VKSL);
}


} // /namespace Xsc



// ================================================================================
