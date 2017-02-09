/*
 * GLSLExtensionAgent.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
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

static const GLSLExtension GLSLEXT_GL_EXT_gpu_shader4               { "GL_EXT_gpu_shader4",                 OutputShaderVersion::GLSL130 };
static const GLSLExtension GLSLEXT_GL_ARB_uniform_buffer_object     { "GL_ARB_uniform_buffer_object",       OutputShaderVersion::GLSL140 };
static const GLSLExtension GLSLEXT_GL_ARB_texture_multisample       { "GL_ARB_texture_multisample",         OutputShaderVersion::GLSL150 };
static const GLSLExtension GLSLEXT_GL_ARB_fragment_coord_conventions{ "GL_ARB_fragment_coord_conventions",  OutputShaderVersion::GLSL150 };
static const GLSLExtension GLSLEXT_GL_ARB_gpu_shader5               { "GL_ARB_gpu_shader5",                 OutputShaderVersion::GLSL330 };
static const GLSLExtension GLSLEXT_GL_ARB_derivative_control        { "GL_ARB_derivative_control",          OutputShaderVersion::GLSL450 };
static const GLSLExtension GLSLEXT_GL_ARB_shading_language_420pack  { "GL_ARB_shading_language_420pack",    OutputShaderVersion::GLSL420 };
static const GLSLExtension GLSLEXT_GL_ARB_shader_image_load_store   { "GL_ARB_shader_image_load_store",     OutputShaderVersion::GLSL420 };
static const GLSLExtension GLSLEXT_GL_ARB_arrays_of_arrays          { "GL_ARB_arrays_of_arrays",            OutputShaderVersion::GLSL430 };
static const GLSLExtension GLSLEXT_GL_ARB_enhanced_layouts          { "GL_ARB_enhanced_layouts",            OutputShaderVersion::GLSL430 };
static const GLSLExtension GLSLEXT_GL_ARB_gpu_shader_int64          { "GL_ARB_gpu_shader_int64",            OutputShaderVersion::GLSL450 };


/*
 * GLSLExtensionAgent class
 */

GLSLExtensionAgent::GLSLExtensionAgent()
{
    /* Establish intrinsic-to-extension map */
    intrinsicExtMap_ = std::map<Intrinsic, GLSLExtension>
    {
        { Intrinsic::AsDouble,  GLSLEXT_GL_ARB_gpu_shader_int64   },
        { Intrinsic::AsFloat,   GLSLEXT_GL_ARB_gpu_shader5        },
        { Intrinsic::AsInt,     GLSLEXT_GL_ARB_gpu_shader5        },
        { Intrinsic::AsUInt_1,  GLSLEXT_GL_ARB_gpu_shader5        },
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
        case ShaderTarget::Undefined:                       break;
        case ShaderTarget::VertexShader:                    return OutputShaderVersion::GLSL130; // actually 110, but this compiler does not support GLSL < 130
        case ShaderTarget::TessellationControlShader:       return OutputShaderVersion::GLSL400;
        case ShaderTarget::TessellationEvaluationShader:    return OutputShaderVersion::GLSL400;
        case ShaderTarget::GeometryShader:                  return OutputShaderVersion::GLSL150;
        case ShaderTarget::FragmentShader:                  return OutputShaderVersion::GLSL130; // actually 110, but this compiler does not support GLSL < 130
        case ShaderTarget::ComputeShader:                   return OutputShaderVersion::GLSL430; // actually 420, but only 430 supports local work group size
    }
    return OutputShaderVersion::GLSL130; // GLSL 110 and 120 are deprecated, so only output them when they are set explicitly
}

std::set<std::string> GLSLExtensionAgent::DetermineRequiredExtensions(
    Program& program, OutputShaderVersion& targetGLSLVersion,
    const ShaderTarget shaderTarget, bool allowExtensions, bool explicitBinding)
{
    /* Store parameters */
    shaderTarget_       = shaderTarget;
    targetGLSLVersion_  = targetGLSLVersion;
    minGLSLVersion_     = GetMinGLSLVersionForTarget(shaderTarget);
    allowExtensions_    = allowExtensions;
    explicitBinding_    = explicitBinding;

    /* Global layout extensions */
    switch (shaderTarget)
    {
        case ShaderTarget::VertexShader:
            //TODO:
            /* Check for explicit binding point or vertex semantics */
            //if (explicitBinding_)
            //    AcquireExtension(GLSLEXT_GL_ARB_shading_language_420pack);
            break;
            
        case ShaderTarget::FragmentShader:
            /* Check for explicit binding point (fragment shader has always binding slots) */
            if (explicitBinding_)
                AcquireExtension(GLSLEXT_GL_ARB_shading_language_420pack);
            break;

        default:
            break;
    }

    /* Visit AST program */
    Visit(&program);

    /* Return final target GLSL version */
    switch (targetGLSLVersion)
    {
        case OutputShaderVersion::GLSL:
            targetGLSLVersion = minGLSLVersion_;
            break;
        case OutputShaderVersion::ESSL:
            targetGLSLVersion = OutputShaderVersion::ESSL300;
            break;
        case OutputShaderVersion::VKSL:
            targetGLSLVersion = OutputShaderVersion::VKSL450;
            break;
        default:
            break;
    }

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
                "' or shader output version '" + ToString(extension.requiredVersion) + "' required"
            );
        }
    }
}


