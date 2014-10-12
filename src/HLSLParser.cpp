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
    EstablishMaps();
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

void HLSLParser::EstablishMaps()
{
    varModifierMap_ = std::map<std::string, VarModifiers>
    {
        { "extern",          VarModifiers::StorageModifier },
        { "nointerpolation", VarModifiers::StorageModifier },
        { "precise",         VarModifiers::StorageModifier },
        { "shared",          VarModifiers::StorageModifier },
        { "groupshared",     VarModifiers::StorageModifier },
        { "static",          VarModifiers::StorageModifier },
        { "uniform",         VarModifiers::StorageModifier },
        { "volatile",        VarModifiers::StorageModifier },
        { "linear",          VarModifiers::StorageModifier },
        { "centroid",        VarModifiers::StorageModifier },
        { "noperspective",   VarModifiers::StorageModifier },
        { "sample",          VarModifiers::StorageModifier },

        { "const",           VarModifiers::TypeModifier    },
        { "row_major",       VarModifiers::TypeModifier    },
        { "column_major",    VarModifiers::TypeModifier    },
    };
}

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

TokenPtr HLSLParser::Accept(const Tokens type)
{
    if (tkn_->Type() != type)
        ErrorUnexpected();
    return AcceptIt();
}

TokenPtr HLSLParser::Accept(const Tokens type, const std::string& spell)
{
    if (tkn_->Type() != type)
        ErrorUnexpected();
    if (tkn_->Spell() != spell)
        Error("unexpected token spelling '" + tkn_->Spell() + "' (expected '" + spell + "')");
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

    while (!Is(Tokens::EndOfStream))
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
    ast->members = ParseVarDeclStmntList();

    return ast;
}

VarDeclStmntPtr HLSLParser::ParseParameter()
{
    auto ast = Make<VarDeclStmnt>();

    /* Parse parameter as single variable declaration */
    if (Is(Tokens::InputModifier))
        ast->commonModifiers.push_back(AcceptIt()->Spell());

    ast->varType = ParseVarType();
    ast->varDecls.push_back(ParseVarDecl());

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

    /* Parse function header */
    ast->attribs = ParseAttributeList();
    ast->returnType = ParseVarType(true);
    ast->name = Accept(Tokens::Ident)->Spell();
    ast->parameters = ParseParameterList();
    
    if (Is(Tokens::Colon))
        ast->semantic = ParseSemantic();

    /* Parse function body */
    ast->codeBlock = ParseCodeBlock();

    return ast;
}

