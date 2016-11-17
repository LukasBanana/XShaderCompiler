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
#include "TypeDenoter.h"
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
    IDENT = (1u << (INDEX))

// Base class for all AST node classes.
struct AST
{
    enum class Types
    {
        Program,
        CodeBlock,
        FunctionCall,
        Attribute,
        SwitchCase,
        SamplerValue,
        PackOffset,
        VarSemantic,
        VarType,
        VarIdent,

        VarDecl,
        TextureDecl,
        SamplerDecl,
        StructDecl,
        AliasDecl,

        FunctionDecl, // Do not use "Stmnt" postfix here (--> FunctionDecl can only appear in global scope)
        VarDeclStmnt,
        BufferDeclStmnt,
        TextureDeclStmnt,
        SamplerDeclStmnt,
        StructDeclStmnt,
        AliasDeclStmnt, // Type alias (typedef)

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
        SuffixExpr,
        ArrayAccessExpr,
        CastExpr,
        VarAccessExpr,
        InitializerExpr,
    };

    virtual ~AST()
    {
    }
    
    virtual Types Type() const = 0;
    virtual void Visit(Visitor* visitor, void* args = nullptr) = 0;

    FLAG_ENUM
    {
        FLAG( isReachable,      30 ), // This AST node is reachable from the main entry point (i.e. the use-count >= 1).
        FLAG( isReachableDone,  31 ), // This AST node was already marked as reachable.
    };

    SourceArea  area;
    Flags       flags;
};

/* --- Base AST nodes --- */

// Statement AST base class.
struct Stmnt : public AST {};

// AST base class with type denoter.
struct TypedAST : public AST
{

    public:

        // Returns a type denoter for AST or throws an std::runtime_error if a type denoter can not be derived.
        const TypeDenoterPtr& GetTypeDenoter();

    protected:

        virtual TypeDenoterPtr DeriveTypeDenoter() = 0;

    private:
    
        TypeDenoterPtr bufferedTypeDenoter_;

};

// Expression AST base class.
struct Expr : public TypedAST {};

// Declaration AST base class.
struct Decl : public TypedAST {};

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
        FLAG( hasSM3ScreenSpace, 0 ), // This shader program uses the Shader Model (SM) 3 screen space (VPOS vs. SV_Position).
    };

    std::vector<StmntPtr>   globalStmnts;               // Global declaration statements

    SourceCodePtr           sourceCode;                 // Preprocessed source code
    InputSemantics          inputSemantics;             // Input semantics for the DAST
    OutputSemantics         outputSemantics;            // Output semantics for the DAST
    FunctionDecl*           entryPointRef   = nullptr;  // Reference to the entry point function declaration.
    std::set<Intrinsic>     usedIntrinsics;             // Set of all used intrinsic (filled by the reference analyzer).
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
        FLAG( isTexFunc, 0 ), // This is a texture function (e.g. "tex.Sample" or "tex.Load").
    };

    VarIdentPtr             varIdent;                           // Either this ...
    TypeDenoterPtr          typeDenoter;                        // ... or this is used.
    std::vector<ExprPtr>    arguments;

    FunctionDecl*           funcDeclRef = nullptr;              // Reference to the function declaration; may be null
    Intrinsic               intrinsic   = Intrinsic::Undefined; // Intrinsic ID (if this is an intrinsic).
};

// Attribute (e.g. "[unroll]" or "[numthreads(x,y,z)]").
struct Attribute : public AST
{
    AST_INTERFACE(Attribute);

    std::string             ident;
    std::vector<ExprPtr>    arguments;
};

// Case block for a switch statement.
struct SwitchCase : public AST
{
    AST_INTERFACE(SwitchCase);

    ExprPtr                 expr; // If null -> default case
    std::vector<StmntPtr>   stmnts;
};

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
    
    // Returns the name of this type (either 'baseType' or 'structDecl->name').
    std::string ToString() const;

    StructDeclPtr   structDecl;             // Optional structure declaration
    TypeDenoterPtr  typeDenoter;

    //TODO: replace this by 'typeDenoter' (but currently heavy use of this member)
    AST*            symbolRef = nullptr;    // Symbol reference for DAST to the type definition; may be null.
};

// Variable (linked-list) identifier.
struct VarIdent : public TypedAST
{
    AST_INTERFACE(VarIdent);

    // Returns the full var-ident string (with '.' separation).
    std::string ToString() const;

    // Returns the last identifier AST node.
    VarIdent* LastVarIdent();

    // Returns a type denoter for the symbol reference of the last variable identifier.
    TypeDenoterPtr DeriveTypeDenoter() override;

