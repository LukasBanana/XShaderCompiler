/*
 * HLSLTree.h
 * 
 * This file is part of the "HLSL Translator" (Copyright (c) 2014 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __HT_HLSL_TREE_H__
#define __HT_HLSL_TREE_H__


#include "Token.h"
#include "Visitor.h"

#include <vector>
#include <string>


namespace HTLib
{


class Visitor;

#define AST_INTERFACE(className)                                \
    Types Type() const override                                 \
    {                                                           \
        return Types::className;                                \
    }                                                           \
    void Visit(Visitor* visitor, void* args = nullptr) override \
    {                                                           \
        visitor->Visit##className(this, args);                  \
    }

//! Base class for all AST node classes.
struct AST
{
    enum class Types
    {
        Program,
        CodeBlock,

        FunctionDecl,
        BufferDecl,
        TextureDecl,
        SamplerStateDecl,
        StructDecl,

        CodeBlockStmnt,
        ForLoopStmnt,
        WhileLoopStmnt,
        DoWhileLoopStmnt,
        IfStmnt,
        ElseStmnt,
        SwitchStmnt,
        VarDeclStmnt,
        AssignSmnt,
        FunctionCallStmnt,
        ReturnStmnt,
        StructDeclStmnt,

        LiteralExpr,
        BinaryExpr,
        UnaryExpr,
        PostUnaryExpr,
        FunctionCallExpr,
        BracketExpr,
        CastExpr,
        VarAccessExpr,

        SwitchCase,
    };

    virtual ~AST()
    {
    }
    
    virtual Types Type() const = 0;
    virtual void Visit(Visitor* visitor, void* args = nullptr) = 0;
};

/* --- Base AST nodes --- */

//! Global declaration base class.
struct GlobalDecl : public AST {};

//! Statement base class.
struct Stmnt : public AST {};

//! Expression base class.
struct Expr : public AST {};

//! Program AST root.
struct Program : public AST
{
    AST_INTERFACE(Program)
    std::vector<GlobalDeclPtr> globalDecls;
};

//! Code block.
struct CodeBlock : public AST
{
    AST_INTERFACE(CodeBlock);
    std::vector<StmntPtr> stmnts;
};

/* --- Statements --- */

//! Code block statement.
struct CodeBlockStmnt : public Stmnt
{
    AST_INTERFACE(CodeBlockStmnt);
    CodeBlockPtr codeBlock;
};

//! 'for'-loop statemnet.
struct ForLoopStmnt : public Stmnt
{
    AST_INTERFACE(ForLoopStmnt);
    AssignSmntPtr   initAssign;
    VarDeclStmntPtr initVarDecl;
    ExprPtr         condition;
    ExprPtr         iteration;
};

//! 'while'-loop statement.
struct WhileLoopStmnt : public Stmnt
{
    AST_INTERFACE(WhileLoopStmnt);
    ExprPtr         condition;
    CodeBlockPtr    codeBlock;
};

//! 'do/while'-loop statement.
struct DoWhileLoopStmnt : public Stmnt
{
    AST_INTERFACE(DoWhileLoopStmnt);
    CodeBlockPtr    codeBlock;
    ExprPtr         condition;
};

//! 'if' statement.
struct IfStmnt : public Stmnt
{
    AST_INTERFACE(IfStmnt);
    ExprPtr         condition;
    CodeBlockPtr    codeBlock;
};

//! 'else' statement.
struct ElseStmnt : public Stmnt
{
    AST_INTERFACE(ElseStmnt);
    CodeBlockPtr codeBlock;
};

//! 'switch' statement.
struct SwitchStmnt : public Stmnt
{
    AST_INTERFACE(SwitchStmnt);
    ExprPtr expr;
    std::vector<SwitchCasePtr> cases;
};

//...

/* --- Expressions --- */

//! Literal expression.
struct LiteralExpr : public Expr
{
    AST_INTERFACE(LiteralExpr);
    TokenPtr literal;
};

//! Binary expressions.
struct BinaryExpr : public Expr
{
    AST_INTERFACE(BinaryExpr);
    ExprPtr     lhsExpr;    // left-hand-side expression
    TokenPtr    op;         // binary operator
    ExprPtr     rhsExpr;    // right-hand-side expression
};

//! (Pre-) Unary expressions.
struct UnaryExpr : public Expr
{
    AST_INTERFACE(UnaryExpr);
    TokenPtr    op;
    ExprPtr     expr;
};

//! Post unary expressions.
struct PostUnaryExpr : public Expr
{
    AST_INTERFACE(PostUnaryExpr);
    ExprPtr     expr;
    TokenPtr    op;
};

//! Function call expression.
struct FunctionCallExpr : public Expr
{
    AST_INTERFACE(FunctionCallExpr);
    //...
};

//! Bracket expression.
struct BracketExpr : public Expr
{
    AST_INTERFACE(BracketExpr);
    ExprPtr expr; // inner expression
};

//...

/* --- Others --- */

struct SwitchCase : public AST
{
    AST_INTERFACE(SwitchCase);
    ExprPtr                 expr; // if null -> default case
    std::vector<StmntPtr>   stmnts;
};

#undef AST_INTERFACE


} // /namespace HTLib


#endif



// ================================================================================