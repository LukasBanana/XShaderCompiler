/*
 * GLSLExtensionAgent.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLSLExtensionAgent.h"
#include "GLSLExtensions.h"
#include "AST.h"
#include "Exception.h"
#include "ReportIdents.h"
#include <algorithm>


namespace Xsc
{


/*
 * GLSLExtensionAgent class
 */

GLSLExtensionAgent::GLSLExtensionAgent()
{
    /* Establish intrinsic-to-extension map */
    intrinsicExtMap_ = std::map<Intrinsic, const char*>
    {
        { Intrinsic::AsDouble,                  E_GL_ARB_gpu_shader_int64    },
        { Intrinsic::AsFloat,                   E_GL_ARB_shader_bit_encoding },
        { Intrinsic::AsInt,                     E_GL_ARB_shader_bit_encoding },
        { Intrinsic::AsUInt_1,                  E_GL_ARB_shader_bit_encoding },
        { Intrinsic::CountBits,                 E_GL_ARB_gpu_shader5         },
        { Intrinsic::DDXCoarse,                 E_GL_ARB_derivative_control  },
        { Intrinsic::DDXFine,                   E_GL_ARB_derivative_control  },
        { Intrinsic::DDYCoarse,                 E_GL_ARB_derivative_control  },
        { Intrinsic::DDYFine,                   E_GL_ARB_derivative_control  },
        { Intrinsic::FirstBitHigh,              E_GL_ARB_gpu_shader5         },
        { Intrinsic::FirstBitLow,               E_GL_ARB_gpu_shader5         },
        { Intrinsic::FrExp,                     E_GL_ARB_gpu_shader_fp64     },
        { Intrinsic::LdExp,                     E_GL_ARB_gpu_shader_fp64     },
        { Intrinsic::Texture_QueryLod,          E_GL_ARB_texture_query_lod   },
        { Intrinsic::Texture_QueryLodUnclamped, E_GL_ARB_texture_query_lod   },
    };
}

static OutputShaderVersion GetMinGLSLVersionForTarget(const ShaderTarget shaderTarget)
{
    switch (shaderTarget)
    {
        case ShaderTarget::Undefined:                       break;
        case ShaderTarget::VertexShader:                    return OutputShaderVersion::GLSL130; // default is 130, but 110 can be used manually
        case ShaderTarget::TessellationControlShader:       return OutputShaderVersion::GLSL400;
        case ShaderTarget::TessellationEvaluationShader:    return OutputShaderVersion::GLSL400;
        case ShaderTarget::GeometryShader:                  return OutputShaderVersion::GLSL150;
        case ShaderTarget::FragmentShader:                  return OutputShaderVersion::GLSL130; // default is 130, but 110 can be used manually
        case ShaderTarget::ComputeShader:                   return OutputShaderVersion::GLSL430; // actually 420, but only 430 supports local work group size
    }
    return OutputShaderVersion::GLSL130; // GLSL 110 and 120 are deprecated, so only output them when they are set explicitly
}

