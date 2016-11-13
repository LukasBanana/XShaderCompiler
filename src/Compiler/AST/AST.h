/*
 * AST.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_AST_H
#define XSC_AST_H


#include "Token.h"
#include "Visitor.h"
#include "Flags.h"
#include "ASTEnums.h"
#include "SourceCode.h"

#include <vector>
#include <string>
#include <set>
#include <map>


namespace Xsc
{


/*
This file contains all node classes for the entire absract syntax tree (AST).
For simplicity only structs with public members are used here.
*/

class Visitor;

/* --- Some helper macros --- */

#define AST_INTERFACE(CLASS_NAME)                               \
    CLASS_NAME(const SourcePosition& astPos)                    \
    {                                                           \
        area = SourceArea(astPos, 1);                           \
    }                                                           \
    Types Type() const override                                 \
    {                                                           \
        return Types::CLASS_NAME;                               \
    }                                                           \
    void Visit(Visitor* visitor, void* args = nullptr) override \
    {                                                           \
        visitor->Visit##CLASS_NAME(this, args);                 \
    }

#define DECL_AST_ALIAS(ALIAS, BASE) \
    using ALIAS         = BASE;     \
    using ALIAS##Ptr    = BASE##Ptr

#define FLAG_ENUM \
    enum : unsigned int

#define FLAG(IDENT, INDEX) \
    IDENT = (1 << (INDEX))

// Base class for all AST node classes.
struct AST
{
    enum class Types
    {
        Program,
        CodeBlock,
        FunctionCall,
        Structure,
        SwitchCase,
        SamplerValue,

        PackOffset,
        VarSemantic,
        VarType,
        VarIdent,
        VarDecl,
        TextureDecl,
        SamplerDecl,

        FunctionDecl, // Do not use "Stmnt" postfix here (--> FunctionDecl can only appear in global scope)
        BufferDeclStmnt,
        TextureDeclStmnt,
        SamplerDeclStmnt,
        StructDeclStmnt,
        VarDeclStmnt,

        NullStmnt,
        CodeBlockStmnt,
        ForLoopStmnt,
        WhileLoopStmnt,
        DoWhileLoopStmnt,
        IfStmnt,
        ElseStmnt,
        SwitchStmnt,
        AssignStmnt,
        ExprStmnt,
        FunctionCallStmnt,
        ReturnStmnt,
        CtrlTransferStmnt,

        ListExpr,
        LiteralExpr,
        TypeNameExpr,
        TernaryExpr,
        BinaryExpr,
        UnaryExpr,
        PostUnaryExpr,
        FunctionCallExpr,
        BracketExpr,
        CastExpr,
        VarAccessExpr,
        InitializerExpr,
    };

    virtual ~AST()
    {
    }
    
    virtual Types Type() const = 0;
    virtual void Visit(Visitor* visitor, void* args = nullptr) = 0;

    SourceArea  area;
    Flags       flags;
};

/* --- Base AST nodes --- */

// Statement base class.
struct Stmnt : public AST {};

// Expression base class.
struct Expr : public AST {};

// Program AST root.
struct Program : public AST
{   
    AST_INTERFACE(Program)

    struct InputSemantics
    {
        std::vector<VarDeclStmnt*> parameters;
    };

    struct OutputSemantics
    {
        VarType*    returnType = nullptr;   // Either this ...
        std::string functionSemantic;       // ... or this is used.
        std::string singleOutputVariable;   // May be empty
    };

    FLAG_ENUM
    {
        FLAG( rcpIntrinsicUsed,     0 ), // The "rcp" intrinsic is used.
        FLAG( sinCosIntrinsicUsed,  1 ), // The "sincos" intrinsic is used.
        FLAG( clipIntrinsicUsed,    2 ), // The "clip" intrinsic is used.
        FLAG( hasSM3ScreenSpace,    3 ), // This shader program uses the Shader Model (SM) 3 screen space (VPOS vs. SV_Position).
    };

    std::vector<StmntPtr>   globalStmnts;       // Global declaration statements

    SourceCodePtr           sourceCode;         // Preprocessed source code
    InputSemantics          inputSemantics;     // Input semantics for the DAST
    OutputSemantics         outputSemantics;    // Output semantics for the DAST
};

// Code block.
struct CodeBlock : public AST
{
    AST_INTERFACE(CodeBlock);

    std::vector<StmntPtr> stmnts;
};

// Sampler state value assignment.
// see https://msdn.microsoft.com/de-de/library/windows/desktop/bb509644(v=vs.85).aspx
struct SamplerValue : public AST
{
    AST_INTERFACE(SamplerValue);

    std::string name;   // Sampler state name
    ExprPtr     value;  // Sampler state value expression
};

