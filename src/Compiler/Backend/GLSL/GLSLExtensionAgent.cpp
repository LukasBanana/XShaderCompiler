/*
 * GLSLExtensionAgent.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLSLExtensionAgent.h"
#include "AST.h"
#include "Exception.h"
#include <algorithm>


namespace Xsc
{


/*
 * Internal GL ARB extension descriptions
 */

static const GLSLExtension GLSLEXT_GL_EXT_gpu_shader4               { "GL_EXT_gpu_shader4",              OutputShaderVersion::GLSL130 };
static const GLSLExtension GLSLEXT_GL_ARB_uniform_buffer_object     { "GL_ARB_uniform_buffer_object",    OutputShaderVersion::GLSL140 };
static const GLSLExtension GLSLEXT_GL_ARB_derivative_control        { "GL_ARB_derivative_control",       OutputShaderVersion::GLSL400 };
static const GLSLExtension GLSLEXT_GL_ARB_shading_language_420pack  { "GL_ARB_shading_language_420pack", OutputShaderVersion::GLSL420 };
static const GLSLExtension GLSLEXT_GL_ARB_shader_image_load_store   { "GL_ARB_shader_image_load_store",  OutputShaderVersion::GLSL420 };


/*
 * GLSLExtensionAgent class
 */

GLSLExtensionAgent::GLSLExtensionAgent()
{
    /* Establish intrinsic-to-extension map */
    intrinsicExtMap_ = std::map<Intrinsic, GLSLExtension>
    {
        { Intrinsic::DDXCoarse, GLSLEXT_GL_ARB_derivative_control },
        { Intrinsic::DDXFine,   GLSLEXT_GL_ARB_derivative_control },
        { Intrinsic::DDYCoarse, GLSLEXT_GL_ARB_derivative_control },
        { Intrinsic::DDYFine,   GLSLEXT_GL_ARB_derivative_control },
    };
}

std::set<std::string> GLSLExtensionAgent::DetermineRequiredExtensions(
    Program& program, OutputShaderVersion& targetGLSLVersion, bool allowExtensions)
{
    targetGLSLVersion_  = targetGLSLVersion;
    allowExtensions_    = allowExtensions;

    Visit(&program);

    if (targetGLSLVersion == OutputShaderVersion::GLSL)
        targetGLSLVersion = minGLSLVersion_;

    return std::move(extensions_);
}


/*
 * ======= Private: =======
 */

void GLSLExtensionAgent::AcquireExtension(const GLSLExtension& extension)
{
    if (targetGLSLVersion_ == OutputShaderVersion::GLSL)
    {
        /* Store minimum required GLSL version */
        minGLSLVersion_ = std::max(minGLSLVersion_, extension.requiredVersion);
    }
    else if (targetGLSLVersion_ < extension.requiredVersion)
    {
        if (allowExtensions_)
        {
            /* Add extension to the resulting set, if the target GLSL version is less than the required extension version */
            extensions_.insert(extension.extensionName);
        }
        else
        {
            /* Extensions not allowed -> runtime error */
            RuntimeErr(
                "GLSL extension '" + extension.extensionName +
                "' or shader output version '" + ShaderVersionToString(extension.requiredVersion) + "' required"
            );
        }
    }
}


/* ------- Visit functions ------- */

#define IMPLEMENT_VISIT_PROC(AST_NAME) \
    void GLSLExtensionAgent::Visit##AST_NAME(AST_NAME* ast, void* args)

IMPLEMENT_VISIT_PROC(FunctionCall)
{
    /* Check for special intrinsics */
    if (ast->intrinsic != Intrinsic::Undefined)
    {
        auto it = intrinsicExtMap_.find(ast->intrinsic);
        if (it != intrinsicExtMap_.end())
            AcquireExtension(it->second);
    }

    /* Default visitor */
    Visitor::VisitFunctionCall(ast, args);
}

IMPLEMENT_VISIT_PROC(Attribute)
{
    /* Check for special attributes */
    if (ast->ident == "earlydepthstencil")
        AcquireExtension(GLSLEXT_GL_ARB_shader_image_load_store);
}

IMPLEMENT_VISIT_PROC(FunctionDecl)
{
    if (ast->flags(AST::isReachable))
    {
        Visit(ast->attribs);

        /* Default visitor */
        Visitor::VisitFunctionDecl(ast, args);
    }
}

IMPLEMENT_VISIT_PROC(BufferDeclStmnt)
{
    AcquireExtension(GLSLEXT_GL_ARB_uniform_buffer_object);

    /* Check for explicit binding point */
    if (!ast->registerName.empty())
        AcquireExtension(GLSLEXT_GL_ARB_shading_language_420pack);

    /* Default visitor */
    Visitor::VisitBufferDeclStmnt(ast, args);
}

IMPLEMENT_VISIT_PROC(TextureDeclStmnt)
{
    /* Check for explicit binding point */
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
    if (IsBitwiseOp(ast->op) || ast->op == BinaryOp::Mod)
        AcquireExtension(GLSLEXT_GL_EXT_gpu_shader4);

    /* Default visitor */
    Visitor::VisitBinaryExpr(ast, args);
}

IMPLEMENT_VISIT_PROC(UnaryExpr)
{
    /* Check if bitwise operators are used -> requires "GL_EXT_gpu_shader4" extensions */
    if (IsBitwiseOp(ast->op))
        AcquireExtension(GLSLEXT_GL_EXT_gpu_shader4);

    /* Default visitor */
    Visitor::VisitUnaryExpr(ast, args);
}

IMPLEMENT_VISIT_PROC(VarAccessExpr)
{
    /* Check if bitwise operators are used -> requires "GL_EXT_gpu_shader4" extensions */
    if (IsBitwiseOp(ast->assignOp) || ast->assignOp == AssignOp::Mod)
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
