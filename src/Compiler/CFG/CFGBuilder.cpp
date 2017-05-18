/*
 * CFGBuilder.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "CFGBuilder.h"


namespace Xsc
{


/*CFGBuilder::CFGBuilder(Log* log) :
    Generator { log }
{
}*/


/*
 * ======= Private: =======
 */


/* ------- Visit functions ------- */

#define IMPLEMENT_VISIT_PROC(AST_NAME) \
    void CFGBuilder::Visit##AST_NAME(AST_NAME* ast, void* args)

IMPLEMENT_VISIT_PROC(Program)
{
    //TODO
}

IMPLEMENT_VISIT_PROC(CodeBlock)
{
    //TODO
}

IMPLEMENT_VISIT_PROC(SwitchCase)
{
    //TODO
}

IMPLEMENT_VISIT_PROC(ArrayDimension)
{
    //TODO
}

IMPLEMENT_VISIT_PROC(TypeSpecifier)
{
    //TODO
}

/* --- Declarations --- */

IMPLEMENT_VISIT_PROC(VarDecl)
{
    //TODO
}

IMPLEMENT_VISIT_PROC(StructDecl)
{
    //TODO
}

IMPLEMENT_VISIT_PROC(SamplerDecl)
{
    //TODO
}

/* --- Declaration statements --- */

IMPLEMENT_VISIT_PROC(FunctionDecl)
{
    //TODO
}

IMPLEMENT_VISIT_PROC(UniformBufferDecl)
{
    //TODO
}

IMPLEMENT_VISIT_PROC(BufferDeclStmnt)
{
    //TODO
}

IMPLEMENT_VISIT_PROC(SamplerDeclStmnt)
{
    //TODO
}

IMPLEMENT_VISIT_PROC(VarDeclStmnt)
{
    //TODO
}

IMPLEMENT_VISIT_PROC(AliasDeclStmnt)
{
    //TODO
}

IMPLEMENT_VISIT_PROC(BasicDeclStmnt)
{
    //TODO
}

/* --- Statements --- */

IMPLEMENT_VISIT_PROC(NullStmnt)
{
    //TODO
}

IMPLEMENT_VISIT_PROC(CodeBlockStmnt)
{
    //TODO
}

IMPLEMENT_VISIT_PROC(ForLoopStmnt)
{
    //TODO
}

IMPLEMENT_VISIT_PROC(WhileLoopStmnt)
{
    //TODO
}

IMPLEMENT_VISIT_PROC(DoWhileLoopStmnt)
{
    //TODO
}

IMPLEMENT_VISIT_PROC(IfStmnt)
{
    //TODO
}

IMPLEMENT_VISIT_PROC(ElseStmnt)
{
    //TODO
}

IMPLEMENT_VISIT_PROC(SwitchStmnt)
{
    //TODO
}

IMPLEMENT_VISIT_PROC(ExprStmnt)
{
    //TODO
}

IMPLEMENT_VISIT_PROC(ReturnStmnt)
{
    //TODO
}

IMPLEMENT_VISIT_PROC(CtrlTransferStmnt)
{
    //TODO
}

/* --- Expressions --- */

IMPLEMENT_VISIT_PROC(SequenceExpr)
{
    //TODO
}

IMPLEMENT_VISIT_PROC(LiteralExpr)
{
    //TODO
}

IMPLEMENT_VISIT_PROC(TypeSpecifierExpr)
{
    //TODO
}

IMPLEMENT_VISIT_PROC(TernaryExpr)
{
    //TODO
}

IMPLEMENT_VISIT_PROC(BinaryExpr)
{
    //TODO
}

IMPLEMENT_VISIT_PROC(UnaryExpr)
{
    //TODO
}

IMPLEMENT_VISIT_PROC(PostUnaryExpr)
{
    //TODO
}

IMPLEMENT_VISIT_PROC(CallExpr)
{
    //TODO
}

IMPLEMENT_VISIT_PROC(BracketExpr)
{
    //TODO
}

IMPLEMENT_VISIT_PROC(ObjectExpr)
{
    //TODO
}

IMPLEMENT_VISIT_PROC(AssignExpr)
{
    //TODO
}

IMPLEMENT_VISIT_PROC(ArrayExpr)
{
    //TODO
}

IMPLEMENT_VISIT_PROC(CastExpr)
{
    //TODO
}

IMPLEMENT_VISIT_PROC(InitializerExpr)
{
    //TODO
}

#undef IMPLEMENT_VISIT_PROC

/* --- Helper functions for code generation --- */


} // /namespace Xsc



// ================================================================================
