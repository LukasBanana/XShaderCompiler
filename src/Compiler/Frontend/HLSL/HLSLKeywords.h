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

// Returns the data type for the specified HLSL keyword (throws std::runtime_error on failure).
DataType HLSLKeywordToDataType(const std::string& keyword);


} // /namespace Xsc


#endif



// ================================================================================