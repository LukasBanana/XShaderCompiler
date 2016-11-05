/*
 * PreProcessorScanner.cpp
 * 
 * This file is part of the "HLSL Translator" (Copyright (c) 2014 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "PreProcessorScanner.h"
#include <cctype>


namespace HTLib
{


PreProcessorScanner::PreProcessorScanner(Log* log) :
    Scanner{ log }
{
}

TokenPtr PreProcessorScanner::Next()
{
    return NextToken(true, true);
}


/*
 * ======= Private: =======
 */

TokenPtr PreProcessorScanner::ScanToken()
{
    /* Scan directive */
    if (Is('#'))
        return ScanDirective();

    /* Scan identifier */
    if (std::isalpha(UChr()) || Is('_'))
        return ScanIdentifier();

    /* Scan punctuation, special characters and brackets */
    switch (Chr())
    {
        case ',': return Make(Token::Types::Comma,     true); break;
        case '(': return Make(Token::Types::LBracket,  true); break;
        case ')': return Make(Token::Types::RBracket,  true); break;
    }

    /* Return miscellaneous token */
    return ScanMisc();
}

TokenPtr PreProcessorScanner::ScanDirective()
{
    /* Take directive begin '#' */
    Take('#');

    /* Ignore white spaces */
    IgnoreWhiteSpaces();

    /* Scan identifier string */
    std::string spell;

    while (std::isalpha(UChr()))
        spell += TakeIt();

    /* Return as identifier */
    return Make(Token::Types::Ident, spell);
}

TokenPtr PreProcessorScanner::ScanIdentifier()
{
    /* Scan identifier string */
    std::string spell;
    spell += TakeIt();

    while (std::isalnum(UChr()) || Is('_'))
        spell += TakeIt();

    /* Return as identifier */
    return Make(Token::Types::Ident, spell);
}

TokenPtr PreProcessorScanner::ScanMisc()
{

    return nullptr;
}


} // /namespace HTLib



// ================================================================================