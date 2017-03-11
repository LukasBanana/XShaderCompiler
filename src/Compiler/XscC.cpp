/*
 * Xsc.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <Xsc/Xsc.h>
#include <XscC/XscC.h>
#include <string.h>
#include <sstream>
#include "Helper.h"


/*
 * Internal functions
 */

static void WriteStringC(const std::string& src, char* dest, size_t maxSize)
{
    if (dest != NULL)
    {
        if (src.size() < maxSize)
            strncpy(dest, src.c_str(), maxSize);
        else
            memset(dest, 0, maxSize);
    }
}

static std::string ReadStringC(const char* src)
{
    return (src != NULL ? std::string(src) : std::string());
}


#ifdef __cplusplus
extern "C" {
#endif


static void InitializeFormatting(struct XscFormatting* s)
{
    s->indent               = "    ";
    s->blanks               = true;
    s->lineMarks            = false;
    s->compactWrappers      = false;
    s->alwaysBracedScopes   = false;
    s->newLineOpenScope     = true;
    s->lineSeparation       = true;
}

static void InitializeOptions(struct XscOptions* s)
{
    s->warnings                 = false;
    s->optimize                 = false;
    s->preprocessOnly           = false;
    s->validateOnly             = false;
    s->allowExtensions          = false;
    s->explicitBinding          = false;
    s->preserveComments         = false;
    s->preferWrappers           = false;
    s->unrollArrayInitializers  = false;
    s->rowMajorAlignment        = false;
    s->obfuscate                = false;
    s->showAST                  = false;
    s->showTimes                = false;
}

static void InitializeNameMangling(struct XscNameMangling* s)
{
    s->inputPrefix          = "xsv_";
    s->outputPrefix         = "xsv_";
    s->reservedWordPrefix   = "xsr_";
    s->temporaryPrefix      = "xst_";
    s->useAlwaysSemantics   = false;
}

static void InitializeIncludeHandler(struct XscIncludeHandler* s)
{
    s->handleIncludePfn = NULL;
    s->searchPaths      = NULL;
}

static void InitializeShaderInput(struct XscShaderInput* s)
{
    s->filename             = NULL;
    s->sourceCode           = NULL;
    s->shaderVersion        = XscEInputHLSL5;
    s->shaderTarget         = XscETargetUndefined;
    s->entryPoint           = "main";
    s->secondaryEntryPoint  = NULL;

    InitializeIncludeHandler(&(s->includeHandler));
}

static void InitializeShaderOutput(struct XscShaderOutput* s)
{
    s->filename             = NULL;
    s->sourceCode           = NULL;
    s->shaderVersion        = XscEOutputGLSL;
    s->vertexSemantics      = NULL;
    s->vertexSemanticsCount = 0;

    InitializeOptions(&(s->options));
    InitializeFormatting(&(s->formatting));
    InitializeNameMangling(&(s->nameMangling));
}

XSC_EXPORT void XscInitialize(struct XscShaderInput* inputDesc, struct XscShaderOutput* outputDesc)
{
    if (inputDesc != NULL)
        InitializeShaderInput(inputDesc);
    if (outputDesc != NULL)
        InitializeShaderOutput(outputDesc);
}

static bool ValidateShaderInput(const struct XscShaderInput* s)
{
    return (s != NULL && s->sourceCode != NULL && s->entryPoint != NULL);
}

static bool ValidateShaderOutput(const struct XscShaderOutput* s)
{
    return (s != NULL && s->sourceCode != NULL && (s->vertexSemanticsCount == 0 || s->vertexSemantics != NULL));
}


/*
 * IncludeHandlerC class
 */

class IncludeHandlerC : public Xsc::IncludeHandler
{

    public:

        IncludeHandlerC(const XscIncludeHandler& handler);

        std::unique_ptr<std::istream> Include(const std::string& filename, bool useSearchPathsFirst) override;

    private:

        XscIncludeHandler handler_;

};

IncludeHandlerC::IncludeHandlerC(const XscIncludeHandler& handler)
{
    handler_.handleIncludePfn   = handler.handleIncludePfn;
    handler_.searchPaths        = handler.searchPaths;
}

std::unique_ptr<std::istream> IncludeHandlerC::Include(const std::string& filename, bool useSearchPathsFirst)
{
    auto stream = Xsc::MakeUnique<std::stringstream>();

    if (handler_.handleIncludePfn)
    {
        /* Include file with callback function */
        auto source = handler_.handleIncludePfn(filename.c_str(), handler_.searchPaths, useSearchPathsFirst);
        *stream << ReadStringC(source);
    }

    return std::move(stream);
}


/*
 * LogC class
 */

class LogC : public Xsc::Log
{

    public:

        LogC(const XscLog* handler);

        void SumitReport(const Xsc::Report& report) override;
        
    private:

        XscLog handler_;

};

LogC::LogC(const XscLog* handler)
{
    if (handler != NULL && handler != XSC_DEFAULT_LOG)
        handler_.handleReportPfn = handler->handleReportPfn;
    else
        handler_.handleReportPfn = NULL;
}

void LogC::SumitReport(const Xsc::Report& report)
{
    if (handler_.handleReportPfn)
    {
        /* Initialize report for C API */
        XscReport r;

        std::vector<const char*> hints;
        for (const auto& h : report.GetHints())
            hints.push_back(h.c_str());

        r.type          = static_cast<XscReportType>(report.Type());
        r.context       = report.Context().c_str();
        r.message       = report.Message().c_str();
        r.line          = report.Line().c_str();
        r.marker        = report.Marker().c_str();
        r.hints         = hints.data();
        r.hintsCount    = hints.size();

        /* Pass report to callback */
        handler_.handleReportPfn(&r, FullIndent().c_str());
    }
}


/*
 * Internal context
 */

struct CompilerContext
{
    std::string outputCode;
};

static struct CompilerContext g_compilerContext;


/*
 * Public functions
 */

XSC_EXPORT bool XscCompileShader(
    const struct XscShaderInput* inputDesc,
    const struct XscShaderOutput* outputDesc,
    const struct XscLog* log/*,
    struct XscReflectionData* reflectionData*/)
{
    if (!ValidateShaderInput(inputDesc) || !ValidateShaderOutput(outputDesc))
        return false;

    /* Copy input descriptor */
    Xsc::ShaderInput in;

    IncludeHandlerC includeHandler(inputDesc->includeHandler);

    auto inputStream = std::make_shared<std::stringstream>();
    *inputStream << inputDesc->sourceCode;

    in.filename             = ReadStringC(inputDesc->filename);
    in.sourceCode           = inputStream;
    in.shaderVersion        = static_cast<Xsc::InputShaderVersion>(inputDesc->shaderVersion);
    in.shaderTarget         = static_cast<Xsc::ShaderTarget>(inputDesc->shaderTarget);
    in.entryPoint           = ReadStringC(inputDesc->entryPoint);
    in.secondaryEntryPoint  = ReadStringC(inputDesc->secondaryEntryPoint);
    in.includeHandler       = (&includeHandler);

    /* Copy output descriptor */
    Xsc::ShaderOutput out;

    std::stringstream outputStream;

    out.filename        = ReadStringC(outputDesc->filename);
    out.sourceCode      = (&outputStream);
    out.shaderVersion   = static_cast<Xsc::OutputShaderVersion>(outputDesc->shaderVersion);

    out.vertexSemantics.resize(outputDesc->vertexSemanticsCount);
    for (size_t i = 0; i < outputDesc->vertexSemanticsCount; ++i)
    {
        out.vertexSemantics[i].semantic = ReadStringC(outputDesc->vertexSemantics[i].semantic);
        out.vertexSemantics[i].location = outputDesc->vertexSemantics[i].location;
    }

    /* Copy output options descriptor */
    out.options.warnings                = outputDesc->options.warnings;
    out.options.optimize                = outputDesc->options.optimize;
    out.options.preprocessOnly          = outputDesc->options.preprocessOnly;
    out.options.validateOnly            = outputDesc->options.validateOnly;
    out.options.allowExtensions         = outputDesc->options.allowExtensions;
    out.options.explicitBinding         = outputDesc->options.explicitBinding;
    out.options.preserveComments        = outputDesc->options.preserveComments;
    out.options.preferWrappers          = outputDesc->options.preferWrappers;
    out.options.unrollArrayInitializers = outputDesc->options.unrollArrayInitializers;
    out.options.rowMajorAlignment       = outputDesc->options.rowMajorAlignment;
    out.options.obfuscate               = outputDesc->options.obfuscate;
    out.options.showAST                 = outputDesc->options.showAST;
    out.options.showTimes               = outputDesc->options.showTimes;

    /* Copy output formatting descriptor */
    out.formatting.indent               = ReadStringC(outputDesc->formatting.indent);
    out.formatting.blanks               = outputDesc->formatting.blanks;
    out.formatting.lineMarks            = outputDesc->formatting.lineMarks;
    out.formatting.compactWrappers      = outputDesc->formatting.compactWrappers;
    out.formatting.alwaysBracedScopes   = outputDesc->formatting.alwaysBracedScopes;
    out.formatting.newLineOpenScope     = outputDesc->formatting.newLineOpenScope;
    out.formatting.lineSeparation       = outputDesc->formatting.lineSeparation;

    /* Copy output name mangling descriptor */
    out.nameMangling.inputPrefix        = ReadStringC(outputDesc->nameMangling.inputPrefix);
    out.nameMangling.outputPrefix       = ReadStringC(outputDesc->nameMangling.outputPrefix);
    out.nameMangling.reservedWordPrefix = ReadStringC(outputDesc->nameMangling.reservedWordPrefix);
    out.nameMangling.temporaryPrefix    = ReadStringC(outputDesc->nameMangling.temporaryPrefix);
    out.nameMangling.useAlwaysSemantics = outputDesc->nameMangling.useAlwaysSemantics;

    /* Initialize log */
    Xsc::StdLog logPrimaryStd;
    LogC logPrimary(log);

    Xsc::Log* logPrimaryRef = nullptr;
    if (log == XSC_DEFAULT_LOG)
        logPrimaryRef = (&logPrimaryStd);
    else
        logPrimaryRef = (&logPrimary);

    /* Compile shader with C++ API */
    bool result = Xsc::CompileShader(in, out, logPrimaryRef);

    if (result)
    {
        g_compilerContext.outputCode = outputStream.str();
        *outputDesc->sourceCode = g_compilerContext.outputCode.c_str();
    }

    if (log == XSC_DEFAULT_LOG)
        logPrimaryStd.PrintAll();

    return result;
}

XSC_EXPORT void XscShaderTargetToString(const enum XscShaderTarget target, char* str, size_t maxSize)
{
    WriteStringC(Xsc::ToString(static_cast<Xsc::ShaderTarget>(target)), str, maxSize);
}

XSC_EXPORT void XscInputShaderVersionToString(const enum XscInputShaderVersion shaderVersion, char* str, size_t maxSize)
{
    WriteStringC(Xsc::ToString(static_cast<Xsc::InputShaderVersion>(shaderVersion)), str, maxSize);
}

XSC_EXPORT void XscOutputShaderVersionToString(const enum XscOutputShaderVersion shaderVersion, char* str, size_t maxSize)
{
    WriteStringC(Xsc::ToString(static_cast<Xsc::OutputShaderVersion>(shaderVersion)), str, maxSize);
}

XSC_EXPORT bool XscIsInputLanguageHLSL(const enum XscInputShaderVersion shaderVersion)
{
    return Xsc::IsLanguageHLSL(static_cast<Xsc::InputShaderVersion>(shaderVersion));
}

XSC_EXPORT bool XscIsInputLanguageGLSL(const enum XscInputShaderVersion shaderVersion)
{
    return Xsc::IsLanguageGLSL(static_cast<Xsc::InputShaderVersion>(shaderVersion));
}

XSC_EXPORT bool XscIsOutputLanguageGLSL(const enum XscOutputShaderVersion shaderVersion)
{
    return Xsc::IsLanguageGLSL(static_cast<Xsc::OutputShaderVersion>(shaderVersion));
}

XSC_EXPORT bool XscIsOutputLanguageESSL(const enum XscOutputShaderVersion shaderVersion)
{
    return Xsc::IsLanguageESSL(static_cast<Xsc::OutputShaderVersion>(shaderVersion));
}

XSC_EXPORT bool XscIsOutputLanguageVKSL(const enum XscOutputShaderVersion shaderVersion)
{
    return Xsc::IsLanguageVKSL(static_cast<Xsc::OutputShaderVersion>(shaderVersion));
}

using GLSLExtensionEnumIterator = std::map<std::string, int>::const_iterator;

XSC_EXPORT void* XscGetGLSLExtensionEnumeration(void* iterator, char* extension, size_t maxSize, int* version)
{
    const auto& extMap = Xsc::GetGLSLExtensionEnumeration();

    /* Get GLSL extension enumeration iterator */
    static GLSLExtensionEnumIterator mapIt;

    if (iterator)
        mapIt = *reinterpret_cast<GLSLExtensionEnumIterator*>(iterator);
    else
        mapIt = extMap.begin();

    if (mapIt != extMap.end())
    {
        /* Return enumeration entry */
        WriteStringC(mapIt->first.c_str(), extension, maxSize);

        if (version)
            *version = mapIt->second;

        /* Move to next element for the next call */
        ++mapIt;

        /* Return reference to iterator */
        return reinterpret_cast<void*>(&mapIt);
    }

    return NULL;
}


#ifdef __cplusplus
} // /extern "C"
#endif



// ================================================================================
