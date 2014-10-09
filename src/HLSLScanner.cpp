/*
 * HLSLScanner.cpp
 * 
 * This file is part of the "HLSL Translator" (Copyright (c) 2014 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "HLSLScanner.h"
#include "HLSLKeywords.h"

#include <cctype>


HLSLScanner::HLSLScanner(Logger* log) :
    log_(log)
{
}

bool HLSLScanner::ScanSource(const std::shared_ptr<SourceCode>& source)
{
    if (source)
    {
        source_ = source;

        /* Take first character from source */
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
                            return Make(Token::Types::ModifyAssignOp, spell);
                        }

                        return Make(Token::Types::DivOp, spell);
                    }
                }
                else
                    comments = false;
            }
            while (comments);

            /* Scan next token */
            return ScanToken();
        }
        catch (const std::runtime_error& err)
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
    throw std::runtime_error(Pos().ToString() + " : " + msg);
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

    /* Scan identifier */
    if (std::isalpha(UChr()) || Is('_'))
        return ScanIdentifier();

    /* Scan number */
    if (std::isdigit(UChr()))
        return ScanNumber();

    /* Scan operators */
    if (Is(':'))
    {
        spell += TakeIt();

        if (Is('='))
        {
            spell += TakeIt();
            return Make(Token::Types::CopyAssignOp, spell);
        }

        return Make(Token::Types::Colon, spell);
    }

    if (Is('='))
        return Make(Token::Types::EqualityOp, spell, true);

    if (Is('~'))
        return Make(Token::Types::BitwiseNotOp, spell, true);

    if (Is('!'))
    {
        spell += TakeIt();

        if (Is('='))
        {
            spell += TakeIt();
            return Make(Token::Types::EqualityOp, spell);
        }

        ErrorUnexpected();
    }

    if (Is('%'))
    {
        spell += TakeIt();

        if (Is('='))
        {
            spell += TakeIt();
            return Make(Token::Types::ModifyAssignOp, spell);
        }

        return Make(Token::Types::DivOp, spell);
    }

    if (Is('*'))
    {
        spell += TakeIt();

        if (Is('='))
        {
            spell += TakeIt();
            return Make(Token::Types::ModifyAssignOp, spell);
        }

        return Make(Token::Types::MulOp, spell);
    }

    if (Is('^'))
    {
        spell += TakeIt();

        if (Is('='))
        {
            spell += TakeIt();
            return Make(Token::Types::ModifyAssignOp, spell);
        }

        return Make(Token::Types::BitwiseXorOp, spell);
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
        {
            spell += TakeIt();
            return Make(Token::Types::ModifyAssignOp, spell);
        }

        return Make(Token::Types::BitwiseAndOp, spell);
    }

    if (Is('|'))
    {
        spell += TakeIt();

        if (Is('='))
        {
            spell += TakeIt();
            return Make(Token::Types::ModifyAssignOp, spell);
        }

        return Make(Token::Types::BitwiseOrOp, spell);
    }

    /* Scan punctuation, special characters and brackets */
    switch (chr_)
    {
        case ';': return Make(Token::Types::Semicolon, true); break;
        case ',': return Make(Token::Types::Comma,     true); break;
        case '.': return Make(Token::Types::Dot,       true); break;
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

TokenPtr HLSLScanner::ScanIdentifier()
{
    /* Scan identifier string */
    std::string spell;
    spell += TakeIt();

    while (std::isalnum(UChr()) || Is('_'))
        spell += TakeIt();

    /* Check for reserved internal names */
    if (spell.compare(0, 6, "__xx__") == 0)
        Error("reserved prefix \"__xx__\" used in identifier \"" + spell + "\"");

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
        {
            spell += TakeIt();
            return Make(Token::Types::ModifyAssignOp, spell);
        }

        return Make(Token::Types::ShiftOp, spell);
    }

    if (Is('='))
        spell += TakeIt();

    return Make(Token::Types::RelationOp, spell);
}

