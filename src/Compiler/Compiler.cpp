/*
 * Compiler.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "Compiler.h"
#include "ReportIdents.h"
#include "Helper.h"

#include "PreProcessor.h"
#include "Optimizer.h"
#include "ReflectionAnalyzer.h"
#include "ASTPrinter.h"

#include "GLSLPreProcessor.h"
#include "GLSLParser.h"
#include "GLSLGenerator.h"

#include "HLSLParser.h"
#include "HLSLAnalyzer.h"
#include "HLSLIntrinsics.h"

#include <sstream>
#include <stdexcept>


namespace Xsc
{


Compiler::Compiler(Log* log) :
    log_ { log }
{
}

bool Compiler::CompileShader(
    const ShaderInput&          inputDesc,
    const ShaderOutput&         outputDesc,
    Reflection::ReflectionData* reflectionData,
    StageTimePoints*            stageTimePoints)
{
    /* Make copy of output descriptor to support validation without output stream */
    std::stringstream dummyOutputStream;

    auto outputDescCopy = outputDesc;

    if (!IsLanguageHLSL(inputDesc.shaderVersion) && !outputDesc.options.preprocessOnly)
    {
        Warning(R_GLSLFrontendIsIncomplete);
        outputDescCopy.options.validateOnly = true;
    }

    if (outputDescCopy.options.validateOnly)
        outputDescCopy.sourceCode = &dummyOutputStream;

    /* Implicitly enable 'explicitBinding' option of 'autoBinding' is enabled */
    if (outputDescCopy.options.autoBinding)
        outputDescCopy.options.explicitBinding = true;

    /* Compile shader with primary function */
    auto result = CompileShaderPrimary(inputDesc, outputDescCopy, reflectionData);

    /* Copy time points to output */
    if (stageTimePoints)
        *stageTimePoints = timePoints_;

    return result;
}


/*
 * ======= Private: =======
 */

bool Compiler::ReturnWithError(const std::string& msg)
{
    if (log_)
        log_->SubmitReport(Report(ReportTypes::Error, msg));
    return false;
}

void Compiler::Warning(const std::string& msg)
{
    if (log_)
        log_->SubmitReport(Report(ReportTypes::Warning, msg));
}

void Compiler::ValidateArguments(const ShaderInput& inputDesc, const ShaderOutput& outputDesc)
{
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
    if (inputDesc.extensions != 0)
        Warning(R_LangExtensionsNotSupported);

    #endif
}

