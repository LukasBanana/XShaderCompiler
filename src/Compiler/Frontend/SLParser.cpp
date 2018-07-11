/*
 * SLParser.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "SLParser.h"
#include "ExprEvaluator.h"
#include "Helper.h"
#include "AST.h"
#include "ASTFactory.h"
#include "ReportIdents.h"
#include "Exception.h"


namespace Xsc
{


SLParser::SLParser(Log* log) :
    Parser { log }
{
}


/*
 * ======= Private: =======
 */

void SLParser::Semi()
{
    Accept(Tokens::Semicolon);
}

/* ------- Parse functions ------- */

ArrayDimensionPtr SLParser::ParseArrayDimension(bool allowDynamicDimension)
{
    auto ast = Make<ArrayDimension>();

    Accept(Tokens::LParen);

    if (Is(Tokens::RParen))
    {
        if (!allowDynamicDimension)
            Error(R_ExpectedExplicitArrayDim, false);
        ast->expr = Make<NullExpr>();
    }
    else
        ast->expr = ParseExpr();

    Accept(Tokens::RParen);

    return UpdateSourceArea(ast);
}

/* --- Statements --- */

NullStmntPtr SLParser::ParseNullStmnt()
{
    /* Parse null statement */
    auto ast = Make<NullStmnt>();
    Semi();
    return ast;
}

CodeBlockStmntPtr SLParser::ParseCodeBlockStmnt()
{
    /* Parse code block statement */
    auto ast = Make<CodeBlockStmnt>();
    ast->codeBlock = ParseCodeBlock();
    return ast;
}

ForLoopStmntPtr SLParser::ParseForLoopStmnt()
{
    auto ast = Make<ForLoopStmnt>();

    /* Parse loop initializer statement (attributes not allowed here) */
    Accept(Tokens::For);
    Accept(Tokens::LBracket);

    ast->initStmnt = ParseForLoopInitializer();

    /* Parse loop condExpr */
    if (!Is(Tokens::Semicolon))
        ast->condition = ParseExprWithSequenceOpt();
    Semi();

    /* Parse loop iteration */
    if (!Is(Tokens::RBracket))
        ast->iteration = ParseExprWithSequenceOpt();
    Accept(Tokens::RBracket);

    /* Parse loop body */
    ast->bodyStmnt = ParseLocalStmnt();

    return ast;
}

WhileLoopStmntPtr SLParser::ParseWhileLoopStmnt()
{
    auto ast = Make<WhileLoopStmnt>();

    /* Parse loop condExpr */
    Accept(Tokens::While);

    Accept(Tokens::LBracket);
    ast->condition = ParseExprWithSequenceOpt();
    Accept(Tokens::RBracket);

    /* Parse loop body */
    ast->bodyStmnt = ParseLocalStmnt();

    return ast;
}

DoWhileLoopStmntPtr SLParser::ParseDoWhileLoopStmnt()
{
    auto ast = Make<DoWhileLoopStmnt>();

    /* Parse loop body */
    Accept(Tokens::Do);
    ast->bodyStmnt = ParseLocalStmnt();

    /* Parse loop condExpr */
    Accept(Tokens::While);

    Accept(Tokens::LBracket);
    ast->condition = ParseExprWithSequenceOpt();
    Accept(Tokens::RBracket);

    Semi();

    return ast;
}

IfStmntPtr SLParser::ParseIfStmnt()
{
    auto ast = Make<IfStmnt>();

    /* Parse if condExpr */
    Accept(Tokens::If);

    Accept(Tokens::LBracket);
    ast->condition = ParseExprWithSequenceOpt();
    Accept(Tokens::RBracket);

    /* Parse if body */
    ast->bodyStmnt = ParseLocalStmnt();

    /* Parse optional else statement */
    if (Is(Tokens::Else))
        ast->elseStmnt = ParseElseStmnt();

    return ast;
}

ElseStmntPtr SLParser::ParseElseStmnt()
{
    /* Parse else statment */
    auto ast = Make<ElseStmnt>();

    Accept(Tokens::Else);
    ast->bodyStmnt = ParseLocalStmnt();

    return ast;
}

