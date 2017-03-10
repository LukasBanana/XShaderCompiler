/*
 * GLSLPreProcessor.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLSLPreProcessor.h"
#include "GLSLExtensions.h"
#include <algorithm>


namespace Xsc
{


GLSLPreProcessor::GLSLPreProcessor(IncludeHandler& includeHandler, Log* log) :
    PreProcessor{ includeHandler, log }
{
}


/*
 * ======= Private: =======
 */

bool GLSLPreProcessor::OnDefineMacro(const Macro& macro)
{
    if (!macro.stdMacro)
    {
        const auto& ident = macro.identTkn->Spell();
        
        /* Macros beginning with 'GL_' are reserved */
        if (ident.compare(0, 3, "GL_") == 0)
        {
            Error("macros beginning with 'GL_' are reserved: " + ident, macro.identTkn.get(), false);
            return false;
        }

        /* Macros containing '__' are reserved */
        const auto underscorePos = ident.find("__");
        if (underscorePos != std::string::npos)
        {
            auto sourceArea = macro.identTkn->Area();
            sourceArea.Offset(static_cast<unsigned int>(underscorePos));
            Error("macros containing consecutive underscores '__' are reserved: " + ident, sourceArea, false);
            return false;
        }
    }
    return PreProcessor::OnDefineMacro(macro);
}

bool GLSLPreProcessor::OnRedefineMacro(const Macro& macro, const Macro& previousMacro)
{
    if (previousMacro.stdMacro)
    {
        Error("illegal redefinition of standard macro: " + previousMacro.identTkn->Spell(), macro.identTkn.get(), false);
        return false;
    }
    else
        return PreProcessor::OnRedefineMacro(macro, previousMacro);
}

bool GLSLPreProcessor::OnUndefineMacro(const Macro& macro)
{
    if (macro.stdMacro)
    {
        Error("illegal undefinition of standard macro: " + macro.identTkn->Spell(), macro.identTkn.get(), false);
        return false;
    }
    else
        return PreProcessor::OnUndefineMacro(macro);
}

void GLSLPreProcessor::ParseDirective(const std::string& directive, bool ignoreUnknown)
{
    if (directive == "version")
        ParseDirectiveVersion();
    else if (directive == "extension")
        ParseDirectiveExtension();
    else
    {
        /*
        If '#version'-directive was not the first directive,
        report error and set version to 1, to avoid this error message multiple times
        */
        if (versionNo_ == 0)
        {
            versionNo_ = 1;
            Error("'#version'-directive must be the first directive", true, false);
        }

        PreProcessor::ParseDirective(directive, ignoreUnknown);
    }
}

// '#' 'version' NUMBER PROFILE?
void GLSLPreProcessor::ParseDirectiveVersion()
{
    std::string version, profile;

    /* Parse version number */
    IgnoreWhiteSpaces();
    version = Accept(Tokens::IntLiteral)->Spell();

    /* Verify GLSL version number */
    versionNo_ = FromString<int>(version);

    static const int versionListGLSL[] =
    {
        110, 120, 130, 140, 150, 330, 400, 410, 420, 430, 440, 450,
    };

    if (std::find(std::begin(versionListGLSL), std::end(versionListGLSL), versionNo_) == std::end(versionListGLSL))
        Error("unknown GLSL version '" + version + "'", true, false);

    /* Parse profile */
    bool isCompatibilityProfile = false;

    IgnoreWhiteSpaces();
    if (Is(Tokens::Ident))
    {
        profile = Accept(Tokens::Ident)->Spell();

        if (versionNo_ < 150)
            Error("versions before 150 do not allow a profile token", true, false);

        if (profile == "core")
            isCompatibilityProfile = false;
        else if (profile == "compatibility")
            isCompatibilityProfile = true;
        else
            Error("invalid version profile '" + profile + "' (must be 'core' or 'compatibility')", true, false);
    }

    /* Write out version */
    Out() << "#version " << version;

    if (versionNo_ >= 150)
    {
        if (isCompatibilityProfile)
            Out() << " compatibility";
        else
            Out() << " core";
    }

    /*
    Define standard macros 'GL_core_profile' and 'GL_compatibility_profile'
    see https://www.khronos.org/opengl/wiki/Core_Language_(GLSL)#Standard_macros
    */
    DefineStandardMacro("GL_core_profile");
    if (isCompatibilityProfile)
        DefineStandardMacro("GL_compatibility_profile");

    DefineStandardMacro("__VERSION__", versionNo_);
}

// '#' 'extension' EXTENSION ':' BEHAVIOR
void GLSLPreProcessor::ParseDirectiveExtension()
{
    /* Parse extension name */
    IgnoreWhiteSpaces(false, true);
    auto extension = Accept(Tokens::Ident)->Spell();

    /* Verify extension */
    const auto& extList = GetGLSLExtensionRefList();
    if (std::find(extList.begin(), extList.end(), extension) == extList.end())
        Error("extension not supported: " + extension, true, false);

    /* Parse behavior */
    IgnoreWhiteSpaces(false, true);
    Accept(Tokens::Colon);

    IgnoreWhiteSpaces(false, true);
    auto behavior = Accept(Tokens::Ident)->Spell();

    /* Verify behavior */
    if (behavior != "enable" && behavior != "require" && behavior != "warn" && behavior != "disable")
        Error("invalid extension behavior '" + behavior + "' (must be 'enable', 'require', 'warn', or 'disable')", true, false);

    /* Write out extension */
    Out() << "#extension " << extension << " : " << behavior;
}


} // /namespace Xsc



// ================================================================================
