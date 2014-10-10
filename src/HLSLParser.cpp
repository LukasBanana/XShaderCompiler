/*
 * HLSLParser.cpp
 * 
 * This file is part of the "HLSL Translator" (Copyright (c) 2014 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "HLSLParser.h"


namespace HTLib
{


HLSLParser::HLSLParser(Logger* log) :
    scanner_{ log },
    log_    { log }
{
}

bool HLSLParser::ParseSource(const std::shared_ptr<SourceCode>& source)
{
    if (!scanner_.ScanSource(source))
        return false;

    AcceptIt();

    try
    {
        #if 1//!!!

        if (log_)
        {
            while (tkn_ && tkn_->Type() != Token::Types::EndOfStream)
            {
                log_->Info(tkn_->Spell());
                AcceptIt();
            };
        }

        #endif
    }
    catch (const std::runtime_error& err)
    {
        /* Add to error and scan next token */
        if (log_)
            log_->Error(err.what());
    }

    return true;
}


/*
 * ======= Private: =======
 */

void HLSLParser::Error(const std::string& msg)
{
    throw std::runtime_error("syntax error (" + scanner_.Pos().ToString() + ") : " + msg);
}

void HLSLParser::ErrorUnexpected()
{
    Error("unexpected token '" + tkn_->Spell() + "'");
}

void HLSLParser::ErrorUnexpected(const std::string& hint)
{
    Error("unexpected token '" + tkn_->Spell() + "' (" + hint + ")");
}

TokenPtr HLSLParser::Accept(const Token::Types type)
{
    if (tkn_->Type() != type)
        ErrorUnexpected();
    return AcceptIt();
}

TokenPtr HLSLParser::AcceptIt()
{
    auto prevTkn = tkn_;
    tkn_ = scanner_.Next();
    return prevTkn;
}


} // /namespace HTLib



// ================================================================================