/*
 * GLSLIntrinsics.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_GLSL_INTRINSICS_H
#define XSC_GLSL_INTRINSICS_H


#include "ASTEnums.h"
#include <string>


namespace Xsc
{


// Returns GLSL keyword for the specified intrinsic.
const std::string* IntrinsicToGLSLKeyword(const Intrinsic intr);


} // /namespace Xsc


#endif



// ================================================================================
