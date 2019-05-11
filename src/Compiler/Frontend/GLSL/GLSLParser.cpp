/*
 * GLSLParser.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLSLParser.h"
#include "GLSLKeywords.h"
#include "Helper.h"
#include "AST.h"
#include "ASTFactory.h"
#include "ReportIdents.h"
#include "Exception.h"


namespace Xsc
{


GLSLParser::GLSLParser(Log* log) :
    SLParser { log }
{
}

ProgramPtr GLSLParser::ParseSource(
    const SourceCodePtr& source, const NameMangling& nameMangling, const InputShaderVersion versionIn, bool enableWarnings)
{
    /* Copy parameters */
    EnableWarnings(enableWarnings);

    GetNameMangling() = nameMangling;

    /* Start scanning source code */
    PushScannerSource(source);

    try
    {
        /* Parse program AST */
        auto ast = ParseProgram(source);
        return (GetReportHandler().HasErrors() ? nullptr : ast);
    }
    catch (const Report& err)
    {
        if (GetLog())
            GetLog()->SubmitReport(err);
    }

    return nullptr;
}


/*
 * ======= Private: =======
 */

ScannerPtr GLSLParser::MakeScanner()
{
    return std::make_shared<GLSLScanner>(GetLog());
}

bool GLSLParser::IsDataType() const
{
    return
    (
        IsBaseDataType() || Is(Tokens::Buffer) || Is(Tokens::Sampler) || Is(Tokens::SamplerState)
    );
}

bool GLSLParser::IsBaseDataType() const
{
    return (Is(Tokens::ScalarType) || Is(Tokens::VectorType) || Is(Tokens::MatrixType) || Is(Tokens::StringType));
}

bool GLSLParser::IsLiteral() const
{
    return (Is(Tokens::BoolLiteral) || Is(Tokens::IntLiteral) || Is(Tokens::FloatLiteral));
}

bool GLSLParser::IsArithmeticUnaryExpr() const
{
    return (Is(Tokens::BinaryOp, "-") || Is(Tokens::BinaryOp, "+"));
}

bool GLSLParser::IsModifier() const
{
    return (Is(Tokens::InputModifier) || Is(Tokens::InterpModifier) || Is(Tokens::TypeModifier) || Is(Tokens::StorageClass));
}

TokenPtr GLSLParser::AcceptIt()
{
    auto tkn = Parser::AcceptIt();

    /* Post-process directives */
    while (Tkn()->Type() == Tokens::Directive)
        ProcessDirective(AcceptIt()->Spell());

    return tkn;
}

void GLSLParser::ProcessDirective(const std::string& ident)
{
    try
    {
        if (ident == "line")
            ProcessDirectiveLine();
        else if (ident == "version")
            ProcessDirectiveVersion();
        else if (ident == "extension")
            ProcessDirectiveExtension();
        else
            RuntimeErr(R_InvalidGLSLDirectiveAfterPP);
    }
    catch (const std::exception& e)
    {
        Error(e.what());
    }
}

void GLSLParser::ProcessDirectiveLine()
{
    int lineNo = 0;
    std::string filename;

    /* Parse '#line'-directive with base class "AcceptIt" functions to avoid recursive calls of this function */
    if (Is(Tokens::IntLiteral))
        lineNo = ParseIntLiteral(Parser::AcceptIt());
    else
        ErrorUnexpected(Tokens::IntLiteral);

    if (Is(Tokens::StringLiteral))
        filename = Parser::AcceptIt()->SpellContent();
    else
        filename = GetScanner().Source()->Filename();

    /* Set new line number and filename */
    auto currentLine = static_cast<int>(GetScanner().PreviousToken()->Pos().Row());
    GetScanner().Source()->NextSourceOrigin(filename, (lineNo - currentLine - 1));
}

void GLSLParser::ProcessDirectiveVersion()
{
    /* Parse version number */
    if (Is(Tokens::IntLiteral))
        version_ = ParseIntLiteral(Parser::AcceptIt());
    else
        ErrorUnexpected(Tokens::IntLiteral);

    /* Parse optional profile */
    if (Is(Tokens::Ident))
    {
        const auto profile = Parser::AcceptIt()->Spell();
        if (profile == "es")
            isESSL_ = true;
        else if (profile == "core")
            isCoreProfile_ = true;
        else if (profile == "compatibility")
            isCoreProfile_ = false;
        else
            Error(R_InvalidGLSLVersionProfile(profile));
    }
}

void GLSLParser::ProcessDirectiveExtension()
{
    std::string extension, behavior;

    /* Parse extension name */
    if (Is(Tokens::Ident))
        extension = Parser::AcceptIt()->Spell();
    else
        ErrorUnexpected(Tokens::Ident);

    /* Parse behavior */
    if (Is(Tokens::Colon))
        Parser::AcceptIt();
    else
        ErrorUnexpected(Tokens::Colon);

    if (Is(Tokens::Ident))
        behavior = Parser::AcceptIt()->Spell();
    else
        ErrorUnexpected(Tokens::Ident);

    /* Store extension state */
    //TODO...
}

/* ------- Symbol table ------- */

void GLSLParser::OpenScope()
{
    typeNameSymbolTable_.OpenScope();
}

void GLSLParser::CloseScope()
{
    typeNameSymbolTable_.CloseScope();
}

void GLSLParser::RegisterTypeName(const std::string& ident)
{
    typeNameSymbolTable_.Register(ident, true, nullptr, false);
}

bool GLSLParser::IsRegisteredTypeName(const std::string& ident) const
{
    return typeNameSymbolTable_.Fetch(ident);
}

