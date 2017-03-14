/*
 * TokenString.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "TokenString.h"
#include "ReportIdents.h"


namespace Xsc
{


void AssertReachedEnd(bool reachedEnd)
{
    if (reachedEnd)
        throw std::runtime_error(R_UnexpectedEndOfStream);
}

void AssertCurrentTokenType(const Token::Types type, const Token::Types expectedType)
{
    if (type != expectedType)
    {
        throw std::runtime_error(
            R_UnexpectedToken(
                Token::TypeToString(type),
                (R_Expected + " " + Token::TypeToString(expectedType))
            )
        );
    }
}


/*
 * DefaultTokenOfInterestFunctor structure
 */

bool DefaultTokenOfInterestFunctor::IsOfInterest(const TokenPtr& token)
{
    auto type = token->Type();
    return (type != Token::Types::Comment && type != Token::Types::WhiteSpaces && type != Token::Types::NewLines);
}


} // /namespace Xsc



// ================================================================================