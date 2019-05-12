/*
 * GLSLPreProcessor.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLSLPreProcessor.h"
#include "GLSLExtensions.h"
#include "ReportIdents.h"
#include <algorithm>


namespace Xsc
{


GLSLPreProcessor::GLSLPreProcessor(IncludeHandler& includeHandler, Log* log) :
    PreProcessor { includeHandler, log }
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
            Error(R_MacrosBeginWithGLReserved(ident), macro.identTkn.get(), false);
            return false;
        }

        /* Macros containing '__' are reserved */
        const auto underscorePos = ident.find("__");
        if (underscorePos != std::string::npos)
        {
            auto sourceArea = macro.identTkn->Area();
            sourceArea.Offset(static_cast<unsigned int>(underscorePos));
            Error(R_MacrosWithTwoUnderscoresReserved(ident), sourceArea, false);
            return false;
        }
    }
    return PreProcessor::OnDefineMacro(macro);
}

bool GLSLPreProcessor::OnRedefineMacro(const Macro& macro, const Macro& previousMacro)
{
    if (previousMacro.stdMacro)
    {
        Error(R_IllegalRedefOfStdMacro(previousMacro.identTkn->Spell()), macro.identTkn.get(), false);
        return false;
    }
    else
        return PreProcessor::OnRedefineMacro(macro, previousMacro);
}

bool GLSLPreProcessor::OnUndefineMacro(const Macro& macro)
{
    if (macro.stdMacro)
    {
        Error(R_IllegalUndefOfStdMacro(macro.identTkn->Spell()), macro.identTkn.get(), false);
        return false;
    }
    else
        return PreProcessor::OnUndefineMacro(macro);
}

bool GLSLPreProcessor::OnSubstitueStdMacro(const Token& identTkn, TokenPtrString& tokenString)
{
    if (identTkn.Spell() == "__FILE__")
    {
        /* Replace '__FILE__' identifier with index of current filename */
        tokenString.PushBack(Make<Token>(Tokens::IntLiteral, "1"));
        return true;
    }
    return PreProcessor::OnSubstitueStdMacro(identTkn, tokenString);
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
            Error(R_VersionMustBeFirstDirective, true, false);
        }

        PreProcessor::ParseDirective(directive, ignoreUnknown);
    }
}

// '#' 'version' NUMBER PROFILE?
void GLSLPreProcessor::ParseDirectiveVersion()
{
    /* Check if version was already defined */
    if (versionDefined_)
    {
        Error(R_GLSLVersionAlreadyDefined(versionNo_), true, false);
        IgnoreDirective();
        return;
    }

    versionDefined_ = true;

    /* Parse version number */
    IgnoreWhiteSpaces();

    auto versionTkn = Accept(Tokens::IntLiteral);
    versionNo_ = std::stoi(versionTkn->Spell());

    /* Parse profile */
    bool isESSL = false;
    bool isCompatibilityProfile = false;
    std::string profile;

    IgnoreWhiteSpaces();
    if (Is(Tokens::Ident))
    {
        profile = Accept(Tokens::Ident)->Spell();

        if (profile == "es")
        {
            /* Parse version for ESSL (OpenGL ES) */
            isESSL = true;
        }
        else
        {
            /* Parse version for GLSL (OpenGL or Vulkan) */
            if (profile == "compatibility")
                isCompatibilityProfile = true;
            else if (profile != "core")
                Error(R_InvalidGLSLVersionProfile(profile), true, false);
        }
    }

    if (isESSL)
    {
        /* Verify ESSL version number */
        static const int versionsESSL[] =
        {
            100, 300, 310, 320, 0
        };

        if (!VerifyVersionNo(versionsESSL))
            Error(R_UnknownESSLVersion(versionNo_), versionTkn.get(), false);
    }
    else
    {
        /* Verify GLSL version number */
        static const int versionsGLSL[] =
        {
            110, 120, 130, 140, 150, 330, 400, 410, 420, 430, 440, 450, 460, 0
        };

        if (!VerifyVersionNo(versionsGLSL))
            Error(R_UnknownGLSLVersion(versionNo_), versionTkn.get(), false);

        /* Only GLSL 150+ allows a profile */
        if (!profile.empty() && versionNo_ < 150)
            Error(R_NoProfileForGLSLVersionBefore150, true, false);
    }

    /* Write out version */
    Out() << "#version " << versionNo_;

    if (!profile.empty())
        Out() << ' ' << profile;

    /*
    Define standard macros: 'GL_core_profile', 'GL_es_profile', 'GL_compatibility_profile'
    see https://www.khronos.org/opengl/wiki/Core_Language_(GLSL)#Standard_macros
    */
    DefineStandardMacro("GL_core_profile");

    if (isESSL)
    {
        DefineStandardMacro("GL_es_profile");
        DefineStandardMacro("GL_ES");
    }
    else if (isCompatibilityProfile)
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
    if (extension != "all")
    {
        const auto& extMap = GetGLSLExtensionVersionMap();
        if (extMap.find(extension) == extMap.end())
            Error(R_ExtensionNotSupported(extension), true, false);
    }

    /* Parse behavior */
    IgnoreWhiteSpaces(false, true);
    Accept(Tokens::Colon);

    IgnoreWhiteSpaces(false, true);
    auto behavior = Accept(Tokens::Ident)->Spell();

    /* Verify behavior */
    if (behavior != "enable" && behavior != "require" && behavior != "warn" && behavior != "disable")
        Error(R_InvalidGLSLExtensionBehavior(behavior), true, false);

    /* Write out extension */
    Out() << "#extension " << extension << " : " << behavior;
}

bool GLSLPreProcessor::VerifyVersionNo(const int* validVersions) const
{
    /* Find version number in array */
    while (*validVersions)
    {
        if (*validVersions == versionNo_)
            return true;
        if (*validVersions > versionNo_)
            return false;
        ++validVersions;
    }
    return false;
}


} // /namespace Xsc



// ================================================================================
