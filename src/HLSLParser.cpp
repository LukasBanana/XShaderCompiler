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

    while (Type() != Tokens::EndOfStream)
        ast->globalDecls.push_back(ParseGlobalDecl());

    return ast;
}

CodeBlockPtr HLSLParser::ParseCodeBlock()
{
    auto ast = Make<CodeBlock>();

    /* Parse statement list */
    Accept(Tokens::LCurly);
    ast->stmnts = ParseStmntList();
    Accept(Tokens::RCurly);

    return ast;
}

BufferDeclIdentPtr HLSLParser::ParseBufferDeclIdent()
{
    //...
    return nullptr;
}

FunctionCallPtr HLSLParser::ParseFunctionCall()
{
    //...
    return nullptr;
}

StructurePtr HLSLParser::ParseStructure()
{
    auto ast = Make<Structure>();

    Accept(Tokens::Struct);

    ast->name = Accept(Tokens::Ident)->Spell();
    ast->members = ParseVarDeclList();

    return ast;
}

/* --- Global declarations --- */

GlobalDeclPtr HLSLParser::ParseGlobalDecl()
{
    switch (Type())
    {
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
    auto ast = Make<FunctionDecl>();

    AcceptIt();//!!!

    return ast;
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
    auto ast = Make<TextureDecl>();

    AcceptIt();//!!!

    return ast;
}

SamplerStateDeclPtr HLSLParser::ParseSamplerStateDecl()
{
    auto ast = Make<SamplerStateDecl>();

    AcceptIt();//!!!

    return ast;
}

StructDeclPtr HLSLParser::ParseStructDecl()
{
    auto ast = Make<StructDecl>();
    ast->structure = ParseStructure();
    Accept(Tokens::Semicolon);
    return ast;
}

DirectiveDeclPtr HLSLParser::ParseDirectiveDecl()
{
    /* Parse pre-processor directive line */
    auto ast = Make<DirectiveDecl>();
    ast->line = Accept(Tokens::Directive)->Spell();
    return ast;
}

/* --- Statements --- */

StmntPtr HLSLParser::ParseStmnt()
{
    #if 1//!!!
    while (Type() != Tokens::Semicolon)
        AcceptIt();
    #endif
    //...
    return nullptr;
}

VarDeclPtr HLSLParser::ParseVarDeclStmnt()
{
    auto ast = Make<VarDecl>();

    AcceptIt();//!!!
    //...

    return ast;
}

/* --- Expressions --- */

ExprPtr HLSLParser::ParseExpr()
{
    return nullptr;
}

/* --- Lists --- */

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

std::vector<StmntPtr> HLSLParser::ParseStmntList()
{
    std::vector<StmntPtr> stmnts;

    while (Type() != Tokens::RCurly)
        stmnts.push_back(ParseStmnt());

    return stmnts;
}

/* --- Others --- */

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


} // /namespace HTLib



// ================================================================================