bool Compiler::CompileShaderPrimary(
    const ShaderInput&          inputDesc,
    const ShaderOutput&         outputDesc,
    Reflection::ReflectionData* reflectionData)
{
    /* Validate arguments */
    ValidateArguments(inputDesc, outputDesc);

    /* ----- Pre-processing ----- */

    timePoints_.preprocessor = Time::now();

    std::unique_ptr<IncludeHandler> stdIncludeHandler;
    if (!inputDesc.includeHandler)
        stdIncludeHandler = std::unique_ptr<IncludeHandler>(new IncludeHandler());

    auto includeHandler = (inputDesc.includeHandler != nullptr ? inputDesc.includeHandler : stdIncludeHandler.get());

    std::unique_ptr<PreProcessor> preProcessor;

    if (IsLanguageHLSL(inputDesc.shaderVersion))
        preProcessor = MakeUnique<PreProcessor>(*includeHandler, log_);
    else if (IsLanguageGLSL(inputDesc.shaderVersion))
        preProcessor = MakeUnique<GLSLPreProcessor>(*includeHandler, log_);

    const bool writeLineMarksInPP = (!outputDesc.options.preprocessOnly || outputDesc.formatting.lineMarks);
    const bool writeLineMarkFilenamesInPP = (!outputDesc.options.preprocessOnly || IsLanguageHLSL(inputDesc.shaderVersion));

    auto processedInput = preProcessor->Process(
        std::make_shared<SourceCode>(inputDesc.sourceCode),
        inputDesc.filename,
        writeLineMarksInPP,
        writeLineMarkFilenamesInPP,
        ((inputDesc.warnings & Warnings::PreProcessor) != 0)
    );

    if (reflectionData)
        reflectionData->macros = preProcessor->ListDefinedMacroIdents();

    if (!processedInput)
        return ReturnWithError(R_PreProcessingSourceFailed);

    if (outputDesc.options.preprocessOnly)
    {
        (*outputDesc.sourceCode) << processedInput->rdbuf();
        return true;
    }

    /* ----- Parsing ----- */

    timePoints_.parser = Time::now();

    std::unique_ptr<IntrinsicAdept> intrinsicAdpet;
    ProgramPtr program;

    if (IsLanguageHLSL(inputDesc.shaderVersion))
    {
        /* Establish intrinsic adept */
        intrinsicAdpet = MakeUnique<HLSLIntrinsicAdept>();

        /* Parse HLSL input code */
        HLSLParser parser(log_);
        program = parser.ParseSource(
            std::make_shared<SourceCode>(std::move(processedInput)),
            outputDesc.nameMangling,
            inputDesc.shaderVersion,
            outputDesc.options.rowMajorAlignment,
            ((inputDesc.warnings & Warnings::Syntax) != 0)
        );
    }
    else if (IsLanguageGLSL(inputDesc.shaderVersion))
    {
        /* Establish intrinsic adept */
        #if 0
        intrinsicAdpet = MakeUnique<GLSLIntrinsicAdept>();
        #else //!!!
        intrinsicAdpet = MakeUnique<HLSLIntrinsicAdept>();
        #endif

        /* Parse GLSL input code */
        GLSLParser parser(log_);
        program = parser.ParseSource(
            std::make_shared<SourceCode>(std::move(processedInput)),
            outputDesc.nameMangling,
            inputDesc.shaderVersion,
            ((inputDesc.warnings & Warnings::Syntax) != 0)
        );
    }

    if (!program)
        return ReturnWithError(R_ParsingSourceFailed);

    /* ----- Context analysis ----- */

    timePoints_.analyzer = Time::now();

    bool analyzerResult = false;

    if (IsLanguageHLSL(inputDesc.shaderVersion))
    {
        /* Analyse HLSL program */
        HLSLAnalyzer analyzer(log_);
        analyzerResult = analyzer.DecorateAST(*program, inputDesc, outputDesc);
    }

    /* Print AST */
    if (outputDesc.options.showAST)
    {
        ASTPrinter printer;
        printer.PrintAST(program.get());
    }

    if (!analyzerResult)
        return ReturnWithError(R_AnalyzingSourceFailed);

    /* Optimize AST */
    timePoints_.optimizer = Time::now();

    if (outputDesc.options.optimize)
    {
        Optimizer optimizer;
        optimizer.Optimize(*program);
    }

    /* ----- Code generation ----- */

    timePoints_.generation = Time::now();

    bool generatorResult = false;

    if (IsLanguageGLSL(outputDesc.shaderVersion) || IsLanguageESSL(outputDesc.shaderVersion) || IsLanguageVKSL(outputDesc.shaderVersion))
    {
        /* Generate GLSL output code */
        GLSLGenerator generator(log_);
        generatorResult = generator.GenerateCode(*program, inputDesc, outputDesc, log_);
    }

    if (!generatorResult)
        return ReturnWithError(R_GeneratingOutputCodeFailed);

    /* ----- Code reflection ----- */

    timePoints_.reflection = Time::now();

    if (reflectionData)
    {
        ReflectionAnalyzer reflectAnalyzer(log_);
        reflectAnalyzer.Reflect(
            *program, inputDesc.shaderTarget, *reflectionData,
            ((inputDesc.warnings & Warnings::CodeReflection) != 0)
        );
    }

    return true;
}


} // /namespace Xsc



// ================================================================================