    std::string             ident;                  // Either this ..
  //TypeDenoterPtr          typeDenoter;            // ... or this is used
    std::vector<ExprPtr>    arrayIndices;           // Optional array indices
    VarIdentPtr             next;                   // Next identifier; may be null.

    AST*                    symbolRef = nullptr;    // Symbol reference for DAST to the variable object; may be null.
    std::string             systemSemantic;         // System semantic (SV_...) for DAST; may be empty.
};

/* --- Declarations --- */

// Variable declaration.
struct VarDecl : public Decl
{
    AST_INTERFACE(VarDecl);

    FLAG_ENUM
    {
        FLAG( isInsideFunc,     0 ), // This variable is declared inside a function.
        FLAG( disableCodeGen,   1 ), // Disables the code generation for this variable declaration.
    };

    // Returns the variable declaration as string.
    std::string ToString() const;

    // Returns a type denoter for this variable declaration or throws an std::runtime_error if the type can not be derived.
    TypeDenoterPtr DeriveTypeDenoter() override;

    std::string                     ident;
    std::vector<ExprPtr>            arrayDims;
    std::vector<VarSemanticPtr>     semantics;
    std::vector<VarDeclStmntPtr>    annotations;                // Annotations can be ignored by analyzers and generators.
    ExprPtr                         initializer;

    VarDeclStmnt*                   declStmntRef    = nullptr;  // Reference to its declaration statement (parent node); may be null
    BufferDeclStmnt*                bufferDeclRef   = nullptr;  // Buffer declaration reference for DAST (optional 'parent's parent node'); may be null
};

// Texture declaration.
struct TextureDecl : public AST /*TODO --> public Decl*/
{
    AST_INTERFACE(TextureDecl);

    std::string                     ident;
    std::vector<ExprPtr>            arrayDims;
    std::string                     registerName;               // May be empty

    TextureDeclStmnt*               declStmntRef    = nullptr;  // Reference to its declaration statement (parent node).
};

// Sampler state declaration.
struct SamplerDecl : public AST /*TODO --> public Decl*/
{
    AST_INTERFACE(SamplerDecl);

    std::string                     ident;
    std::vector<ExprPtr>            arrayDims;
    std::string                     registerName;   // May be empty
    std::vector<SamplerValuePtr>    samplerValues;  // State values for a sampler decl-ident.
};

// StructDecl object.
struct StructDecl : public Decl
{
    AST_INTERFACE(StructDecl);

    FLAG_ENUM
    {
        FLAG( isShaderInput,    0 ), // This structure is used as shader input.
        FLAG( isShaderOutput,   1 ), // This structure is used as shader output.
    };

    // Returns a descriptive string of the function signature (e.g. "struct s" or "struct <anonymous>").
    std::string SignatureToString() const;

    // Returns true if this is an anonymous structure.
    bool IsAnonymous() const;

    // Returns the VarDecl AST node inside this struct decl for the specified identifier, or null if there is no such VarDecl.
    VarDecl* Fetch(const std::string& ident) const;

    // Returns a type denoter for this structure.
    TypeDenoterPtr DeriveTypeDenoter() override;

    std::string                     ident;                      // May be empty (for anonymous structures).
    std::string                     baseStructName;             // May be empty (if no inheritance is used).
    std::vector<VarDeclStmntPtr>    members;

    StructDecl*                     baseStructRef   = nullptr;  // Optional reference to base struct
    std::string                     aliasName;                  // Alias name for input and output interface blocks of the DAST.
    std::map<std::string, VarDecl*> systemValuesRef;            // List of members with system value semantic (SV_...).
};

// Type alias declaration.
struct AliasDecl : Decl
{
    AST_INTERFACE(AliasDecl);

    TypeDenoterPtr DeriveTypeDenoter() override;

    std::string     ident;                  // Type identifier
    TypeDenoterPtr  typeDenoter;            // Type denoter of the aliased type

    AliasDeclStmnt* declStmntRef = nullptr; // Reference to decl-stmnt.
};

/* --- Declaration statements --- */

// Function declaration.
struct FunctionDecl : public Stmnt
{
    AST_INTERFACE(FunctionDecl);

    FLAG_ENUM
    {
        FLAG( isEntryPoint, 0 ), // This function is the main entry point.
    };

    // Returns true if this function declaration is just a forward declaration (without function body).
    bool IsForwardDecl() const;
    
    // Returns true if this declaration is an intrinsic (no code must be generated for this declaration!).
    virtual bool IsIntrinsic() const;

    // Returns a descriptive string of the function signature (e.g. "void f(int x)").
    std::string SignatureToString(bool useParamNames = true) const;

