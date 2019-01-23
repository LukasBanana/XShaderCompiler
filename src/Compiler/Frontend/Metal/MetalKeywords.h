/*
 * MetalKeywords.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_METAL_KEYWORDS_H
#define XSC_METAL_KEYWORDS_H


#include "Token.h"
#include "ASTEnums.h"
#include <string>
#include <memory>
#include <set>


namespace Xsc
{


/* ----- DataType Mapping ----- */

// Returns the Metal keyword for the specified data type or null on failure.
const std::string* DataTypeToMetalKeyword(const DataType t);


/* ----- StorageClass Mapping ----- */

// Returns the Metal keyword for the specified storage class or null on failure.
const std::string* StorageClassToMetalKeyword(const StorageClass t);


/* ----- InterpModifier Mapping ----- */

// Returns the Metal keyword for the specified interpolation modifier or null on failure.
const std::string* InterpModifierToMetalKeyword(const InterpModifier t);


/* ----- BufferType Mapping ----- */

// Returns the Metal keyword for the specified buffer type or null on failure.
const std::string* BufferTypeToMetalKeyword(const BufferType t);


/* ----- SamplerType Mapping ----- */

// Returns the Metal keyword for the specified sampler type or null on failure.
const std::string* SamplerTypeToMetalKeyword(const SamplerType t);


/* ----- Semantic Mapping ----- */

// Returns the Metal keyword for the specified semantic or null on failure.
std::unique_ptr<std::string> SemanticToMetalKeyword(const IndexedSemantic& semantic);


/* ----- Reserved Metal Keywords ----- */

// Returns the set of all reserved Metal keywords (functions, intrinsics, types etc.).
const std::set<std::string>& ReservedMetalKeywords();


} // /namespace Xsc


#endif



// ================================================================================
