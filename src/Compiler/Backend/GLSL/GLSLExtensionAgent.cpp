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
static const GLSLExtension GLSLEXT_GL_ARB_arrays_of_arrays          { "GL_ARB_arrays_of_arrays",         OutputShaderVersion::GLSL430 };


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

static OutputShaderVersion GetMinGLSLVersionForTarget(const ShaderTarget shaderTarget)
{
    switch (shaderTarget)
    {
        case ShaderTarget::VertexShader:                    return OutputShaderVersion::GLSL110;
        case ShaderTarget::TessellationControlShader:       return OutputShaderVersion::GLSL400;
        case ShaderTarget::TessellationEvaluationShader:    return OutputShaderVersion::GLSL400;
        case ShaderTarget::GeometryShader:                  return OutputShaderVersion::GLSL150;
        case ShaderTarget::FragmentShader:                  return OutputShaderVersion::GLSL110;
        case ShaderTarget::ComputeShader:                   return OutputShaderVersion::GLSL430; // actually 420, but only 430 supports local work group size
    }
    return OutputShaderVersion::GLSL110;
}

std::set<std::string> GLSLExtensionAgent::DetermineRequiredExtensions(
    Program& program, OutputShaderVersion& targetGLSLVersion, const ShaderTarget shaderTarget, bool allowExtensions)
{
    shaderTarget_       = shaderTarget;
    targetGLSLVersion_  = targetGLSLVersion;
    minGLSLVersion_     = GetMinGLSLVersionForTarget(shaderTarget);
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

IMPLEMENT_VISIT_PROC(VarDecl)
{
    /* Check for arrays of arrays */
    if (ast->GetTypeDenoter()->NumDimensions() >= 2)
        AcquireExtension(GLSLEXT_GL_ARB_arrays_of_arrays);

    /* Default visitor */
    Visitor::VisitVarDecl(ast, args);
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
    if (auto slotRegister = Register::GetForTarget(ast->slotRegisters, shaderTarget_))
        AcquireExtension(GLSLEXT_GL_ARB_shading_language_420pack);

    /* Default visitor */
    Visitor::VisitBufferDeclStmnt(ast, args);
}

IMPLEMENT_VISIT_PROC(TextureDeclStmnt)
{
    /* Check for explicit binding point */
    for (auto& texDecl : ast->textureDecls)
    {
        if (auto slotRegister = Register::GetForTarget(texDecl->slotRegisters, shaderTarget_))
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
