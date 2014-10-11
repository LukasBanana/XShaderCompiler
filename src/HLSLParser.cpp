/*
 * HLSLParser.cpp
 * 
 * This file is part of the "HLSL Translator" (Copyright (c) 2014 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "HLSLParser.h"
#include "HLSLTree.h"


namespace HTLib
{


HLSLParser::HLSLParser(Logger* log) :
    scanner_{ log },
    log_    { log }
{
}

ProgramPtr HLSLParser::ParseSource(const std::shared_ptr<SourceCode>& source)
{
    if (!scanner_.ScanSource(source))
        return false;

    AcceptIt();

    try
    {
        return ParseProgram();
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

/* ------- Parse functions ------- */

ProgramPtr HLSLParser::ParseProgram()
{
    auto ast = Make<Program>();

    while (true)
    {
        auto globDecl = ParseGlobalDecl();
        if (globDecl)
            ast->globalDecls.push_back(globDecl);
        else
            break;
    }

    return ast;
}

std::string HLSLParser::ParseRegister()
{
    /* Parse ': register(IDENT)' */
    Accept(Tokens::Colon);
    Accept(Tokens::Register);
    Accept(Tokens::LBracket);
    auto registerName = Accept(Tokens::Ident)->Spell();
    Accept(Tokens::RBracket);
    return registerName;
}

std::vector<VarDeclPtr> HLSLParser::ParseVarDeclList()
{
    std::vector<VarDeclPtr> members;

    Accept(Tokens::LCurly);

    /* Parse all var-decl statements */
    while (Type() != Tokens::RCurly)
        members.push_back(ParseVarDeclStmnt());

    AcceptIt();

    return members;
}

/* --- Global declarations --- */

GlobalDeclPtr HLSLParser::ParseGlobalDecl()
{
    switch (Type())
    {
        case Tokens::EndOfStream:
            return nullptr;
        case Tokens::Texture:
            return ParseTextureDecl();
        case Tokens::Buffer:
            return ParseBufferDecl();
        case Tokens::SamplerState:
            return ParseSamplerStateDecl();
        case Tokens::Struct:
            return ParseStructDecl();
        case Tokens::Directive:
            return ParseDirectiveDecl();
        default:
            return ParseFunctionDecl();
    }
    return nullptr;
}

FunctionDeclPtr HLSLParser::ParseFunctionDecl()
{
    return nullptr;
}

BufferDeclPtr HLSLParser::ParseBufferDecl()
{
    auto ast = Make<BufferDecl>();

    /* Parse buffer header */
    ast->bufferType = Accept(Tokens::Buffer)->Spell();
    ast->name = Accept(Tokens::Ident)->Spell();

    /* Parse optional register */
    if (Type() == Tokens::Colon)
        ast->registerName = ParseRegister();

    /* Parse buffer body */
    ast->members = ParseVarDeclList();

    Accept(Tokens::Semicolon);

    return ast;
}

TextureDeclPtr HLSLParser::ParseTextureDecl()
{
    return nullptr;
}

SamplerStateDeclPtr HLSLParser::ParseSamplerStateDecl()
{
    return nullptr;
}

StructDeclPtr HLSLParser::ParseStructDecl()
{
    return nullptr;
}

DirectiveDeclPtr HLSLParser::ParseDirectiveDecl()
{
    /* Parse pre-processor directive line */
    auto ast = Make<DirectiveDecl>();
    ast->line = Accept(Tokens::Directive)->Spell();
    return ast;
}

/* --- Statements --- */

VarDeclPtr HLSLParser::ParseVarDeclStmnt()
{
    auto ast = Make<VarDecl>();

    AcceptIt();//!!!
    //...

    return ast;
}


} // /namespace HTLib



// ================================================================================