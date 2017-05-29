/*
 * GLSLKeywords.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_GLSL_KEYWORDS_H
#define XSC_GLSL_KEYWORDS_H


#include "Token.h"
#include "ASTEnums.h"
#include <string>
#include <memory>
#include <set>


namespace Xsc
{


// Returns the Keyword-to-Token map for GLSL.
const KeywordMapType& GLSLKeywords();

// Returns the GLSL keyword for the specified data type or null on failure.
const std::string* DataTypeToGLSLKeyword(const DataType t);

// Returns the GLSL keyword for the specified storage class or null on failure.
const std::string* StorageClassToGLSLKeyword(const StorageClass t);

// Returns the GLSL keyword for the specified interpolation modifier or null on failure.
const std::string* InterpModifierToGLSLKeyword(const InterpModifier t);

// Returns the GLSL keyword for the specified buffer type or null on failure.
const std::string* BufferTypeToGLSLKeyword(const BufferType t, bool useVulkanGLSL = false, bool separateSamplers = true);

// Returns the GLSL keyword for the specified sampler type or null on failure.
const std::string* SamplerTypeToGLSLKeyword(const SamplerType t);

// Returns the sampler type for the specified GLSL keyword or throws an std::runtime_error on failure.
SamplerType GLSLKeywordToSamplerType(const std::string& keyword);

// Returns the GLSL keyword for the specified attribut value or null on failure.
const std::string* AttributeValueToGLSLKeyword(const AttributeValue t);

// Returns the GLSL keyword for the specified geometry primtive type or null on failure.
const std::string* PrimitiveTypeToGLSLKeyword(const PrimitiveType t);

// Returns the GLSL keyword for the specified image layout format or null on failure.
const std::string* ImageLayoutFormatToGLSLKeyword(const ImageLayoutFormat t);

// Returns the GLSL keyword for the specified semantic.
// Special cases if 'useVulkanGLSL' is true.
// see https://www.khronos.org/registry/vulkan/specs/misc/GL_KHR_vulkan_glsl.txt
std::unique_ptr<std::string> SemanticToGLSLKeyword(const IndexedSemantic& semantic, bool useVulkanGLSL = false);

// Returns the set of all reserved GLSL keywords (functions, intrinsics, types etc.).
const std::set<std::string>& ReservedGLSLKeywords();

// Returns the GLSL data type for specified semantic.
DataType SemanticToGLSLDataType(const Semantic t);


} // /namespace Xsc


#endif



// ================================================================================