std::set<std::string> GLSLExtensionAgent::DetermineRequiredExtensions(
    Program& program, OutputShaderVersion& targetGLSLVersion, const ShaderTarget shaderTarget,
    bool allowExtensions, bool explicitBinding, bool separateShaders, const OnReportProc& onReportExtension)
{
    /* Store parameters */
    shaderTarget_       = shaderTarget;
    targetGLSLVersion_  = targetGLSLVersion;
    minGLSLVersion_     = GetMinGLSLVersionForTarget(shaderTarget);
    allowExtensions_    = allowExtensions;
    explicitBinding_    = explicitBinding;
    onReportExtension_  = onReportExtension;

    /* Global layout extensions */
    switch (shaderTarget)
    {
        case ShaderTarget::VertexShader:
        case ShaderTarget::FragmentShader:
            /* Check for explicit binding for vertex input attributes or fragment shader outputs */
            if (explicitBinding_)
                AcquireExtension(E_GL_ARB_explicit_attrib_location);
            break;

        default:
            break;
    }

    if (separateShaders)
    {
        if (shaderTarget != ShaderTarget::ComputeShader)
            AcquireExtension(E_GL_ARB_separate_shader_objects);
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

void GLSLExtensionAgent::AcquireExtension(const std::string& extension, const std::string& reason, const AST* ast)
{
    /* Find extension in version map */
    auto it = GetGLSLExtensionVersionMap().find(extension);
    if (it != GetGLSLExtensionVersionMap().end())
    {
        const auto requiredVersion = static_cast<OutputShaderVersion>(it->second);

        if (targetGLSLVersion_ == OutputShaderVersion::GLSL)
        {
            /* Store minimum required GLSL version */
            minGLSLVersion_ = std::max(minGLSLVersion_, requiredVersion);
        }
        else if (targetGLSLVersion_ < requiredVersion)
        {
            if (allowExtensions_)
            {
                /* Add extension to the resulting set, if the target GLSL version is less than the required extension version */
                extensions_.insert(extension);

                /* Report warning */
                if (onReportExtension_)
                    onReportExtension_(R_GLSLExtensionAcquired(extension, ToString(requiredVersion), reason), ast);
            }
            else
            {
                /* Report error if required extension is not allowed */
                if (onReportExtension_)
                    onReportExtension_(R_GLSLExtensionOrVersionRequired(extension, ToString(requiredVersion), reason), ast);
            }
        }
    }
    else
        RuntimeErr(R_NoGLSLExtensionVersionRegisterd(extension), ast);
}


/* ------- Visit functions ------- */

#define IMPLEMENT_VISIT_PROC(AST_NAME) \
    void GLSLExtensionAgent::Visit##AST_NAME(AST_NAME* ast, void* args)

IMPLEMENT_VISIT_PROC(Program)
{
    if (ast->layoutFragment.fragCoordUsed)
        AcquireExtension(E_GL_ARB_fragment_coord_conventions, R_FragmentCoordinate);

    VISIT_DEFAULT(Program);
}

IMPLEMENT_VISIT_PROC(Attribute)
{
    /* Check for special attributes */
    if (ast->attributeType == AttributeType::EarlyDepthStencil)
        AcquireExtension(E_GL_ARB_shader_image_load_store, R_EarlyDepthStencil, ast);
}

IMPLEMENT_VISIT_PROC(VarDecl)
{
    /* Check for arrays of arrays */
    if (ast->GetTypeDenoter()->NumDimensions() >= 2)
        AcquireExtension(E_GL_ARB_arrays_of_arrays, R_MultiDimArray, ast);

    /* Check for packoffsets */
    if (ast->packOffset)
        AcquireExtension(E_GL_ARB_enhanced_layouts, R_PackOffsetLayout, ast);

    VISIT_DEFAULT(VarDecl);
}

IMPLEMENT_VISIT_PROC(BufferDecl)
{
    if (ast->flags(AST::isReachable))
    {
        /* Check for arrays of arrays */
        if (ast->GetTypeDenoter()->NumDimensions() >= 2)
            AcquireExtension(E_GL_ARB_arrays_of_arrays, R_MultiDimArray, ast);

        /* Check for buffer types */
        const auto bufferType = ast->GetBufferType();

        if (bufferType == BufferType::TextureCubeArray)
            AcquireExtension(E_GL_ARB_texture_cube_map_array, R_TextureCubeArray, ast);

        if (IsRWBufferType(bufferType))
        {
            if ( bufferType == BufferType::RWStructuredBuffer       ||
                 bufferType == BufferType::RWByteAddressBuffer      ||
                 bufferType == BufferType::AppendStructuredBuffer   ||
                 bufferType == BufferType::ConsumeStructuredBuffer )
            {
                AcquireExtension(E_GL_ARB_shader_storage_buffer_object, R_RWStructuredBufferObject, ast);
            }
            else
                AcquireExtension(E_GL_ARB_shader_image_load_store, R_RWTextureObject, ast);
        }

        VISIT_DEFAULT(BufferDecl);
    }
}

IMPLEMENT_VISIT_PROC(FunctionDecl)
{
    if (ast->flags(AST::isReachable))
    {
        Visit(ast->declStmntRef->attribs);

        VISIT_DEFAULT(FunctionDecl);
    }
}

IMPLEMENT_VISIT_PROC(UniformBufferDecl)
{
    if (ast->flags(AST::isReachable))
    {
        if (targetGLSLVersion_ == OutputShaderVersion::GLSL || targetGLSLVersion_ >= OutputShaderVersion::GLSL140)
        {
            AcquireExtension(E_GL_ARB_uniform_buffer_object, R_ConstantBuffer, ast);

            /* Check for explicit binding point */
            if (explicitBinding_)
            {
                if (auto slotRegister = Register::GetForTarget(ast->slotRegisters, shaderTarget_))
                    AcquireExtension(E_GL_ARB_shading_language_420pack, R_ExplicitBindingSlot, slotRegister);
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
            if (auto slotRegister = Register::GetForTarget(bufferDecl->slotRegisters, shaderTarget_))
                AcquireExtension(E_GL_ARB_shading_language_420pack, R_ExplicitBindingSlot, slotRegister);
        }
    }

    /* Check for multi-sampled textures */
    if (IsTextureMSBufferType(ast->typeDenoter->bufferType))
        AcquireExtension(E_GL_ARB_texture_multisample, R_MultiSampledTexture, ast);

    VISIT_DEFAULT(BufferDeclStmnt);
}

IMPLEMENT_VISIT_PROC(BasicDeclStmnt)
{
    /* Only visit declaration object (not attributes here) */
    Visit(ast->declObject);
}

IMPLEMENT_VISIT_PROC(BinaryExpr)
{
    /* Check if bitwise operators are used -> requires "GL_EXT_gpu_shader4" extensions */
    if (IsBitwiseOp(ast->op) || ast->op == BinaryOp::Mod)
        AcquireExtension(E_GL_EXT_gpu_shader4, R_BitwiseOperator, ast);

    VISIT_DEFAULT(BinaryExpr);
}

IMPLEMENT_VISIT_PROC(UnaryExpr)
{
    /* Check if bitwise operators are used -> requires "GL_EXT_gpu_shader4" extensions */
    if (IsBitwiseOp(ast->op))
        AcquireExtension(E_GL_EXT_gpu_shader4, R_BitwiseOperator, ast);

    VISIT_DEFAULT(UnaryExpr);
}

IMPLEMENT_VISIT_PROC(CallExpr)
{
    /* Check for special intrinsics */
    if (ast->intrinsic != Intrinsic::Undefined)
    {
        auto it = intrinsicExtMap_.find(ast->intrinsic);
        if (it != intrinsicExtMap_.end())
            AcquireExtension(it->second, R_Intrinsic(ast->ident), ast);
    }

    VISIT_DEFAULT(CallExpr);
}

IMPLEMENT_VISIT_PROC(AssignExpr)
{
    /* Check if bitwise operators are used -> requires "GL_EXT_gpu_shader4" extensions */
    if (IsBitwiseOp(ast->op) || ast->op == AssignOp::Mod)
        AcquireExtension(E_GL_EXT_gpu_shader4, R_BitwiseOperator, ast);

    VISIT_DEFAULT(AssignExpr);
}

IMPLEMENT_VISIT_PROC(InitializerExpr)
{
    AcquireExtension(E_GL_ARB_shading_language_420pack, R_InitializerList, ast);

    VISIT_DEFAULT(InitializerExpr);
}

#undef IMPLEMENT_VISIT_PROC


} // /namespace Xsc



// ================================================================================
