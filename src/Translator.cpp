/*
 * Translator.cpp
 * 
 * This file is part of the "HLSL Translator" (Copyright (c) 2014 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "HT/Translator.h"
#include "HLSLParser.h"
#include "HLSLAnalyzer.h"
#include "GLSLGenerator.h"


namespace HTLib
{


_HT_EXPORT_ bool TranslateHLSLtoGLSL(
    const std::shared_ptr<std::istream>& input,
    std::ostream& output,
    const std::string& entryPoint,
    const ShaderTargets shaderTarget,
    const ShaderVersions shaderVersion,
    IncludeHandler* includeHandler,
    const Options& options,
    Logger* log)
{
    /* Parse HLSL input code */
    HLSLParser parser(log);
    auto program = parser.ParseSource(std::make_shared<SourceCode>(input));

    if (!program)
    {
        if (log)
            log->Error("parsing input code failed");
        return false;
    }

    /* Small context analysis */
    HLSLAnalyzer analyzer(log);
    if (!analyzer.DecorateAST(program.get(), entryPoint, shaderTarget, shaderVersion))
    {
        if (log)
            log->Error("analyzing input code failed");
        return false;
    }

    /* Generate GLSL output code */
    GLSLGenerator generator(log, includeHandler, options);
    if (!generator.GenerateCode(program.get(), output, entryPoint, shaderTarget, shaderVersion))
    {
        if (log)
            log->Error("generating output code failed");
        return false;
    }

    return true;
}


} // /namespace HLSLTrans



// ================================================================================