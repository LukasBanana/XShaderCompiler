/*
 * Scanner.cpp
 * 
 * This file is part of the "HLSL Translator" (Copyright (c) 2014 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "Scanner.h"
#include <cctype>


namespace HTLib
{


Scanner::Scanner(Log* log) :
    log_{ log }
{
}

Scanner::~Scanner()
{
}

bool Scanner::ScanSource(const std::shared_ptr<SourceCode>& source)
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

SourcePosition Scanner::Pos() const
{
    return (source_ != nullptr ? source_->Pos() : SourcePosition::ignore);
}


/*
 * ======= Private: =======
 */

TokenPtr Scanner::NextToken(bool scanComments, bool scanWhiteSpaces)
{
    while (true)
    {
        try
        {
            /* Ignore white spaces and comments */
            bool comments = true;

            do
            {
                /* Scan or ignore white spaces */
                if (scanWhiteSpaces && std::isspace(UChr()))
                    return ScanWhiteSpaces();
                else
                    IgnoreWhiteSpaces();

                /* Check for end-of-file */
                if (Is(0))
                    return Make(Token::Types::EndOfStream);
                
                /* Scan commentaries */
                if (Is('/'))
                {
                    auto prevChr = TakeIt();

                    if (Is('/'))
                    {
                        auto tkn = ScanCommentLine(scanComments);
                        if (tkn)
                            return tkn;
                    }
                    else if (Is('*'))
                    {
                        auto tkn = ScanCommentBlock(scanComments);
                        if (tkn)
                            return tkn;
                    }
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

char Scanner::Take(char chr)
{
    if (chr_ != chr)
        ErrorUnexpected(chr);
    return TakeIt();
}

char Scanner::TakeIt()
{
    /* Get next character and return previous one */
    auto prevChr = chr_;
    chr_ = source_->Next();
    return prevChr;
}

void Scanner::Error(const std::string& msg)
{
    throw std::runtime_error("lexical error (" + Pos().ToString() + ") : " + msg);
}

void Scanner::ErrorUnexpected()
{
    auto chr = TakeIt();
    Error("unexpected character '" + std::string(1, chr) + "'");
}

void Scanner::ErrorUnexpected(char expectedChar)
{
    auto chr = TakeIt();
    Error("unexpected character '" + std::string(1, chr) + "' (expected '" + std::string(1, expectedChar) + "')");
}

void Scanner::Ignore(const std::function<bool(char)>& pred)
{
    while (pred(Chr()))
        TakeIt();
}

void Scanner::IgnoreWhiteSpaces()
{
    while (std::isspace(UChr()))
        TakeIt();
}

TokenPtr Scanner::ScanWhiteSpaces()
{
    std::string spell;
    
    while (std::isspace(UChr()))
        spell += TakeIt();

    return Make(Token::Types::WhiteSpaces, spell);
}

TokenPtr Scanner::ScanCommentLine(bool scanComments)
{
    if (scanComments)
    {
        std::string spell;

        TakeIt(); // Ignore second '/' from commentary line beginning
        while (chr_ != '\n')
            spell += TakeIt();

        return Make(Token::Types::Comment, spell);
    }
    else
    {
        Ignore([](char chr) { return chr != '\n'; });
        return nullptr;
    }
}

TokenPtr Scanner::ScanCommentBlock(bool scanComments)
{
    std::string spell;

    TakeIt(); // Ignore first '*' from commentary block beginning

    while (!Is(0))
    {
        /* Scan comment block ending */
        if (Is('*'))
        {
            TakeIt();
            if (Is('/'))
            {
                TakeIt();
                break;
            }
            else if (scanComments)
                spell += '*';
        }

        if (scanComments)
            spell += TakeIt();
        else
            TakeIt();
    }

    return (scanComments ? Make(Token::Types::Comment, spell) : nullptr);
}

TokenPtr Scanner::Make(const Token::Types& type, bool takeChr)
{
    if (takeChr)
    {
        std::string spell;
        spell += TakeIt();
        return std::make_shared<Token>(Pos(), type, std::move(spell));
    }
    return std::make_shared<Token>(Pos(), type);
}

TokenPtr Scanner::Make(const Token::Types& type, std::string& spell, bool takeChr)
{
    if (takeChr)
        spell += TakeIt();
    return std::make_shared<Token>(Pos(), type, std::move(spell));
}

TokenPtr Scanner::Make(const Token::Types& type, std::string& spell, const SourcePosition& pos, bool takeChr)
{
    if (takeChr)
        spell += TakeIt();
    return std::make_shared<Token>(pos, type, std::move(spell));
}


} // /namespace HTLib



// ================================================================================