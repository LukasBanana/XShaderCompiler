/*
 * Translator.cpp
 * 
 * This file is part of the "HLSL Translator" (Copyright (c) 2014 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "HT/Translator.h"
#include "PreProcessor.h"
#include "HLSLParser.h"
#include "HLSLAnalyzer.h"
#include "GLSLGenerator.h"
#include "ASTPrinter.h"
#include <fstream>


namespace HTLib
{


/*
 * Default include handler class
 */

class StdIncludeHandler : public IncludeHandler
{

    public:
    
        std::unique_ptr<std::istream> Include(const std::string& filename) override;

};

std::unique_ptr<std::istream> StdIncludeHandler::Include(const std::string& filename)
{
    auto stream = std::unique_ptr<std::istream>(new std::ifstream(filename));
    if (!stream->good())
        throw std::runtime_error("failed to include file: \"" + filename + "\"");
    return stream;
}


/*
 * Public functions
 */

HTLIB_EXPORT bool TranslateHLSLtoGLSL(
    const ShaderInput& inputDesc, const ShaderOutput& outputDesc, Log* log)
{
    /* Validate arguments */
    if (!inputDesc.sourceCode)
        throw std::invalid_argument("input stream must not be null");
    if (!outputDesc.sourceCode)
        throw std::invalid_argument("output stream must not be null");

    /* Pre-process input code */
    std::unique_ptr<IncludeHandler> stdIncludeHandler;
    if (!inputDesc.includeHandler)
        stdIncludeHandler = std::unique_ptr<IncludeHandler>(new StdIncludeHandler());

    PreProcessor preProcessor(
        (inputDesc.includeHandler != nullptr ? *inputDesc.includeHandler : *stdIncludeHandler),
        log
    );

    auto processedInput = preProcessor.Process(std::make_shared<SourceCode>(inputDesc.sourceCode));

    if (!processedInput)
    {
        if (log)
            log->Error("preprocessing input code failed");
        return false;
    }

    /* Parse HLSL input code */
    HLSLParser parser(outputDesc.options, log);
    auto program = parser.ParseSource(std::make_shared<SourceCode>(processedInput));

    if (!program)
    {
        if (log)
            log->Error("parsing input code failed");
        return false;
    }

    /* Small context analysis */
    HLSLAnalyzer analyzer(log);
    if (!analyzer.DecorateAST(program.get(), inputDesc.entryPoint, inputDesc.shaderTarget, inputDesc.shaderVersion, outputDesc.shaderVersion, outputDesc.options))
    {
        if (log)
            log->Error("analyzing input code failed");
        return false;
    }

    /* Print debug output */
    if (outputDesc.options.dumpAST && log)
    {
        ASTPrinter dumper;
        dumper.DumpAST(program.get(), *log);
    }

    /* Generate GLSL output code */
    GLSLGenerator generator(log, outputDesc.options);
    if (!generator.GenerateCode(program.get(), *outputDesc.sourceCode, inputDesc.entryPoint, inputDesc.shaderTarget, inputDesc.shaderVersion, outputDesc.shaderVersion))
    {
        if (log)
            log->Error("generating output code failed");
        return false;
    }

    return true;
}


} // /namespace HTLib



// ================================================================================