SwitchStmntPtr SLParser::ParseSwitchStmnt()
{
    auto ast = Make<SwitchStmnt>();

    /* Parse switch selector */
    Accept(Tokens::Switch);

    Accept(Tokens::LBracket);
    ast->selector = ParseExprWithSequenceOpt();
    Accept(Tokens::RBracket);

    /* Parse switch cases */
    Accept(Tokens::LCurly);
    ast->cases = ParseSwitchCaseList();
    Accept(Tokens::RCurly);

    return ast;
}

CtrlTransferStmntPtr SLParser::ParseCtrlTransferStmnt()
{
    /* Parse control transfer statement */
    auto ast = Make<CtrlTransferStmnt>();

    auto ctrlTransfer = Accept(Tokens::CtrlTransfer)->Spell();
    ast->transfer = StringToCtrlTransfer(ctrlTransfer);

    UpdateSourceArea(ast);

    Semi();

    return ast;
}

ReturnStmntPtr SLParser::ParseReturnStmnt()
{
    auto ast = Make<ReturnStmnt>();

    Accept(Tokens::Return);

    if (!Is(Tokens::Semicolon))
        ast->expr = ParseExprWithSequenceOpt();

    UpdateSourceArea(ast);

    Semi();

    return ast;
}

ExprStmntPtr SLParser::ParseExprStmnt(const ExprPtr& expr)
{
    /* Parse expression statement */
    auto ast = Make<ExprStmnt>();

    if (expr)
    {
        ast->expr = expr;
        ast->area = expr->area;
    }
    else
        ast->expr = ParseExprWithSequenceOpt();

    Semi();

    return UpdateSourceArea(ast);
}

/* --- Expressions --- */

ExprPtr SLParser::ParseExpr()
{
    /* Parse generic expression, then post expression */
    return ParseGenericExpr();
}

ExprPtr SLParser::ParseExprWithSequenceOpt()
{
    /* Parse generic expression, then post expression */
    auto ast = ParseExpr();

    /* Parse optional sequence expression */
    if (Is(Tokens::Comma))
        return ParseSequenceExpr(ast);

    return ast;
}

ExprPtr SLParser::ParseArrayIndex()
{
    auto area = Tkn()->Area();

    Accept(Tokens::LParen);

    auto ast = ParseExpr();
    ast->area = area;

    Accept(Tokens::RParen);

    return UpdateSourceArea(ast);
}

ExprPtr SLParser::ParseInitializer()
{
    Accept(Tokens::AssignOp, "=");
    return ParseExpr();
}

SequenceExprPtr SLParser::ParseSequenceExpr(const ExprPtr& firstExpr)
{
    auto ast = Make<SequenceExpr>();

    /* Parse first expression */
    if (firstExpr)
        ast->Append(firstExpr);
    else
        ast->Append(ParseExpr());

    Accept(Tokens::Comma);

    /* Parse further sub expressions in sequence */
    ast->Append(ParseExprWithSequenceOpt());

    return ast;
}

ArrayExprPtr SLParser::ParseArrayExpr(const ExprPtr& expr)
{
    auto ast = Make<ArrayExpr>();

    /* Take sub expression and parse array dimensions */
    ast->prefixExpr     = expr;
    ast->arrayIndices   = ParseArrayIndexList();

    return UpdateSourceArea(ast, expr.get());
}

InitializerExprPtr SLParser::ParseInitializerExpr()
{
    /* Parse initializer list expression */
    auto ast = Make<InitializerExpr>();
    ast->exprs = ParseInitializerList();
    return UpdateSourceArea(ast);
}

/* --- Lists --- */

std::vector<VarDeclPtr> SLParser::ParseVarDeclList(VarDeclStmnt* declStmntRef, TokenPtr firstIdentTkn)
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

std::vector<VarDeclStmntPtr> SLParser::ParseParameterList()
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

std::vector<StmntPtr> SLParser::ParseLocalStmntList()
{
    std::vector<StmntPtr> stmnts;

    while (!Is(Tokens::RCurly))
    {
        ParseStmntWithCommentOpt(
            stmnts,
            [this]()
            {
                return this->ParseLocalStmnt();
            }
        );
    }

    return stmnts;
}

std::vector<ExprPtr> SLParser::ParseExprList(const Tokens listTerminatorToken, bool allowLastComma)
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