// Function call.
struct FunctionCall : public AST
{
    AST_INTERFACE(FunctionCall);

    FLAG_ENUM
    {
        FLAG( isMulFunc,    0 ), // This is a "mul" function.
        FLAG( isRcpFunc,    1 ), // This is a "rcp" function.
        FLAG( isTexFunc,    2 ), // This is a texture function (e.g. "tex.Sample" or "tex.Load").
        FLAG( isAtomicFunc, 3 ), // This is an atomic (or rather interlocked) function (e.g. "InterlockedAdd").
    };

    VarIdentPtr             name;
    std::vector<ExprPtr>    arguments;
};

// Structure object.
struct Structure : public AST
{
    AST_INTERFACE(Structure);

    FLAG_ENUM
    {
        FLAG( isReferenced,     0 ), // This structure is referenced (or rather used) at least once (use-count >= 1).
        FLAG( wasMarked,        1 ), // This structure was already marked by the "ReferenceAnalyzer" visitor.
        FLAG( isShaderInput,    2 ), // This structure is used as shader input.
        FLAG( isShaderOutput,   3 ), // This structure is used as shader output.
    };

    // Returns a descriptive string of the function signature (e.g. "struct s" or "struct <anonymous>").
    std::string SignatureToString() const;

    std::string                     name;               // May be empty (for anonymous structures).
    std::vector<VarDeclStmntPtr>    members;

    std::string                     aliasName;          // Alias name for input and output interface blocks of the DAST.
    std::map<std::string, VarDecl*> systemValuesRef;    // List of members with system value semantic (SV_...).
};

/* --- Variables --- */

// Pack offset.
struct PackOffset : public AST
{
    AST_INTERFACE(PackOffset);

    std::string ToString() const;

    std::string registerName;
    std::string vectorComponent; // May be empty
};

// Variable semantic.
struct VarSemantic : public AST
{
    AST_INTERFACE(VarSemantic);

    std::string ToString() const;

    std::string     semantic;
    PackOffsetPtr   packOffset;
    std::string     registerName; // May be empty
};

// Variable data type.
struct VarType : public AST
{
    AST_INTERFACE(VarType);
    
    // Returns the name of this type (either 'baseType' or 'structType->name').
    std::string ToString() const;

    //TODO --> change this to "TypeDenoterPtr typeDenoter"!!!
    std::string     baseType;               // Either this ...
    StructurePtr    structType;             // ... or this is used.

    AST*            symbolRef = nullptr;    // Symbol reference for DAST to the type definition; may be null.
};

// Variable (linked-list) identifier.
struct VarIdent : public AST
{
    AST_INTERFACE(VarIdent);

    // Returns the full var-ident string (with '.' separation).
    std::string ToString() const;

    // Returns the last identifier AST node.
    VarIdent* LastVarIdent();

    std::string             ident;
    std::vector<ExprPtr>    arrayIndices;
    VarIdentPtr             next;                   // Next identifier; may be null.

    AST*                    symbolRef = nullptr;    // Symbol reference for DAST to the variable object; may be null.
    std::string             systemSemantic;         // System semantic (SV_...) for DAST; may be empty.
};

// Variable declaration.
struct VarDecl : public AST
{
    AST_INTERFACE(VarDecl);

    FLAG_ENUM
    {
        FLAG( isInsideFunc,     0 ), // This variable is declared inside a function.
        FLAG( disableCodeGen,   1 ), // Disables the code generation for this variable declaration.
    };

    // Returns the variable declaration as string.
    std::string ToString() const;

    std::string                 name;
    std::vector<ExprPtr>        arrayDims;
    std::vector<VarSemanticPtr> semantics;
    ExprPtr                     initializer;

    BufferDeclStmnt*            bufferDeclRef   = nullptr; // Buffer declaration reference for DAST; may be null
    VarDeclStmnt*               declStmntRef    = nullptr; // Reference to its declaration statement; may be null
};

// Texture declaration.
struct TextureDecl : public AST
{
    AST_INTERFACE(TextureDecl);

    FLAG_ENUM
    {
        FLAG( isReferenced, 0 ), // This buffer is referenced (or rather used) at least once (use-count >= 1).
    };

    std::string                     ident;
    std::vector<ExprPtr>            arrayIndices;
    std::string                     registerName;   // May be empty
};

// Sampler state declaration.
struct SamplerDecl : public AST
{
    AST_INTERFACE(SamplerDecl);

    std::string                     ident;
    std::vector<ExprPtr>            arrayIndices;
    std::string                     registerName;   // May be empty
    std::vector<SamplerValuePtr>    samplerValues;  // State values for a sampler decl-ident.
};

/* --- Declaration statements --- */

// Function declaration.
struct FunctionDecl : public Stmnt
{
    AST_INTERFACE(FunctionDecl);

