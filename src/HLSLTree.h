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
#include "Flags.h"

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

#define FLAGS           \
    Flags flags;        \
    enum : unsigned int

//! Base class for all AST node classes.
struct AST
{
    enum class Types
    {
        Program,
        CodeBlock,
        BufferDeclIdent,
        FunctionCall,
        Structure,

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
        CtrlTransferStmnt,

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

//! Buffer declaration identifier.
struct BufferDeclIdent : public AST
{
    AST_INTERFACE(BufferDeclIdent);
    std::string ident;
    std::string registerName; // may be empty
};

//! Function call.
struct FunctionCall : public AST
{
    AST_INTERFACE(FunctionCall);
    std::string             name;
    std::vector<ExprPtr>    arguments;
};

//! Structure object.
struct Structure : public AST
{
    AST_INTERFACE(Structure);
    std::string                     name;
    std::vector<VarDeclStmntPtr>    members;
};

/* --- Global declarations --- */

//! Function declaration.
struct FunctionDecl : public GlobalDecl
{
    FLAGS
    {
        isEntryPoint    = (1 << 0), // This function is the main entry point.
        isUsed          = (1 << 1), // This function is used at least once (use-count >= 1).
    };
    AST_INTERFACE(FunctionDecl);
    std::vector<FunctionCallPtr>    attribs; // attribute list
    VarTypePtr                      returnType;
    std::string                     name;
    std::vector<VarDeclStmntPtr>    parameters;
    std::string                     semantic;
    CodeBlockPtr                    codeBlock;
};

//! Buffer (cbuffer, tbuffer) declaration.
struct BufferDecl : public GlobalDecl
{
    AST_INTERFACE(BufferDecl);
    std::string                     bufferType;
    std::string                     name;
    std::string                     registerName; // may be empty
    std::vector<VarDeclStmntPtr>    members;
};

//! Texture declaration.
struct TextureDecl : public GlobalDecl
{
    AST_INTERFACE(TextureDecl);
    std::string                     textureType;
    std::string                     genericType;
    std::vector<BufferDeclIdentPtr> names;
};

//! Sampler state declaration.
struct SamplerStateDecl : public GlobalDecl
{
    AST_INTERFACE(SamplerStateDecl);
    std::vector<BufferDeclIdentPtr> names;
};

//! Structure declaration.
struct StructDecl : public GlobalDecl
{
    FLAGS
    {
        isShaderInput   = (1 << 0), // This structure is used as shader input.
        isShaderOutput  = (1 << 1), // This structure is used as shader output.
    };
    AST_INTERFACE(StructDecl);
    StructurePtr structure;
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
    std::string     semantic;
    PackOffsetPtr   packOffset;
    std::string     registerName; // may be empty
};

//! Variable data type.
struct VarType : public AST
{
    AST_INTERFACE(VarType);
    std::string     baseType;   // either this ...
    StructurePtr    structType; // ... or this is used.
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
    std::vector<FunctionCallPtr>    attribs; // attribute list
    AssignSmntPtr                   initAssign;
    VarDeclStmntPtr                 initVarDecl;
    ExprPtr                         condition;
    ExprPtr                         iteration;
};

//! 'while'-loop statement.
struct WhileLoopStmnt : public Stmnt
{
    AST_INTERFACE(WhileLoopStmnt);
    std::vector<FunctionCallPtr>    attribs; // attribute list
    ExprPtr                         condition;
    CodeBlockPtr                    codeBlock;
};

//! 'do/while'-loop statement.
struct DoWhileLoopStmnt : public Stmnt
{
    AST_INTERFACE(DoWhileLoopStmnt);
    std::vector<FunctionCallPtr>    attribs; // attribute list
    CodeBlockPtr                    codeBlock;
    ExprPtr                         condition;
};

//! 'if' statement.
struct IfStmnt : public Stmnt
{
    AST_INTERFACE(IfStmnt);
    std::vector<FunctionCallPtr>    attribs; // attribute list
    ExprPtr                         condition;
    CodeBlockPtr                    codeBlock;
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
    std::vector<std::string>    storageModifiers; // storage classes or interpolation modifiers
    std::string                 typeModifier; // may be empty
    VarTypePtr                  varType;
    std::vector<VarDeclPtr>     varDecls;
};

//! Variable assign statement.
struct AssignSmnt : public Stmnt
{
    AST_INTERFACE(AssignSmnt);
    VarIdentPtr varIdent;
    std::string op;
    ExprPtr     expr;
};

//! Function call statement.
struct FunctionCallStmnt : public Stmnt
{
    AST_INTERFACE(FunctionCallStmnt);
    FunctionCallPtr call;
};

//! Returns statement.
struct ReturnStmnt : public Stmnt
{
    AST_INTERFACE(ReturnStmnt);
    ExprPtr expr; // may be null
};

//! Structure declaration statement.
struct StructDeclStmnt : public Stmnt
{
    AST_INTERFACE(StructDeclStmnt);
    StructurePtr structure;
};

//! Control transfer statement.
struct CtrlTransferStmnt : public Stmnt
{
    AST_INTERFACE(CtrlTransferStmnt);
    std::string instruction; // "continue", "break"
};

/* --- Expressions --- */

//! Literal expression.
struct LiteralExpr : public Expr
{
    AST_INTERFACE(LiteralExpr);
    std::string literal;
};

//! Binary expressions.
struct BinaryExpr : public Expr
{
    AST_INTERFACE(BinaryExpr);
    ExprPtr     lhsExpr;    // left-hand-side expression
    std::string op;         // binary operator
    ExprPtr     rhsExpr;    // right-hand-side expression
};

//! (Pre-) Unary expressions.
struct UnaryExpr : public Expr
{
    AST_INTERFACE(UnaryExpr);
    std::string op;
    ExprPtr     expr;
};

//! Post unary expressions.
struct PostUnaryExpr : public Expr
{
    AST_INTERFACE(PostUnaryExpr);
    ExprPtr     expr;
    std::string op;
};

//! Function call expression.
struct FunctionCallExpr : public Expr
{
    AST_INTERFACE(FunctionCallExpr);
    FunctionCallPtr call;
};

//! Bracket expression.
struct BracketExpr : public Expr
{
    AST_INTERFACE(BracketExpr);
    ExprPtr expr; // inner expression
};

//! Cast expression.
struct CastExpr : public Expr
{
    AST_INTERFACE(CastExpr);
    VarTypePtr  castType;
    ExprPtr     expr;
};

//! Variable access expression.
struct VarAccessExpr : public Expr
{
    AST_INTERFACE(VarAccessExpr);
    VarIdentPtr varIdent;
    std::string assignOp;   // may be empty
    ExprPtr     assignExpr; // may be null
};

/* --- Others --- */

//! Case block for a switch statement.
struct SwitchCase : public AST
{
    AST_INTERFACE(SwitchCase);
    ExprPtr                 expr; // if null -> default case
    std::vector<StmntPtr>   stmnts;
};

#undef AST_INTERFACE
#undef DECL_AST_ALIAS
#undef FLAGS


} // /namespace HTLib


#endif



// ================================================================================