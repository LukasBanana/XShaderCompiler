/*
 * PreProcessor.cpp
 * 
 * This file is part of the "HLSL Translator" (Copyright (c) 2014 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "PreProcessor.h"
#include <sstream>


namespace HTLib
{


PreProcessor::PreProcessor(IncludeHandler& includeHandler, Log* log) :
    includeHandler_ { includeHandler },
    log_            { log            },
    scanner_        { log            }
{
}

std::shared_ptr<std::iostream> PreProcessor::Process(const std::shared_ptr<SourceCode>& input)
{
    if (!scanner_.ScanSource(input))
        return nullptr;

    AcceptIt();

    output_ = std::make_shared<std::stringstream>();

    try
    {
        ParseProgram();

        auto output = output_->str();

        return output_;
    }
    catch (const std::exception& err)
    {
        if (log_)
            log_->Error(err.what());
    }

    return nullptr;
}


/*
 * ======= Private: =======
 */

void PreProcessor::Error(const std::string& msg)
{
    throw std::runtime_error("pre-processor error (" + scanner_.Pos().ToString() + ") : " + msg);
}

void PreProcessor::ErrorUnexpected()
{
    Error("unexpected token '" + tkn_->Spell() + "'");
}

void PreProcessor::ErrorUnexpected(const std::string& hint)
{
    Error("unexpected token '" + tkn_->Spell() + "' (" + hint + ")");
}

TokenPtr PreProcessor::Accept(const Token::Types type)
{
    if (tkn_->Type() != type)
        ErrorUnexpected();
    return AcceptIt();
}

TokenPtr PreProcessor::AcceptIt()
{
    auto prevTkn = tkn_;
    tkn_ = scanner_.Next();
    return prevTkn;
}

void PreProcessor::ParseProgram()
{
    while (!Is(Token::Types::EndOfStream))
    {
        switch (Type())
        {
            case Token::Types::Directive:
                ParseDirective();
                break;
            case Token::Types::Comment:
                ParesComment();
                break;
            default:
                ParseMisc();
                break;
        }
    }
}

void PreProcessor::ParesComment()
{
    *output_ << "/*" << Accept(Token::Types::Comment)->Spell() << "*/";
}

void PreProcessor::ParseMisc()
{
    *output_ << AcceptIt()->Spell();
}

void PreProcessor::ParseDirective()
{
    /* Parse pre-processor directive */
    auto directive = Accept(Token::Types::Directive)->Spell();

    if (directive == "define")
        ParseDirectiveDefine();
    else if (directive == "undef")
        ParseDirectiveUndef();
    else if (directive == "include")
        ParseDirectiveInclude();
    else if (directive == "if")
        ParseDirectiveIf();
    else if (directive == "ifdef")
        ParseDirectiveIfdef();
    else if (directive == "ifndef")
        ParseDirectiveIfndef();
    else if (directive == "pragma")
        ParseDirectivePragma();
}

// #define IDENT ( '(' IDENT+ ')' )? (TOKEN-STRING)?
void PreProcessor::ParseDirectiveDefine()
{
    if (Is(Token::Types::WhiteSpaces))
        AcceptIt();

    auto ident = Accept(Token::Types::Ident);

    //todo...
}

void PreProcessor::ParseDirectiveUndef()
{
    //todo...
}

void PreProcessor::ParseDirectiveInclude()
{
    //todo...
}

void PreProcessor::ParseDirectiveIf()
{
    //todo...
}

void PreProcessor::ParseDirectiveIfdef()
{
    //todo...
}

void PreProcessor::ParseDirectiveIfndef()
{
    //todo...
}

void PreProcessor::ParseDirectivePragma()
{
    //todo...
}


} // /namespace HTLib



// ================================================================================