/* ------- Parse functions ------- */

ProgramPtr GLSLParser::ParseProgram(const SourceCodePtr& source)
{
    auto ast = Make<Program>();

    OpenScope();

    /* Keep reference to preprocessed source code */
    ast->sourceCode = source;

    while (true)
    {
        /* Check if end of stream has been reached */
        if (Is(Tokens::EndOfStream))
            break;

        /* Parse next global declaration */
        ParseStmtWithCommentOpt(ast->globalStmts, std::bind(&GLSLParser::ParseGlobalStmt, this));
    }

    CloseScope();

    return ast;
}

CodeBlockPtr GLSLParser::ParseCodeBlock()
{
    auto ast = Make<CodeBlock>();

    /* Parse statement list */
    Accept(Tokens::LCurly);
    OpenScope();
    {
        ast->stmts = ParseLocalStmtList();
    }
    CloseScope();
    Accept(Tokens::RCurly);

    return ast;
}

VarDeclStmtPtr GLSLParser::ParseParameter()
{
    auto ast = Make<VarDeclStmt>();

    /* Parse parameter as single variable declaration */
    ast->typeSpecifier = ParseTypeSpecifier();

    ast->varDecls.push_back(ParseVarDecl(ast.get()));

    /* Mark with 'parameter' flag */
    ast->flags << VarDeclStmt::isParameter;

    return UpdateSourceArea(ast);
}

StmtPtr GLSLParser::ParseLocalStmt()
{
    return ParseStmt();
}

StmtPtr GLSLParser::ParseForStmtInitializer()
{
    return ParseStmt();
}

SwitchCasePtr GLSLParser::ParseSwitchCase()
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
        ParseStmtWithCommentOpt(ast->stmts, std::bind(&GLSLParser::ParseStmt, this));

    return ast;
}

VarDeclPtr GLSLParser::ParseVarDecl(VarDeclStmt* declStmtRef, const TokenPtr& identTkn)
{
    auto ast = Make<VarDecl>();

    /* Store reference to parent node */
    ast->declStmtRef = declStmtRef;

    /* Parse variable declaration */
    ast->ident = ParseIdent(identTkn, &ast->area);

    /* Parse optional array dimension, semantic, and annocations */
    ast->arrayDims = ParseArrayDimensionList(true);

    /* Parse optional initializer expression */
    if (Is(Tokens::AssignOp, "="))
        ast->initializer = ParseInitializer();

    return ast;
}

// QUALIFIER ( '=' EXPR )?
AttributePtr GLSLParser::ParseAttribute()
{
    auto ast = Make<Attribute>();

    /* Parse layout qualifier */
    auto attribIdent = ParseIdent();
    ast->attributeType = GLSLKeywordToAttributeType(attribIdent);

    UpdateSourceArea(ast);

    if (ast->attributeType == AttributeType::Undefined)
        Error(R_UnknownLayoutQualifier(attribIdent));

    /* Parse optional layout qualifier value */
    if (Is(Tokens::AssignOp, "="))
    {
        AcceptIt();
        ast->arguments.push_back(ParseExpr());
    }

    return ast;
}

TypeSpecifierPtr GLSLParser::ParseTypeSpecifier(bool parseVoidType, const TokenPtr& inputTkn)
{
    auto ast = Make<TypeSpecifier>();

    /* Parse optional first input token */
    if (inputTkn)
        ParseModifiers(ast.get(), true, inputTkn);

    /* Parse modifiers and primitive types */
    while (IsModifier() || Is(Tokens::PrimitiveType))
        ParseModifiers(ast.get(), true);

    /* Parse variable type denoter with optional struct declaration */
    ast->typeDenoter = ParseTypeDenoterWithStructDeclOpt(ast->structDecl);

    return UpdateSourceArea(ast);
}

#if 0

BufferDeclPtr GLSLParser::ParseBufferDecl(BufferDeclStmt* declStmtRef, const TokenPtr& identTkn)
{
    auto ast = Make<BufferDecl>();

    /* Store reference to parent node */
    ast->declStmtRef = declStmtRef;

    /* Parse identifier, optional array dimension list, and optional slot registers */
    ast->ident          = ParseIdent(identTkn);
    ast->arrayDims      = ParseArrayDimensionList();

    return ast;
}

SamplerDeclPtr GLSLParser::ParseSamplerDecl(SamplerDeclStmt* declStmtRef, const TokenPtr& identTkn)
{
    auto ast = Make<SamplerDecl>();

    /* Store reference to parent node */
    ast->declStmtRef = declStmtRef;

    /* Parse identifier, optional array dimension list, and optional slot registers */
    ast->ident          = ParseIdent(identTkn);
    ast->arrayDims      = ParseArrayDimensionList();

    return ast;
}

#endif

