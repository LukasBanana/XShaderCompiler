/*
 * TokenString.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "TokenString.h"


namespace Xsc
{


bool DefaultTokenOfInterestFunctor::IsOfInterest(const TokenPtr& token)
{
    auto type = token->Type();
    return (type != Token::Types::Comment && type != Token::Types::WhiteSpaces && type != Token::Types::NewLines);
}


} // /namespace Xsc



// ================================================================================