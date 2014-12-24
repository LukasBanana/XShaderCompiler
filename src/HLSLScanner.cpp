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


HLSLScanner::HLSLScanner(Logger* log) :
    log_{ log }
{
}

bool HLSLScanner::ScanSource(const std::shared_ptr<SourceCode>& source)
{
    if (source && source->IsValid())
    {
        /* Store source stream and take first character */
        source_ = source;
        TakeIt();
        return true;
    }
    return false;
}

TokenPtr HLSLScanner::Next()
{
    while (true)
    {
        try
        {
            /* Ignore white spaces and comments */
            bool comments = true;

            do
            {
                IgnoreWhiteSpaces();

                /* Check for end-of-file */
                if (Is(0))
                    return Make(Token::Types::EndOfStream);
                
                /* Scan commentaries */
                if (Is('/'))
                {
                    auto prevChr = TakeIt();

                    if (Is('/'))
                        IgnoreCommentLine();
                    else if (Is('*'))
                        IgnoreCommentBlock();
                    else
                    {
                        std::string spell;
                        spell += prevChr;

                        if (Is('='))
                        {
                            spell += TakeIt();
                            return Make(Token::Types::AssignOp, spell);
                        }

                        return Make(Token::Types::BinaryOp, spell);
                    }
                }
                else
                    comments = false;
            }
            while (comments);

            /* Scan next token */
            return ScanToken();
        }
        catch (const std::exception& err)
        {
            /* Add to error and scan next token */
            if (log_)
                log_->Error(err.what());
        }
    }

    return nullptr;
}

SourcePosition HLSLScanner::Pos() const
{
    return source_ != nullptr ? source_->Pos() : SourcePosition::ignore;
}


/*
 * ======= Private: =======
 */

char HLSLScanner::Take(char chr)
{
    if (chr_ != chr)
        ErrorUnexpected(chr);
    return TakeIt();
}

char HLSLScanner::TakeIt()
{
    /* Get next character and return previous one */
    auto prevChr = chr_;
    chr_ = source_->Next();
    return prevChr;
}

void HLSLScanner::Error(const std::string& msg)
{
    throw std::runtime_error("lexical error (" + Pos().ToString() + ") : " + msg);
}

void HLSLScanner::ErrorUnexpected()
{
    auto chr = TakeIt();
    Error("unexpected character '" + std::string(1, chr) + "'");
}

void HLSLScanner::ErrorUnexpected(char expectedChar)
{
    auto chr = TakeIt();
    Error("unexpected character '" + std::string(1, chr) + "' (expected '" + std::string(1, expectedChar) + "')");
}

void HLSLScanner::ErrorEOF()
{
    Error("unexpected end-of-file");
}

void HLSLScanner::ErrorLetterInNumber()
{
    Error("letter '" + std::string(1, chr_) + "' is not allowed within a number");
}

void HLSLScanner::Ignore(const std::function<bool (char)>& pred)
{
    while (pred(chr_))
        TakeIt();
}

void HLSLScanner::IgnoreWhiteSpaces()
{
    Ignore([](char chr) { return std::isspace(static_cast<unsigned char>(chr)) != 0; });
}

void HLSLScanner::IgnoreCommentLine()
{
    Ignore([](char chr) { return chr != '\n'; });
}

void HLSLScanner::IgnoreCommentBlock()
{
    while (true)
    {
        if (Is(0))
            return;

        /* Scan comment block ending */
        if (Is('*'))
        {
            TakeIt();
            if (Is('/'))
            {
                TakeIt();
                return;
            }
        }

        TakeIt();
    }
}

TokenPtr HLSLScanner::Make(const Token::Types& type, bool takeChr)
{
    if (takeChr)
    {
        std::string spell;
        spell += TakeIt();
        return std::make_shared<Token>(Pos(), type, std::move(spell));
    }
    return std::make_shared<Token>(Pos(), type);
}

TokenPtr HLSLScanner::Make(const Token::Types& type, std::string& spell, bool takeChr)
{
    if (takeChr)
        spell += TakeIt();
    return std::make_shared<Token>(Pos(), type, std::move(spell));
}

TokenPtr HLSLScanner::Make(const Token::Types& type, std::string& spell, const SourcePosition& pos, bool takeChr)
{
    if (takeChr)
        spell += TakeIt();
    return std::make_shared<Token>(pos, type, std::move(spell));
}

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
        return ScanAssignShiftRelationOp(chr_);

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
    switch (chr_)
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