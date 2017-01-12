/*
 * HLSLScanner.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "HLSLScanner.h"
#include "HLSLKeywords.h"
#include <cctype>


namespace Xsc
{


HLSLScanner::HLSLScanner(Log* log) :
    Scanner{ log }
{
}

TokenPtr HLSLScanner::Next()
{
    return NextToken(false, false);
}


/*
 * ======= Private: =======
 */

TokenPtr HLSLScanner::ScanToken()
{
    std::string spell;

    /* Scan directive (beginning with '#') */
    if (Is('#'))
        return ScanDirective();

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

    /* Scan operators */
    if (Is('='))
    {
        spell += TakeIt();
        if (Is('='))
            return Make(Tokens::BinaryOp, spell, true);
        return Make(Tokens::AssignOp, spell);
    }

    if (Is('~'))
        return Make(Tokens::UnaryOp, spell, true);

    if (Is('!'))
    {
        spell += TakeIt();
        if (Is('='))
            return Make(Tokens::BinaryOp, spell, true);
        return Make(Tokens::UnaryOp, spell);
    }

    if (Is('%'))
    {
        spell += TakeIt();
        if (Is('='))
            return Make(Tokens::AssignOp, spell, true);
        return Make(Tokens::BinaryOp, spell);
    }

    if (Is('*'))
    {
        spell += TakeIt();
        if (Is('='))
            return Make(Tokens::AssignOp, spell, true);
        return Make(Tokens::BinaryOp, spell);
    }

    if (Is('^'))
    {
        spell += TakeIt();
        if (Is('='))
            return Make(Tokens::AssignOp, spell, true);
        return Make(Tokens::BinaryOp, spell);
    }

    if (Is('+'))
        return ScanPlusOp();
    if (Is('-'))
        return ScanMinusOp();

    if (Is('<') || Is('>'))
        return ScanAssignShiftRelationOp(Chr());

    if (Is('&'))
    {
        spell += TakeIt();
        if (Is('='))
            return Make(Tokens::AssignOp, spell, true);
        if (Is('&'))
            return Make(Tokens::BinaryOp, spell, true);
        return Make(Tokens::BinaryOp, spell);
    }

    if (Is('|'))
    {
        spell += TakeIt();
        if (Is('='))
            return Make(Tokens::AssignOp, spell, true);
        if (Is('|'))
            return Make(Tokens::BinaryOp, spell, true);
        return Make(Tokens::BinaryOp, spell);
    }

    /* Scan punctuation, special characters and brackets */
    switch (Chr())
    {
        case ':': return Make(Tokens::Colon,     true); break;
        case ';': return Make(Tokens::Semicolon, true); break;
        case ',': return Make(Tokens::Comma,     true); break;
        case '?': return Make(Tokens::TernaryOp, true); break;
        case '(': return Make(Tokens::LBracket,  true); break;
        case ')': return Make(Tokens::RBracket,  true); break;
        case '{': return Make(Tokens::LCurly,    true); break;
        case '}': return Make(Tokens::RCurly,    true); break;
        case '[': return Make(Tokens::LParen,    true); break;
        case ']': return Make(Tokens::RParen,    true); break;
    }

    ErrorUnexpected();

    return nullptr;
}

TokenPtr HLSLScanner::ScanDirective()
{
    std::string spell;

    /* Take directive begin '#' */
    Take('#');

    /* Ignore white spaces (but not new-lines) */
    IgnoreWhiteSpaces(false);

    /* Scan identifier string */
    StoreStartPos();

    while (std::isalpha(UChr()))
        spell += TakeIt();

    /* Return as identifier */
    return Make(Token::Types::Directive, spell);
}

TokenPtr HLSLScanner::ScanIdentifier()
{
    /* Scan identifier string */
    std::string spell;
    spell += TakeIt();

    while (std::isalnum(UChr()) || Is('_'))
        spell += TakeIt();

    /* Scan reserved words */
    auto it = HLSLKeywords().find(spell);
    if (it != HLSLKeywords().end())
    {
        if (it->second == Token::Types::Reserved)
            Error("keyword '" + spell + "' is reserved for future use");
        else if (it->second == Token::Types::Unsupported)
            Error("keyword '" + spell + "' is currently not supported");
        else
            return Make(it->second, spell);
    }

    /* Return as identifier */
    return Make(Tokens::Ident, spell);
}

TokenPtr HLSLScanner::ScanAssignShiftRelationOp(const char chr)
{
    std::string spell;
    spell += TakeIt();

    if (Is(chr))
    {
        spell += TakeIt();

        if (Is('='))
            return Make(Tokens::AssignOp, spell, true);

        return Make(Tokens::BinaryOp, spell);
    }

    if (Is('='))
        spell += TakeIt();

    return Make(Tokens::BinaryOp, spell);
}

TokenPtr HLSLScanner::ScanPlusOp()
{
    std::string spell;
    spell += TakeIt();
    
    if (Is('+'))
        return Make(Tokens::UnaryOp, spell, true);
    else if (Is('='))
        return Make(Tokens::AssignOp, spell, true);

    return Make(Tokens::BinaryOp, spell);
}

TokenPtr HLSLScanner::ScanMinusOp()
{
    std::string spell;
    spell += TakeIt();

    if (Is('-'))
        return Make(Tokens::UnaryOp, spell, true);
    else if (Is('='))
        return Make(Tokens::AssignOp, spell, true);

    return Make(Tokens::BinaryOp, spell);
}


} // /namespace Xsc



// ================================================================================