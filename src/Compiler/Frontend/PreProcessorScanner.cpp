/*
 * PreProcessorScanner.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "PreProcessorScanner.h"
#include <cctype>


namespace Xsc
{


PreProcessorScanner::PreProcessorScanner(Log* log) :
    Scanner { log }
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
    std::string spell;

    /* Scan directive (beginning with '#'), or directive concatenation ('##') */
    if (Is('#'))
        return ScanDirectiveOrDirectiveConcat();

    /* Scan identifier */
    if (std::isalpha(UChr()) || Is('_'))
        return ScanIdentifier();

    /* Scan number */
    if (Is('.'))
        return ScanNumberOrDot();
    if (std::isdigit(UChr()))
        return ScanNumber();

    /* Scan string literal */
    if (Is('\"'))
        return ScanStringLiteral();

    /* Scan character literal */
    if (Is('\''))
        return ScanCharLiteral();

    /* Scan operators */
    if (Is('='))
    {
        spell += TakeIt();
        if (Is('='))
            return Make(Tokens::BinaryOp, spell, true);
        return Make(Tokens::Misc, spell);
    }

    if (Is('!'))
    {
        spell += TakeIt();
        if (Is('='))
            return Make(Tokens::BinaryOp, spell, true);
        return Make(Tokens::UnaryOp, spell);
    }

    if (Is('<'))
    {
        spell += TakeIt();
        if (Is('<'))
            spell += TakeIt();
        else if (Is('='))
            spell += TakeIt();
        return Make(Tokens::BinaryOp, spell);
    }

    if (Is('>'))
    {
        spell += TakeIt();
        if (Is('>'))
            spell += TakeIt();
        else if (Is('='))
            spell += TakeIt();
        return Make(Tokens::BinaryOp, spell);
    }

    if (Is('&'))
    {
        spell += TakeIt();
        if (Is('&'))
            spell += TakeIt();
        return Make(Tokens::BinaryOp, spell);
    }

    if (Is('|'))
    {
        spell += TakeIt();
        if (Is('|'))
            spell += TakeIt();
        return Make(Tokens::BinaryOp, spell);
    }

    /* Scan punctuation, special characters and brackets */
    switch (Chr())
    {
        case  ':': return Make(Token::Types::Colon,     true); break;
        case  ',': return Make(Token::Types::Comma,     true); break;
        case  '?': return Make(Token::Types::TernaryOp, true); break;
        case  '(': return Make(Token::Types::LBracket,  true); break;
        case  ')': return Make(Token::Types::RBracket,  true); break;
        case  '~': return Make(Token::Types::UnaryOp,   true); break;
        case  '^': return Make(Token::Types::BinaryOp,  true); break;
        case  '%': return Make(Token::Types::BinaryOp,  true); break;
        case  '+': return Make(Token::Types::BinaryOp,  true); break;
        case  '-': return Make(Token::Types::BinaryOp,  true); break;
        case  '*': return Make(Token::Types::BinaryOp,  true); break;
        case  '/': return Make(Token::Types::BinaryOp,  true); break;
        case '\\': return Make(Token::Types::LineBreak, true); break;
    }

    /* Return miscellaneous token */
    return Make(Token::Types::Misc, true);
}

TokenPtr PreProcessorScanner::ScanDirectiveOrDirectiveConcat()
{
    std::string spell;

    /* Take directive begin '#' */
    Take('#');

    /* Check for concatenation */
    if (Is('#'))
    {
        TakeIt();
        spell = "##";
        return Make(Token::Types::DirectiveConcat, spell);
    }

    /* Ignore white spaces (but not new-lines) */
    IgnoreWhiteSpaces(false);

    /* Scan identifier string */
    StoreStartPos();

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


} // /namespace Xsc



// ================================================================================