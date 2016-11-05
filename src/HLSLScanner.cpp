/*
 * HLSLScanner.cpp
 * 
 * This file is part of the "HLSL Translator" (Copyright (c) 2014 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "HLSLScanner.h"
#include "HLSLKeywords.h"

#include <cctype>


namespace HTLib
{


HLSLScanner::HLSLScanner(Log* log) :
    Scanner{ log }
{
}


/*
 * ======= Private: =======
 */

TokenPtr HLSLScanner::ScanToken()
{
    std::string spell;

    /* Scan directive */
    if (Is('#'))
        return ScanDirective();

    /* Scan identifier */
    if (std::isalpha(UChr()) || Is('_'))
        return ScanIdentifier();

    /* Scan number */
    if (std::isdigit(UChr()))
        return ScanNumber();

    /* Scan operators */
    if (Is('='))
    {
        spell += TakeIt();

        if (Is('='))
            return Make(Token::Types::BinaryOp, spell, true);

        return Make(Token::Types::AssignOp, spell);
    }

    if (Is('~'))
        return Make(Token::Types::UnaryOp, spell, true);

    if (Is('!'))
    {
        spell += TakeIt();

        if (Is('='))
            return Make(Token::Types::BinaryOp, spell, true);

        return Make(Token::Types::UnaryOp, spell);
    }

    if (Is('%'))
    {
        spell += TakeIt();

        if (Is('='))
            return Make(Token::Types::AssignOp, spell, true);

        return Make(Token::Types::BinaryOp, spell);
    }

    if (Is('*'))
    {
        spell += TakeIt();

        if (Is('='))
            return Make(Token::Types::AssignOp, spell, true);

        return Make(Token::Types::BinaryOp, spell);
    }

    if (Is('^'))
    {
        spell += TakeIt();

        if (Is('='))
            return Make(Token::Types::AssignOp, spell, true);

        return Make(Token::Types::BinaryOp, spell);
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
            return Make(Token::Types::AssignOp, spell, true);
        if (Is('&'))
            return Make(Token::Types::BinaryOp, spell, true);

        return Make(Token::Types::BinaryOp, spell);
    }

    if (Is('|'))
    {
        spell += TakeIt();

        if (Is('='))
            return Make(Token::Types::AssignOp, spell, true);
        if (Is('|'))
            return Make(Token::Types::BinaryOp, spell, true);

        return Make(Token::Types::BinaryOp, spell);
    }

    /* Scan punctuation, special characters and brackets */
    switch (Chr())
    {
        case ':': return Make(Token::Types::Colon,     true); break;
        case ';': return Make(Token::Types::Semicolon, true); break;
        case ',': return Make(Token::Types::Comma,     true); break;
        case '.': return Make(Token::Types::Dot,       true); break;
        case '?': return Make(Token::Types::TernaryOp, true); break;
        case '(': return Make(Token::Types::LBracket,  true); break;
        case ')': return Make(Token::Types::RBracket,  true); break;
        case '{': return Make(Token::Types::LCurly,    true); break;
        case '}': return Make(Token::Types::RCurly,    true); break;
        case '[': return Make(Token::Types::LParen,    true); break;
        case ']': return Make(Token::Types::RParen,    true); break;
    }

    ErrorUnexpected();

    return nullptr;
}

TokenPtr HLSLScanner::ScanDirective()
{
    std::string spell;
    bool takeNextLine = false;

    while (!Is('\n') || takeNextLine)
    {
        takeNextLine = false;
        if (Is('\\'))
            takeNextLine = true;
        spell += TakeIt();
    }

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
        return Make(it->second, spell);

    /* Return as identifier */
    return Make(Token::Types::Ident, spell);
}

TokenPtr HLSLScanner::ScanAssignShiftRelationOp(const char chr)
{
    std::string spell;
    spell += TakeIt();

    if (Is(chr))
    {
        spell += TakeIt();

        if (Is('='))
            return Make(Token::Types::AssignOp, spell, true);

        return Make(Token::Types::BinaryOp, spell);
    }

    if (Is('='))
        spell += TakeIt();

    return Make(Token::Types::BinaryOp, spell);
}

TokenPtr HLSLScanner::ScanPlusOp()
{
    std::string spell;
    spell += TakeIt();
    
    if (Is('+'))
        return Make(Token::Types::UnaryOp, spell, true);
    else if (Is('='))
        return Make(Token::Types::AssignOp, spell, true);

    return Make(Token::Types::BinaryOp, spell);
}

TokenPtr HLSLScanner::ScanMinusOp()
{
    std::string spell;
    spell += TakeIt();

    if (Is('-'))
        return Make(Token::Types::UnaryOp, spell, true);
    else if (Is('='))
        return Make(Token::Types::AssignOp, spell, true);

    return Make(Token::Types::BinaryOp, spell);
}

TokenPtr HLSLScanner::ScanNumber()
{
    if (!std::isdigit(UChr()))
        Error("expected digit");
    
    /* Take first number (literals like ".0" are not allowed) */
    std::string spell;

    const auto startChr = TakeIt();
    spell += startChr;

    /* Parse integer or floating-point number */
    auto type = Token::Types::IntLiteral;

    ScanDecimalLiteral(spell);

    if (Is('.'))
    {
        spell += TakeIt();
        
        if (std::isdigit(UChr()))
            ScanDecimalLiteral(spell);
        else
            Error("floating-point literals must have a decimal on both sides of the dot (e.g. '0.0' but not '0.' or '.0')");

        type = Token::Types::FloatLiteral;
    }

    if (Is('f') || Is('F'))
        TakeIt();

    if (std::isalpha(UChr()) || Is('.'))
        ErrorLetterInNumber();

    /* Create number token */
    return Make(type, spell);
}

void HLSLScanner::ScanDecimalLiteral(std::string& spell)
{
    while (std::isdigit(UChr()))
        spell += TakeIt();
}


} // /namespace HTLib



// ================================================================================