/*
 * Visitor.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "Visitor.h"
#include "AST.h"
#include "ReportIdents.h"


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

IMPLEMENT_VISIT_PROC(Attribute)
{
    Visit(ast->arguments);
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
    Visit(ast->localStmnts);
}

IMPLEMENT_VISIT_PROC(AliasDecl)
{
    // do nothing
}

/* --- Declaration statements --- */

IMPLEMENT_VISIT_PROC(FunctionDecl)
{
    Visit(ast->returnType);
    Visit(ast->parameters);
    Visit(ast->annotations);
    Visit(ast->codeBlock);
}

IMPLEMENT_VISIT_PROC(UniformBufferDecl)
{
    Visit(ast->attribs);
    Visit(ast->slotRegisters);
    Visit(ast->localStmnts);
}

IMPLEMENT_VISIT_PROC(BufferDeclStmnt)
{
    Visit(ast->attribs);
    Visit(ast->bufferDecls);
}

IMPLEMENT_VISIT_PROC(SamplerDeclStmnt)
{
    Visit(ast->attribs);
    Visit(ast->samplerDecls);
}

#if 1//TODO: replace by "BasicDeclStmnt"

IMPLEMENT_VISIT_PROC(StructDeclStmnt)
{
    Visit(ast->attribs);
    Visit(ast->structDecl);
}

#endif

IMPLEMENT_VISIT_PROC(VarDeclStmnt)
{
    Visit(ast->attribs);
    Visit(ast->typeSpecifier);
    Visit(ast->varDecls);
}

IMPLEMENT_VISIT_PROC(AliasDeclStmnt)
{
    Visit(ast->attribs);
    Visit(ast->structDecl);
    Visit(ast->aliasDecls);
}

IMPLEMENT_VISIT_PROC(BasicDeclStmnt)
{
    Visit(ast->attribs);
    Visit(ast->declObject);
}

/* --- Statements --- */

IMPLEMENT_VISIT_PROC(NullStmnt)
{
    Visit(ast->attribs);
}

IMPLEMENT_VISIT_PROC(CodeBlockStmnt)
{
    Visit(ast->attribs);
    Visit(ast->codeBlock);
}

IMPLEMENT_VISIT_PROC(ForLoopStmnt)
{
    Visit(ast->attribs);
    Visit(ast->initStmnt);
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
    Visit(ast->attribs);
    Visit(ast->bodyStmnt);
}

IMPLEMENT_VISIT_PROC(SwitchStmnt)
{
    Visit(ast->attribs);
    Visit(ast->selector);
    Visit(ast->cases);
}

IMPLEMENT_VISIT_PROC(ExprStmnt)
{
    Visit(ast->attribs);
    Visit(ast->expr);
}

IMPLEMENT_VISIT_PROC(ReturnStmnt)
{
    Visit(ast->attribs);
    Visit(ast->expr);
}

IMPLEMENT_VISIT_PROC(CtrlTransferStmnt)
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

IMPLEMENT_VISIT_PROC(TypeSpecifierExpr)
{
    Visit(ast->typeSpecifier);
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

IMPLEMENT_VISIT_PROC(ObjectExpr)
{
    Visit(ast->prefixExpr);
}

IMPLEMENT_VISIT_PROC(ArrayExpr)
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

#undef IMPLEMENT_VISIT_PROC


} // /namespace Xsc



// ================================================================================