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
    className(const SourcePosition& astPos)                     \
    {                                                           \
        pos = astPos;                                           \
    }                                                           \
    Types Type() const override                                 \
    {                                                           \
        return Types::className;                                \
    }                                                           \
    void Visit(Visitor* visitor, void* args = nullptr) override \
    {                                                           \
        visitor->Visit##className(this, args);                  \
    }

#define DECL_AST_ALIAS(alias, base) \
    typedef base alias;             \
    typedef base##Ptr alias##Ptr

//! Base class for all AST node classes.
struct AST
{
    enum class Types
    {
        Program,
        CodeBlock,
        Terminal,
        TextureDeclIdent,

        FunctionDecl,
        BufferDecl,
        TextureDecl,
        SamplerStateDecl,
        StructDecl,
        DirectiveDecl,

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

        PackOffset,
        VarSemantic,
        VarType,
        VarIdent,
        VarDecl,
    };

    virtual ~AST()
    {
    }
    
    virtual Types Type() const = 0;
    virtual void Visit(Visitor* visitor, void* args = nullptr) = 0;

    SourcePosition pos;
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

//! Terminal symbol.
struct Terminal : public AST
{
    AST_INTERFACE(Terminal);
    std::string spell;
};

DECL_AST_ALIAS( Ident,    Terminal );
DECL_AST_ALIAS( Literal,  Terminal );
DECL_AST_ALIAS( BinaryOp, Terminal );
DECL_AST_ALIAS( UnaryOp,  Terminal );
DECL_AST_ALIAS( Semantic, Terminal );

//! Buffer declaration identifier.
struct TextureDeclIdent : public AST
{
    AST_INTERFACE(TextureDeclIdent);
    std::string ident;
    std::string registerName; // may be empty
};

/* --- Global declarations --- */

//! Function declaration.
struct FunctionDecl : public GlobalDecl
{
    AST_INTERFACE(FunctionDecl);
    VarTypePtr              returnType;
    std::string             name;
    std::vector<VarDeclPtr> parameters;
    SemanticPtr             semantic;
    CodeBlockPtr            codeBlock;
};

//! Buffer (cbuffer, tbuffer) declaration.
struct BufferDecl : public GlobalDecl
{
    AST_INTERFACE(BufferDecl);
    std::string             bufferType;
    std::string             name;
    std::string             registerName; // may be empty
    std::vector<VarDeclPtr> members;
};

//! Texture declaration.
struct TextureDecl : public GlobalDecl
{
    AST_INTERFACE(TextureDecl);
    std::string                         textureType;
    std::string                         genericType;
    std::vector<TextureDeclIdentPtr>    names;
};

//! Sampler state declaration.
struct SamplerStateDecl : public GlobalDecl
{
    AST_INTERFACE(SamplerStateDecl);
    std::vector<TextureDeclIdentPtr> names;
};

//! Structure declaration.
struct StructDecl : public GlobalDecl
{
    AST_INTERFACE(StructDecl);
    std::string             name;
    std::vector<VarDeclPtr> members;
};

//! Direvtive declaration.
struct DirectiveDecl : public GlobalDecl
{
    AST_INTERFACE(DirectiveDecl);
    std::string line;
};

/* --- Variables --- */

//! Pack offset.
struct PackOffset : public AST
{
    AST_INTERFACE(PackOffset);
    std::string registerName;
    std::string vectorComponent; // may be empty
};

//! Variable semantic.
struct VarSemantic : public AST
{
    AST_INTERFACE(VarSemantic);
    SemanticPtr     semantic;
    PackOffsetPtr   packOffset;
    std::string     registerName; // may be empty
};

//! Variable data type.
struct VarType : public AST
{
    AST_INTERFACE(VarType);
    std::string     baseType;   // either this ...
    StructDeclPtr   structType; // ... or this is used.
};

//! Variable (linked-list) identifier.
struct VarIdent : public AST
{
    AST_INTERFACE(VarIdent);
    std::string             ident;
    std::vector<ExprPtr>    arrayIndices;
    VarIdentPtr             next;
};

//! Variable declaration.
struct VarDecl : public AST
{
    AST_INTERFACE(VarDecl);
    std::string                 name;
    std::vector<ExprPtr>        arrayDims;
    std::vector<VarSemanticPtr> semantics;
    ExprPtr                     initializer;
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

//! Variable declaration statement.
struct VarDeclStmnt : public Stmnt
{
    AST_INTERFACE(VarDeclStmnt);
    std::vector<std::string>    storageClasses;
    std::vector<std::string>    interpModifiers;
    std::string                 typeModifier; // may be empty
    std::vector<VarDeclPtr>     varDecls;
};

/* --- Expressions --- */

//! Literal expression.
struct LiteralExpr : public Expr
{
    AST_INTERFACE(LiteralExpr);
    LiteralPtr literal;
};

//! Binary expressions.
struct BinaryExpr : public Expr
{
    AST_INTERFACE(BinaryExpr);
    ExprPtr     lhsExpr;    // left-hand-side expression
    BinaryOpPtr op;         // binary operator
    ExprPtr     rhsExpr;    // right-hand-side expression
};

//! (Pre-) Unary expressions.
struct UnaryExpr : public Expr
{
    AST_INTERFACE(UnaryExpr);
    UnaryOpPtr  op;
    ExprPtr     expr;
};

//! Post unary expressions.
struct PostUnaryExpr : public Expr
{
    AST_INTERFACE(PostUnaryExpr);
    ExprPtr     expr;
    UnaryOpPtr  op;
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
#undef DECL_AST_ALIAS


} // /namespace HTLib


#endif



// ================================================================================