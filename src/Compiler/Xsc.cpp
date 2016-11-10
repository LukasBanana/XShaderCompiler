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
#include "ASTPrinter.h"
#include <fstream>
#include <sstream>


namespace Xsc
{


XSC_EXPORT bool CompileShader(
    const ShaderInput& inputDesc, const ShaderOutput& outputDesc, Log* log)
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

    if (!processedInput)
        return SubmitError("preprocessing input code failed");

    if (outputDesc.options.preprocessOnly)
    {
        *outputDesc.sourceCode << processedInput->rdbuf();
        return true;
    }

    /* Parse HLSL input code */
    HLSLParser parser(log);
    auto program = parser.ParseSource(std::make_shared<SourceCode>(processedInput));

    if (!program)
        return SubmitError("parsing input code failed");

    /* Small context analysis */
    HLSLAnalyzer analyzer(log);
    if (!analyzer.DecorateAST(*program, inputDesc, outputDesc))
        return SubmitError("analyzing input code failed");

    /* Print debug output */
    if (outputDesc.options.dumpAST && log)
    {
        ASTPrinter dumper;
        dumper.DumpAST(program.get(), *log);
    }

    /* Generate GLSL output code */
    GLSLGenerator generator(log, outputDesc.options);
    if (!generator.GenerateCode(*program, inputDesc, outputDesc))
        return SubmitError("generating output code failed");

    return true;
}


} // /namespace Xsc



// ================================================================================