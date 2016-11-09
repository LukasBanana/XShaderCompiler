/*
 * Parser.cpp
 * 
 * This file is part of the "HLSL Translator" (Copyright (c) 2014 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "Parser.h"


namespace Xsc
{


Parser::~Parser()
{
}


/*
 * ======= Protected: =======
 */

Parser::Parser(Log* log) :
    reportHandler_  { "syntax", log },
    log_            { log           }
{
}

void Parser::PushScannerSource(const SourceCodePtr& source, const std::string& filename)
{
    /* Add current token to previous scanner */
    if (!scannerStack_.empty())
        scannerStack_.top().nextToken = tkn_;

    /* Make a new token scanner */
    auto scanner = MakeScanner();
    if (!scanner)
        throw std::runtime_error("failed to create token scanner");

    scannerStack_.push({ scanner, filename, nullptr });

    /* Start scanning */
    if (!scanner->ScanSource(source))
        throw std::runtime_error("failed to scan source code");

    AcceptIt();
}

bool Parser::PopScannerSource()
{
    /* Get previous scanner */
    if (scannerStack_.empty())
        return false;

    scannerStack_.pop();

    if (scannerStack_.empty())
        return false;

    /* Reset previous 'next token' */
    tkn_ = scannerStack_.top().nextToken;

    return (tkn_ != nullptr);
}

Scanner& Parser::GetScanner()
{
    if (scannerStack_.empty())
        throw std::runtime_error("missing token scanner");
    return *(scannerStack_.top().scanner);
}

std::string Parser::GetCurrentFilename() const
{
    return (!scannerStack_.empty() ? scannerStack_.top().filename : "");
}

static SourceArea GetTokenArea(Token* tkn)
{
    return (tkn != nullptr ? tkn->Area() : SourceArea::ignore);
}

void Parser::Error(const std::string& msg, Token* tkn, const HLSLErr errorCode)
{
    reportHandler_.ErrorBreak(msg, GetScanner().Source(), GetTokenArea(tkn), errorCode);
}

void Parser::Error(const std::string& msg, bool prevToken, const HLSLErr errorCode)
{
    Error(msg, prevToken ? GetScanner().PreviousToken().get() : GetScanner().ActiveToken().get(), errorCode);
}

void Parser::ErrorUnexpected(const std::string& hint)
{
    std::string msg = "unexpected token: " + Token::TypeToString(tkn_->Type());

    if (!hint.empty())
        msg += " (" + hint + ")";

    Error(msg, false);
}

void Parser::ErrorUnexpected(const Tokens type)
{
    auto typeName = Token::TypeToString(type);
    if (typeName.empty())
        ErrorUnexpected();
    else
        ErrorUnexpected("expected: " + typeName);
}

void Parser::ErrorInternal(const std::string& msg, const std::string& procName)
{
    reportHandler_.ErrorBreak(msg + " (in function: " + procName + ")");
}

void Parser::Warning(const std::string& msg, Token* tkn)
{
    reportHandler_.Warning(msg, GetScanner().Source(), GetTokenArea(tkn));
}

void Parser::Warning(const std::string& msg, bool prevToken)
{
    Warning(msg, prevToken ? GetScanner().PreviousToken().get() : GetScanner().ActiveToken().get());
}

TokenPtr Parser::Accept(const Tokens type)
{
    if (tkn_->Type() != type)
        ErrorUnexpected(type);
    return AcceptIt();
}

TokenPtr Parser::Accept(const Tokens type, const std::string& spell)
{
    if (tkn_->Type() != type)
        ErrorUnexpected(type);
    if (tkn_->Spell() != spell)
        Error("unexpected token spelling '" + tkn_->Spell() + "' (expected '" + spell + "')");
    return AcceptIt();
}

TokenPtr Parser::AcceptIt()
{
    auto prevTkn = tkn_;
    tkn_ = GetScanner().Next();
    return prevTkn;
}

void Parser::PushTokenString(const TokenPtrString& tokenString)
{
    /* Push token string onto stack in the scanner and accept first token */
    GetScanner().PushTokenString(tokenString);
    AcceptIt();
}

void Parser::PopTokenString()
{
    /* Pop token string from the stack in the scanner */
    GetScanner().PopTokenString();
}

void Parser::IgnoreWhiteSpaces(bool includeNewLines)//, bool includeComments)
{
    while ( Is(Tokens::WhiteSpaces) || ( includeNewLines && Is(Tokens::NewLines) ) /*|| ( includeComments && Is(Tokens::Comment) )*/ )
        AcceptIt();
}

void Parser::IgnoreNewLines()
{
    while (Is(Tokens::NewLines))
        AcceptIt();
}


} // /namespace Xsc



// ================================================================================