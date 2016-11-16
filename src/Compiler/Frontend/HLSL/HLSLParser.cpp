/*
 * HLSLParser.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "HLSLParser.h"
#include "HLSLKeywords.h"
#include "ConstExprEvaluator.h"
#include "AST.h"


namespace Xsc
{


HLSLParser::HLSLParser(Log* log) :
    Parser{ log }
{
}

ProgramPtr HLSLParser::ParseSource(const SourceCodePtr& source)
{
    PushScannerSource(source);

    try
    {
        auto ast = ParseProgram(source);
        return (GetReportHandler().HasErros() ? nullptr : ast);
    }
    catch (const Report& err)
    {
        if (GetLog())
            GetLog()->SumitReport(err);
    }

    return nullptr;
}


/*
 * ======= Private: =======
 */

ScannerPtr HLSLParser::MakeScanner()
{
    return std::make_shared<HLSLScanner>(GetLog());
}

void HLSLParser::Semi()
{
    Accept(Tokens::Semicolon);
}

bool HLSLParser::IsDataType() const
{
    return
    (
        IsBaseDataType() || Is(Tokens::Vector) || Is(Tokens::Matrix) || Is(Tokens::Texture) || Is(Tokens::Sampler)
    );
}

bool HLSLParser::IsBaseDataType() const
{
    return (Is(Tokens::ScalarType) || Is(Tokens::VectorType) || Is(Tokens::MatrixType) || Is(Tokens::StringType));
}

bool HLSLParser::IsLiteral() const
{
    return (Is(Tokens::BoolLiteral) || Is(Tokens::IntLiteral) || Is(Tokens::FloatLiteral) || Is(Tokens::StringLiteral));
}

bool HLSLParser::IsPrimaryExpr() const
{
    return
    (
        IsLiteral() || IsDataType() || IsArithmeticUnaryExpr() ||
        Is(Tokens::Struct) || Is(Tokens::UnaryOp) || Is(Tokens::LBracket) || Is(Tokens::LCurly) || Is(Tokens::Ident)
    );
}

bool HLSLParser::IsArithmeticUnaryExpr() const
{
    return (Is(Tokens::BinaryOp, "-") || Is(Tokens::BinaryOp, "+"));
}

/* ------- Parse functions ------- */

ProgramPtr HLSLParser::ParseProgram(const SourceCodePtr& source)
{
    auto ast = Make<Program>();

    /* Keep reference to preprocessed source code */
    ast->sourceCode = source;

    while (true)
    {
        /* Ignore all null statements */
        while (Is(Tokens::Semicolon))
            AcceptIt();

        /* Check if end of stream has been reached */
        if (Is(Tokens::EndOfStream))
            break;

        /* Parse next global declaration */
        ast->globalStmnts.push_back(ParseGlobalStmnt());
    }

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

    ast->varIdent = varIdent;

    /* Parse argument list */
    ast->arguments = ParseArgumentList();

    /* Update AST area */
    ast->area = ast->varIdent->area;

    return ast;
}

FunctionCallPtr HLSLParser::ParseFunctionCall(const TypeDenoterPtr& typeDenoter)
{
    auto ast = Make<FunctionCall>();

    /* Take type denoter */
    ast->typeDenoter = typeDenoter;

    /* Parse argument list */
    ast->arguments = ParseArgumentList();

    return UpdateSourceArea(ast);
}

VarDeclStmntPtr HLSLParser::ParseParameter()
{
    auto ast = Make<VarDeclStmnt>();

    /* Parse parameter as single variable declaration */
    while (Is(Tokens::InputModifier) || Is(Tokens::TypeModifier) || Is(Tokens::StorageModifier))
    {
        if (Is(Tokens::InputModifier))
            ast->inputModifier = AcceptIt()->Spell();
        else if (Is(Tokens::TypeModifier))
            ast->typeModifiers.push_back(AcceptIt()->Spell());
        else if (Is(Tokens::StorageModifier))
            ast->storageModifiers.push_back(AcceptIt()->Spell());
    }

    ast->varType = ParseVarType();
    ast->varDecls.push_back(ParseVarDecl(ast.get()));

    return ast;
}

SwitchCasePtr HLSLParser::ParseSwitchCase()
{
    auto ast = Make<SwitchCase>();

    /* Parse switch case header */
    if (Is(Tokens::Case))
    {
        Accept(Tokens::Case);
        ast->expr = ParseExpr();
    }
    else
        Accept(Tokens::Default);
    Accept(Tokens::Colon);

    /* Parse switch case statement list */
    while (!Is(Tokens::Case) && !Is(Tokens::Default) && !Is(Tokens::RCurly))
        ast->stmnts.push_back(ParseStmnt());

    return ast;
}

SamplerValuePtr HLSLParser::ParseSamplerValue()
{
    auto ast = Make<SamplerValue>();

    /* Parse state name */
    ast->name = ParseIdent();

    /* Parse value expression */
    Accept(Tokens::AssignOp, "=");
    ast->value = ParseExpr();
    Semi();

    return ast;
}

/* --- Variables --- */