StructDeclPtr GLSLParser::ParseStructDecl(bool parseStructTkn, const TokenPtr& identTkn)
{
    auto tkn = Tkn();
    auto ast = Make<StructDecl>();

    /* Parse structure declaration */
    if (parseStructTkn)
    {
        Accept(Tokens::Struct);
        UpdateSourceArea(ast);
    }

    if (Is(Tokens::Ident) || identTkn)
    {
        /* Parse structure name */
        tkn = Tkn();
        ast->ident = (identTkn ? identTkn->Spell() : ParseIdent());
        UpdateSourceArea(ast);

        /* Register type name in symbol table */
        RegisterTypeName(ast->ident);

        /* Check of illegal inheritance (not supported in GLSL) */
        if (Is(Tokens::Colon))
            Error(R_IllegalInheritance);
    }

    GetReportHandler().PushContextDesc(ast->ToString());
    {
        /* Parse member variable declarations */
        ast->localStmts = ParseGlobalStmtList();

        for (auto& stmt : ast->localStmts)
        {
            if (stmt->Type() == AST::Types::VarDeclStmt)
            {
                /* Store copy in member variable list */
                ast->varMembers.push_back(std::static_pointer_cast<VarDeclStmt>(stmt));
            }
            else
                Error(R_IllegalDeclStmtInsideDeclOf(ast->ToString()), stmt->area, false);
        }

        /* Decorate all member variables with a reference to this structure declaration */
        for (auto& varDeclStmt : ast->varMembers)
        {
            for (auto& varDecl : varDeclStmt->varDecls)
                varDecl->structDeclRef = ast.get();
        }
    }
    GetReportHandler().PopContextDesc();

    return ast;
}

