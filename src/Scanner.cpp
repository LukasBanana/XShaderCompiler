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

TokenPtr Scanner::ActiveToken() const
{
    return activeToken_;
}

TokenPtr Scanner::PreviousToken() const
{
    return prevToken_;
}

SourcePosition Scanner::Pos() const
{
    return nextStartPos_;//(source_ != nullptr ? source_->Pos() : SourcePosition::ignore);
}


/*
 * ======= Private: =======
 */

//private
TokenPtr Scanner::NextToken(bool scanComments, bool scanWhiteSpaces)
{
    /* Store previous token */
    prevToken_ = activeToken_;

    /* Scan next token */
    auto tkn = NextTokenScan(scanComments, scanWhiteSpaces);

    /* Store new active token */
    activeToken_ = tkn;

    return tkn;
}

//private
TokenPtr Scanner::NextTokenScan(bool scanComments, bool scanWhiteSpaces)
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
                {
                    StoreStartPos();
                    return ScanWhiteSpaces(false);
                }
                else
                    IgnoreWhiteSpaces();

                /* Check for end-of-file */
                if (Is(0))
                {
                    StoreStartPos();
                    return Make(Tokens::EndOfStream);
                }
                
                /* Scan commentaries */
                if (Is('/'))
                {
                    StoreStartPos();

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
                            return Make(Tokens::AssignOp, spell);
                        }

                        return Make(Tokens::BinaryOp, spell);
                    }
                }
                else
                    comments = false;
            }
            while (comments);

            /* Scan next token */
            StoreStartPos();
            return ScanToken();
        }
        catch (const Report& err)
        {
            /* Add to error and scan next token */
            if (log_)
                log_->SumitReport(err);
        }
    }

    return nullptr;
}

//private
void Scanner::StoreStartPos()
{
    /* Store current source position as start position for the next token */
    nextStartPos_ = source_->Pos();
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
    throw Report(Report::Types::Error, "lexical error (" + Pos().ToString() + ") : " + msg);
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

void Scanner::IgnoreWhiteSpaces(bool includeNewLines)
{
    while ( std::isspace(UChr()) && ( includeNewLines || !IsNewLine() ) )
        TakeIt();
}

TokenPtr Scanner::ScanWhiteSpaces(bool includeNewLines)
{
    std::string spell;
    
    if (!includeNewLines)
    {
        /* Scan new-line characters */
        while (IsNewLine())
            spell += TakeIt();

        if (!spell.empty())
            return Make(Tokens::NewLines, spell);
    }

    /* Scan other white spaces */
    while ( std::isspace(UChr()) && ( includeNewLines || !IsNewLine() ) )
        spell += TakeIt();

    return Make(Tokens::WhiteSpaces, spell);
}

TokenPtr Scanner::ScanCommentLine(bool scanComments)
{
    if (scanComments)
    {
        std::string spell = "//";

        TakeIt(); // Ignore second '/' from commentary line beginning
        while (chr_ != '\n')
            spell += TakeIt();

        return Make(Tokens::Comment, spell);
    }
    else
    {
        Ignore([](char chr) { return chr != '\n'; });
        return nullptr;
    }
}

TokenPtr Scanner::ScanCommentBlock(bool scanComments)
{
    std::string spell = "/*";

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

    spell += "*/";

    return (scanComments ? Make(Tokens::Comment, spell) : nullptr);
}

TokenPtr Scanner::ScanStringLiteral()
{
    std::string spell;

    Take('\"');
    {
        while (!Is('\"'))
            spell += TakeIt();
    }
    Take('\"');

    return Make(Tokens::StringLiteral, spell);
}

TokenPtr Scanner::ScanNumber()
{
    if (!std::isdigit(UChr()))
        Error("expected digit");
    
    /* Take first number (literals like ".0" are not allowed) */
    std::string spell;

    const auto startChr = TakeIt();
    spell += startChr;

    /* Parse integer or floating-point number */
    auto type = Tokens::IntLiteral;

    ScanDecimalLiteral(spell);

    if (Is('.'))
    {
        spell += TakeIt();
        
        if (std::isdigit(UChr()))
            ScanDecimalLiteral(spell);
        else
            Error("floating-point literals must have a decimal on both sides of the dot (e.g. '0.0' but not '0.' or '.0')");

        type = Tokens::FloatLiteral;
    }

    if (Is('f') || Is('F'))
        TakeIt();

    if (std::isalpha(UChr()) || Is('.'))
        Error("letter '" + std::string(1, Chr()) + "' is not allowed within a number");

    /* Create number token */
    return Make(type, spell);
}

void Scanner::ScanDecimalLiteral(std::string& spell)
{
    while (std::isdigit(UChr()))
        spell += TakeIt();
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