BufferDeclPtr HLSLParser::ParseBufferDecl()
{
    auto ast = Make<BufferDecl>();

    /* Parse buffer header */
    ast->bufferType = Accept(Tokens::Buffer)->Spell();
    ast->name = Accept(Tokens::Ident)->Spell();

    /* Parse optional register */
    if (Is(Tokens::Colon))
        ast->registerName = ParseRegister();

    /* Parse buffer body */
    ast->members = ParseVarDeclStmntList();

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

/* --- Variables --- */

FunctionCallPtr HLSLParser::ParseAttribute()
{
    auto ast = Make<FunctionCall>();

    Accept(Tokens::LParen);

    ast->name = Accept(Tokens::Ident)->Spell();

    if (Is(Tokens::LBracket))
    {
        AcceptIt();

        if (!Is(Tokens::RBracket))
        {
            while (true)
            {
                ast->arguments.push_back(ParseExpr());
                if (Is(Tokens::Comma))
                    AcceptIt();
                else
                    break;
            }
        }

        Accept(Tokens::RBracket);
    }

    Accept(Tokens::RParen);

    return ast;
}

PackOffsetPtr HLSLParser::ParsePackOffset(bool parseColon)
{
    auto ast = Make<PackOffset>();

    /* Parse ': packoffset( IDENT (.COMPONENT)? )' */
    if (parseColon)
        Accept(Tokens::Colon);
    
    Accept(Tokens::PackOffset);
    Accept(Tokens::LBracket);

    ast->registerName = Accept(Tokens::Ident)->Spell();

    if (Is(Tokens::Dot))
    {
        AcceptIt();
        ast->vectorComponent = Accept(Tokens::Ident)->Spell();
    }

    Accept(Tokens::RBracket);

    return ast;
}

ExprPtr HLSLParser::ParseArrayDimension()
{
    Accept(Tokens::LParen);
    auto ast = ParseExpr();
    Accept(Tokens::RParen);
    return ast;
}

ExprPtr HLSLParser::ParseInitializer()
{
    Accept(Tokens::AssignOp, "=");
    return ParseExpr();
}

VarSemanticPtr HLSLParser::ParseVarSemantic()
{
    auto ast = Make<VarSemantic>();

    Accept(Tokens::Colon);

    if (Is(Tokens::Register))
        ast->registerName = ParseRegister(false);
    else if (Is(Tokens::PackOffset))
        ast->packOffset = ParsePackOffset(false);
    else
        ast->semantic = Accept(Tokens::Ident)->Spell();

    return ast;
}

VarTypePtr HLSLParser::ParseVarType(bool parseVoidType)
{
    auto ast = Make<VarType>();

    if (Is(Tokens::Void))
    {
        if (parseVoidType)
            ast->baseType = AcceptIt()->Spell();
        else
            Error("'void' type not allowed in this context");
    }
    else if (Is(Tokens::Ident) || Is(Tokens::ScalarType) || Is(Tokens::VectorType) || Is(Tokens::MatrixType) || Is(Tokens::Texture) || Is(Tokens::SamplerState))
        ast->baseType = AcceptIt()->Spell();
    else if (Is(Tokens::Struct))
        ast->structType = ParseStructure();
    else
        ErrorUnexpected("expected type specifier");

    return ast;
}

VarDeclPtr HLSLParser::ParseVarDecl()
{
    auto ast = Make<VarDecl>();

    /* Parse variable declaration */
    ast->name = Accept(Tokens::Ident)->Spell();
    ast->arrayDims = ParseArrayDimensionList();
    ast->semantics = ParseVarSemanticList();

    if (Is(Tokens::AssignOp, "="))
        ast->initializer = ParseInitializer();

    return ast;
}

/* --- Statements --- */

StmntPtr HLSLParser::ParseStmnt()
{
    #if 1//!!!
    while (!Is(Tokens::Semicolon))
        AcceptIt();
    AcceptIt();
    #endif
    //...
    return nullptr;
}

VarDeclStmntPtr HLSLParser::ParseVarDeclStmnt()
{
    auto ast = Make<VarDeclStmnt>();

    while (true)
    {
        if (Is(Tokens::Ident))
        {
            /* Parse storage class, interpolation- or type modifiers */
            auto ident = AcceptIt()->Spell();

            auto it = varModifierMap_.find(ident);
            if (it == varModifierMap_.end())
            {
                /* Parse base variable type */
                ast->varType = Make<VarType>();
                ast->varType->baseType = ident;
                break;
            }
            else
                ast->commonModifiers.push_back(ident);
        }
        else if (Is(Tokens::Struct))
        {
            /* Parse structure variable type */
            ast->varType = Make<VarType>();
            ast->varType->structType = ParseStructure();
            break;
        }
        else if (Is(Tokens::ScalarType) || Is(Tokens::VectorType) || Is(Tokens::MatrixType) || Is(Tokens::Texture) || Is(Tokens::SamplerState))
        {
            /* Parse base variable type */
            ast->varType = Make<VarType>();
            ast->varType->baseType = AcceptIt()->Spell();
            break;
        }
        else
            ErrorUnexpected();
    }

    /* Parse variable declarations */
    ast->varDecls = ParseVarDeclList();
    Accept(Tokens::Semicolon);

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
    std::vector<VarDeclPtr> varDecls;

    /* Parse variable declaration list */
    while (true)
    {
        varDecls.push_back(ParseVarDecl());
        if (Is(Tokens::Comma))
            AcceptIt();
        else
            break;
    }

    return varDecls;
}

std::vector<VarDeclStmntPtr> HLSLParser::ParseVarDeclStmntList()
{
    std::vector<VarDeclStmntPtr> members;

    Accept(Tokens::LCurly);

    /* Parse all variable declaration statements */
    while (!Is(Tokens::RCurly))
        members.push_back(ParseVarDeclStmnt());

    AcceptIt();

    return members;
}

std::vector<VarDeclStmntPtr> HLSLParser::ParseParameterList()
{
    std::vector<VarDeclStmntPtr> parameters;

    Accept(Tokens::LBracket);

    /* Parse all variable declaration statements */
    if (!Is(Tokens::RBracket))
    {
        while (true)
        {
            parameters.push_back(ParseParameter());
            if (Is(Tokens::Comma))
                AcceptIt();
            else
                break;
        }
    }

    Accept(Tokens::RBracket);

    return parameters;
}

std::vector<StmntPtr> HLSLParser::ParseStmntList()
{
    std::vector<StmntPtr> stmnts;

    while (!Is(Tokens::RCurly))
        stmnts.push_back(ParseStmnt());

    return stmnts;
}

std::vector<ExprPtr> HLSLParser::ParseArrayDimensionList()
{
    std::vector<ExprPtr> arrayDims;

    while (Is(Tokens::LParen))
        arrayDims.push_back(ParseArrayDimension());

    return arrayDims;
}

std::vector<VarSemanticPtr> HLSLParser::ParseVarSemanticList()
{
    std::vector<VarSemanticPtr> semantics;

    while (Is(Tokens::Colon))
        semantics.push_back(ParseVarSemantic());

    return semantics;
}

std::vector<FunctionCallPtr> HLSLParser::ParseAttributeList()
{
    std::vector<FunctionCallPtr> attribs;

    while (Is(Tokens::LParen))
        attribs.push_back(ParseAttribute());

    return attribs;
}

/* --- Others --- */

std::string HLSLParser::ParseRegister(bool parseColon)
{
    /* Parse ': register(IDENT)' */
    if (parseColon)
        Accept(Tokens::Colon);
    
    Accept(Tokens::Register);
    Accept(Tokens::LBracket);

    auto registerName = Accept(Tokens::Ident)->Spell();

    Accept(Tokens::RBracket);

    return registerName;
}

std::string HLSLParser::ParseSemantic()
{
    Accept(Tokens::Colon);
    return Accept(Tokens::Ident)->Spell();
}


} // /namespace HTLib



// ================================================================================