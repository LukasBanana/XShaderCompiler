/*
 * GLSLHelper.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLSLHelper.h"
#include "AST.h"


namespace Xsc
{


bool MustResolveStructForTarget(const ShaderTarget shaderTarget, StructDecl* ast)
{
    return
    (
        ( shaderTarget == ShaderTarget::VertexShader && ast->flags(StructDecl::isShaderInput) ) ||
        ( shaderTarget == ShaderTarget::FragmentShader && ast->flags(StructDecl::isShaderOutput) ) ||
        ( shaderTarget == ShaderTarget::ComputeShader && ( ast->flags(StructDecl::isShaderInput) || ast->flags(StructDecl::isShaderOutput) ) )
    );
}


} // /namespace Xsc



// ================================================================================
