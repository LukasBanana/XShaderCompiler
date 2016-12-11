/*
 * HLSLKeywords.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_HLSL_KEYWORDS_H
#define XSC_HLSL_KEYWORDS_H


#include "Token.h"
#include "ASTEnums.h"
#include <map>
#include <string>


namespace Xsc
{


using KeywordMapType = std::map<std::string, Token::Types>;

// Returns the keywords map (which is an exception for identifiers).
const KeywordMapType& HLSLKeywords();

// Returns the data type for the specified HLSL keyword or throws an std::runtime_error on failure.
DataType HLSLKeywordToDataType(const std::string& keyword);

// Returns the storage class for the specified HLSL keyword or throws an std::runtime_error on failure.
StorageClass HLSLKeywordToStorageClass(const std::string& keyword);

// Returns the interpolation modifier for the specified HLSL keyword or throws an std::runtime_error on failure.
InterpModifier HLSLKeywordToInterpModifier(const std::string& keyword);

// Returns the type modifier for the specified HLSL keyword or throws an std::runtime_error on failure.
TypeModifier HLSLKeywordToTypeModifier(const std::string& keyword);

// Returns the uniform buffer type for the specified HLSL keyword or throws an std::runtime_error on failure.
UniformBufferType HLSLKeywordToUniformBufferType(const std::string& keyword);

// Returns the buffer type for the specified HLSL keyword or throws an std::runtime_error on failure.
BufferType HLSLKeywordToBufferType(const std::string& keyword);

// Returns the buffer type for the specified HLSL keyword or throws an std::runtime_error on failure.
SamplerType HLSLKeywordToSamplerType(const std::string& keyword);

// Returns the attribute type for the specified HLSL keyword or returns AttributeType::Undefined.
AttributeType HLSLKeywordToAttributeType(const std::string& keyword);

// Returns the semantic for the specified identifier or Semantic::UserDefined if the identifier is not reserved.
IndexedSemantic HLSLKeywordToSemantic(const std::string& ident, bool useD3D10Semantics = true);


} // /namespace Xsc


#endif



// ================================================================================