AttributePtr HLSLParser::ParseAttribute()
{
    auto ast = Make<Attribute>();

    Accept(Tokens::LParen);

    ast->ident = ParseIdent();

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

    ast->registerName = ParseIdent();

    if (Is(Tokens::Dot))
    {
        AcceptIt();
        ast->vectorComponent = ParseIdent();
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
        ast->semantic = ParseIdent();

    return ast;
}

VarIdentPtr HLSLParser::ParseVarIdent()
{
    auto ast = Make<VarIdent>();

    /* Parse variable single identifier */
    ast->ident          = ParseIdent();
    ast->arrayIndices   = ParseArrayDimensionList();
    
    if (Is(Tokens::Dot))
    {
        /* Parse next variable identifier */
        AcceptIt();
        ast->next = ParseVarIdent();
    }

    return UpdateSourceArea(ast);
}

VarTypePtr HLSLParser::ParseVarType(bool parseVoidType)
{
    auto ast = Make<VarType>();

    /* Parse variable type denoter with optional struct declaration */
    ast->typeDenoter = ParseTypeDenoterWithStructDeclOpt(ast->structDecl);

    return UpdateSourceArea(ast);
}

VarDeclPtr HLSLParser::ParseVarDecl(VarDeclStmnt* declStmntRef, const TokenPtr& identTkn)
{
    auto ast = Make<VarDecl>();

    /* Store reference to parent node */
    ast->declStmntRef   = declStmntRef;

    /* Parse variable declaration */
    ast->ident          = (identTkn ? identTkn->Spell() : ParseIdent());
    ast->arrayDims      = ParseArrayDimensionList();
    ast->semantics      = ParseVarSemanticList();
    ast->annotations    = ParseAnnotationList();

    /* Parse optional initializer expression */
    if (Is(Tokens::AssignOp, "="))
        ast->initializer = ParseInitializer();

    /* Update source area */
    ast->area.Update(ast->ident);

    return ast;
}

TextureDeclPtr HLSLParser::ParseTextureDecl(TextureDeclStmnt* declStmntRef)
{
    auto ast = Make<TextureDecl>();

    /* Store reference to parent node */
    ast->declStmntRef = declStmntRef;

    /* Parse identifier and array dimension list (array dimension can be optional) */
    ast->ident      = ParseIdent();
    ast->arrayDims  = ParseArrayDimensionList();

    /* Parse register name (not allowed for local variables!) */
    if (Is(Tokens::Colon))
        ast->registerName = ParseRegister(true);

    return ast;
}

SamplerDeclPtr HLSLParser::ParseSamplerDecl()
{
    auto ast = Make<SamplerDecl>();

    /* Parse identifier and array dimension list (array dimension can be optional) */
    ast->ident      = ParseIdent();
    ast->arrayDims  = ParseArrayDimensionList();

    /* Parse register name (not allowed for local variables!) */
    if (Is(Tokens::Colon))
        ast->registerName = ParseRegister(true);

    /* Parse optional static sampler state */
    if (Is(Tokens::LCurly))
    {
        AcceptIt();
        ast->samplerValues = ParseSamplerValueList();
        Accept(Tokens::RCurly);
    }

    return ast;
}

StructDeclPtr HLSLParser::ParseStructDecl(bool parseStructTkn, const TokenPtr& identTkn)
{
    auto tkn = Tkn();
    auto ast = Make<StructDecl>();

    /* Parse structure declaration */
    if (parseStructTkn)
        Accept(Tokens::Struct);

    if (Is(Tokens::Ident) || identTkn)
    {
        /* Parse structure name */
        tkn = Tkn();
        ast->ident = (identTkn ? identTkn->Spell() : ParseIdent());

        /* Parse optional inheritance (not documented in HLSL but supported; only single inheritance) */
        if (Is(Tokens::Colon))
        {
            AcceptIt();

            ast->baseStructName = ParseIdent();
            if (ast->baseStructName == ast->ident)
                Error("recursive inheritance is not allowed");

            if (Is(Tokens::Comma))
                Error("multiple inheritance is not allowed", false);
        }
    }

    GetReportHandler().PushContextDesc("struct " + ast->SignatureToString());
    {
        /* Parse member variable declarations */
        auto members = ParseVarDeclStmntList();
        ast->members.insert(ast->members.end(), members.begin(), members.end());
    }
    GetReportHandler().PopContextDesc();

    return ast;
}

AliasDeclPtr HLSLParser::ParseAliasDecl(TypeDenoterPtr typeDenoter)
{
    auto ast = Make<AliasDecl>();

    /* Parse alias identifier */
    auto identTkn = Tkn();
    ast->ident = ParseIdent();

    /* Parse optional array dimensions */
    if (Is(Tokens::LParen))
    {
        /* Make array type denoter and use input as base type denoter */
        auto arrayTypeDenoter = std::make_shared<ArrayTypeDenoter>();
        {
            arrayTypeDenoter->arrayDims         = ParseArrayDimensionList();
            arrayTypeDenoter->baseTypeDenoter   = typeDenoter;
        }
        typeDenoter = arrayTypeDenoter;
    }

    /* Store final type denoter in alias declaration */
    ast->typeDenoter = typeDenoter;

    return ast;
}

/* --- Declaration statements --- */

StmntPtr HLSLParser::ParseGlobalStmnt()
{
    switch (TknType())
    {
        case Tokens::Sampler:
            return ParseSamplerDeclStmnt();
        case Tokens::Texture:
            return ParseTextureDeclStmnt();
        case Tokens::UniformBuffer:
            return ParseBufferDeclStmnt();
        case Tokens::Typedef:
            return ParseAliasDeclStmnt();
        case Tokens::TypeModifier:
        case Tokens::StorageModifier:
            return ParseVarDeclStmnt();
        case Tokens::LParen:
        case Tokens::Void:
            return ParseFunctionDecl();
        default:
            return ParseStructDeclOrVarDeclOrFunctionDeclStmnt();
    }
}

StmntPtr HLSLParser::ParseStructDeclOrVarDeclOrFunctionDeclStmnt()
{
    auto varType = ParseVarType();

    if (varType->structDecl && Is(Tokens::Semicolon))
    {
        auto ast = Make<StructDeclStmnt>();

        ast->structDecl = varType->structDecl;
        Semi();

        return ast;
    }

    auto identTkn = Accept(Tokens::Ident);

    if (Is(Tokens::LBracket))
    {
        /* Parse function declaration statement */
        return ParseFunctionDecl(varType, identTkn);
    }
    else
    {
        /* Parse variable declaration statement */
        auto ast = Make<VarDeclStmnt>();

        ast->varType    = varType;
        ast->varDecls   = ParseVarDeclList(ast.get(), identTkn);

        Semi();

        return ast;
    }
}

FunctionDeclPtr HLSLParser::ParseFunctionDecl(const VarTypePtr& returnType, const TokenPtr& identTkn)
{
    auto ast = Make<FunctionDecl>();

    /* Parse function attributes and return type */
    if (returnType)
        ast->returnType = returnType;
    else
    {
        ast->attribs    = ParseAttributeList();
        ast->returnType = ParseVarType(true);
    }

    /* Parse function identifier */
    if (identTkn)
    {
        ast->area   = identTkn->Area();
        ast->ident  = identTkn->Spell();
    }
    else
    {
        ast->area   = GetScanner().ActiveToken()->Area();
        ast->ident  = ParseIdent();
    }

    /* Parse parameters */
    ast->parameters = ParseParameterList();
    
    if (Is(Tokens::Colon))
        ast->semantic = ParseSemantic();

    ast->annotations = ParseAnnotationList();

    /* Parse optional function body */
    if (Is(Tokens::Semicolon))
        AcceptIt();
    else
    {
        GetReportHandler().PushContextDesc(ast->SignatureToString(false));
        {
            localScope_ = true;
            ast->codeBlock = ParseCodeBlock();
            localScope_ = false;
        }
        GetReportHandler().PopContextDesc();
    }

    return ast;
}

BufferDeclStmntPtr HLSLParser::ParseBufferDeclStmnt()
{
    auto ast = Make<BufferDeclStmnt>();

    /* Parse buffer header */
    ast->bufferType = Accept(Tokens::UniformBuffer)->Spell();
    ast->ident      = ParseIdent();

    /* Parse optional register */
    if (Is(Tokens::Colon))
        ast->registerName = ParseRegister();

    GetReportHandler().PushContextDesc(ast->bufferType + " " + ast->ident);
    {
        /* Parse buffer body */
        ast->members = ParseVarDeclStmntList();

        /* Parse optional semicolon (this seems to be optional for cbuffer, and tbuffer) */
        if (Is(Tokens::Semicolon))
            Semi();
    }
    GetReportHandler().PopContextDesc();

    return ast;
}

TextureDeclStmntPtr HLSLParser::ParseTextureDeclStmnt()
{
    auto ast = Make<TextureDeclStmnt>();

    ast->textureType = Accept(Tokens::Texture)->Spell();

    /* Parse optional generic color type ('<' colorType '>') */
    if (Is(Tokens::BinaryOp, "<"))
    {
        AcceptIt();

        if (Is(Tokens::ScalarType) || Is(Tokens::VectorType))
            ast->colorType = AcceptIt()->Spell();
        else
            ErrorUnexpected("expected scalar or vector type denoter");

        Accept(Tokens::BinaryOp, ">");
    }

    ast->textureDecls = ParseTextureDeclList(ast.get());

    Semi();

    return ast;
}

SamplerDeclStmntPtr HLSLParser::ParseSamplerDeclStmnt()
{
    auto ast = Make<SamplerDeclStmnt>();

    ast->samplerType    = Accept(Tokens::Sampler)->Spell();
    ast->samplerDecls   = ParseSamplerDeclList();

    Semi();

    return ast;
}

/*StructDeclStmntPtr HLSLParser::ParseStructDeclStmnt()
{
    auto ast = Make<StructDeclStmnt>();
    
    ast->structDecl = ParseStructDecl();
    Semi();

    return ast;
}*/

VarDeclStmntPtr HLSLParser::ParseVarDeclStmnt()
{
    auto ast = Make<VarDeclStmnt>();

    while (true)
    {
        if (Is(Tokens::StorageModifier))
        {
            /* Parse storage modifiers */
            auto modifier = AcceptIt()->Spell();
            ast->storageModifiers.push_back(modifier);
        }
        else if (Is(Tokens::TypeModifier))
        {
            /* Parse type modifier (const, row_major, column_major) */
            auto modifier = AcceptIt()->Spell();
            ast->typeModifiers.push_back(modifier);
        }
        else if (Is(Tokens::Ident) || IsDataType())
        {
            /* Parse type denoter */
            ast->varType = Make<VarType>();
            ast->varType->typeDenoter = ParseTypeDenoter();
            UpdateSourceArea(ast->varType);
            break;
        }
        else if (Is(Tokens::Struct))
        {
            /* Parse structure variable type */
            ast->varType = MakeVarType(ParseStructDecl());
            break;
        }
        else
            ErrorUnexpected();
    }

    /* Parse variable declarations */
    ast->varDecls = ParseVarDeclList(ast.get());
    Semi();

    return UpdateSourceArea(ast);
}

// 'typedef' type_denoter IDENT;
AliasDeclStmntPtr HLSLParser::ParseAliasDeclStmnt()
{
    auto ast = Make<AliasDeclStmnt>();

    /* Parse type alias declaration */
    Accept(Tokens::Typedef);

    /* Parse type denoter with optional struct declaration */
    auto typeDenoter = ParseTypeDenoterWithStructDeclOpt(ast->structDecl);

    /* Parse type aliases */
    ast->aliasDecls = ParseAliasDeclList(typeDenoter);

    Semi();

    /* Store references in decls to this statement */
    for (auto& decl : ast->aliasDecls)
        decl->declStmntRef = ast.get();

    return ast;
}

/* --- Statements --- */

StmntPtr HLSLParser::ParseStmnt()
{
    /* Parse optional attributes */
    std::vector<AttributePtr> attribs;
    if (Is(Tokens::LParen))
        attribs = ParseAttributeList();

    /* Determine which kind of statement the next one is */
    switch (TknType())
    {
        case Tokens::Semicolon:
            return ParseNullStmnt();
        case Tokens::LCurly:
            return ParseCodeBlockStmnt();
        case Tokens::Return:
            return ParseReturnStmnt();
        case Tokens::Ident:
            return ParseVarDeclOrAssignOrFunctionCallStmnt();
        case Tokens::For:
            return ParseForLoopStmnt(attribs);
        case Tokens::While:
            return ParseWhileLoopStmnt(attribs);
        case Tokens::Do:
            return ParseDoWhileLoopStmnt(attribs);
        case Tokens::If:
            return ParseIfStmnt(attribs);
        case Tokens::Switch:
            return ParseSwitchStmnt(attribs);
        case Tokens::CtrlTransfer:
            return ParseCtrlTransferStmnt();
        case Tokens::Struct:
            return ParseStructDeclOrVarDeclStmnt();
        case Tokens::Typedef:
            return ParseAliasDeclStmnt();
        case Tokens::Sampler:
            return ParseSamplerDeclStmnt();
        case Tokens::TypeModifier:
        case Tokens::StorageModifier:
            return ParseVarDeclStmnt();
        default:
            break;
    }

    if (IsDataType())
        return ParseVarDeclStmnt();

    /* Parse statement of arbitrary expression */
    return ParseExprStmnt();
}

NullStmntPtr HLSLParser::ParseNullStmnt()
{
    /* Parse null statement */
    auto ast = Make<NullStmnt>();
    Semi();
    return ast;
}

CodeBlockStmntPtr HLSLParser::ParseCodeBlockStmnt()
{
    /* Parse code block statement */
    auto ast = Make<CodeBlockStmnt>();
    ast->codeBlock = ParseCodeBlock();
    return ast;
}

ForLoopStmntPtr HLSLParser::ParseForLoopStmnt(const std::vector<AttributePtr>& attribs)
{
    auto ast = Make<ForLoopStmnt>();
    ast->attribs = attribs;

    /* Parse loop init */
    Accept(Tokens::For);
    Accept(Tokens::LBracket);

    ast->initSmnt = ParseStmnt();

    /* Parse loop condExpr */
    if (!Is(Tokens::Semicolon))
        ast->condition = ParseExpr(true);
    Semi();

    /* Parse loop iteration */
    if (!Is(Tokens::RBracket))
        ast->iteration = ParseExpr(true);
    Accept(Tokens::RBracket);

    /* Parse loop body */
    ast->bodyStmnt = ParseStmnt();

    return ast;
}

WhileLoopStmntPtr HLSLParser::ParseWhileLoopStmnt(const std::vector<AttributePtr>& attribs)
{
    auto ast = Make<WhileLoopStmnt>();
    ast->attribs = attribs;

    /* Parse loop condExpr */
    Accept(Tokens::While);

    Accept(Tokens::LBracket);
    ast->condition = ParseExpr(true);
    Accept(Tokens::RBracket);

    /* Parse loop body */
    ast->bodyStmnt = ParseStmnt();

    return ast;
}

DoWhileLoopStmntPtr HLSLParser::ParseDoWhileLoopStmnt(const std::vector<AttributePtr>& attribs)
{
    auto ast = Make<DoWhileLoopStmnt>();
    ast->attribs = attribs;

    /* Parse loop body */
    Accept(Tokens::Do);
    ast->bodyStmnt = ParseStmnt();

    /* Parse loop condExpr */
    Accept(Tokens::While);

    Accept(Tokens::LBracket);
    ast->condition = ParseExpr(true);
    Accept(Tokens::RBracket);

    Semi();

    return ast;
}

IfStmntPtr HLSLParser::ParseIfStmnt(const std::vector<AttributePtr>& attribs)
{
    auto ast = Make<IfStmnt>();
    ast->attribs = attribs;

    /* Parse if condExpr */
    Accept(Tokens::If);

    Accept(Tokens::LBracket);
    ast->condition = ParseExpr(true);
    Accept(Tokens::RBracket);

    /* Parse if body */
    ast->bodyStmnt = ParseStmnt();

    /* Parse optional else statement */
    if (Is(Tokens::Else))
        ast->elseStmnt = ParseElseStmnt();

    return ast;
}

ElseStmntPtr HLSLParser::ParseElseStmnt()
{
    /* Parse else statment */
    auto ast = Make<ElseStmnt>();

    Accept(Tokens::Else);
    ast->bodyStmnt = ParseStmnt();

    return ast;
}

SwitchStmntPtr HLSLParser::ParseSwitchStmnt(const std::vector<AttributePtr>& attribs)
{
    auto ast = Make<SwitchStmnt>();
    ast->attribs = attribs;

    /* Parse switch selector */
    Accept(Tokens::Switch);

    Accept(Tokens::LBracket);
    ast->selector = ParseExpr(true);
    Accept(Tokens::RBracket);

    /* Parse switch cases */
    Accept(Tokens::LCurly);
    ast->cases = ParseSwitchCaseList();
    Accept(Tokens::RCurly);

    return ast;
}

CtrlTransferStmntPtr HLSLParser::ParseCtrlTransferStmnt()
{
    /* Parse control transfer statement */
    auto ast = Make<CtrlTransferStmnt>();
    
    auto ctrlTransfer = Accept(Tokens::CtrlTransfer)->Spell();
    ast->transfer = StringToCtrlTransfer(ctrlTransfer);

    Semi();

    return ast;
}

ReturnStmntPtr HLSLParser::ParseReturnStmnt()
{
    auto ast = Make<ReturnStmnt>();

    Accept(Tokens::Return);

    if (!Is(Tokens::Semicolon))
        ast->expr = ParseExpr(true);

    Semi();

    return ast;
}

ExprStmntPtr HLSLParser::ParseExprStmnt(const VarIdentPtr& varIdent)
{
    /* Parse expression statement */
    auto ast = Make<ExprStmnt>();

    if (varIdent)
    {
        /* Make var-ident to a var-access expression */
        auto expr = Make<VarAccessExpr>();
        expr->varIdent = varIdent;
        ast->expr = ParseExpr(true, expr);
    }
    else
        ast->expr = ParseExpr(true);

    Semi();

    return ast;
}

StmntPtr HLSLParser::ParseStructDeclOrVarDeclStmnt()
{
    /* Parse structure declaration statement */
    auto ast = Make<StructDeclStmnt>();
    
    ast->structDecl = ParseStructDecl();

    if (!Is(Tokens::Semicolon))
    {
        /* Parse variable declaration with previous structure type */
        auto varDeclStmnt = Make<VarDeclStmnt>();

        varDeclStmnt->varType = MakeVarType(ast->structDecl);
        
        /* Parse variable declarations */
        varDeclStmnt->varDecls = ParseVarDeclList(varDeclStmnt.get());
        Semi();

        return varDeclStmnt;
    }
    else
        Semi();

    return ast;
}

StmntPtr HLSLParser::ParseVarDeclOrAssignOrFunctionCallStmnt()
{
    /*
    Parse variable identifier first [ ident ( '.' ident )* ],
    then check if only a single identifier is required
    */
    auto varIdent = ParseVarIdent();
    
    if (Is(Tokens::LBracket))
    {
        /* Parse function call as expression statement */
        auto ast = Make<ExprStmnt>();
        
        ast->expr = ParseFunctionCallExpr(varIdent);
        Semi();

        return ast;
    }
    else if (Is(Tokens::AssignOp))
    {
        /* Parse assignment statement */
        auto ast = Make<AssignStmnt>();
        
        ast->varIdent   = varIdent;
        ast->op         = StringToAssignOp(AcceptIt()->Spell());
        ast->expr       = ParseExpr(true);
        Semi();

        return ast;
    }
    else if (Is(Tokens::UnaryOp, "++") || Is(Tokens::UnaryOp, "--"))
    {
        /* Parse expression statement */
        return ParseExprStmnt(varIdent);
    }

    if (!varIdent->next)
    {
        /* Parse variable declaration statement */
        auto ast = Make<VarDeclStmnt>();

        ast->varType = Make<VarType>();
        ast->varType->typeDenoter = ParseAliasTypeDenoter(varIdent->ident);
        ast->varDecls = ParseVarDeclList(ast.get());
        Semi();

        return ast;
    }

    ErrorUnexpected("expected variable declaration, assignment or function call statement");
    
    return nullptr;
}

/* --- Expressions --- */

ExprPtr HLSLParser::ParseExpr(bool allowComma, const ExprPtr& initExpr)
{
    /* Parse primary expression */
    ExprPtr ast = (initExpr ? initExpr : ParseGenericExpr());

    /* Parse optional post-unary expression (e.g. 'x++', 'x--') */
    if (Is(Tokens::UnaryOp))
    {
        auto unaryExpr = Make<PostUnaryExpr>();
        unaryExpr->expr = ast;
        unaryExpr->op = StringToUnaryOp(AcceptIt()->Spell());
        ast = unaryExpr;
    }

    /* Parse optional list expression */
    if (allowComma && Is(Tokens::Comma))
    {
        AcceptIt();

        auto listExpr = Make<ListExpr>();
        listExpr->firstExpr = ast;
        listExpr->nextExpr = ParseExpr(true);

        return listExpr;
    }

    return ast;
}

ExprPtr HLSLParser::ParsePrimaryExpr()
{
    /* Determine which kind of expression the next one is */
    if (IsLiteral())
        return ParseLiteralExpr();
    if (IsDataType() || Is(Tokens::Struct))
        return ParseTypeNameOrFunctionCallExpr();
    if (Is(Tokens::UnaryOp) || IsArithmeticUnaryExpr())
        return ParseUnaryExpr();
    if (Is(Tokens::LBracket))
        return ParseBracketOrCastExpr();
    if (Is(Tokens::LCurly))
        return ParseInitializerExpr();
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

    ast->type   = TknType();
    ast->value  = AcceptIt()->Spell();

    return UpdateSourceArea(ast);
}

ExprPtr HLSLParser::ParseTypeNameOrFunctionCallExpr()
{
    /* Parse type name */
    if (!IsDataType() && !Is(Tokens::Struct))
        ErrorUnexpected("expected type name or function call expression");

    auto typeDenoter = ParseTypeDenoter();

    /* Determine which kind of expression this is */
    if (Is(Tokens::LBracket))
    {
        /* Return function call expression */
        return ParseFunctionCallExpr(nullptr, typeDenoter);
    }

    /* Return type name expression */
    auto ast = Make<TypeNameExpr>();
    ast->typeDenoter = typeDenoter;

    return ast;
}

UnaryExprPtr HLSLParser::ParseUnaryExpr()
{
    if (!Is(Tokens::UnaryOp) && !IsArithmeticUnaryExpr())
        ErrorUnexpected("expected unary expression operator");

    /* Parse unary expression */
    auto ast = Make<UnaryExpr>();

    ast->op     = StringToUnaryOp(AcceptIt()->Spell());
    ast->expr   = ParsePrimaryExpr();

    return ast;
}

bool HLSLParser::IsLhsOfCastExpr(const ExprPtr& expr) const
{
    /* Type name (float, int3 etc.) is always allowed for a cast expression */
    return (expr->Type() == AST::Types::TypeNameExpr || expr->Type() == AST::Types::VarAccessExpr);
}

VarTypePtr HLSLParser::MakeVarType(const StructDeclPtr& structDecl)
{
    auto ast = Make<VarType>();

    ast->structDecl     = structDecl;
    ast->typeDenoter    = std::make_shared<StructTypeDenoter>(structDecl.get());

    return ast;
}

/* === Parse functions === */

ExprPtr HLSLParser::ParseBracketOrCastExpr()
{
    ExprPtr expr;
    SourceArea area(GetScanner().Pos(), 1);

    /* Parse expression inside the bracket */
    Accept(Tokens::LBracket);
    {
        if (ActiveParsingState().activeTemplate)
        {
            /* Inside brackets, '<' and '>' are allowed as binary operators (albeit an active template is being parsed) */
            auto parsingState = ActiveParsingState();
            parsingState.activeTemplate = false;
            PushParsingState(parsingState);
            {
                expr = ParseExpr(true);
            }
            PopParsingState();
        }
        else
            expr = ParseExpr(true);
    }
    Accept(Tokens::RBracket);

    /*
    Parse cast expression if the expression inside the bracket is the left-hand-side of a cast expression,
    and if the next token belongs to a primary expression.
    */
    if (IsLhsOfCastExpr(expr) && IsPrimaryExpr())
    {
        /* Return cast expression */
        auto ast = Make<CastExpr>();
        
        ast->area       = area;
        ast->typeExpr   = expr;
        ast->expr       = ParsePrimaryExpr();

        return UpdateSourceArea(ast);
    }

    /* Return bracket expression */
    auto ast = Make<BracketExpr>();

    ast->area = area;
    ast->expr = expr;

    expr = ast;

    /* Parse optional array-access expression */
    if (Is(Tokens::LParen))
        expr = ParseArrayAccessExpr(expr);

    /* Parse optional suffix expression */
    if (Is(Tokens::Dot))
        expr = ParseSuffixExpr(expr);

    return UpdateSourceArea(expr);
}

SuffixExprPtr HLSLParser::ParseSuffixExpr(const ExprPtr& expr)
{
    auto ast = Make<SuffixExpr>();

    /* Take sub expression */
    ast->expr = expr;

    /* Parse suffix after dot */
    Accept(Tokens::Dot);
    ast->varIdent = ParseVarIdent();

    return UpdateSourceArea(ast, expr.get());
}

ArrayAccessExprPtr HLSLParser::ParseArrayAccessExpr(const ExprPtr& expr)
{
    auto ast = Make<ArrayAccessExpr>();

    /* Take sub expression and parse array dimensions */
    ast->expr           = expr;
    ast->arrayIndices   = ParseArrayDimensionList();

    return UpdateSourceArea(ast, expr.get());
}

ExprPtr HLSLParser::ParseVarAccessOrFunctionCallExpr()
{
    /* Parse variable identifier first (for variables and functions) */
    auto varIdent = ParseVarIdent();
    if (Is(Tokens::LBracket))
        return ParseFunctionCallExpr(varIdent);
    return ParseVarAccessExpr(varIdent);
}

VarAccessExprPtr HLSLParser::ParseVarAccessExpr(const VarIdentPtr& varIdent)
{
    auto ast = Make<VarAccessExpr>();

    if (varIdent)
        ast->varIdent = varIdent;
    else
        ast->varIdent = ParseVarIdent();

    ast->area = ast->varIdent->area;

    /* Parse optional assign expression */
    if (Is(Tokens::AssignOp))
    {
        ast->assignOp   = StringToAssignOp(AcceptIt()->Spell());
        ast->assignExpr = ParseExpr();
    }

    return UpdateSourceArea(ast);
}

ExprPtr HLSLParser::ParseFunctionCallExpr(const VarIdentPtr& varIdent, const TypeDenoterPtr& typeDenoter)
{
    /* Parse function call expression */
    auto ast = Make<FunctionCallExpr>();

    if (typeDenoter)
        ast->call = ParseFunctionCall(typeDenoter);
    else
        ast->call = ParseFunctionCall(varIdent);

    /* Update source area */
    UpdateSourceArea(ast, ast->call.get());

    /* Parse optional array-access expression */
    ExprPtr expr = ast;

    if (Is(Tokens::LParen))
        expr = ParseArrayAccessExpr(expr);

    /* Parse optional suffix expression */
    if (Is(Tokens::Dot))
        expr = ParseSuffixExpr(expr);

    return expr;
}

InitializerExprPtr HLSLParser::ParseInitializerExpr()
{
    /* Parse initializer list expression */
    auto ast = Make<InitializerExpr>();
    ast->exprs = ParseInitializerList();
    return UpdateSourceArea(ast);
}

/* --- Lists --- */

std::vector<VarDeclPtr> HLSLParser::ParseVarDeclList(VarDeclStmnt* declStmntRef, TokenPtr firstIdentTkn)
{
    std::vector<VarDeclPtr> varDecls;

    /* Parse variable declaration list */
    while (true)
    {
        varDecls.push_back(ParseVarDecl(declStmntRef, firstIdentTkn));
        firstIdentTkn = nullptr;
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

std::vector<VarDeclStmntPtr> HLSLParser::ParseAnnotationList()
{
    std::vector<VarDeclStmntPtr> annotations;

    if (Is(Tokens::BinaryOp, "<"))
    {
        AcceptIt();

        while (!Is(Tokens::BinaryOp, ">"))
            annotations.push_back(ParseVarDeclStmnt());
        
        AcceptIt();
    }

    return annotations;
}

std::vector<StmntPtr> HLSLParser::ParseStmntList()
{
    std::vector<StmntPtr> stmnts;

    while (!Is(Tokens::RCurly))
        stmnts.push_back(ParseStmnt());

    return stmnts;
}

std::vector<ExprPtr> HLSLParser::ParseExprList(const Tokens listTerminatorToken, bool allowLastComma)
{
    std::vector<ExprPtr> exprs;

    /* Parse all argument expressions */
    if (!Is(listTerminatorToken))
    {
        while (true)
        {
            exprs.push_back(ParseExpr());
            if (Is(Tokens::Comma))
            {
                AcceptIt();
                if (allowLastComma && Is(listTerminatorToken))
                    break;
            }
            else
                break;
        }
    }

    return exprs;
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
    Accept(Tokens::LBracket);
    auto exprs = ParseExprList(Tokens::RBracket);
    Accept(Tokens::RBracket);
    return exprs;
}

std::vector<ExprPtr> HLSLParser::ParseInitializerList()
{
    Accept(Tokens::LCurly);
    auto exprs = ParseExprList(Tokens::RCurly, true);
    Accept(Tokens::RCurly);
    return exprs;
}

std::vector<VarSemanticPtr> HLSLParser::ParseVarSemanticList()
{
    std::vector<VarSemanticPtr> semantics;

    while (Is(Tokens::Colon))
        semantics.push_back(ParseVarSemantic());

    return semantics;
}

std::vector<AttributePtr> HLSLParser::ParseAttributeList()
{
    std::vector<AttributePtr> attribs;

    while (Is(Tokens::LParen))
        attribs.push_back(ParseAttribute());

    return attribs;
}

std::vector<SwitchCasePtr> HLSLParser::ParseSwitchCaseList()
{
    std::vector<SwitchCasePtr> cases;

    while (Is(Tokens::Case) || Is(Tokens::Default))
        cases.push_back(ParseSwitchCase());

    return cases;
}

std::vector<TextureDeclPtr> HLSLParser::ParseTextureDeclList(TextureDeclStmnt* declStmntRef)
{
    std::vector<TextureDeclPtr> textureDecls;

    textureDecls.push_back(ParseTextureDecl(declStmntRef));

    while (Is(Tokens::Comma))
    {
        AcceptIt();
        textureDecls.push_back(ParseTextureDecl(declStmntRef));
    }

    return textureDecls;
}

std::vector<SamplerDeclPtr> HLSLParser::ParseSamplerDeclList()
{
    std::vector<SamplerDeclPtr> samplerDecls;

    samplerDecls.push_back(ParseSamplerDecl());

    while (Is(Tokens::Comma))
    {
        AcceptIt();
        samplerDecls.push_back(ParseSamplerDecl());
    }

    return samplerDecls;
}

std::vector<SamplerValuePtr> HLSLParser::ParseSamplerValueList()
{
    std::vector<SamplerValuePtr> samplerValues;

    while (!Is(Tokens::RCurly))
        samplerValues.push_back(ParseSamplerValue());

    return samplerValues;
}

std::vector<AliasDeclPtr> HLSLParser::ParseAliasDeclList(TypeDenoterPtr typeDenoter)
{
    std::vector<AliasDeclPtr> aliasDecls;

    aliasDecls.push_back(ParseAliasDecl(typeDenoter));

    while (Is(Tokens::Comma))
    {
        AcceptIt();
        aliasDecls.push_back(ParseAliasDecl(typeDenoter));
    }

    return aliasDecls;
}

/* --- Others --- */

std::string HLSLParser::ParseIdent()
{
    return Accept(Tokens::Ident)->Spell();
}

std::string HLSLParser::ParseRegister(bool parseColon)
{
    if (localScope_)
        Error("semantics are not allowed in local scope", false, HLSLErr::ERR_SEMANTICS, false);

    /* Parse ': register(IDENT)' */
    if (parseColon)
        Accept(Tokens::Colon);
    
    Accept(Tokens::Register);
    Accept(Tokens::LBracket);

    auto registerName = ParseIdent();

    Accept(Tokens::RBracket);

    return registerName;
}

std::string HLSLParser::ParseSemantic()
{
    Accept(Tokens::Colon);
    return ParseIdent();
}

TypeDenoterPtr HLSLParser::ParseTypeDenoter(bool allowVoidType)
{
    if (Is(Tokens::Void))
    {
        /* Parse void type denoter */
        if (allowVoidType)
            return ParseVoidTypeDenoter();
        else
            Error("'void' type not allowed in this context");
        return nullptr;
    }
    else
    {
        /* Parse primary type denoter and optional array dimensions */
        auto typeDenoter = ParseTypeDenoterPrimary();

        if (Is(Tokens::LParen))
        {
            /* Make array type denoter */
            auto arrayTypeDenoter = std::make_shared<ArrayTypeDenoter>();
            {
                arrayTypeDenoter->arrayDims         = ParseArrayDimensionList();
                arrayTypeDenoter->baseTypeDenoter   = typeDenoter;
            }
            typeDenoter = arrayTypeDenoter;
        }

        return typeDenoter;
    }
}

TypeDenoterPtr HLSLParser::ParseTypeDenoterPrimary()
{
    if (IsBaseDataType())
        return ParseBaseTypeDenoter();
    else if (Is(Tokens::Vector))
        return ParseBaseVectorTypeDenoter();
    else if (Is(Tokens::Matrix))
        return ParseBaseMatrixTypeDenoter();
    else if (Is(Tokens::Ident))
        return ParseAliasTypeDenoter();
    else if (Is(Tokens::Struct))
        return ParseStructTypeDenoter();
    else if (Is(Tokens::Texture))
        return ParseTextureTypeDenoter();
    else if (Is(Tokens::Sampler))
        return ParseSamplerTypeDenoter();
    else
        ErrorUnexpected("expected type denoter", GetScanner().ActiveToken().get(), true);
    return nullptr;
}

TypeDenoterPtr HLSLParser::ParseTypeDenoterWithStructDeclOpt(StructDeclPtr& structDecl, bool allowVoidType)
{
    if (Is(Tokens::Struct))
    {
        AcceptIt();

        if (Is(Tokens::LCurly))
        {
            /* Parse struct-decl */
            structDecl = ParseStructDecl(false);

            /* Make struct type denoter with reference to the structure of this alias decl */
            return std::make_shared<StructTypeDenoter>(structDecl.get());
        }
        else
        {
            /* Parse struct ident token */
            auto structIdentTkn = Accept(Tokens::Ident);

            if (Is(Tokens::LCurly) || Is(Tokens::Colon))
            {
                /* Parse struct-decl */
                structDecl = ParseStructDecl(false, structIdentTkn);

                /* Make struct type denoter with reference to the structure of this alias decl */
                return std::make_shared<StructTypeDenoter>(structDecl.get());
            }
            else
            {
                /* Make struct type denoter without struct decl */
                return std::make_shared<StructTypeDenoter>(structIdentTkn->Spell());
            }
        }
    }
    else
        return ParseTypeDenoter(allowVoidType);
}

VoidTypeDenoterPtr HLSLParser::ParseVoidTypeDenoter()
{
    Accept(Tokens::Void);
    return std::make_shared<VoidTypeDenoter>();
}

BaseTypeDenoterPtr HLSLParser::ParseBaseTypeDenoter()
{
    if (IsBaseDataType())
    {
        auto keyword = AcceptIt()->Spell();

        /* Make base type denoter by data type keyword */
        auto typeDenoter = std::make_shared<BaseTypeDenoter>();
        typeDenoter->dataType = HLSLKeywordToDataType(keyword);
        return typeDenoter;
    }
    ErrorUnexpected("expected base type denoter", nullptr, true);
    return nullptr;
}

// matrix < ScalarType, '1'-'4', '1'-'4' >;
BaseTypeDenoterPtr HLSLParser::ParseBaseVectorTypeDenoter()
{
    std::string vectorType;

    /* Parse scalar type */
    Accept(Tokens::Vector);
    Accept(Tokens::BinaryOp, "<");
    
    PushParsingState({ true });
    {
        vectorType = Accept(Tokens::ScalarType)->Spell();

        /* Parse vector dimension */
        Accept(Tokens::Comma);
        int dim = ParseAndEvaluateVectorDimension();

        /* Build final type denoter */
        vectorType += std::to_string(dim);
    }
    PopParsingState();

    Accept(Tokens::BinaryOp, ">");

    /* Make base type denoter by data type keyword */
    auto typeDenoter = std::make_shared<BaseTypeDenoter>();
    typeDenoter->dataType = HLSLKeywordToDataType(vectorType);

    return typeDenoter;
}

// matrix < ScalarType, '1'-'4', '1'-'4' >;
BaseTypeDenoterPtr HLSLParser::ParseBaseMatrixTypeDenoter()
{
    std::string matrixType;

    /* Parse scalar type */
    Accept(Tokens::Matrix);
    Accept(Tokens::BinaryOp, "<");
    
    PushParsingState({ true });
    {
        matrixType = Accept(Tokens::ScalarType)->Spell();

        /* Parse matrix dimensions */
        Accept(Tokens::Comma);
        int dimM = ParseAndEvaluateVectorDimension();

        Accept(Tokens::Comma);
        int dimN = ParseAndEvaluateVectorDimension();

        /* Build final type denoter */
        matrixType += std::to_string(dimM) + 'x' + std::to_string(dimN);
    }
    PopParsingState();

    Accept(Tokens::BinaryOp, ">");

    /* Make base type denoter by data type keyword */
    auto typeDenoter = std::make_shared<BaseTypeDenoter>();
    typeDenoter->dataType = HLSLKeywordToDataType(matrixType);

    return typeDenoter;
}

TextureTypeDenoterPtr HLSLParser::ParseTextureTypeDenoter()
{
    /* Make texture type denoter */
    Accept(Tokens::Texture);
    return std::make_shared<TextureTypeDenoter>();
}

SamplerTypeDenoterPtr HLSLParser::ParseSamplerTypeDenoter()
{
    /* Make sampler type denoter */
    Accept(Tokens::Sampler);
    return std::make_shared<SamplerTypeDenoter>();
}

StructTypeDenoterPtr HLSLParser::ParseStructTypeDenoter()
{
    /* Parse optional 'struct' keyword */
    if (Is(Tokens::Struct))
        AcceptIt();

    /* Parse identifier */
    auto ident = ParseIdent();

    /* Make struct type denoter */
    auto typeDenoter = std::make_shared<StructTypeDenoter>(ident);

    return typeDenoter;
}

AliasTypeDenoterPtr HLSLParser::ParseAliasTypeDenoter(std::string ident)
{
    /* Parse identifier */
    if (ident.empty())
        ident = ParseIdent();

    /* Make alias type denoter per default (change this to a struct type later) */
    return std::make_shared<AliasTypeDenoter>(ident);
}

Variant HLSLParser::ParseAndEvaluateConstExpr()
{
    /* Parse expression */
    auto tkn = Tkn();
    auto expr = ParseExpr();

    try
    {
        /* Evaluate expression and throw error on var-access */
        ConstExprEvaluator exprEvaluator;
        return exprEvaluator.EvaluateExpr(*expr, [](VarAccessExpr* ast) -> Variant { throw ast; });
    }
    catch (const std::exception& e)
    {
        Error(e.what(), tkn.get());
    }
    catch (const VarAccessExpr* expr)
    {
        GetReportHandler().Error(true, "expected constant expression", GetScanner().Source(), expr->area);
    }

    return Variant();
}

int HLSLParser::ParseAndEvaluateConstIntExpr()
{
    auto tkn = Tkn();
    auto value = ParseAndEvaluateConstExpr();

    if (value.Type() != Variant::Types::Int)
        Error("expected integral constant expression", tkn.get());

    return static_cast<int>(value.Int());
}

int HLSLParser::ParseAndEvaluateVectorDimension()
{
    auto tkn = Tkn();
    auto value = ParseAndEvaluateConstIntExpr();

    if (value < 1 || value > 4)
        Error("vector and matrix dimensions must be between 1 and 4", tkn.get());

    return value;
}


} // /namespace Xsc



// ================================================================================
