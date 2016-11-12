/*
 * Visitor.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "Visitor.h"
#include "AST.h"


namespace Xsc
{


#define IMPLEMENT_VISIT_PROC(AST_NAME) \
    void Visitor::Visit##AST_NAME(AST_NAME* ast, void* args)


Visitor::~Visitor()
{
    // dummy
}

IMPLEMENT_VISIT_PROC(Program)
{
    Visit(ast->globalStmnts);
}

IMPLEMENT_VISIT_PROC(CodeBlock)
{
    Visit(ast->stmnts);
}

IMPLEMENT_VISIT_PROC(BufferDecl)
{
    Visit(ast->arrayIndices);
}

IMPLEMENT_VISIT_PROC(SamplerDecl)
{
    Visit(ast->arrayIndices);
    Visit(ast->samplerValues);
}

IMPLEMENT_VISIT_PROC(FunctionCall)
{
    Visit(ast->name);
    Visit(ast->arguments);
}

IMPLEMENT_VISIT_PROC(Structure)
{
    Visit(ast->members);
}

IMPLEMENT_VISIT_PROC(SwitchCase)
{
    Visit(ast->expr);
    Visit(ast->stmnts);
}

IMPLEMENT_VISIT_PROC(SamplerValue)
{
    Visit(ast->value);
}

/* --- Variables --- */

IMPLEMENT_VISIT_PROC(PackOffset)
{
    // do nothing
}

IMPLEMENT_VISIT_PROC(VarSemantic)
{
    // do nothing
}

IMPLEMENT_VISIT_PROC(VarType)
{
    Visit(ast->structType);
}

IMPLEMENT_VISIT_PROC(VarIdent)
{
    Visit(ast->arrayIndices);
    Visit(ast->next);
}

IMPLEMENT_VISIT_PROC(VarDecl)
{
    Visit(ast->arrayDims);
    Visit(ast->semantics);
    Visit(ast->initializer);
}

/* --- Declaration statements --- */

IMPLEMENT_VISIT_PROC(FunctionDecl)
{
    Visit(ast->attribs);
    Visit(ast->returnType);
    Visit(ast->parameters);
    Visit(ast->codeBlock);
}

IMPLEMENT_VISIT_PROC(BufferDeclStmnt)
{
    Visit(ast->members);
}

IMPLEMENT_VISIT_PROC(TextureDeclStmnt)
{
    Visit(ast->bufferDecls);
}

IMPLEMENT_VISIT_PROC(SamplerDeclStmnt)
{
    Visit(ast->samplerDecls);
}

IMPLEMENT_VISIT_PROC(StructDeclStmnt)
{
    Visit(ast->structure);
}

/* --- Statements --- */

IMPLEMENT_VISIT_PROC(NullStmnt)
{
    // do nothing
}

IMPLEMENT_VISIT_PROC(CodeBlockStmnt)
{
    Visit(ast->codeBlock);
}

IMPLEMENT_VISIT_PROC(ForLoopStmnt)
{
    Visit(ast->attribs);
    Visit(ast->initSmnt);
    Visit(ast->condition);
    Visit(ast->iteration);
    Visit(ast->bodyStmnt);
}

IMPLEMENT_VISIT_PROC(WhileLoopStmnt)
{
    Visit(ast->attribs);
    Visit(ast->condition);
    Visit(ast->bodyStmnt);
}

IMPLEMENT_VISIT_PROC(DoWhileLoopStmnt)
{
    Visit(ast->attribs);
    Visit(ast->bodyStmnt);
    Visit(ast->condition);
}

IMPLEMENT_VISIT_PROC(IfStmnt)
{
    Visit(ast->attribs);
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
    Visit(ast->attribs);
    Visit(ast->selector);
    Visit(ast->cases);
}

IMPLEMENT_VISIT_PROC(VarDeclStmnt)
{
    Visit(ast->varType);
    Visit(ast->varDecls);
}

IMPLEMENT_VISIT_PROC(AssignStmnt)
{
    Visit(ast->varIdent);
    Visit(ast->expr);
}

IMPLEMENT_VISIT_PROC(ExprStmnt)
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

IMPLEMENT_VISIT_PROC(CtrlTransferStmnt)
{
    // do nothing
}

/* --- Expressions --- */

IMPLEMENT_VISIT_PROC(ListExpr)
{
    Visit(ast->firstExpr);
    Visit(ast->nextExpr);
}

IMPLEMENT_VISIT_PROC(LiteralExpr)
{
    // do nothing
}

IMPLEMENT_VISIT_PROC(TypeNameExpr)
{
    // do nothing
}

IMPLEMENT_VISIT_PROC(TernaryExpr)
{
    Visit(ast->condExpr);
    Visit(ast->thenExpr);
    Visit(ast->elseExpr);
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
    Visit(ast->varIdentSuffix);
}

IMPLEMENT_VISIT_PROC(BracketExpr)
{
    Visit(ast->expr);
    Visit(ast->varIdentSuffix);
}

IMPLEMENT_VISIT_PROC(CastExpr)
{
    Visit(ast->typeExpr);
    Visit(ast->expr);
}

IMPLEMENT_VISIT_PROC(VarAccessExpr)
{
    Visit(ast->varIdent);
    Visit(ast->assignExpr);
}

IMPLEMENT_VISIT_PROC(InitializerExpr)
{
    Visit(ast->exprs);
}


#undef IMPLEMENT_VISIT_PROC


} // /namespace Xsc



// ================================================================================