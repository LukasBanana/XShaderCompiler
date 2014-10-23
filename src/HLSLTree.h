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
#include <set>


namespace HTLib
{


/*
This file contains all node classes for the entire HLSL absract syntax tree.
For simplicity only structs with public members are used here.
*/

class Visitor;

/* --- Some helper macros --- */

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

#define FLAG_ENUM enum : unsigned int

#define FLAG(name, index) name = (1 << (index))

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
        UniformBufferDecl,
        StorageBufferDecl,
        TextureDecl,
        SamplerDecl,
        StructDecl,
        DirectiveDecl,

        NullStmnt,
        DirectiveStmnt,
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

        ListExpr,
        LiteralExpr,
        TypeNameExpr,
        BinaryExpr,
        UnaryExpr,
        PostUnaryExpr,
        FunctionCallExpr,
        BracketExpr,
        CastExpr,
        VarAccessExpr,
        InitializerExpr,

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

    SourcePosition  pos;
    Flags           flags;
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
    //! GL ARB extension structure.
    struct ARBExtension
    {
        std::string extensionName;
        int         requiredVersion;
    };
    FLAG_ENUM
    {
        FLAG( rcpIntrinsicUsed,          0 ), // The "rcp" intrinsic is used.
        FLAG( interlockedIntrinsicsUsed, 1 ), // Some 'Interlocked...' intrinsics are used.
    };
    AST_INTERFACE(Program)
    std::vector<GlobalDeclPtr> globalDecls;
    std::set<std::string> requiredExtensions;
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
    FLAG_ENUM
    {
        FLAG( isReferenced, 0 ), // This buffer is referenced (or rather used) at least once (use-count >= 1).
    };
    AST_INTERFACE(BufferDeclIdent);
    std::string ident;
    std::string registerName; // may be empty
};

//! Function call.
struct FunctionCall : public AST
{
    FLAG_ENUM
    {
        FLAG( isMulFunc, 0 ), // This is a "mul" function.
        FLAG( isRcpFunc, 1 ), // This is a "rcp" function.
        FLAG( isTexFunc, 2 ), // This is a texture function (e.g. "tex.Sample" or "tex.Load").
    };
    AST_INTERFACE(FunctionCall);
    VarIdentPtr             name;
    std::vector<ExprPtr>    arguments;
};

//! Structure object.
struct Structure : public AST
{
    FLAG_ENUM
    {
        FLAG( isReferenced,     0 ), // This structure is referenced (or rather used) at least once (use-count >= 1).
        FLAG( wasMarked,        1 ), // This structure was already marked by the "ReferenceAnalyzer" visitor.
        FLAG( isShaderInput,    2 ), // This structure is used as shader input.
        FLAG( isShaderOutput,   3 ), // This structure is used as shader output.
    };
    AST_INTERFACE(Structure);
    std::string                     name;
    std::vector<VarDeclStmntPtr>    members;
    std::string                     aliasName; // alias name for input and output interface blocks of the DAST.
};

/* --- Global declarations --- */

//! Function declaration.
struct FunctionDecl : public GlobalDecl
{
    FLAG_ENUM
    {
        FLAG( isReferenced, 0 ), // This function is referenced (or rather used) at least once (use-count >= 1).
        FLAG( wasMarked,    1 ), // This function was already marked by the "ReferenceAnalyzer" visitor.
        FLAG( isEntryPoint, 2 ), // This function is the main entry point.
    };
    AST_INTERFACE(FunctionDecl);
    std::vector<FunctionCallPtr>    attribs; // attribute list
    VarTypePtr                      returnType;
    std::string                     name;
    std::vector<VarDeclStmntPtr>    parameters;
    std::string                     semantic;
    CodeBlockPtr                    codeBlock;
};

//! Uniform buffer (cbuffer, tbuffer) declaration.
struct UniformBufferDecl : public GlobalDecl
{
    FLAG_ENUM
    {
        FLAG( isReferenced, 0 ), // This uniform buffer is referenced (or rather used) at least once (use-count >= 1).
        FLAG( wasMarked,    1 ), // This uniform buffer was already marked by the "ReferenceAnalyzer" visitor.
    };
    AST_INTERFACE(UniformBufferDecl);
    std::string                     bufferType;
    std::string                     name;
    std::string                     registerName; // may be empty
    std::vector<VarDeclStmntPtr>    members;
};

//! Texture declaration.
struct TextureDecl : public GlobalDecl
{
    FLAG_ENUM
    {
        FLAG( isReferenced, 0 ), // This texture is referenced (or rather used) at least once (use-count >= 1).
    };
    AST_INTERFACE(TextureDecl);
    std::string                     textureType;
    std::string                     colorType;
    std::vector<BufferDeclIdentPtr> names;
};