    // Returns true if the specified function declaration has the same signature as this function.
    bool EqualsSignature(const FunctionDecl& rhs) const;

    // Returns the minimal number of arguments for a call to this function.
    std::size_t NumMinArgs() const;

    // Returns the maximal number of arguments for a call to this function (this is merely: parameters.size()).
    std::size_t NumMaxArgs() const;

    // Returns true if the specified type denoter matches the parameter.
    virtual bool MatchParameterWithTypeDenoter(std::size_t paramIndex, const TypeDenoter& argType, bool implicitConversion) const;

    std::vector<AttributePtr>       attribs;                    // Attribute list
    VarTypePtr                      returnType;
    std::string                     ident;
    std::vector<VarDeclStmntPtr>    parameters;
    std::string                     semantic;                   // May be empty
    std::vector<VarDeclStmntPtr>    annotations;                // Annotations can be ignored by analyzers and generators.
    CodeBlockPtr                    codeBlock;                  // May be null (if this AST node is a forward declaration).

    //TODO: currently unused
    FunctionDecl*                   definitionRef   = nullptr;  // Reference to the actual function definition (only for forward declarations).
};

/*
Special case: Function intrinsic declaration (this is only for pre-defined AST nodes!).
No "Visit", no "Type" functions override.
*/
struct IntrinsicDecl : public FunctionDecl
{
    IntrinsicDecl() :
        FunctionDecl{ SourcePosition::ignore }
    {
    }

    bool IsIntrinsic() const override;
    bool MatchParameterWithTypeDenoter(std::size_t paramIndex, const TypeDenoter& argType, bool implicitConversion) const override;

    Intrinsic intrinsic = Intrinsic::Undefined; // Intrinsic ID.
};

//TODO --> maybe separate this structure into "BufferDeclStmnt" and "BufferDecl" (like in the other declaration AST nodes)
// Uniform buffer (cbuffer, tbuffer) declaration.
struct BufferDeclStmnt : public Stmnt
{
    AST_INTERFACE(BufferDeclStmnt);

    std::string                     bufferType;
    std::string                     ident;
    std::string                     registerName; // May be empty
    std::vector<VarDeclStmntPtr>    members;
};

// Texture declaration.
struct TextureDeclStmnt : public Stmnt
{
    AST_INTERFACE(TextureDeclStmnt);

    std::string                 textureType;
    std::string                 colorType;      //TODO: replace with TypeDenoterPtr
    std::vector<TextureDeclPtr> textureDecls;
};

// Sampler declaration.
struct SamplerDeclStmnt : public Stmnt
{
    AST_INTERFACE(SamplerDeclStmnt);

    std::string                 samplerType;
    std::vector<SamplerDeclPtr> samplerDecls;
};

// StructDecl declaration statement.
struct StructDeclStmnt : public Stmnt
{
    AST_INTERFACE(StructDeclStmnt);
    StructDeclPtr structDecl;
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
    
    // Returns the VarDecl AST node inside this var-decl statement for the specified identifier, or null if there is no such VarDecl.
    VarDecl* Fetch(const std::string& ident) const;

    std::string                 inputModifier;      // in, out, inout, uniform
    std::vector<std::string>    storageModifiers;   // extern, nointerpolation, precise, shared, groupshared, static, volatile
    std::vector<std::string>    typeModifiers;      // const, row_major, column_major
    VarTypePtr                  varType;
    std::vector<VarDeclPtr>     varDecls;
};

// Type alias declaration statement.
struct AliasDeclStmnt : public Stmnt
{
    AST_INTERFACE(AliasDeclStmnt);

    StructDeclPtr               structDecl; // Optional structure declaration
    std::vector<AliasDeclPtr>   aliasDecls; // Type aliases
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

    std::vector<AttributePtr>   attribs; // Attribute list
    StmntPtr                    initSmnt;
    ExprPtr                     condition;
    ExprPtr                     iteration;
    StmntPtr                    bodyStmnt;
};

// 'while'-loop statement.
struct WhileLoopStmnt : public Stmnt
{
    AST_INTERFACE(WhileLoopStmnt);

    std::vector<AttributePtr>   attribs; // Attribute list
    ExprPtr                     condition;
    StmntPtr                    bodyStmnt;
};

// 'do/while'-loop statement.
struct DoWhileLoopStmnt : public Stmnt
{
    AST_INTERFACE(DoWhileLoopStmnt);

    std::vector<AttributePtr>   attribs; // Attribute list
    StmntPtr                    bodyStmnt;
    ExprPtr                     condition;
};

// 'if' statement.
struct IfStmnt : public Stmnt
{
    AST_INTERFACE(IfStmnt);

