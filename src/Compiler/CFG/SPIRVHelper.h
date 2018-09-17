/*
 * SPIRVHelper.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2018 by Lukas Hermanns)
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

// Returns the SPIR-V generator name by the specified ID number.
const char* GetSPIRVGeneratorNameById(unsigned int id);

// Returns the specified SPIR-V version as string, or returns null if the version number is unknown.
const char* GetSPIRVVersionStringOrNull(unsigned int version);


} // /namespace SPIRVHelper

} // /namespace Xsc


#endif



// ================================================================================