/* ------- Visit functions ------- */

#define IMPLEMENT_VISIT_PROC(AST_NAME) \
    void GLSLExtensionAgent::Visit##AST_NAME(AST_NAME* ast, void* args)

IMPLEMENT_VISIT_PROC(Program)
{
    if (ast->layoutFragment.fragCoordUsed)
        AcquireExtension(GLSLEXT_GL_ARB_fragment_coord_conventions);

    VISIT_DEFAULT(Program);
}

IMPLEMENT_VISIT_PROC(FunctionCall)
{
    /* Check for special intrinsics */
    if (ast->intrinsic != Intrinsic::Undefined)
    {
        auto it = intrinsicExtMap_.find(ast->intrinsic);
        if (it != intrinsicExtMap_.end())
            AcquireExtension(it->second);
    }

    VISIT_DEFAULT(FunctionCall);
}

IMPLEMENT_VISIT_PROC(Attribute)
{
    /* Check for special attributes */
    if (ast->attributeType == AttributeType::EarlyDepthStencil)
        AcquireExtension(GLSLEXT_GL_ARB_shader_image_load_store);
}

IMPLEMENT_VISIT_PROC(VarDecl)
{
    /* Check for arrays of arrays */
    if (ast->GetTypeDenoter()->NumDimensions() >= 2)
        AcquireExtension(GLSLEXT_GL_ARB_arrays_of_arrays);

    /* Check for packoffsets */
    if (ast->packOffset)
        AcquireExtension(GLSLEXT_GL_ARB_enhanced_layouts);

    VISIT_DEFAULT(VarDecl);
}

IMPLEMENT_VISIT_PROC(FunctionDecl)
{
    if (ast->flags(AST::isReachable))
    {
        Visit(ast->attribs);

        VISIT_DEFAULT(FunctionDecl);
    }
}

IMPLEMENT_VISIT_PROC(UniformBufferDecl)
{
    if (ast->flags(AST::isReachable))
    {
        if (targetGLSLVersion_ == OutputShaderVersion::GLSL || targetGLSLVersion_ >= OutputShaderVersion::GLSL140)
        {
            AcquireExtension(GLSLEXT_GL_ARB_uniform_buffer_object);

            /* Check for explicit binding point */
            if (explicitBinding_)
            {
                if (Register::GetForTarget(ast->slotRegisters, shaderTarget_) != nullptr)
                    AcquireExtension(GLSLEXT_GL_ARB_shading_language_420pack);
            }

            VISIT_DEFAULT(UniformBufferDecl);
        }
    }
}

IMPLEMENT_VISIT_PROC(BufferDeclStmnt)
{
    /* Check for explicit binding point */
    if (explicitBinding_)
    {
        for (auto& bufferDecl : ast->bufferDecls)
        {
            if (Register::GetForTarget(bufferDecl->slotRegisters, shaderTarget_) != nullptr)
                AcquireExtension(GLSLEXT_GL_ARB_shading_language_420pack);
        }
    }

    /* Check for multi-sampled textures */
    if (IsTextureMSBufferType(ast->typeDenoter->bufferType))
        AcquireExtension(GLSLEXT_GL_ARB_texture_multisample);

    VISIT_DEFAULT(BufferDeclStmnt);
}

IMPLEMENT_VISIT_PROC(BinaryExpr)
{
    /* Check if bitwise operators are used -> requires "GL_EXT_gpu_shader4" extensions */
    if (IsBitwiseOp(ast->op) || ast->op == BinaryOp::Mod)
        AcquireExtension(GLSLEXT_GL_EXT_gpu_shader4);

    VISIT_DEFAULT(BinaryExpr);
}

IMPLEMENT_VISIT_PROC(UnaryExpr)
{
    /* Check if bitwise operators are used -> requires "GL_EXT_gpu_shader4" extensions */
    if (IsBitwiseOp(ast->op))
        AcquireExtension(GLSLEXT_GL_EXT_gpu_shader4);

    VISIT_DEFAULT(UnaryExpr);
}

IMPLEMENT_VISIT_PROC(VarAccessExpr)
{
    /* Check if bitwise operators are used -> requires "GL_EXT_gpu_shader4" extensions */
    if (IsBitwiseOp(ast->assignOp) || ast->assignOp == AssignOp::Mod)
        AcquireExtension(GLSLEXT_GL_EXT_gpu_shader4);

    VISIT_DEFAULT(VarAccessExpr);
}

IMPLEMENT_VISIT_PROC(InitializerExpr)
{
    AcquireExtension(GLSLEXT_GL_ARB_shading_language_420pack);

    VISIT_DEFAULT(InitializerExpr);
}

#undef IMPLEMENT_VISIT_PROC


} // /namespace Xsc



// ================================================================================
