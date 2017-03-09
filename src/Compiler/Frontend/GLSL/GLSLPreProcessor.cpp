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

void GLSLPreProcessor::ParseDirective(const std::string& directive, bool ignoreUnknown)
{
    if (directive == "version")
        ParseDirectiveVersion();
    else if (directive == "extension")
        ParseDirectiveExtension();
    else
        PreProcessor::ParseDirective(directive, ignoreUnknown);
}

// '#' 'version' NUMBER PROFILE?
void GLSLPreProcessor::ParseDirectiveVersion()
{
    std::string version, profile;

    /* Parse version number */
    IgnoreWhiteSpaces();
    version = Accept(Tokens::IntLiteral)->Spell();

    /* Verify GLSL version number */
    const auto versionNo = FromString<int>(version);

    static const int versionListGLSL[] =
    {
        110, 120, 130, 140, 150, 330, 400, 410, 420, 430, 440, 450,
    };

    if (std::find(std::begin(versionListGLSL), std::end(versionListGLSL), versionNo) == std::end(versionListGLSL))
        Error("unknown GLSL version '" + version + "'", true, false);

    /* Parse profile */
    bool isCompatibilityProfile = false;

    IgnoreWhiteSpaces();
    if (Is(Tokens::Ident))
    {
        profile = Accept(Tokens::Ident)->Spell();

        if (versionNo < 150)
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

    if (versionNo >= 150)
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

    DefineStandardMacro("__VERSION__", versionNo);
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
