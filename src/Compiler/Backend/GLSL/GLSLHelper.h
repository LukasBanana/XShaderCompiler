/*
 * GLSLHelper.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_GLSL_HELPER_H
#define XSC_GLSL_HELPER_H


#include <Xsc/Xsc.h>
#include "Visitor.h"


namespace Xsc
{


// Returns true if the specified AST structure must be resolved (i.e. structure is removed, and its members are used as global variables).
bool MustResolveStructForTarget(const ShaderTarget shaderTarget, StructDecl* ast);


} // /namespace Xsc


#endif



// ================================================================================