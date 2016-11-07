/*
 * TokenString.cpp
 * 
 * This file is part of the "HLSL Translator" (Copyright (c) 2014 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "TokenString.h"


namespace HTLib
{


bool PreProcessorTokenOfInterestFunctor::IsOfInterest(const TokenPtr& token)
{
    auto type = token->Type();
    return (type != Token::Types::Comment && type != Token::Types::WhiteSpaces && type != Token::Types::NewLines);
}


} // /namespace HTLib



// ================================================================================