FunctionDeclPtr GLSLParser::ParseFunctionDecl(BasicDeclStmt* declStmtRef, const TypeSpecifierPtr& returnType, const TokenPtr& identTkn)
{
    auto ast = Make<FunctionDecl>();

    /* Store reference to declaration statement parent node */
    ast->declStmtRef = declStmtRef;

    if (returnType)
    {
        /* Take previously parsed return type */
        ast->returnType = returnType;
    }
    else
    {
        /* Parse (and ignore) optional 'inline' keyword */
        if (Is(Tokens::Inline))
            AcceptIt();

        /* Parse return type */
        ast->returnType = ParseTypeSpecifier(true);
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

    /* Parse optional function body */
    if (Is(Tokens::Semicolon))
        AcceptIt();
    else
    {
        GetReportHandler().PushContextDesc(ast->ToString(false));
        {
            ast->codeBlock = ParseCodeBlock();
        }
        GetReportHandler().PopContextDesc();
    }

    return ast;
}

UniformBufferDeclPtr GLSLParser::ParseUniformBufferDecl(const TokenPtr& identTkn)
{
    auto ast = Make<UniformBufferDecl>();

    /* Parse buffer header */
    ast->bufferType = UniformBufferType::ConstantBuffer;
    ast->ident      = ParseIdent(identTkn);

    UpdateSourceArea(ast);

    GetReportHandler().PushContextDesc(ast->ToString());
    {
        /* Parse buffer body */
        ast->localStmts = ParseGlobalStmtList();

        /* Copy variable declarations into separated list */
        for (auto& stmt : ast->localStmts)
        {
            if (stmt->Type() == AST::Types::VarDeclStmt)
                ast->varMembers.push_back(std::static_pointer_cast<VarDeclStmt>(stmt));
            else
                Error(R_OnlyFieldsAllowedInUniformBlock, stmt->area, false);
        }

        /* Decorate all member variables with a reference to this buffer declaration */
        for (auto& varDeclStmt : ast->varMembers)
        {
            for (auto& varDecl : varDeclStmt->varDecls)
                varDecl->bufferDeclRef = ast.get();
        }

        Semi();
    }
    GetReportHandler().PopContextDesc();

    return ast;
}

/* --- Declaration statements --- */

StmtPtr GLSLParser::ParseGlobalStmt()
{
    if (Is(Tokens::LayoutQualifier))
    {
        /* Parse attributes and statement */
        auto attribs = ParseAttributeList();
        auto ast = ParseGlobalStmtPrimary(!attribs.empty());
        ast->attribs = std::move(attribs);
        return ast;
    }
    else
    {
        /* Parse statement only */
        return ParseGlobalStmtPrimary();
    }
}

StmtPtr GLSLParser::ParseGlobalStmtPrimary(bool hasAttribs)
{
    switch (TknType())
    {
        #if 0
        case Tokens::Sampler:
        case Tokens::SamplerState:
            return ParseGlobalStmtWithSamplerTypeDenoter();
        case Tokens::Buffer:
            return ParseGlobalStmtWithBufferTypeDenoter();
        #endif
        case Tokens::UniformBuffer:
            return ParseUniformDeclStmt();
        case Tokens::Struct:
            return ParseStmtWithStructDecl();
        #if 0
        case Tokens::Void:
        case Tokens::Inline:
            return ParseFunctionDeclStmt();
        #endif
        default:
            if ( hasAttribs && ( Is(Tokens::InputModifier, "in") || Is(Tokens::InputModifier, "out") ))
                return ParseGlobalStmtWithLayoutQualifier();
            else
                return ParseGlobalStmtWithTypeSpecifier();
    }
}

StmtPtr GLSLParser::ParseGlobalStmtWithTypeSpecifier(const TokenPtr& inputTkn)
{
    /* Parse type specifier */
    auto typeSpecifier = ParseTypeSpecifier(false, inputTkn);

    /* Is this only a struct declaration? */
    if (typeSpecifier->structDecl && Is(Tokens::Semicolon))
    {
        /* Convert type specifier into struct declaration statement */
        auto ast = Make<BasicDeclStmt>();

        auto structDecl = typeSpecifier->structDecl;
        structDecl->declStmtRef = ast.get();

        ast->declObject = structDecl;

        Semi();

        return ast;
    }

    /* Parse identifier */
    auto identTkn = Accept(Tokens::Ident);

    /* Is this a function declaration? */
    if (Is(Tokens::LBracket))
    {
        /* Parse function declaration statement */
        return ParseFunctionDeclStmt(typeSpecifier, identTkn);
    }
    else
    {
        /* Parse variable declaration statement */
        auto ast = Make<VarDeclStmt>();

        ast->typeSpecifier  = typeSpecifier;
        ast->varDecls       = ParseVarDeclList(ast.get(), identTkn);

        Semi();

        return UpdateSourceArea(ast, ast->typeSpecifier.get());
    }
}

StmtPtr GLSLParser::ParseGlobalStmtWithLayoutQualifier()
{
    auto inputTkn = Accept(Tokens::InputModifier);

    if (Is(Tokens::Semicolon))
    {
        AcceptIt();

        /* Parse in/out token */
        auto ast = Make<LayoutStmt>();

        if (inputTkn->Spell() == "in")
            ast->isInput = true;
        else if (inputTkn->Spell() == "out")
            ast->isOutput = true;

        return ast;
    }

    return ParseGlobalStmtWithTypeSpecifier(inputTkn);
}

#if 0

StmtPtr GLSLParser::ParseGlobalStmtWithSamplerTypeDenoter()
{
    /* Parse sampler type denoter and identifier */
    auto typeDenoter = ParseSamplerTypeDenoter();
    auto identTkn = Accept(Tokens::Ident);

    if (Is(Tokens::LBracket))
    {
        /* Make variable type from type denoter, then parse function declaration */
        return ParseFunctionDeclStmt(ASTFactory::MakeTypeSpecifier(typeDenoter), identTkn);
    }
    else
    {
        /* Parse sampler declaration statement with sampler type denoter */
        return ParseSamplerDeclStmt(typeDenoter, identTkn);
    }
}

StmtPtr GLSLParser::ParseGlobalStmtWithBufferTypeDenoter()
{
    /* Parse buffer type denoter and identifier */
    auto typeDenoter = ParseBufferTypeDenoter();
    auto identTkn = Accept(Tokens::Ident);

    if (Is(Tokens::LBracket))
    {
        /* Make variable type from type denoter, then parse function declaration */
        return ParseFunctionDeclStmt(ASTFactory::MakeTypeSpecifier(typeDenoter), identTkn);
    }
    else
    {
        /* Parse buffer declaration statement with sampler type denoter */
        return ParseBufferDeclStmt(typeDenoter, identTkn);
    }
}

#endif

BasicDeclStmtPtr GLSLParser::ParseFunctionDeclStmt(const TypeSpecifierPtr& returnType, const TokenPtr& identTkn)
{
    auto ast = Make<BasicDeclStmt>();

    /* Parse functoin declaration object */
    ast->declObject = ParseFunctionDecl(ast.get(), returnType, identTkn);

    return ast;
}

StmtPtr GLSLParser::ParseUniformDeclStmt()
{
    Accept(Tokens::UniformBuffer);

    if (Is(Tokens::Ident))
    {
        /* Parse identifer and check if it's a registerd type name */
        auto identTkn = AcceptIt();

        if (IsRegisteredTypeName(identTkn->Spell()))
        {
            /* Parse variable declaration */
            return ParseVarDeclStmt(true, identTkn);
        }
        else
        {
            /* Parse uniform buffer declaration */
            return ParseUniformBufferDeclStmt(identTkn);
        }
    }
    else
    {
        /* Parse variable declaration */
        return ParseVarDeclStmt(true);
    }
}

BasicDeclStmtPtr GLSLParser::ParseUniformBufferDeclStmt(const TokenPtr& identTkn)
{
    auto ast = Make<BasicDeclStmt>();

    auto uniformBufferDecl = ParseUniformBufferDecl(identTkn);
    ast->declObject = uniformBufferDecl;

    uniformBufferDecl->declStmtRef = ast.get();

    return ast;
}

#if 0

BufferDeclStmtPtr GLSLParser::ParseBufferDeclStmt(const BufferTypeDenoterPtr& typeDenoter, const TokenPtr& identTkn)
{
    auto ast = Make<BufferDeclStmt>();

    ast->typeDenoter = (typeDenoter ? typeDenoter : ParseBufferTypeDenoter());
    ast->bufferDecls = ParseBufferDeclList(ast.get(), identTkn);

    Semi();

    if (identTkn)
        ast->area = identTkn->Area();
    else
        UpdateSourceArea(ast);

    return ast;
}

SamplerDeclStmtPtr GLSLParser::ParseSamplerDeclStmt(const SamplerTypeDenoterPtr& typeDenoter, const TokenPtr& identTkn)
{
    auto ast = Make<SamplerDeclStmt>();

    ast->typeDenoter = (typeDenoter ? typeDenoter : ParseSamplerTypeDenoter());
    ast->samplerDecls = ParseSamplerDeclList(ast.get(), identTkn);

    Semi();

    if (identTkn)
        ast->area = identTkn->Area();
    else
        UpdateSourceArea(ast);

    return ast;
}

#endif

VarDeclStmtPtr GLSLParser::ParseVarDeclStmt(bool isUniform, const TokenPtr& identTkn)
{
    auto ast = Make<VarDeclStmt>();

    /* Parse type specifier and all variable declarations */
    ast->typeSpecifier  = ParseTypeSpecifier();
    ast->varDecls       = ParseVarDeclList(ast.get());

    Semi();

    return UpdateSourceArea(ast);
}

/* --- Statements --- */

StmtPtr GLSLParser::ParseStmt()
{
    /* Determine which kind of statement the next one is */
    switch (TknType())
    {
        case Tokens::Semicolon:
            return ParseNullStmt();
        case Tokens::LCurly:
            return ParseScopeStmt();
        case Tokens::Return:
            return ParseReturnStmt();
        #if 0
        case Tokens::Ident:
            return ParseStmtWithIdent();
        #endif
        case Tokens::For:
            return ParseForStmt();
        case Tokens::While:
            return ParseWhileStmt();
        case Tokens::Do:
            return ParseDoWhileStmt();
        case Tokens::If:
            return ParseIfStmt();
        case Tokens::Switch:
            return ParseSwitchStmt();
        case Tokens::CtrlTransfer:
            return ParseJumpStmt();
        case Tokens::Struct:
            return ParseStmtWithStructDecl();
        #if 0
        case Tokens::Sampler:
        case Tokens::SamplerState:
            return ParseSamplerDeclStmt();
        #endif
        case Tokens::StorageClass:
        case Tokens::InterpModifier:
        case Tokens::TypeModifier:
            return ParseVarDeclStmt();
        default:
            break;
    }

    if (IsDataType())
        return ParseVarDeclStmt();

    /* Parse statement of arbitrary expression */
    return ParseExprStmt();
}

StmtPtr GLSLParser::ParseStmtWithStructDecl()
{
    /* Parse structure declaration statement */
    auto ast = Make<BasicDeclStmt>();

    auto structDecl = ParseStructDecl();
    structDecl->declStmtRef = ast.get();

    ast->declObject = structDecl;

    if (!Is(Tokens::Semicolon))
    {
        /* Parse variable declaration with previous structure type */
        auto varDeclStmt = Make<VarDeclStmt>();

        varDeclStmt->typeSpecifier = ASTFactory::MakeTypeSpecifier(structDecl);

        /* Parse variable declarations */
        varDeclStmt->varDecls = ParseVarDeclList(varDeclStmt.get());
        Semi();

        return UpdateSourceArea(varDeclStmt);
    }
    else
        Semi();

    return ast;
}

#if 0

#if 1//TODO: clean this up!!!

// ~~~~~~~~~~~~ MIGHT BE INCOMPLETE ~~~~~~~~~~~~~~~

StmtPtr GLSLParser::ParseStmtWithIdent()
{
    /* Parse the identifier as object expression (can be converted later) */
    auto identExpr = ParseIdentExpr();

    auto expr = ParseExprWithSuffixOpt(identExpr);

    if (Is(Tokens::LBracket) || Is(Tokens::UnaryOp) || Is(Tokens::BinaryOp) || Is(Tokens::TernaryOp))
    {
        /* Parse expression statement (function call, variable access, etc.) */
        PushPreParsedAST(expr);
        return ParseExprStmt();
    }
    else if (Is(Tokens::Semicolon))
    {
        /* Return immediatly with expression statement */
        return ParseExprStmt(expr);
    }
    else if (Is(Tokens::Comma))
    {
        /* Parse sequence expression */
        return ParseExprStmt(ParseSequenceExpr(expr));
    }
    else if (expr == identExpr)
    {
        /* Convert variable identifier to alias type denoter */
        auto ast = Make<VarDeclStmt>();

        ast->typeSpecifier              = Make<TypeSpecifier>();
        ast->typeSpecifier->typeDenoter = ParseTypeDenoterWithArrayOpt(std::make_shared<StructTypeDenoter>(identExpr->ident));

        UpdateSourceArea(ast->typeSpecifier, identExpr.get());

        ast->varDecls = ParseVarDeclList(ast.get());
        Semi();

        return UpdateSourceArea(ast, identExpr.get());
    }
    else
        return ParseExprStmt(expr);

    #if 0//DEAD CODE
    ErrorUnexpected(R_ExpectedVarOrAssignOrFuncCall, nullptr, true);

    return nullptr;
    #endif
}

#endif

#endif

/* --- Expressions --- */

ExprPtr GLSLParser::ParsePrimaryExpr()
{
    /* Primary prefix of primary expression */
    return ParseExprWithSuffixOpt(ParsePrimaryExprPrefix());
}

ExprPtr GLSLParser::ParsePrimaryExprPrefix()
{
    /* Check if a pre-parsed AST node is available */
    if (auto preParsedAST = PopPreParsedAST())
    {
        if (preParsedAST->Type() == AST::Types::IdentExpr)
        {
            /* Parse call expression or return pre-parsed object expression */
            auto identExpr = std::static_pointer_cast<IdentExpr>(preParsedAST);
            if (Is(Tokens::LBracket))
                return ParseCallExpr(identExpr);
            else
                return identExpr;
        }
        else if (preParsedAST->Type() == AST::Types::CallExpr)
        {
            /* Return pre-parsed call expression */
            return std::static_pointer_cast<CallExpr>(preParsedAST);
        }
        else
            ErrorInternal(R_UnexpectedPreParsedAST, __FUNCTION__);
    }

    /* Determine which kind of expression the next one is */
    if (IsLiteral())
        return ParseLiteralExpr();
    if (IsModifier())
        return ParseTypeSpecifierExpr();
    if (IsDataType() || Is(Tokens::Struct))
        return ParseTypeSpecifierOrCallExpr();
    if (Is(Tokens::UnaryOp) || IsArithmeticUnaryExpr())
        return ParseUnaryExpr();
    if (Is(Tokens::LBracket))
        return ParseBracketExpr();
    if (Is(Tokens::LCurly))
        return ParseInitializerExpr();
    if (Is(Tokens::Ident))
        return ParseIdentOrCallExpr();

    ErrorUnexpected(R_ExpectedPrimaryExpr, nullptr, true);

    return nullptr;
}

ExprPtr GLSLParser::ParseExprWithSuffixOpt(ExprPtr expr)
{
    /* Parse optional suffix expressions */
    while (true)
    {
        if (Is(Tokens::LParen))
            expr = ParseSubscriptExpr(expr);
        else if (Is(Tokens::Dot))
            expr = ParseIdentOrCallExpr(expr);
        else if (Is(Tokens::AssignOp))
            expr = ParseAssignExpr(expr);
        else if (Is(Tokens::UnaryOp))
            expr = ParsePostUnaryExpr(expr);
        else
            break;
    }

    return UpdateSourceArea(expr);
}

LiteralExprPtr GLSLParser::ParseLiteralExpr()
{
    if (!IsLiteral())
        ErrorUnexpected(R_ExpectedLiteralExpr);

    /* Parse literal */
    auto ast = Make<LiteralExpr>();

    ast->dataType   = TokenToDataType(*Tkn());
    ast->value      = AcceptIt()->Spell();

    return UpdateSourceArea(ast);
}

ExprPtr GLSLParser::ParseTypeSpecifierOrCallExpr()
{
    /* Parse type denoter with optional structure delcaration */
    if (!IsDataType() && !Is(Tokens::Struct))
        ErrorUnexpected(R_ExpectedTypeNameOrFuncCall);

    StructDeclPtr structDecl;
    auto typeDenoter = ParseTypeDenoter(true, &structDecl);

    /* Determine which kind of expression this is */
    if (Is(Tokens::LBracket) && !structDecl)
    {
        /* Return function call expression */
        return ParseCallExpr(nullptr, typeDenoter);
    }

    /* Return type name expression */
    auto ast = Make<TypeSpecifierExpr>();
    {
        ast->typeSpecifier               = ASTFactory::MakeTypeSpecifier(typeDenoter);
        ast->typeSpecifier->structDecl   = structDecl;
    }
    UpdateSourceArea(ast->typeSpecifier, structDecl.get());

    return UpdateSourceArea(ast, structDecl.get());
}

TypeSpecifierExprPtr GLSLParser::ParseTypeSpecifierExpr()
{
    auto ast = Make<TypeSpecifierExpr>();

    /* Parse type specifier */
    ast->typeSpecifier = ParseTypeSpecifier();

    return UpdateSourceArea(ast);
}

UnaryExprPtr GLSLParser::ParseUnaryExpr()
{
    if (!Is(Tokens::UnaryOp) && !IsArithmeticUnaryExpr())
        ErrorUnexpected(R_ExpectedUnaryOp);

    /* Parse unary expression (e.g. "++x", "!x", "+x", "-x") */
    auto ast = Make<UnaryExpr>();

    ast->op     = StringToUnaryOp(AcceptIt()->Spell(), false);
    ast->expr   = ParsePrimaryExpr();

    return UpdateSourceArea(ast);
}

UnaryExprPtr GLSLParser::ParsePostUnaryExpr(const ExprPtr& expr)
{
    if (!Is(Tokens::UnaryOp))
        ErrorUnexpected(R_ExpectedUnaryOp);

    /* Parse post-unary expression (e.g. "x++", "x--") */
    auto ast = Make<UnaryExpr>();

    ast->expr   = expr;
    ast->op     = StringToUnaryOp(AcceptIt()->Spell(), true);

    UpdateSourceArea(ast, expr.get());
    UpdateSourceAreaOffset(ast);

    return ast;
}

BracketExprPtr GLSLParser::ParseBracketExpr()
{
    auto ast = Make<BracketExpr>();

    Accept(Tokens::LBracket);
    {
        ast->expr = ParseExpr();
    }
    Accept(Tokens::RBracket);

    return UpdateSourceArea(ast);
}

IdentExprPtr GLSLParser::ParseIdentExpr(const ExprPtr& prefixExpr)
{
    /* Parse prefix token if prefix expression is specified  */
    if (prefixExpr)
    {
        /* Parse '.' prefix */
        if (Is(Tokens::Dot))
            AcceptIt();
        else
            ErrorUnexpected(R_ExpectedIdentPrefix);
    }

    auto ast = Make<IdentExpr>();

    if (prefixExpr)
        ast->area = prefixExpr->area;

    /* Take sub expression and parse identifier */
    ast->prefixExpr = prefixExpr;
    ast->ident      = ParseIdent();

    return UpdateSourceArea(ast);
}

AssignExprPtr GLSLParser::ParseAssignExpr(const ExprPtr& lvalueExpr)
{
    auto ast = Make<AssignExpr>();

    /* Take sub expression and parse assignment */
    ast->area       = lvalueExpr->area;
    ast->lvalueExpr = lvalueExpr;

    /* Parse assign expression */
    if (Is(Tokens::AssignOp))
    {
        ast->op         = StringToAssignOp(AcceptIt()->Spell());
        UpdateSourceAreaOffset(ast);
        ast->rvalueExpr = ParseExpr();
    }
    else
        ErrorUnexpected(Tokens::AssignOp);

    return UpdateSourceArea(ast);
}

ExprPtr GLSLParser::ParseIdentOrCallExpr(const ExprPtr& prefixExpr)
{
    /* Parse variable identifier first (for variables and functions) */
    auto identExpr = ParseIdentExpr(prefixExpr);

    if (Is(Tokens::LBracket))
        return ParseCallExpr(identExpr);

    return identExpr;
}

CallExprPtr GLSLParser::ParseCallExpr(const IdentExprPtr& identExpr, const TypeDenoterPtr& typeDenoter)
{
    if (identExpr)
    {
        /* Make new identifier token with source position from input */
        auto identTkn = std::make_shared<Token>(identExpr->area.Pos(), Tokens::Ident, identExpr->ident);

        /* Parse call expression and take prefix expression from input */
        return ParseCallExprWithPrefixOpt(identExpr->prefixExpr, identExpr->isStatic, identTkn);
    }
    else if (typeDenoter)
    {
        /* Parse call expression with type denoter */
        return ParseCallExprAsTypeCtor(typeDenoter);
    }
    else
    {
        /* Parse completely new call expression */
        return ParseCallExprWithPrefixOpt();
    }
}

CallExprPtr GLSLParser::ParseCallExprWithPrefixOpt(const ExprPtr& prefixExpr, bool isStatic, const TokenPtr& identTkn)
{
    auto ast = Make<CallExpr>();

    /* Take prefix expression */
    ast->prefixExpr = prefixExpr;
    ast->isStatic   = isStatic;

    /* Parse function name */
    if (identTkn)
    {
        /* Take identifier token */
        ast->ident  = identTkn->Spell();
        ast->area   = identTkn->Area();
    }
    else
    {
        /* Parse identifier token */
        ast->ident = ParseIdent();
        UpdateSourceArea(ast);
    }

    /* Parse argument list */
    ast->arguments = ParseArgumentList();

    return UpdateSourceArea(ast);
}

// Parse function call as a type constructor (e.g. "vec4(...)")
CallExprPtr GLSLParser::ParseCallExprAsTypeCtor(const TypeDenoterPtr& typeDenoter)
{
    auto ast = Make<CallExpr>();

    /* Take type denoter */
    ast->typeDenoter = typeDenoter;

    /* Parse argument list */
    ast->arguments = ParseArgumentList();

    return UpdateSourceArea(ast);
}

/* --- Lists --- */

std::vector<StmtPtr> GLSLParser::ParseGlobalStmtList()
{
    std::vector<StmtPtr> stmts;

    Accept(Tokens::LCurly);

    /* Parse all variable declaration statements */
    while (!Is(Tokens::RCurly))
    {
        /* Parse next global declaration */
        ParseStmtWithCommentOpt(stmts, std::bind(&GLSLParser::ParseGlobalStmt, this));
    }

    AcceptIt();

    return stmts;
}

// 'layout' '(' QUALIFIER ( ',' QUALIFIER )* ')'
std::vector<AttributePtr> GLSLParser::ParseAttributeList()
{
    std::vector<AttributePtr> attribs;

    /* Parse layout qualifier */
    Accept(Tokens::LayoutQualifier);
    Accept(Tokens::LBracket);

    while (true)
    {
        attribs.push_back(ParseAttribute());

        if (Is(Tokens::Comma))
            AcceptIt();
        else
            break;
    }

    Accept(Tokens::RBracket);

    return attribs;
}

#if 0

std::vector<BufferDeclPtr> GLSLParser::ParseBufferDeclList(BufferDeclStmt* declStmtRef, const TokenPtr& identTkn)
{
    std::vector<BufferDeclPtr> bufferDecls;

    bufferDecls.push_back(ParseBufferDecl(declStmtRef, identTkn));

    while (Is(Tokens::Comma))
    {
        AcceptIt();
        bufferDecls.push_back(ParseBufferDecl(declStmtRef));
    }

    return bufferDecls;
}

std::vector<SamplerDeclPtr> GLSLParser::ParseSamplerDeclList(SamplerDeclStmt* declStmtRef, const TokenPtr& identTkn)
{
    std::vector<SamplerDeclPtr> samplerDecls;

    samplerDecls.push_back(ParseSamplerDecl(declStmtRef, identTkn));

    while (Is(Tokens::Comma))
    {
        AcceptIt();
        samplerDecls.push_back(ParseSamplerDecl(declStmtRef));
    }

    return samplerDecls;
}

#endif

/* --- Others --- */

TypeDenoterPtr GLSLParser::ParseTypeDenoter(bool allowVoidType, StructDeclPtr* structDecl)
{
    if (Is(Tokens::Void))
    {
        /* Parse void type denoter */
        if (allowVoidType)
            return ParseVoidTypeDenoter();

        Error(R_NotAllowedInThisContext(R_VoidTypeDen));
        return nullptr;
    }
    else
    {
        /* Parse primary type denoter and optional array dimensions */
        auto typeDenoter = ParseTypeDenoterPrimary(structDecl);

        if (Is(Tokens::LParen))
        {
            /* Make array type denoter */
            typeDenoter = std::make_shared<ArrayTypeDenoter>(typeDenoter, ParseArrayDimensionList());
        }

        return typeDenoter;
    }
}

TypeDenoterPtr GLSLParser::ParseTypeDenoterPrimary(StructDeclPtr* structDecl)
{
    if (IsBaseDataType())
        return ParseBaseTypeDenoter();
    else if (Is(Tokens::Ident) || Is(Tokens::Struct))
    {
        if (structDecl)
            return ParseStructTypeDenoterWithStructDeclOpt(*structDecl);
        else
            return ParseStructTypeDenoter();
    }
    else if (Is(Tokens::StorageBuffer))
        return ParseBufferTypeDenoter();
    else if (Is(Tokens::Sampler) || Is(Tokens::SamplerState))
        return ParseSamplerTypeDenoter();

    ErrorUnexpected(R_ExpectedTypeDen, GetScanner().ActiveToken().get(), true);
    return nullptr;
}

TypeDenoterPtr GLSLParser::ParseTypeDenoterWithStructDeclOpt(StructDeclPtr& structDecl, bool allowVoidType)
{
    if (Is(Tokens::Struct))
        return ParseStructTypeDenoterWithStructDeclOpt(structDecl);
    else
        return ParseTypeDenoter(allowVoidType);
}

VoidTypeDenoterPtr GLSLParser::ParseVoidTypeDenoter()
{
    Accept(Tokens::Void);
    return std::make_shared<VoidTypeDenoter>();
}

BaseTypeDenoterPtr GLSLParser::ParseBaseTypeDenoter()
{
    if (IsBaseDataType())
    {
        auto keyword = AcceptIt()->Spell();

        /* Make base type denoter by data type keyword */
        auto typeDenoter = std::make_shared<BaseTypeDenoter>();
        typeDenoter->dataType = ParseDataType(keyword);
        return typeDenoter;
    }
    ErrorUnexpected(R_ExpectedBaseTypeDen, nullptr, true);
    return nullptr;
}

BufferTypeDenoterPtr GLSLParser::ParseBufferTypeDenoter()
{
    /* Make buffer type denoter */
    Accept(Tokens::StorageBuffer);
    return std::make_shared<BufferTypeDenoter>(BufferType::GenericBuffer);
}

SamplerTypeDenoterPtr GLSLParser::ParseSamplerTypeDenoter()
{
    /* Make sampler type denoter */
    auto samplerType = ParseSamplerType();
    return std::make_shared<SamplerTypeDenoter>(samplerType);
}

StructTypeDenoterPtr GLSLParser::ParseStructTypeDenoter()
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

StructTypeDenoterPtr GLSLParser::ParseStructTypeDenoterWithStructDeclOpt(StructDeclPtr& structDecl)
{
    /* Parse 'struct' keyword */
    Accept(Tokens::Struct);

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

DataType GLSLParser::ParseDataType(const std::string& keyword)
{
    try
    {
        return GLSLKeywordToDataType(keyword);
    }
    catch (const std::exception& e)
    {
        Error(e.what());
    }
    return DataType::Undefined;
}

PrimitiveType GLSLParser::ParsePrimitiveType()
{
    try
    {
        return GLSLKeywordToPrimitiveType(Accept(Tokens::PrimitiveType)->Spell());
    }
    catch (const std::exception& e)
    {
        Error(e.what());
    }
    return PrimitiveType::Undefined;
}

InterpModifier GLSLParser::ParseInterpModifier()
{
    try
    {
        return GLSLKeywordToInterpModifier(Accept(Tokens::InterpModifier)->Spell());
    }
    catch (const std::exception& e)
    {
        Error(e.what());
    }
    return InterpModifier::Undefined;
}

TypeModifier GLSLParser::ParseTypeModifier()
{
    Accept(Tokens::TypeModifier, "const");
    return TypeModifier::Const;
}

StorageClass GLSLParser::ParseStorageClass()
{
    try
    {
        return GLSLKeywordToStorageClass(Accept(Tokens::StorageClass)->Spell());
    }
    catch (const std::exception& e)
    {
        Error(e.what());
    }
    return StorageClass::Undefined;
}

SamplerType GLSLParser::ParseSamplerType()
{
    try
    {
        if (Is(Tokens::Sampler) || Is(Tokens::SamplerState))
            return GLSLKeywordToSamplerType(AcceptIt()->Spell());
        else
            ErrorUnexpected(R_ExpectedSamplerOrSamplerState);
    }
    catch (const std::exception& e)
    {
        Error(e.what());
    }
    return SamplerType::Undefined;
}

bool GLSLParser::ParseModifiers(TypeSpecifier* typeSpecifier, bool allowPrimitiveType, const TokenPtr& inputTkn)
{
    if (Is(Tokens::InputModifier) || inputTkn)
    {
        /* Parse input modifier */
        std::string modifier;

        if (inputTkn)
            modifier = inputTkn->Spell();
        else
            modifier = AcceptIt()->Spell();

        if (modifier == "in")
            typeSpecifier->isInput = true;
        else if (modifier == "out")
            typeSpecifier->isOutput = true;
        else if (modifier == "inout")
        {
            typeSpecifier->isInput = true;
            typeSpecifier->isOutput = true;
        }
        #if 0
        else if (modifier == "uniform")
            typeSpecifier->isUniform = true;
        #endif
    }
    else if (Is(Tokens::InterpModifier))
    {
        /* Parse interpolation modifier */
        typeSpecifier->interpModifiers.insert(ParseInterpModifier());
    }
    else if (Is(Tokens::TypeModifier))
    {
        /* Parse type modifier ('const' only) */
        typeSpecifier->SetTypeModifier(ParseTypeModifier());
    }
    else if (Is(Tokens::StorageClass))
    {
        /* Parse storage class */
        typeSpecifier->storageClasses.insert(ParseStorageClass());
    }
    else if (Is(Tokens::PrimitiveType))
    {
        /* Parse primitive type */
        if (!allowPrimitiveType)
            Error(R_NotAllowedInThisContext(R_PrimitiveType), false, false);

        auto primitiveType = ParsePrimitiveType();

        if (typeSpecifier->primitiveType == PrimitiveType::Undefined)
            typeSpecifier->primitiveType = primitiveType;
        else if (typeSpecifier->primitiveType == primitiveType)
            Error(R_DuplicatedPrimitiveType, true, false);
        else if (typeSpecifier->primitiveType != primitiveType)
            Error(R_ConflictingPrimitiveTypes, true, false);
    }
    else
        return false;

    return true;
}


} // /namespace Xsc



// ================================================================================
