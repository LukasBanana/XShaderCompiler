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

void Parser::PushScannerSource(const std::shared_ptr<SourceCode>& source, const std::string& filename)
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

std::string Parser::GetCurrentFilename() const
{
    return (!scannerStack_.empty() ? scannerStack_.top().filename : "");
}

void Parser::Error(const std::string& msg)
{
    throw Report(Report::Types::Error, "syntax error (" + GetScanner().Pos().ToString() + ") : " + msg);
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
        log_->SumitReport(Report(Report::Types::Warning, "warning (" + GetScanner().Pos().ToString() + ") : " + msg));
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

Scanner& Parser::GetScanner()
{
    if (scannerStack_.empty())
        throw std::runtime_error("missing token scanner");
    return *(scannerStack_.top().scanner);
}


} // /namespace HTLib



// ================================================================================