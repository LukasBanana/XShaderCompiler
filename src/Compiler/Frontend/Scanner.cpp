/*
 * Scanner.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "Scanner.h"
#include "Helper.h"
#include "ReportIdents.h"
#include <cctype>


namespace Xsc
{


Scanner::Scanner(Log* log) :
    log_{ log }
{
}

Scanner::~Scanner()
{
}

bool Scanner::ScanSource(const SourceCodePtr& source)
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

void Scanner::PushTokenString(const TokenPtrString& tokenString)
{
    tokenStringItStack_.push(tokenString.Begin());
}

void Scanner::PopTokenString()
{
    tokenStringItStack_.pop();
}

TokenPtrString::ConstIterator Scanner::TopTokenStringIterator() const
{
    return (!tokenStringItStack_.empty() ? tokenStringItStack_.top() : TokenPtrString::ConstIterator());
}

TokenPtr Scanner::ActiveToken() const
{
    return activeToken_;
}

TokenPtr Scanner::PreviousToken() const
{
    return prevToken_;
}


/*
 * ======= Protected: =======
 */

//private
TokenPtr Scanner::NextToken(bool scanComments, bool scanWhiteSpaces)
{
    TokenPtr tkn;
    
    /* Store previous token */
    prevToken_ = activeToken_;

    if (!tokenStringItStack_.empty() && !tokenStringItStack_.top().ReachedEnd())
    {
        /* Scan next token from token string */
        auto& tokenStringIt = tokenStringItStack_.top();
        tkn = *(tokenStringIt++);
    }
    else
    {
        /* Scan next token from token sub-scanner */
        tkn = NextTokenScan(scanComments, scanWhiteSpaces);
    }

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
            comment_.clear();
            commentFirstLine_ = true;
            bool hasComments = true;

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
                    commentStartPos_ = nextStartPos_.Column();

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
                    hasComments = false;
            }
            while (hasComments);

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

TokenPtr Scanner::Make(const Token::Types& type, bool takeChr)
{
    if (takeChr)
    {
        std::string spell;
        spell += TakeIt();
        return MakeShared<Token>(Pos(), type, std::move(spell));
    }
    return MakeShared<Token>(Pos(), type);
}

TokenPtr Scanner::Make(const Token::Types& type, std::string& spell, bool takeChr)
{
    if (takeChr)
        spell += TakeIt();
    return MakeShared<Token>(Pos(), type, std::move(spell));
}

TokenPtr Scanner::Make(const Token::Types& type, std::string& spell, const SourcePosition& pos, bool takeChr)
{
    if (takeChr)
        spell += TakeIt();
    return MakeShared<Token>(pos, type, std::move(spell));
}

/* ----- Report Handling ----- */

[[noreturn]]
void Scanner::Error(const std::string& msg)
{
    throw Report(Report::Types::Error, R_LexicalError() + " (" + Pos().ToString() + ") : " + msg);
}

[[noreturn]]
void Scanner::ErrorUnexpected()
{
    auto chr = TakeIt();
    Error(R_UnexpectedChar(std::string(1, chr)));
}

[[noreturn]]
void Scanner::ErrorUnexpected(char expectedChar)
{
    auto chr = TakeIt();
    Error(R_UnexpectedChar(std::string(1, chr), std::string(1, expectedChar)));
}

[[noreturn]]
void Scanner::ErrorUnexpectedEOS()
{
    Error(R_UnexpectedEndOfStream());
}

/* ----- Scanning ----- */

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
    std::string spell;

    TakeIt(); // Ignore second '/' from commentary line beginning

    while (!Is('\n'))
        spell += TakeIt();

    /* Store commentary string */
    AppendComment(spell);

    if (scanComments)
    {
        spell = "//" + spell;
        return Make(Tokens::Comment, spell);
    }
    
    return nullptr;
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
            else
                spell += '*';
        }
        else
            spell += TakeIt();
    }

    /* Store commentary string */
    AppendMultiLineComment(spell);

    if (scanComments)
    {
        spell = "/*" + spell + "*/";
        return Make(Tokens::Comment, spell);
    }

    return nullptr;
}

