/*
 * Visitor.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "Visitor.h"
#include "AST.h"
#include "ReportIdents.h"


namespace Xsc
{


#define IMPLEMENT_VISIT_PROC(AST_NAME) \
    void Visitor::Visit##AST_NAME(AST_NAME* ast, void* args)


IMPLEMENT_VISIT_PROC(Program)
{
    Visit(ast->globalStmts);
}

IMPLEMENT_VISIT_PROC(CodeBlock)
{
    Visit(ast->stmts);
}

IMPLEMENT_VISIT_PROC(Attribute)
{
    Visit(ast->arguments);
}

IMPLEMENT_VISIT_PROC(SwitchCase)
{
    Visit(ast->expr);
    Visit(ast->stmts);
}

IMPLEMENT_VISIT_PROC(SamplerValue)
{
    Visit(ast->value);
}

IMPLEMENT_VISIT_PROC(Register)
{
    // do nothing
}

IMPLEMENT_VISIT_PROC(PackOffset)
{
    // do nothing
}

IMPLEMENT_VISIT_PROC(ArrayDimension)
{
    Visit(ast->expr);
}

IMPLEMENT_VISIT_PROC(TypeSpecifier)
{
    Visit(ast->structDecl);
}

/* --- Declarations --- */

IMPLEMENT_VISIT_PROC(VarDecl)
{
    Visit(ast->namespaceExpr);
    Visit(ast->arrayDims);
    Visit(ast->slotRegisters);
    Visit(ast->packOffset);
    Visit(ast->annotations);
    Visit(ast->initializer);
}

IMPLEMENT_VISIT_PROC(BufferDecl)
{
    Visit(ast->arrayDims);
    Visit(ast->slotRegisters);
    Visit(ast->annotations);
}

IMPLEMENT_VISIT_PROC(SamplerDecl)
{
    Visit(ast->arrayDims);
    Visit(ast->slotRegisters);
    Visit(ast->samplerValues);
}

IMPLEMENT_VISIT_PROC(StructDecl)
{
    Visit(ast->localStmts);
}

IMPLEMENT_VISIT_PROC(AliasDecl)
{
    // do nothing
}

IMPLEMENT_VISIT_PROC(FunctionDecl)
{
    Visit(ast->returnType);
    Visit(ast->parameters);
    Visit(ast->annotations);
    Visit(ast->codeBlock);
}

IMPLEMENT_VISIT_PROC(UniformBufferDecl)
{
    Visit(ast->slotRegisters);
    Visit(ast->localStmts);
}

/* --- Declaration statements --- */

IMPLEMENT_VISIT_PROC(BufferDeclStmt)
{
    Visit(ast->attribs);
    Visit(ast->bufferDecls);
}

IMPLEMENT_VISIT_PROC(SamplerDeclStmt)
{
    Visit(ast->attribs);
    Visit(ast->samplerDecls);
}

IMPLEMENT_VISIT_PROC(VarDeclStmt)
{
    Visit(ast->attribs);
    Visit(ast->typeSpecifier);
    Visit(ast->varDecls);
}

IMPLEMENT_VISIT_PROC(AliasDeclStmt)
{
    Visit(ast->attribs);
    Visit(ast->structDecl);
    Visit(ast->aliasDecls);
}

IMPLEMENT_VISIT_PROC(BasicDeclStmt)
{
    Visit(ast->attribs);
    Visit(ast->declObject);
}

/* --- Statements --- */

IMPLEMENT_VISIT_PROC(NullStmt)
{
    Visit(ast->attribs);
}

IMPLEMENT_VISIT_PROC(ScopeStmt)
{
    Visit(ast->attribs);
    Visit(ast->codeBlock);
}

IMPLEMENT_VISIT_PROC(ForStmt)
{
    Visit(ast->attribs);
    Visit(ast->initStmt);
    Visit(ast->condition);
    Visit(ast->iteration);
    Visit(ast->bodyStmt);
}

IMPLEMENT_VISIT_PROC(WhileStmt)
{
    Visit(ast->attribs);
    Visit(ast->condition);
    Visit(ast->bodyStmt);
}

IMPLEMENT_VISIT_PROC(DoWhileStmt)
{
    Visit(ast->attribs);
    Visit(ast->bodyStmt);
    Visit(ast->condition);
}

IMPLEMENT_VISIT_PROC(IfStmt)
{
    Visit(ast->attribs);
    Visit(ast->condition);
    Visit(ast->bodyStmt);
    Visit(ast->elseStmt);
}

IMPLEMENT_VISIT_PROC(SwitchStmt)
{
    Visit(ast->attribs);
    Visit(ast->selector);
    Visit(ast->cases);
}

IMPLEMENT_VISIT_PROC(ExprStmt)
{
    Visit(ast->attribs);
    Visit(ast->expr);
}

IMPLEMENT_VISIT_PROC(ReturnStmt)
{
    Visit(ast->attribs);
    Visit(ast->expr);
}

IMPLEMENT_VISIT_PROC(JumpStmt)
{
    Visit(ast->attribs);
}

IMPLEMENT_VISIT_PROC(LayoutStmt)
{
    Visit(ast->attribs);
}

/* --- Expressions --- */

IMPLEMENT_VISIT_PROC(NullExpr)
{
    // do nothing
}

IMPLEMENT_VISIT_PROC(SequenceExpr)
{
    Visit(ast->exprs);
}

IMPLEMENT_VISIT_PROC(LiteralExpr)
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

IMPLEMENT_VISIT_PROC(CallExpr)
{
    Visit(ast->prefixExpr);
    Visit(ast->arguments);
}

IMPLEMENT_VISIT_PROC(BracketExpr)
{
    Visit(ast->expr);
}

IMPLEMENT_VISIT_PROC(AssignExpr)
{
    Visit(ast->lvalueExpr);
    Visit(ast->rvalueExpr);
}

IMPLEMENT_VISIT_PROC(IdentExpr)
{
    Visit(ast->prefixExpr);
}

IMPLEMENT_VISIT_PROC(SubscriptExpr)
{
    Visit(ast->prefixExpr);
    Visit(ast->arrayIndices);
}

IMPLEMENT_VISIT_PROC(CastExpr)
{
    Visit(ast->typeSpecifier);
    Visit(ast->expr);
}

IMPLEMENT_VISIT_PROC(InitializerExpr)
{
    Visit(ast->exprs);
}

IMPLEMENT_VISIT_PROC(ExprProxy)
{
    // do not visit sub nodes in proxy AST nodes
}

#undef IMPLEMENT_VISIT_PROC


} // /namespace Xsc



// ================================================================================