std::vector<ArrayDimensionPtr> SLParser::ParseArrayDimensionList(bool allowDynamicDimension)
{
    std::vector<ArrayDimensionPtr> arrayDims;

    while (Is(Tokens::LParen))
        arrayDims.push_back(ParseArrayDimension(allowDynamicDimension));

    return arrayDims;
}

std::vector<ExprPtr> SLParser::ParseArrayIndexList()
{
    std::vector<ExprPtr> exprs;

    while (Is(Tokens::LParen))
        exprs.push_back(ParseArrayIndex());

    return exprs;
}

std::vector<ExprPtr> SLParser::ParseArgumentList()
{
    Accept(Tokens::LBracket);
    auto exprs = ParseExprList(Tokens::RBracket);
    Accept(Tokens::RBracket);
    return exprs;
}

std::vector<ExprPtr> SLParser::ParseInitializerList()
{
    Accept(Tokens::LCurly);
    auto exprs = ParseExprList(Tokens::RCurly, true);
    Accept(Tokens::RCurly);
    return exprs;
}

std::vector<SwitchCasePtr> SLParser::ParseSwitchCaseList()
{
    std::vector<SwitchCasePtr> cases;

    while (Is(Tokens::Case) || Is(Tokens::Default))
        cases.push_back(ParseSwitchCase());

    return cases;
}

/* --- Others --- */

std::string SLParser::ParseIdent(TokenPtr identTkn, SourceArea* area)
{
    /* Parse identifier */
    if (!identTkn)
        identTkn = Accept(Tokens::Ident);

    auto ident = identTkn->Spell();

    /* Return source area of identifier */
    if (area)
        *area = identTkn->Area();

    /* Check overlapping of reserved prefixes for name mangling */
    if (auto prefix = FindNameManglingPrefix(ident))
        Error(R_IdentNameManglingConflict(ident, *prefix), identTkn.get(), false);

    return ident;
}

TypeDenoterPtr SLParser::ParseTypeDenoterWithArrayOpt(const TypeDenoterPtr& baseTypeDenoter)
{
    if (Is(Tokens::LParen))
    {
        auto arrayTypeDenoter = std::make_shared<ArrayTypeDenoter>(baseTypeDenoter);

        /* Parse array dimension list */
        arrayTypeDenoter->arrayDims = ParseArrayDimensionList();

        return arrayTypeDenoter;
    }
    return baseTypeDenoter;
}

VoidTypeDenoterPtr SLParser::ParseVoidTypeDenoter()
{
    Accept(Tokens::Void);
    return std::make_shared<VoidTypeDenoter>();
}

Variant SLParser::ParseAndEvaluateConstExpr()
{
    /* Parse expression */
    auto tkn = Tkn();
    auto expr = ParseExpr();

    try
    {
        /* Evaluate expression and throw error on object access */
        ExprEvaluator exprEvaluator;
        return exprEvaluator.Evaluate(*expr, [](ObjectExpr* expr) -> Variant { throw expr; });
    }
    catch (const std::exception& e)
    {
        Error(e.what(), tkn.get());
    }
    catch (const ObjectExpr* expr)
    {
        GetReportHandler().SubmitReport(
            true, ReportTypes::Error, R_SyntaxError,
            R_ExpectedConstExpr, GetScanner().Source(), expr->area
        );
    }

    return Variant();
}

int SLParser::ParseAndEvaluateConstExprInt()
{
    auto tkn = Tkn();
    auto value = ParseAndEvaluateConstExpr();

    if (value.Type() != Variant::Types::Int)
        Error(R_ExpectedConstIntExpr, tkn.get());

    return static_cast<int>(value.Int());
}

int SLParser::ParseAndEvaluateVectorDimension()
{
    auto tkn = Tkn();
    auto value = ParseAndEvaluateConstExprInt();

    if (value < 1 || value > 4)
        Error(R_VectorAndMatrixDimOutOfRange(value), tkn.get());

    return value;
}

void SLParser::ParseStmntWithCommentOpt(std::vector<StmntPtr>& stmnts, const std::function<StmntPtr()>& parseFunction)
{
    /* Parse next statement with optional commentary */
    auto comment = GetScanner().GetComment();

    auto ast = parseFunction();
    stmnts.push_back(ast);

    ast->comment = std::move(comment);
}


} // /namespace Xsc



// ================================================================================
