/*
 * ReferenceAnalyzer.cpp
 * 
 * This file is part of the "HLSL Translator" (Copyright (c) 2014 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "ReferenceAnalyzer.h"
#include "HLSLTree.h"


namespace HTLib
{


ReferenceAnalyzer::ReferenceAnalyzer(const ASTSymbolTable& symTable) :
    symTable_{ &symTable }
{
}

void ReferenceAnalyzer::MarkFunctionsFromEntryPoint(FunctionDecl* ast)
{
    Visit(ast);
}


/*
 * ======= Private: =======
 */

/* ------- Visit functions ------- */

#define IMPLEMENT_VISIT_PROC(className) \
    void ReferenceAnalyzer::Visit##className(className* ast, void* args)

IMPLEMENT_VISIT_PROC(Program)
{
    for (auto& globDecl : ast->globalDecls)
        Visit(globDecl);
}

IMPLEMENT_VISIT_PROC(CodeBlock)
{
    for (auto& stmnt : ast->stmnts)
        Visit(stmnt);
}

IMPLEMENT_VISIT_PROC(FunctionCall)
{
    /* Mark this function to be used */
    auto symbol = symTable_->Fetch(FullVarIdent(ast->name));
    if (symbol && symbol->Type() == AST::Types::FunctionDecl)
    {
        symbol->flags << FunctionDecl::isUsed;

        /* Visit referenced function */
        Visit(symbol);
    }

    /* Visit arguments */
    for (auto& arg : ast->arguments)
        Visit(arg);
}

IMPLEMENT_VISIT_PROC(SwitchCase)
{
    Visit(ast->expr);
    for (auto& stmnt : ast->stmnts)
        Visit(stmnt);
}

/* --- Global declarations --- */

IMPLEMENT_VISIT_PROC(FunctionDecl)
{
    /* Check if this function was already marked by this analyzer */
    if (ast->flags(FunctionDecl::wasMarked))
        return;
    ast->flags << FunctionDecl::wasMarked;

    /* Analyze function */
    Visit(ast->returnType);
    for (auto& param : ast->parameters)
        Visit(param);
    Visit(ast->codeBlock);
}

/* --- Statements --- */

IMPLEMENT_VISIT_PROC(CodeBlockStmnt)
{
    Visit(ast->codeBlock);
}

IMPLEMENT_VISIT_PROC(ForLoopStmnt)
{
    Visit(ast->initSmnt);
    Visit(ast->condition);
    Visit(ast->iteration);
    Visit(ast->bodyStmnt);
}

IMPLEMENT_VISIT_PROC(WhileLoopStmnt)
{
    Visit(ast->condition);
    Visit(ast->bodyStmnt);
}

IMPLEMENT_VISIT_PROC(DoWhileLoopStmnt)
{
    Visit(ast->bodyStmnt);
    Visit(ast->condition);
}

IMPLEMENT_VISIT_PROC(IfStmnt)
{
    Visit(ast->condition);
    Visit(ast->bodyStmnt);
    Visit(ast->elseStmnt);
}

IMPLEMENT_VISIT_PROC(ElseStmnt)
{
    Visit(ast->bodyStmnt);
}

IMPLEMENT_VISIT_PROC(SwitchStmnt)
{
    Visit(ast->selector);

    for (auto& switchCase : ast->cases)
        Visit(switchCase);
}

IMPLEMENT_VISIT_PROC(VarDeclStmnt)
{
    for (auto& varDecl : ast->varDecls)
        Visit(varDecl);
}

IMPLEMENT_VISIT_PROC(AssignSmnt)
{
    Visit(ast->expr);
}

IMPLEMENT_VISIT_PROC(FunctionCallStmnt)
{
    Visit(ast->call);
}

IMPLEMENT_VISIT_PROC(ReturnStmnt)
{
    Visit(ast->expr);
}

/* --- Expressions --- */

IMPLEMENT_VISIT_PROC(ListExpr)
{
    Visit(ast->firstExpr);
    Visit(ast->nextExpr);
}

IMPLEMENT_VISIT_PROC(BinaryExpr)
{
    Visit(ast->lhsExpr);
    Visit(ast->rhsExpr);
}

IMPLEMENT_VISIT_PROC(UnaryExpr)
{
    Visit(ast->expr);
}

IMPLEMENT_VISIT_PROC(PostUnaryExpr)
{
    Visit(ast->expr);
}

IMPLEMENT_VISIT_PROC(FunctionCallExpr)
{
    Visit(ast->call);
}

IMPLEMENT_VISIT_PROC(BracketExpr)
{
    Visit(ast->expr);
}

IMPLEMENT_VISIT_PROC(CastExpr)
{
    Visit(ast->expr);
}

IMPLEMENT_VISIT_PROC(VarAccessExpr)
{
    Visit(ast->assignExpr);
}

IMPLEMENT_VISIT_PROC(InitializerExpr)
{
    for (auto& expr : ast->exprs)
        Visit(expr);
}

/* --- Variables --- */

IMPLEMENT_VISIT_PROC(VarIdent)
{
    for (auto& index : ast->arrayIndices)
        Visit(index);
    Visit(ast->next);
}

IMPLEMENT_VISIT_PROC(VarDecl)
{
    for (auto& dim : ast->arrayDims)
        Visit(dim);
    Visit(ast->initializer);
}

#undef IMPLEMENT_VISIT_PROC


} // /namespace HTLib



// ================================================================================