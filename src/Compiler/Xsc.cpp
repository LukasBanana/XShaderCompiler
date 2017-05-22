/*
 * Xsc.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <Xsc/Xsc.h>
#include "PreProcessor.h"
#include "GLSLPreProcessor.h"
#include "GLSLGenerator.h"
#include "HLSLParser.h"
#include "HLSLAnalyzer.h"
#include "HLSLIntrinsics.h"
#include "Optimizer.h"
#include "ReflectionAnalyzer.h"
#include "ASTPrinter.h"
#include "ASTEnums.h"
#include "ReportIdents.h"
#include "SPIRVDisassembler.h"
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
    auto SubmitError = [log](const std::string& msg)
    {
        if (log)
            log->SumitReport(Report(ReportTypes::Error, msg));
        return false;
    };

    /* Validate arguments */
    if (!inputDesc.sourceCode)
        throw std::invalid_argument(R_InputStreamCantBeNull);
    
    if (!outputDesc.sourceCode)
        throw std::invalid_argument(R_OutputStreamCantBeNull);

    const auto& nameMngl = outputDesc.nameMangling;
    
    if (nameMngl.reservedWordPrefix.empty())
        throw std::invalid_argument(R_NameManglingPrefixResCantBeEmpty);

    if (nameMngl.temporaryPrefix.empty())
        throw std::invalid_argument(R_NameManglingPrefixTmpCantBeEmpty);
    
    if ( nameMngl.reservedWordPrefix == nameMngl.inputPrefix     ||
         nameMngl.reservedWordPrefix == nameMngl.outputPrefix    ||
         nameMngl.reservedWordPrefix == nameMngl.temporaryPrefix ||
         nameMngl.temporaryPrefix    == nameMngl.inputPrefix     ||
         nameMngl.temporaryPrefix    == nameMngl.outputPrefix )
    {
        throw std::invalid_argument(R_OverlappingNameManglingPrefixes);
    }

    if (!nameMngl.namespacePrefix.empty())
    {
        if ( nameMngl.namespacePrefix == nameMngl.inputPrefix        ||
             nameMngl.namespacePrefix == nameMngl.outputPrefix       ||
             nameMngl.namespacePrefix == nameMngl.reservedWordPrefix ||
             nameMngl.namespacePrefix == nameMngl.temporaryPrefix )
        {
            throw std::invalid_argument(R_OverlappingNameManglingPrefixes);
        }
    }

    #ifndef XSC_ENABLE_LANGUAGE_EXT
    
    /* Report warning, if language extensions acquired but compiler was not build with them */
    if (inputDesc.extensions != 0 && log != nullptr)
        log->SumitReport(Report(ReportTypes::Warning, R_LangExtensionsNotSupported));
    
    #endif

    /* ----- Pre-processing ----- */

    timePoints[0] = Time::now();

    std::unique_ptr<IncludeHandler> stdIncludeHandler;
    if (!inputDesc.includeHandler)
        stdIncludeHandler = std::unique_ptr<IncludeHandler>(new IncludeHandler());

    auto includeHandler = (inputDesc.includeHandler != nullptr ? inputDesc.includeHandler : stdIncludeHandler.get());

    std::unique_ptr<PreProcessor> preProcessor;

    if (IsLanguageHLSL(inputDesc.shaderVersion))
        preProcessor = MakeUnique<PreProcessor>(*includeHandler, log);
    else if (IsLanguageGLSL(inputDesc.shaderVersion))
        preProcessor = MakeUnique<GLSLPreProcessor>(*includeHandler, log);

    auto processedInput = preProcessor->Process(
        std::make_shared<SourceCode>(inputDesc.sourceCode),
        inputDesc.filename,
        true,
        ((inputDesc.warnings & Warnings::PreProcessor) != 0)
    );

    if (reflectionData)
        reflectionData->macros = preProcessor->ListDefinedMacroIdents();

    if (!processedInput)
        return SubmitError(R_PreProcessingSourceFailed);

    if (outputDesc.options.preprocessOnly)
    {
        (*outputDesc.sourceCode) << processedInput->rdbuf();
        return true;
    }

    /* ----- Parsing ----- */

    timePoints[1] = Time::now();

    std::unique_ptr<IntrinsicAdept> intrinsicAdpet;
    ProgramPtr program;

    if (IsLanguageHLSL(inputDesc.shaderVersion))
    {
        /* Establish intrinsic adept */
        intrinsicAdpet = MakeUnique<HLSLIntrinsicAdept>();

        /* Parse HLSL input code */
        HLSLParser parser(log);
        program = parser.ParseSource(
            std::make_shared<SourceCode>(std::move(processedInput)),
            outputDesc.nameMangling,
            inputDesc.shaderVersion,
            outputDesc.options.rowMajorAlignment,
            ((inputDesc.warnings & Warnings::Syntax) != 0)
        );
    }

    if (!program)
        return SubmitError(R_ParsingSourceFailed);

    /* ----- Context analysis ----- */

    timePoints[2] = Time::now();

    bool analyzerResult = false;

    if (IsLanguageHLSL(inputDesc.shaderVersion))
    {
        /* Analyse HLSL program */
        HLSLAnalyzer analyzer(log);
        analyzerResult = analyzer.DecorateAST(*program, inputDesc, outputDesc);
    }

    /* Print AST */
    if (outputDesc.options.showAST && log)
    {
        ASTPrinter printer;
        printer.PrintAST(program.get(), *log);
    }

    if (!analyzerResult)
        return SubmitError(R_AnalyzingSourceFailed);

    /* Optimize AST */
    timePoints[3] = Time::now();

    if (outputDesc.options.optimize)
    {
        Optimizer optimizer;
        optimizer.Optimize(*program);
    }

    /* ----- Code generation ----- */

    timePoints[4] = Time::now();

    bool generatorResult = false;

    if (IsLanguageGLSL(outputDesc.shaderVersion) || IsLanguageESSL(outputDesc.shaderVersion) || IsLanguageVKSL(outputDesc.shaderVersion))
    {
        /* Generate GLSL output code */
        GLSLGenerator generator(log);
        generatorResult = generator.GenerateCode(*program, inputDesc, outputDesc, log);
    }

    if (!generatorResult)
        return SubmitError(R_GeneratingOutputCodeFailed);

    /* ----- Code reflection ----- */

    timePoints[5] = Time::now();

    if (reflectionData)
    {
        ReflectionAnalyzer reflectAnalyzer(log);
        reflectAnalyzer.Reflect(
            *program, inputDesc.shaderTarget, *reflectionData,
            ((inputDesc.warnings & Warnings::CodeReflection) != 0)
        );
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

    /* Check for supported feature */
    if (!IsLanguageHLSL(inputDesc.shaderVersion) && !outputDesc.options.preprocessOnly)
    {
        if (log)
            log->SumitReport(Report(ReportTypes::Error, R_OnlyPreProcessingForNonHLSL));
        return false;
    }

    /* Make copy of output descriptor to support validation without output stream */
    std::stringstream dummyOutputStream;

    auto outputDescCopy = outputDesc;

    if (outputDescCopy.options.validateOnly)
        outputDescCopy.sourceCode = &dummyOutputStream;

    /* Implicitly enable 'explicitBinding' option of 'autoBinding' is enabled */
    if (outputDescCopy.options.autoBinding)
        outputDescCopy.options.explicitBinding = true;

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
            log->SumitReport(Report(ReportTypes::Info, "timing " + processName + std::to_string(duration) + " ms"));
        };

        PrintTimePoint("pre-processing:   ", timePoints[0], timePoints[1]);
        PrintTimePoint("parsing:          ", timePoints[1], timePoints[2]);
        PrintTimePoint("context analysis: ", timePoints[2], timePoints[3]);
        PrintTimePoint("optimization:     ", timePoints[3], timePoints[4]);
        PrintTimePoint("code generation:  ", timePoints[4], timePoints[5]);
    }

    return result;
}

XSC_EXPORT void DisassembleShader(
    std::istream& streamIn, std::ostream& streamOut, const AssemblyFormatting& formatting)
{
    SPIRVDisassembler disassembler;
    disassembler.Parse(streamIn);
    disassembler.Print(streamOut, formatting.idPrefixChar);
}


} // /namespace Xsc



// ================================================================================