    FLAG_ENUM
    {
        FLAG( isReferenced, 0 ), // This function is referenced (or rather used) at least once (use-count >= 1).
        FLAG( wasMarked,    1 ), // This function was already marked by the "ReferenceAnalyzer" visitor.
        FLAG( isEntryPoint, 2 ), // This function is the main entry point.
    };
    
    // Returns a descriptive string of the function signature (e.g. "void f(int x)").
    std::string SignatureToString(bool useParamNames = true) const;

    std::vector<FunctionCallPtr>    attribs;            // Attribute list
    VarTypePtr                      returnType;
    std::string                     name;
    std::vector<VarDeclStmntPtr>    parameters;
    std::string                     semantic;           // May be empty
    CodeBlockPtr                    codeBlock;          // May be null (if this AST node is a forward declaration).

    std::vector<FunctionDecl*>      forwardDeclsRef;    // List of forward declarations to this function for the DAST.
};

// Uniform buffer (cbuffer, tbuffer) declaration.
struct BufferDeclStmnt : public Stmnt
{
    AST_INTERFACE(BufferDeclStmnt);

    FLAG_ENUM
    {
        FLAG( isReferenced, 0 ), // This uniform buffer is referenced (or rather used) at least once (use-count >= 1).
        FLAG( wasMarked,    1 ), // This uniform buffer was already marked by the "ReferenceAnalyzer" visitor.
    };
    
    std::string                     bufferType;
    std::string                     name;
    std::string                     registerName; // May be empty
    std::vector<VarDeclStmntPtr>    members;
};

// Texture declaration.
struct TextureDeclStmnt : public Stmnt
{
    AST_INTERFACE(TextureDeclStmnt);

    FLAG_ENUM
    {
        FLAG( isReferenced, 0 ), // This texture is referenced (or rather used) at least once (use-count >= 1).
    };

    std::string                 textureType;
    std::string                 colorType;
    std::vector<TextureDeclPtr> textureDecls;
};

// Sampler declaration.
struct SamplerDeclStmnt : public Stmnt
{
    AST_INTERFACE(SamplerDeclStmnt);

    std::string                 samplerType;
    std::vector<SamplerDeclPtr> samplerDecls;
};

// Structure declaration statement.
struct StructDeclStmnt : public Stmnt
{
    AST_INTERFACE(StructDeclStmnt);
    StructurePtr structure;
};

// Variable declaration statement.
struct VarDeclStmnt : public Stmnt
{
    AST_INTERFACE(VarDeclStmnt);

    FLAG_ENUM
    {
        FLAG( isShaderInput,    2 ), // This variable is used as shader input.
        FLAG( isShaderOutput,   3 ), // This variable is used as shader output.
    };

    // Returns the var-decl statement as string.
    std::string ToString(bool useVarNames = true) const;

    std::string                 inputModifier;      // in, out, inout, uniform
    std::vector<std::string>    storageModifiers;   // extern, nointerpolation, precise, shared, groupshared, static, volatile
    std::vector<std::string>    typeModifiers;      // const, row_major, column_major
    VarTypePtr                  varType;
    std::vector<VarDeclPtr>     varDecls;
};

/* --- Statements --- */

// Null statement.
struct NullStmnt : public Stmnt
{
    AST_INTERFACE(NullStmnt);
};

// Code block statement.
struct CodeBlockStmnt : public Stmnt
{
    AST_INTERFACE(CodeBlockStmnt);

    CodeBlockPtr codeBlock;
};

// 'for'-loop statemnet.
struct ForLoopStmnt : public Stmnt
{
    AST_INTERFACE(ForLoopStmnt);

    std::vector<FunctionCallPtr>    attribs; // Attribute list
    StmntPtr                        initSmnt;
    ExprPtr                         condition;
    ExprPtr                         iteration;
    StmntPtr                        bodyStmnt;
};

// 'while'-loop statement.
struct WhileLoopStmnt : public Stmnt
{
    AST_INTERFACE(WhileLoopStmnt);

    std::vector<FunctionCallPtr>    attribs; // Attribute list
    ExprPtr                         condition;
    StmntPtr                        bodyStmnt;
};

// 'do/while'-loop statement.
struct DoWhileLoopStmnt : public Stmnt
{
    AST_INTERFACE(DoWhileLoopStmnt);

    std::vector<FunctionCallPtr>    attribs; // Attribute list
    StmntPtr                        bodyStmnt;
    ExprPtr                         condition;
};

// 'if' statement.
struct IfStmnt : public Stmnt
{
    AST_INTERFACE(IfStmnt);

    std::vector<FunctionCallPtr>    attribs;    // Attribute list
    ExprPtr                         condition;
    StmntPtr                        bodyStmnt;
    ElseStmntPtr                    elseStmnt;  // May be null
};

