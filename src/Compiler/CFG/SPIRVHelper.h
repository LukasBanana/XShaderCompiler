/*
 * SPIRVHelper.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_SPIRV_HELPER_H
#define XSC_SPIRV_HELPER_H


#include <spirv/1.2/spirv.hpp11>


namespace Xsc
{

namespace SPIRVHelper
{


// Returns true if the specified SPIR-V instruction op-code has a type-ID.
bool HasTypeId(const spv::Op opCode);

// Returns true if the specified SPIR-V instruction op-code has a result-ID.
bool HasResultId(const spv::Op opCode);


} // /namespace SPIRVHelper

} // /namespace Xsc


#endif



// ================================================================================
