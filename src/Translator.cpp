/*
 * Translator.cpp
 * 
 * This file is part of the "HLSL Translator" (Copyright (c) 2014 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "HT/Translator.h"
#include "HLSLParser.h"


namespace HTLib
{


_HT_EXPORT_ bool TranslateHLSLtoGLSL(
    std::istream& input,
    std::ostream& output,
    const std::string& entryPoint,
    const ShaderTargets shaderTarget,
    const ShaderVersions shaderVersion,
    IncludeHandler* includeHandler,
    const Options& options,
    Logger* log)
{
    HLSLParser parser(log);

    if (!parser.ParseSource(std::make_shared<SourceCode>(input)))
    {
        if (log)
            log->Error("parsing input code failed");
        return false;
    }

    //...

    return true;
}


} // /namespace HLSLTrans



// ================================================================================