// 'else' statement.
struct ElseStmnt : public Stmnt
{
    AST_INTERFACE(ElseStmnt);

    StmntPtr bodyStmnt;
};

// 'switch' statement.
struct SwitchStmnt : public Stmnt
{
    AST_INTERFACE(SwitchStmnt);

    std::vector<FunctionCallPtr>    attribs; // Attribute list
    ExprPtr                         selector;
    std::vector<SwitchCasePtr>      cases;
};

// Variable assign statement.
struct AssignStmnt : public Stmnt
{
    AST_INTERFACE(AssignStmnt);

    VarIdentPtr varIdent;
    AssignOp    op          = AssignOp::Undefined;
    ExprPtr     expr;
};

// Arbitrary expression statement.
struct ExprStmnt : public Stmnt
{
    AST_INTERFACE(ExprStmnt);

    ExprPtr expr;
};

// Function call statement.
struct FunctionCallStmnt : public Stmnt
{
    AST_INTERFACE(FunctionCallStmnt);

    FunctionCallPtr call;
};

// Returns statement.
struct ReturnStmnt : public Stmnt
{
    AST_INTERFACE(ReturnStmnt);

    ExprPtr expr; // May be null
};

// Control transfer statement.
struct CtrlTransferStmnt : public Stmnt
{
    AST_INTERFACE(CtrlTransferStmnt);

    CtrlTransfer transfer = CtrlTransfer::Undefined; // break, continue, discard
};

/* --- Expressions --- */

// List expression ( expr ',' expr ).
struct ListExpr : public Expr
{
    AST_INTERFACE(ListExpr);

    ExprPtr firstExpr;
    ExprPtr nextExpr;
};

// Literal expression.
struct LiteralExpr : public Expr
{
    AST_INTERFACE(LiteralExpr);

    Token::Types    type    = Token::Types::Undefined;
    std::string     value;
};

// Type name expression (used for simpler cast-expression parsing).
struct TypeNameExpr : public Expr
{
    AST_INTERFACE(TypeNameExpr);

    std::string typeName;
};

// Ternary expression.
struct TernaryExpr : public Expr
{
    AST_INTERFACE(TernaryExpr);

    ExprPtr condExpr; // Condition expression
    ExprPtr thenExpr; // <then> case expression
    ExprPtr elseExpr; // <else> case expression
};

// Binary expressions.
struct BinaryExpr : public Expr
{
    AST_INTERFACE(BinaryExpr);

    ExprPtr     lhsExpr;                        // Left-hand-side expression
    BinaryOp    op      = BinaryOp::Undefined;  // Binary operator
    ExprPtr     rhsExpr;                        // Right-hand-side expression
};

// (Pre-) Unary expressions.
struct UnaryExpr : public Expr
{
    AST_INTERFACE(UnaryExpr);

    UnaryOp op      = UnaryOp::Undefined;
    ExprPtr expr;
};

// Post unary expressions (e.g. x++, x--)
struct PostUnaryExpr : public Expr
{
    AST_INTERFACE(PostUnaryExpr);

    ExprPtr expr;
    UnaryOp op      = UnaryOp::Undefined;
};

// Function call expression.
struct FunctionCallExpr : public Expr
{
    AST_INTERFACE(FunctionCallExpr);

    FunctionCallPtr call;
    VarIdentPtr     varIdentSuffix; // Optional var-ident suffix
};

// Bracket expression.
struct BracketExpr : public Expr
{
    AST_INTERFACE(BracketExpr);

    ExprPtr     expr;           // Inner expression
    VarIdentPtr varIdentSuffix; // Optional var-ident suffix
};

// Cast expression.
struct CastExpr : public Expr
{
    AST_INTERFACE(CastExpr);

    ExprPtr typeExpr;
    ExprPtr expr;
};

// Variable access expression.
struct VarAccessExpr : public Expr
{
    AST_INTERFACE(VarAccessExpr);

    VarIdentPtr varIdent;
    std::string assignOp;   // May be empty
    ExprPtr     assignExpr; // May be null
};

// Initializer list expression.
struct InitializerExpr : public Expr
{
    AST_INTERFACE(InitializerExpr);

    std::vector<ExprPtr> exprs;
};

/* --- Others --- */

// Case block for a switch statement.
struct SwitchCase : public AST
{
    AST_INTERFACE(SwitchCase);

    ExprPtr                 expr; // If null -> default case
    std::vector<StmntPtr>   stmnts;
};

#undef AST_INTERFACE
#undef DECL_AST_ALIAS
#undef FLAG_ENUM
#undef FLAG


} // /namespace Xsc


#endif



// ================================================================================