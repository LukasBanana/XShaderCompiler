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
 * ======= Private: =======
 */

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


} // /namespace HTLib



// ================================================================================