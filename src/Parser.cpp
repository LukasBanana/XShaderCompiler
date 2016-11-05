/*
 * Parser.cpp
 * 
 * This file is part of the "HLSL Translator" (Copyright (c) 2014 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "Parser.h"


namespace HTLib
{


Parser::~Parser()
{
}


/*
 * ======= Protected: =======
 */

Parser::Parser(Log* log) :
    log_{ log }
{
}

void Parser::PushScannerSource(const std::shared_ptr<SourceCode>& source)
{
    /* Add current token to previous scanner */
    if (!scannerStack_.empty())
        scannerStack_.top().nextToken = tkn_;

    /* Make a new token scanner */
    auto scanner = MakeScanner();
    if (!scanner)
        throw std::runtime_error("failed to create token scanner");

    scannerStack_.push({ scanner, nullptr });

    /* Start scanning */
    if (!scanner->ScanSource(source))
        throw std::runtime_error("failed to scan source code");

    AcceptIt();
}

void Parser::PopScannerSource()
{
    /* Get previous scanner */
    if (scannerStack_.empty())
        throw std::runtime_error("failed to pop previous token scanner from stack");

    auto entry = scannerStack_.top();

    /* Reset previous 'next token' */
    tkn_ = entry.nextToken;

    /* Pop entry from stack */
    scannerStack_.pop();
}

void Parser::Error(const std::string& msg)
{
    throw std::runtime_error("syntax error (" + GetScanner().Pos().ToString() + ") : " + msg);
}

void Parser::ErrorUnexpected()
{
    Error("unexpected token '" + tkn_->Spell() + "'");
}

void Parser::ErrorUnexpected(const std::string& hint)
{
    Error("unexpected token '" + tkn_->Spell() + "' (" + hint + ")");
}

void Parser::Warning(const std::string& msg)
{
    if (log_)
        log_->Warning("warning (" + GetScanner().Pos().ToString() + ") : " + msg);
}

TokenPtr Parser::Accept(const Tokens type)
{
    if (tkn_->Type() != type)
        ErrorUnexpected();
    return AcceptIt();
}

TokenPtr Parser::Accept(const Tokens type, const std::string& spell)
{
    if (tkn_->Type() != type)
        ErrorUnexpected();
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

void Parser::IgnoreWhiteSpaces(bool includeNewLines)
{
    while ( Is(Tokens::WhiteSpaces) || ( includeNewLines && Is(Tokens::NewLines) ) )
        AcceptIt();
}

void Parser::IgnoreNewLines()
{
    while (Is(Tokens::NewLines))
        AcceptIt();
}

Scanner& Parser::GetScanner()
{
    if (scannerStack_.empty())
        throw std::runtime_error("missing token scanner");
    return *(scannerStack_.top().scanner);
}


} // /namespace HTLib



// ================================================================================