TokenPtr HLSLScanner::ScanPlusOp()
{
    std::string spell;
    spell += TakeIt();
    
    if (Is('+'))
    {
        spell += TakeIt();
        return Make(Token::Types::PostAssignOp, spell);
    }
    else if (Is('='))
    {
        spell += TakeIt();
        return Make(Token::Types::ModifyAssignOp, spell);
    }

    return Make(Token::Types::AddOp, spell);
}

TokenPtr HLSLScanner::ScanMinusOp()
{
    std::string spell;
    spell += TakeIt();

    if (Is('-'))
    {
        spell += TakeIt();
        return Make(Token::Types::PostAssignOp, spell);
    }
    else if (Is('='))
    {
        spell += TakeIt();
        return Make(Token::Types::ModifyAssignOp, spell);
    }
    else if (Is('>'))
    {
        spell += TakeIt();
        return Make(Token::Types::Arrow, spell);
    }

    return Make(Token::Types::SubOp, spell);
}

TokenPtr HLSLScanner::ScanNumber()
{
    if (!std::isdigit(UChr()))
        Error("expected digit");
    
    /* Take first number (literals like ".0" are not allowed) */
    std::string spell;

    const auto startChr = TakeIt();
    spell += startChr;

    /* Check for hex, octal or binary number */
    if (startChr == '0')
    {
        switch (chr_)
        {
            case 'x':
                TakeIt();
                return ScanHexNumber();
            case 'o':
                TakeIt();
                return ScanOctNumber();
            case 'b':
                TakeIt();
                return ScanBinNumber();
        }
    }

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

    if (std::isalpha(UChr()) || Is('.'))
        ErrorLetterInNumber();

    /* Create number token */
    return Make(type, spell);
}

TokenPtr HLSLScanner::ScanHexNumber()
{
    /* Scan hex literal */
    std::string spell;
    while (std::isxdigit(UChr()))
        spell += TakeIt();

    /* Check for wrong appendix */
    if ( ( std::isalpha(UChr()) && !std::isxdigit(UChr()) ) || Is('.') )
        ErrorLetterInNumber();

    /* Convert literal to decimal */
    spell = std::to_string(HexToNum<int>(spell));
    return Make(Token::Types::IntLiteral, spell);
}

TokenPtr HLSLScanner::ScanOctNumber()
{
    /* Scan octal literal */
    std::string spell;
    while (chr_ >= '0' && chr_ <= '7')
        spell += TakeIt();

    /* Check for wrong appendix */
    if (std::isalpha(UChr()) || Is('.') )
        ErrorLetterInNumber();

    /* Convert literal to decimal */
    spell = std::to_string(OctToNum<int>(spell));
    return Make(Token::Types::IntLiteral, spell);
}

TokenPtr HLSLScanner::ScanBinNumber()
{
    /* Scan binary literal */
    std::string spell;
    while (Is('1') || Is('0'))
        spell += TakeIt();

    /* Check for wrong appendix */
    if (std::isalpha(UChr()) || Is('.') )
        ErrorLetterInNumber();

    /* Convert literal to decimal */
    spell = std::to_string(BinToNum<int>(spell));
    return Make(Token::Types::IntLiteral, spell);
}

void HLSLScanner::ScanDecimalLiteral(std::string& spell)
{
    while (std::isdigit(UChr()))
        spell += TakeIt();
}

bool HLSLScanner::IsEscapeChar() const
{
    return
        ( chr_ >= '0' && chr_ <= '7') ||
        Is('\\') || Is('\"') || Is('\'') ||
        Is('\0') || Is( '?') || Is( 'a') ||
        Is( 'b') || Is( 'f') || Is( 'n') ||
        Is( 'r') || Is( 't') || Is( 'v') ||
        Is( 'x') || Is( 'u') || Is( 'U');
}



// ================================================================================