TokenPtr Scanner::ScanStringLiteral()
{
    std::string spell;

    spell += Take('\"');
    
    while (!Is('\"'))
    {
        if (Is(0))
            ErrorUnexpectedEOS();
        spell += TakeIt();
    }
    
    spell += Take('\"');

    return Make(Tokens::StringLiteral, spell);
}

TokenPtr Scanner::ScanCharLiteral()
{
    std::string spell;

    spell += Take('\'');
    
    while (!Is('\''))
    {
        if (Is(0))
            ErrorUnexpectedEOS();
        spell += TakeIt();
    }
    
    spell += Take('\'');

    return Make(Tokens::CharLiteral, spell);
}

// see https://msdn.microsoft.com/de-de/library/windows/desktop/bb509567(v=vs.85).aspx
TokenPtr Scanner::ScanNumber(bool startWithDot)
{
    std::string spell;

    /* Parse integer or floating-point number */
    auto type       = Tokens::IntLiteral;
    auto preDigits  = false;
    auto postDigits = false;

    if (!startWithDot)
        preDigits = ScanDigitSequence(spell);

    /* Check for exponent part (without fractional part) */
    if ( !startWithDot && ( Is('e') || Is('E') ) )
        startWithDot = true;

    /* Check for fractional part */
    if (startWithDot || Is('.'))
    {
        type = Tokens::FloatLiteral;

        /* Scan floating-point dot */
        if (startWithDot)
            spell += '.';
        else
            spell += TakeIt();
        
        /* Scan (optional) right hand side digit-sequence */
        postDigits = ScanDigitSequence(spell);

        if (!preDigits && !postDigits)
            Error("missing decimal part in floating-point number");

        /* Check for exponent-part */
        if (Is('e') || Is('E'))
        {
            spell += TakeIt();

            /* Check for sign */
            if (Is('-') || Is('+'))
                spell += TakeIt();

            /* Scan exponent digit sequence */
            if (!ScanDigitSequence(spell))
                Error(R_MissingDigitSequenceAfterExpr());
        }

        /* Check for floating-suffix */
        if (Is('f') || Is('F') || Is('h') || Is('H') || Is('l') || Is('L'))
            spell += TakeIt();
    }
    else
    {
        /* Check for hex numbers */
        if (spell == "0" && Is('x'))
        {
            spell += TakeIt();
            while ( std::isdigit(UChr()) || ( Chr() >= 'a' && Chr() <= 'f' ) || ( Chr() >= 'A' && Chr() <= 'F' ) )
                spell += TakeIt();
        }
        
        /* Check for integer-suffix */
        if (Is('u') || Is('U') || Is('l') || Is('L'))
            spell += TakeIt();
    }

    /* Create number token */
    return Make(type, spell);
}

TokenPtr Scanner::ScanNumberOrDot()
{
    std::string spell;

    spell += Take('.');

    if (Is('.'))
        return ScanVarArg(spell);
    if (std::isdigit(UChr()))
        return ScanNumber(true);

    return Make(Tokens::Dot, spell);
}

TokenPtr Scanner::ScanVarArg(std::string& spell)
{
    spell += Take('.');
    spell += Take('.');
    return Make(Tokens::VarArg, spell);
}

bool Scanner::ScanDigitSequence(std::string& spell)
{
    bool result = (std::isdigit(UChr()) != 0);

    while (std::isdigit(UChr()))
        spell += TakeIt();

    return result;
}


/*
 * ======= Private: =======
 */

void Scanner::AppendComment(const std::string& s)
{
    if (commentFirstLine_)
        commentFirstLine_ = false;
    else
        comment_ += '\n';

    if (commentStartPos_ > 0)
    {
        /* Append left-trimed commentary string */
        auto firstNot = s.find_first_not_of(" \t");
        if (firstNot == std::string::npos)
            comment_ += s;
        else
            comment_ += s.substr(std::min(firstNot, static_cast<std::size_t>(commentStartPos_ - 1)));
    }
    else
    {
        /* Append full commentary string */
        comment_ += s;
    }
}

void Scanner::AppendMultiLineComment(const std::string& s)
{
    std::size_t start = 0, end = 0;

    while (end < s.size())
    {
        /* Get next comment line */
        end = s.find('\n', start);

        AppendComment(end < s.size() ? s.substr(start, end - start) : s.substr(start));

        start = end + 1;
    }
}


} // /namespace Xsc



// ================================================================================