//! Sampler declaration.
struct SamplerDecl : public GlobalDecl
{
    AST_INTERFACE(SamplerDecl);
    std::string                     samplerType;
    std::vector<BufferDeclIdentPtr> names;
};

//! Structure declaration.
struct StructDecl : public GlobalDecl
{
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
    std::string     baseType;               // either this ...
    StructurePtr    structType;             // ... or this is used.
    AST*            symbolRef = nullptr;    // symbol reference for DAST to the type definition; may be null.
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
    FLAG_ENUM
    {
        FLAG( isInsideFunc, 0 ), // This variable is declared inside a function.
    };
    AST_INTERFACE(VarDecl);
    std::string                 name;
    std::vector<ExprPtr>        arrayDims;
    std::vector<VarSemanticPtr> semantics;
    ExprPtr                     initializer;
    UniformBufferDecl*          uniformBufferRef = nullptr; // uniform buffer reference for DAST; may be null
};

/* --- Statements --- */

//! Null statement.
struct NullStmnt : public Stmnt
{
    AST_INTERFACE(NullStmnt);
};

//! Pre-processor directive statement.
struct DirectiveStmnt : public Stmnt
{
    AST_INTERFACE(DirectiveStmnt);
    std::string line;
};

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
    StmntPtr                        initSmnt;
    ExprPtr                         condition;
    ExprPtr                         iteration;
    StmntPtr                        bodyStmnt;
};

//! 'while'-loop statement.
struct WhileLoopStmnt : public Stmnt
{
    AST_INTERFACE(WhileLoopStmnt);
    std::vector<FunctionCallPtr>    attribs; // attribute list
    ExprPtr                         condition;
    StmntPtr                        bodyStmnt;
};

//! 'do/while'-loop statement.
struct DoWhileLoopStmnt : public Stmnt
{
    AST_INTERFACE(DoWhileLoopStmnt);
    std::vector<FunctionCallPtr>    attribs; // attribute list
    StmntPtr                        bodyStmnt;
    ExprPtr                         condition;
};

//! 'if' statement.
struct IfStmnt : public Stmnt
{
    AST_INTERFACE(IfStmnt);
    std::vector<FunctionCallPtr>    attribs; // attribute list
    ExprPtr                         condition;
    StmntPtr                        bodyStmnt;
    ElseStmntPtr                    elseStmnt; // may be null
};

//! 'else' statement.
struct ElseStmnt : public Stmnt
{
    AST_INTERFACE(ElseStmnt);
    StmntPtr bodyStmnt;
};

//! 'switch' statement.
struct SwitchStmnt : public Stmnt
{
    AST_INTERFACE(SwitchStmnt);
    std::vector<FunctionCallPtr>    attribs; // attribute list
    ExprPtr                         selector;
    std::vector<SwitchCasePtr>      cases;
};

//! Variable declaration statement.
struct VarDeclStmnt : public Stmnt
{
    FLAG_ENUM
    {
        FLAG( isShaderInput,    0 ), // This variable is used as shader input.
        FLAG( isShaderOutput,   1 ), // This variable is used as shader output.
    };
    AST_INTERFACE(VarDeclStmnt);
    std::vector<std::string>    commonModifiers; // storage classes, interpolation modifiers or input modifiers,
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

//! List expression ( expr ',' expr ).
struct ListExpr : public Expr
{
    AST_INTERFACE(ListExpr);
    ExprPtr firstExpr;
    ExprPtr nextExpr;
};

//! Literal expression.
struct LiteralExpr : public Expr
{
    AST_INTERFACE(LiteralExpr);
    std::string literal;
};

//! Type name expression (used for simpler cast-expression parsing).
struct TypeNameExpr : public Expr
{
    AST_INTERFACE(TypeNameExpr);
    std::string typeName;
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
    ExprPtr typeExpr;
    ExprPtr expr;
};

//! Variable access expression.
struct VarAccessExpr : public Expr
{
    AST_INTERFACE(VarAccessExpr);
    VarIdentPtr varIdent;
    std::string assignOp;   // may be empty
    ExprPtr     assignExpr; // may be null
};

//! Initializer list expression.
struct InitializerExpr : public Expr
{
    AST_INTERFACE(InitializerExpr);
    std::vector<ExprPtr> exprs;
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
#undef FLAG_ENUM
#undef FLAG

/* --- Helper functions --- */

//! Returns the full variabel identifier name.
std::string FullVarIdent(const VarIdentPtr& varIdent);


} // /namespace HTLib


#endif



// ================================================================================