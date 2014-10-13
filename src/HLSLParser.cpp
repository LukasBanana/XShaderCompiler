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

bool HLSLParser::IsDataType() const
{
    return Is(Tokens::ScalarType) || Is(Tokens::VectorType) || Is(Tokens::MatrixType) || Is(Tokens::Texture) || Is(Tokens::SamplerState);
}

bool HLSLParser::IsLiteral() const
{
    return Is(Tokens::BoolLiteral) || Is(Tokens::IntLiteral) || Is(Tokens::FloatLiteral);
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

FunctionCallPtr HLSLParser::ParseFunctionCall(VarIdentPtr varIdent)
{
    auto ast = Make<FunctionCall>();

    /* Parse function name (as variable identifier) */
    if (!varIdent)
    {
        if (IsDataType())
        {
            varIdent = Make<VarIdent>();
            varIdent->ident = AcceptIt()->Spell();
        }
        else
            varIdent = ParseVarIdent();
    }

    ast->name = varIdent;

    /* Parse argument list */
    ast->arguments = ParseArgumentList();

    return ast;
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

    ast->name = Make<VarIdent>();
    ast->name->ident = Accept(Tokens::Ident)->Spell();

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

VarIdentPtr HLSLParser::ParseVarIdent()
{
    auto ast = Make<VarIdent>();

    /* Parse variable single identifier */
    ast->ident = Accept(Tokens::Ident)->Spell();
    ast->arrayIndices = ParseArrayDimensionList();
    
    if (Is(Tokens::Dot))
    {
        /* Parse next variable identifier */
        AcceptIt();
        ast->next = ParseVarIdent();
    }

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
    else if (Is(Tokens::Ident) || IsDataType())
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
    /* Determine which kind of statement the next one is */
    if (IsDataType())
        return ParseVarDeclStmnt();

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
        else if (IsDataType())
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
    /* Parse primary expression */
    auto ast = ParsePrimaryExpr();

    /* Parse optional post-unary expression */
    if (Is(Tokens::UnaryOp))
    {
        auto unaryExpr = Make<PostUnaryExpr>();
        unaryExpr->expr = ast;
        unaryExpr->op = AcceptIt()->Spell();
        ast = unaryExpr;
    }

    /* Parse optional binary expression */
    if (Is(Tokens::BinaryOp))
    {
        auto binExpr = Make<BinaryExpr>();

        binExpr->lhsExpr = ast;
        binExpr->op = AcceptIt()->Spell();
        binExpr->rhsExpr = ParseExpr();

        return binExpr;
    }

    return ast;
}

ExprPtr HLSLParser::ParsePrimaryExpr()
{
    /* Determine which kind of expression the next one is */
    if (IsLiteral())
        return ParseLiteralExpr();
    if (IsDataType())
        return ParseTypeNameOrFunctionCallExpr();
    if (Is(Tokens::UnaryOp) || Is(Tokens::BinaryOp, "-"))
        return ParseUnaryExpr();
    if (Is(Tokens::LBracket))
        return ParseBracketOrCastExpr();
    if (Is(Tokens::Ident))
        return ParseVarAccessOrFunctionCallExpr();

    ErrorUnexpected("expected primary expression");
    return nullptr;
}

LiteralExprPtr HLSLParser::ParseLiteralExpr()
{
    if (!IsLiteral())
        ErrorUnexpected("expected literal expression");

    /* Parse literal */
    auto ast = Make<LiteralExpr>();    
    ast->literal = AcceptIt()->Spell();
    return ast;
}

ExprPtr HLSLParser::ParseTypeNameOrFunctionCallExpr()
{
    /* Parse type name */
    if (!IsDataType())
        ErrorUnexpected("expected type name or function call expression");

    auto typeName = AcceptIt()->Spell();

    /* Determine which kind of expression this is */
    if (Is(Tokens::LBracket))
    {
        /* Return function call expression */
        auto varIdent = Make<VarIdent>();
        varIdent->ident = typeName;
        return ParseFunctionCallExpr(varIdent);
    }

    /* Return type name expression */
    auto ast = Make<TypeNameExpr>();
    ast->typeName = typeName;
    return ast;
}

UnaryExprPtr HLSLParser::ParseUnaryExpr()
{
    if (!Is(Tokens::UnaryOp) && !Is(Tokens::BinaryOp, "-"))
        ErrorUnexpected("expected unary expression operator");

    /* Parse unary expression */
    auto ast = Make<UnaryExpr>();
    ast->op = AcceptIt()->Spell();
    ast->expr = ParsePrimaryExpr();
    return ast;
}

ExprPtr HLSLParser::ParseBracketOrCastExpr()
{
    /* Parse expression inside the bracket */
    Accept(Tokens::LBracket);
    auto expr = ParseExpr();
    Accept(Tokens::RBracket);

    /*
    Parse cast expression the expression inside the bracket is a type name
    (single identifier (for a struct name) or a data type)
    !!!!!!!!!!!!!!!!!
    !TODO! -> This must be extended with the contextual analyzer,
    becauses expressions like "(x)" are no cast expression is "x" is a variable and not a structure!!!
    !!!!!!!!!!!!!!!!!
    */
    if (expr->Type() == AST::Types::TypeNameExpr || expr->Type() == AST::Types::VarAccessExpr)
    {
        /* Return cast expression */
        auto ast = Make<CastExpr>();
        ast->typeExpr = expr;
        ast->expr = ParsePrimaryExpr();
        return ast;
    }

    /* Return bracket expression */
    auto ast = Make<BracketExpr>();
    ast->expr = expr;
    return ast;
}

ExprPtr HLSLParser::ParseVarAccessOrFunctionCallExpr()
{
    /* Parse variable identifier first (for variables and functions) */
    auto varIdent = ParseVarIdent();
    if (Is(Tokens::LBracket))
        return ParseFunctionCallExpr(varIdent);
    return ParseVarAccessExpr(varIdent);
}

VarAccessExprPtr HLSLParser::ParseVarAccessExpr(VarIdentPtr varIdent)
{
    auto ast = Make<VarAccessExpr>();

    if (varIdent)
        ast->varIdent = varIdent;
    else
        ast->varIdent = ParseVarIdent();

    /* Parse optional assign expression */
    if (Is(Tokens::AssignOp))
    {
        ast->assignOp = AcceptIt()->Spell();
        ast->assignExpr = ParseExpr();
    }

    return ast;
}

FunctionCallExprPtr HLSLParser::ParseFunctionCallExpr(VarIdentPtr varIdent)
{
    /* Parse function call expression */
    auto ast = Make<FunctionCallExpr>();
    ast->call = ParseFunctionCall(varIdent);
    return ast;
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

std::vector<ExprPtr> HLSLParser::ParseArgumentList()
{
    std::vector<ExprPtr> arguments;

    Accept(Tokens::LBracket);

    /* Parse all argument expressions */
    if (!Is(Tokens::RBracket))
    {
        while (true)
        {
            arguments.push_back(ParseExpr());
            if (Is(Tokens::Comma))
                AcceptIt();
            else
                break;
        }
    }

    Accept(Tokens::RBracket);

    return arguments;
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