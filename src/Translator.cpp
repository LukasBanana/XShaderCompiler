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


class StdIncludeHandler : public IncludeHandler
{

    public:
    
        std::unique_ptr<std::istream> Include(const std::string& filename) override
        {
            auto stream = std::unique_ptr<std::istream>(new std::ifstream(filename));
            if (!stream->good())
                throw std::runtime_error("failed to include file: \"" + filename + "\"");
            return stream;
        }

};


HTLIB_EXPORT bool TranslateHLSLtoGLSL(
    const std::shared_ptr<std::istream>&    input,
    std::ostream&                           output,
    const std::string&                      entryPoint,
    const ShaderTargets                     shaderTarget,
    const InputShaderVersions               inputShaderVersion,
    const OutputShaderVersions              outputShaderVersion,
    IncludeHandler*                         includeHandler,
    const Options&                          options,
    Log*                                    log)
{
    /* Validate arguments */
    if (!input)
        throw std::invalid_argument("input stream must not be null");

    /* Pre-process input code */
    std::unique_ptr<IncludeHandler> stdIncludeHandler;
    if (!includeHandler)
        stdIncludeHandler = std::unique_ptr<IncludeHandler>(new StdIncludeHandler());

    PreProcessor preProcessor(
        (includeHandler != nullptr ? *includeHandler : *stdIncludeHandler),
        log
    );

    auto processedInput = preProcessor.Process(std::make_shared<SourceCode>(input));

    /* Parse HLSL input code */
    HLSLParser parser(options, log);
    auto program = parser.ParseSource(std::make_shared<SourceCode>(processedInput));

    if (!program)
    {
        if (log)
            log->Error("parsing input code failed");
        return false;
    }

    /* Small context analysis */
    HLSLAnalyzer analyzer(log);
    if (!analyzer.DecorateAST(program.get(), entryPoint, shaderTarget, inputShaderVersion, outputShaderVersion, options))
    {
        if (log)
            log->Error("analyzing input code failed");
        return false;
    }

    /* Print debug output */
    if (options.dumpAST && log)
    {
        ASTPrinter dumper;
        dumper.DumpAST(program.get(), *log);
    }

    /* Generate GLSL output code */
    GLSLGenerator generator(log, options);
    if (!generator.GenerateCode(program.get(), output, entryPoint, shaderTarget, inputShaderVersion, outputShaderVersion))
    {
        if (log)
            log->Error("generating output code failed");
        return false;
    }

    return true;
}


} // /namespace HTLib



// ================================================================================