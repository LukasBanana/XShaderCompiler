/*
 * GLSLExtensionAgent.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLSLExtensionAgent.h"
#include "AST.h"


namespace Xsc
{


/*
 * Internal GL ARB extension descriptions
 */

static const GLSLExtension GLSLEXT_GL_EXT_gpu_shader4               { "GL_EXT_gpu_shader4",                 130 };
static const GLSLExtension GLSLEXT_GL_ARB_derivative_control        { "GL_ARB_derivative_control",          400 };
static const GLSLExtension GLSLEXT_GL_ARB_shading_language_420pack  { "GL_ARB_shading_language_420pack",    420 };
static const GLSLExtension GLSLEXT_GL_ARB_shader_image_load_store   { "GL_ARB_shader_image_load_store",     420 };


/*
 * GLSLExtensionAgent class
 */

//TODO: HLSL specific names like "ddx_coarse" must be abstracted!
GLSLExtensionAgent::GLSLExtensionAgent()
{
    /* Establish function-to-extension map */
    funcToExtMap_ = std::map<std::string, GLSLExtension>
    {
        { "ddx_coarse", GLSLEXT_GL_ARB_derivative_control },
        { "ddy_coarse", GLSLEXT_GL_ARB_derivative_control },
        { "ddx_fine",   GLSLEXT_GL_ARB_derivative_control },
        { "ddy_fine",   GLSLEXT_GL_ARB_derivative_control },
    };
}

std::set<std::string> GLSLExtensionAgent::DetermineRequiredExtensions(
    Program& program, const OutputShaderVersion targetGLSLVersion)
{
    targetGLSLVersion_ = static_cast<int>(targetGLSLVersion);
    Visit(&program);
    return std::move(extensions_);
}


/*
 * ======= Private: =======
 */

void GLSLExtensionAgent::AcquireExtension(const GLSLExtension& extension)
{
    /* Add extension to the resulting set, if the target GLSL version is less than the required extension version */
    if (targetGLSLVersion_ < extension.requiredVersion)
        extensions_.insert(extension.extensionName);
}


/* ------- Visit functions ------- */

#define IMPLEMENT_VISIT_PROC(AST_NAME) \
    void GLSLExtensionAgent::Visit##AST_NAME(AST_NAME* ast, void* args)

IMPLEMENT_VISIT_PROC(FunctionCall)
{
    if (ast->name)
    {
        /* Check for special function (or intrinsic) */
        auto name = ast->name->ToString();

        auto it = funcToExtMap_.find(name);
        if (it != funcToExtMap_.end())
            AcquireExtension(it->second);
    }

    /* Default visitor */
    Visitor::VisitFunctionCall(ast, args);
}

IMPLEMENT_VISIT_PROC(FunctionDecl)
{
    if (ast->flags(FunctionDecl::isReferenced))
    {
        /* Check for special attributes */
        for (auto& attrib : ast->attribs)
        {
            auto name = attrib->name->ToString();
            if (name == "earlydepthstencil")
                AcquireExtension(GLSLEXT_GL_ARB_shader_image_load_store);
        }

        /* Default visitor */
        Visitor::VisitFunctionDecl(ast, args);
    }
}

IMPLEMENT_VISIT_PROC(TextureDeclStmnt)
{
    /* Check for explicit texture binding */
    for (auto& name : ast->textureDecls)
    {
        if (!name->registerName.empty())
            AcquireExtension(GLSLEXT_GL_ARB_shading_language_420pack);
    }

    /* Default visitor */
    Visitor::VisitTextureDeclStmnt(ast, args);
}

IMPLEMENT_VISIT_PROC(BinaryExpr)
{
    /* Check if bitwise operators are used -> requires "GL_EXT_gpu_shader4" extensions */
    if (IsBitwiseOp(ast->op))
        AcquireExtension(GLSLEXT_GL_EXT_gpu_shader4);

    /* Default visitor */
    Visitor::VisitBinaryExpr(ast, args);
}

IMPLEMENT_VISIT_PROC(VarAccessExpr)
{
    /* Check if bitwise operators are used -> requires "GL_EXT_gpu_shader4" extensions */
    const auto& op = ast->assignOp;
    if (op == "|=" || op == "&=" || op == "^=" || op == "%=")
        AcquireExtension(GLSLEXT_GL_EXT_gpu_shader4);

    /* Default visitor */
    Visitor::VisitVarAccessExpr(ast, args);
}

IMPLEMENT_VISIT_PROC(InitializerExpr)
{
    AcquireExtension(GLSLEXT_GL_ARB_shading_language_420pack);

    /* Default visitor */
    Visitor::VisitInitializerExpr(ast, args);
}

#undef IMPLEMENT_VISIT_PROC


} // /namespace Xsc



// ================================================================================