    std::vector<AttributePtr>   attribs;    // Attribute list
    ExprPtr                     condition;
    StmntPtr                    bodyStmnt;
    ElseStmntPtr                elseStmnt;  // May be null
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

    std::vector<AttributePtr>   attribs; // Attribute list
    ExprPtr                     selector;
    std::vector<SwitchCasePtr>  cases;
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

    TypeDenoterPtr DeriveTypeDenoter() override;

    ExprPtr firstExpr;
    ExprPtr nextExpr;
};

// Literal expression.
struct LiteralExpr : public Expr
{
    AST_INTERFACE(LiteralExpr);

    TypeDenoterPtr DeriveTypeDenoter() override;

    Token::Types    type    = Token::Types::Undefined;
    std::string     value;
};

// Type name expression (used for simpler cast-expression parsing).
struct TypeNameExpr : public Expr
{
    AST_INTERFACE(TypeNameExpr);

    TypeDenoterPtr DeriveTypeDenoter() override;

    TypeDenoterPtr typeDenoter;
};

// Ternary expression.
struct TernaryExpr : public Expr
{
    AST_INTERFACE(TernaryExpr);

    TypeDenoterPtr DeriveTypeDenoter() override;

    ExprPtr condExpr; // Condition expression
    ExprPtr thenExpr; // <then> case expression
    ExprPtr elseExpr; // <else> case expression
};

// Binary expressions.
struct BinaryExpr : public Expr
{
    AST_INTERFACE(BinaryExpr);

    TypeDenoterPtr DeriveTypeDenoter() override;

    ExprPtr     lhsExpr;                        // Left-hand-side expression
    BinaryOp    op      = BinaryOp::Undefined;  // Binary operator
    ExprPtr     rhsExpr;                        // Right-hand-side expression
};

// (Pre-) Unary expressions.
struct UnaryExpr : public Expr
{
    AST_INTERFACE(UnaryExpr);

    TypeDenoterPtr DeriveTypeDenoter() override;

    UnaryOp op      = UnaryOp::Undefined;
    ExprPtr expr;
};

// Post unary expressions (e.g. x++, x--)
struct PostUnaryExpr : public Expr
{
    AST_INTERFACE(PostUnaryExpr);

    TypeDenoterPtr DeriveTypeDenoter() override;

    ExprPtr expr;
    UnaryOp op      = UnaryOp::Undefined;
};

// Function call expression.
struct FunctionCallExpr : public Expr
{
    AST_INTERFACE(FunctionCallExpr);

    TypeDenoterPtr DeriveTypeDenoter() override;

    FunctionCallPtr call;
};

// Bracket expression.
struct BracketExpr : public Expr
{
    AST_INTERFACE(BracketExpr);

    TypeDenoterPtr DeriveTypeDenoter() override;

    ExprPtr expr;           // Inner expression
};

// Suffix expression (e.g. "foo().suffix").
struct SuffixExpr : public Expr
{
    AST_INTERFACE(SuffixExpr);

    TypeDenoterPtr DeriveTypeDenoter() override;

    ExprPtr     expr;       // Sub expression (left hand side)
    VarIdentPtr varIdent;   // Suffix var identifier (right hand side)
};

// Array-access expression (e.g. "foo()[arrayAccess]").
struct ArrayAccessExpr : public Expr
{
    AST_INTERFACE(ArrayAccessExpr);

    TypeDenoterPtr DeriveTypeDenoter() override;

    ExprPtr                 expr;           // Sub expression (left hand side)
    std::vector<ExprPtr>    arrayIndices;   // Array indices (right hand side)
};

// Cast expression.
struct CastExpr : public Expr
{
    AST_INTERFACE(CastExpr);

    TypeDenoterPtr DeriveTypeDenoter() override;

    ExprPtr typeExpr;
    ExprPtr expr;
};

// Variable access expression.
struct VarAccessExpr : public Expr
{
    AST_INTERFACE(VarAccessExpr);

    TypeDenoterPtr DeriveTypeDenoter() override;

    VarIdentPtr varIdent;
    AssignOp    assignOp    = AssignOp::Undefined;  // May be undefined
    ExprPtr     assignExpr;                         // May be null
};

// Initializer list expression.
struct InitializerExpr : public Expr
{
    AST_INTERFACE(InitializerExpr);

    TypeDenoterPtr DeriveTypeDenoter() override;

    // Returns the number of scalar elements (with recursion).
    unsigned int NumElements() const;

    std::vector<ExprPtr> exprs;
};

#undef AST_INTERFACE
#undef DECL_AST_ALIAS
#undef FLAG_ENUM
#undef FLAG


} // /namespace Xsc


#endif



// ================================================================================
