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

    /* Scan number */
    if (std::isdigit(UChr()))
        return ScanNumber();

    /* Scan string literal */
    if (Is('\"'))
        return ScanStringLiteral();

    /* Scan punctuation, special characters and brackets */
    switch (Chr())
    {
        case  ',': return Make(Token::Types::Comma,     true); break;
        case  '(': return Make(Token::Types::LBracket,  true); break;
        case  ')': return Make(Token::Types::RBracket,  true); break;
        case  '*': return Make(Token::Types::BinaryOp,  true); break;
        case  '/': return Make(Token::Types::BinaryOp,  true); break;
        case '\\': return Make(Token::Types::LineBreak, true); break;
    }

    /* Return miscellaneous token */
    return ScanMisc();
}

TokenPtr PreProcessorScanner::ScanDirective()
{
    /* Take directive begin '#' */
    Take('#');

    /* Ignore white spaces (but not new-lines) */
    IgnoreWhiteSpaces(false);

    /* Scan identifier string */
    StoreStartPos();

    std::string spell;
    while (std::isalpha(UChr()))
        spell += TakeIt();

    /* Return as identifier */
    return Make(Token::Types::Directive, spell);
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
    /* Scan string as long as no identifier or directive character appears */
    std::string spell;
    
    while (!std::isalnum(UChr()) && !std::isspace(UChr()) && !Is(0) && std::string("_,()#\"/\\*").find(Chr()) == std::string::npos)
        spell += TakeIt();

    /* Return as misc token */
    return Make(Token::Types::Misc, spell);
}


} // /namespace HTLib



// ================================================================================