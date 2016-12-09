/*
 * AST.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_AST_H
#define XSC_AST_H


#include <Xsc/Targets.h>
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
    static const Types classType = Types::CLASS_NAME;           \
    CLASS_NAME(const SourcePosition& astPos)                    \
    {                                                           \
        area = SourceArea(astPos, 1);                           \
    }                                                           \
    CLASS_NAME(const SourceArea& astArea)                       \
    {                                                           \
        area = astArea;                                         \
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
        Register,
        PackOffset,
        VarType,
        VarIdent,

        VarDecl,
        //BufferDecl,
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
        ExprStmnt,
        ReturnStmnt,
        CtrlTransferStmnt,

        NullExpr,
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

    virtual ~AST();
    
    virtual Types Type() const = 0;
    virtual void Visit(Visitor* visitor, void* args = nullptr) = 0;

    #ifdef XSC_ENABLE_MEMORY_POOL

    void* operator new (std::size_t count);
    void operator delete (void* ptr);

    #endif

    FLAG_ENUM
    {
        FLAG( isReachable, 30 ), // This AST node is reachable from the main entry point (i.e. the use-count >= 1).
        FLAG( isDeadCode,  29 ), // This AST node is dead code (after return path).
    };

    // Returns this AST node as the specified sub class if this AST node has the correct type. Otherwise, null is returned.
    template <typename T>
    T* As()
    {
        return (Type() == T::classType ? static_cast<T*>(this) : nullptr);
    }

    SourceArea  area;
    Flags       flags;
};

/* --- Base AST nodes --- */

// Statement AST base class.
struct Stmnt : public AST
{
    std::string comment;
};

// AST base class with type denoter.
struct TypedAST : public AST
{

    public:

        // Returns a type denoter for AST or throws an std::runtime_error if a type denoter can not be derived.
        const TypeDenoterPtr& GetTypeDenoter();

    protected:

        virtual TypeDenoterPtr DeriveTypeDenoter() = 0;

        void ResetBufferedTypeDenoter();

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

    FLAG_ENUM
    {
        FLAG( hasSM3ScreenSpace, 0 ), // This shader program uses the Shader Model (SM) 3 screen space (VPOS vs. SV_Position).
        FLAG( isFragCoordUsed,   1 ), // This shader program makes use of the fragment coordinate (SV_Position, gl_FragCoord).
    };

    std::vector<StmntPtr>               globalStmnts;               // Global declaration statements

    std::vector<ASTPtr>                 disabledAST;                // AST nodes that have been disabled for code generation (not part of the default visitor).

    SourceCodePtr                       sourceCode;                 // Preprocessed source code
    FunctionDecl*                       entryPointRef   = nullptr;  // Reference to the entry point function declaration.
    std::map<Intrinsic, IntrinsicUsage> usedIntrinsics;             // Set of all used intrinsic (filled by the reference analyzer).
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
        // If this function call is an intrinsic, it's wrapper function can be inlined (i.e. no wrapper function must be generated)
        // e.g. "clip(a);" can not be converted to "if (a < 0) { discard; }".
        FLAG( canInlineIntrinsicWrapper, 0 ),
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

    std::string             ident;      //TODO: change to AttributeType enum
    std::vector<ExprPtr>    arguments;
};

// Case block for a switch statement.
struct SwitchCase : public AST
{
    AST_INTERFACE(SwitchCase);

    // Returns true, if this is a default case (if 'expr' is null).
    bool IsDefaultCase() const;

    ExprPtr                 expr; // If null -> default case
    std::vector<StmntPtr>   stmnts;
};

// Register (e.g. ": register(t0)").
struct Register : public AST
{
    AST_INTERFACE(Register);

    std::string ToString() const;

    // Returns the first slot register for the specified shader target or null, if there is no register.
    static Register* GetForTarget(const std::vector<RegisterPtr>& registers, const ShaderTarget shaderTarget);

    ShaderTarget    shaderTarget    = ShaderTarget::Undefined;  // Shader target (or profile). Undefined means all targets are affected.
    RegisterType    registerType    = RegisterType::Undefined;
    int             slot            = 0;                        // Zero-based register slot index. By default 0.
};

// Pack offset.
struct PackOffset : public AST
{
    AST_INTERFACE(PackOffset);

    std::string ToString() const;

    std::string registerName;
    std::string vectorComponent; // May be empty
};

// Variable data type.
struct VarType : public AST //TypedAST
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

    // Returns the type denoter for this AST node or the last sub node.
    TypeDenoterPtr GetExplicitTypeDenoter(bool recursive);

    // Moves the next identifier into this one (i.e. removes the first identifier).
    void PopFront();

    std::string             ident;                                  // Either this ..
  //TypeDenoterPtr          typeDenoter;                            // ... or this is used
    std::vector<ExprPtr>    arrayIndices;                           // Optional array indices
    VarIdentPtr             next;                                   // Next identifier; may be null.

    AST*                    symbolRef       = nullptr;              // Symbol reference for DAST to the variable object; may be null.
};

/* --- Declarations --- */

// Variable declaration.
struct VarDecl : public Decl
{
    AST_INTERFACE(VarDecl);

    FLAG_ENUM
    {
        FLAG( isShaderInput,    0 ), // This variable is used as shader input.
        FLAG( isShaderOutput,   1 ), // This variable is used as shader output.
        FLAG( isSystemValue,    2 ), // This variable is a system value.
        FLAG( disableCodeGen,   3 ), // Disables the code generation for this variable declaration.
        FLAG( wasRenamed,       4 ), // This variable was renamed by a converter visitor.

        isShaderInputSV     = (isShaderInput  | isSystemValue), // This variable a used as shader input, and it is a system value.
        isShaderOutputSV    = (isShaderOutput | isSystemValue), // This variable a used as shader output, and it is a system value.
    };

    // Returns the variable declaration as string.
    std::string ToString() const;

    // Returns a type denoter for this variable declaration or throws an std::runtime_error if the type can not be derived.
    TypeDenoterPtr DeriveTypeDenoter() override;

    std::string                     ident;
    std::vector<ExprPtr>            arrayDims;
    IndexedSemantic                 semantic;
    PackOffsetPtr                   packOffset;
    std::vector<VarDeclStmntPtr>    annotations;                // Annotations can be ignored by analyzers and generators.
    ExprPtr                         initializer;

    VarDeclStmnt*                   declStmntRef    = nullptr;  // Reference to its declaration statement (parent node); may be null
    BufferDeclStmnt*                bufferDeclRef   = nullptr;  // Buffer declaration reference for DAST (optional 'parent's parent node'); may be null
};

// Texture declaration.
struct TextureDecl : public Decl
{
    AST_INTERFACE(TextureDecl);

    TypeDenoterPtr DeriveTypeDenoter() override;

    std::string                     ident;
    std::vector<ExprPtr>            arrayDims;
    std::vector<RegisterPtr>        slotRegisters;

    TextureDeclStmnt*               declStmntRef    = nullptr;  // Reference to its declaration statement (parent node).
};

// Sampler state declaration.
struct SamplerDecl : public Decl
{
    AST_INTERFACE(SamplerDecl);

    TypeDenoterPtr DeriveTypeDenoter() override;

    std::string                     ident;
    std::vector<ExprPtr>            arrayDims;
    std::vector<RegisterPtr>        slotRegisters;
    std::string                     textureIdent;               // Optional variable identifier of the texture object (for DX9 effect files)
    std::vector<SamplerValuePtr>    samplerValues;              // State values for a sampler decl-ident.

    SamplerDeclStmnt*               declStmntRef    = nullptr;  // Reference to its declaration statmenet (parent node).
};

// StructDecl object.
struct StructDecl : public Decl
{
    AST_INTERFACE(StructDecl);

    FLAG_ENUM
    {
        FLAG( isShaderInput,  0 ), // This structure is used as shader input.
        FLAG( isShaderOutput, 1 ), // This structure is used as shader output.
        FLAG( isNestedStruct, 2 ), // This is a nested structure within another structure.
    };

    // Returns a descriptive string of the function signature (e.g. "struct s" or "struct <anonymous>").
    std::string SignatureToString() const;

    // Returns true if this is an anonymous structure.
    bool IsAnonymous() const;

    // Returns the VarDecl AST node inside this struct decl for the specified identifier, or null if there is no such VarDecl.
    VarDecl* Fetch(const std::string& ident) const;

    // Returns a type denoter for this structure.
    TypeDenoterPtr DeriveTypeDenoter() override;

    // Returns true if this structure has at least one member that is not a system value.
    bool HasNonSystemValueMembers() const;

    // Returns the total number of members (include all base structures.
    std::size_t NumMembers() const;

    // Returns a list with the type denoters of all members (including all base structures).
    void CollectMemberTypeDenoters(std::vector<TypeDenoterPtr>& memberTypeDens) const;

    std::string                     ident;                      // May be empty (for anonymous structures).
    std::string                     baseStructName;             // May be empty (if no inheritance is used).
    std::vector<VarDeclStmntPtr>    members;

    StructDecl*                     baseStructRef   = nullptr;  // Optional reference to base struct
    std::string                     aliasName;                  // Alias name for input and output interface blocks of the DAST.
    std::map<std::string, VarDecl*> systemValuesRef;            // List of members with system value semantic (SV_...).
    std::vector<StructDecl*>        nestedStructDeclRefs;       // References to all nested structures within this structure.
};

// Type alias declaration.
struct AliasDecl : public Decl
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

    struct ParameterSemantics
    {
        void Add(VarDecl* varDecl);

        std::vector<VarDecl*> varDeclRefs;      // References to all variable declarations of the user defined semantics
        std::vector<VarDecl*> varDeclRefsSV;    // References to all variable declarations of the system value semantics
    };

    FLAG_ENUM
    {
        FLAG( isEntryPoint,            0 ), // This function is the main entry point.
        FLAG( hasNonReturnControlPath, 1 ), // At least one control path does not return a value.
    };

    // Returns true if this function declaration is just a forward declaration (without function body).
    bool IsForwardDecl() const;

    // Returns true if this function has a void return type.
    bool HasVoidReturnType() const;
    
    // Returns a descriptive string of the function signature (e.g. "void f(int x)").
    std::string SignatureToString(bool useParamNames = true) const;

    // Returns true if the specified function declaration has the same signature as this function.
    bool EqualsSignature(const FunctionDecl& rhs) const;

    // Returns the minimal number of arguments for a call to this function.
    std::size_t NumMinArgs() const;

    // Returns the maximal number of arguments for a call to this function (this is merely: parameters.size()).
    std::size_t NumMaxArgs() const;

    // Returns true if the specified type denoter matches the parameter.
    bool MatchParameterWithTypeDenoter(std::size_t paramIndex, const TypeDenoter& argType, bool implicitConversion) const;

    std::vector<AttributePtr>       attribs;                                // Attribute list
    VarTypePtr                      returnType;
    std::string                     ident;
    std::vector<VarDeclStmntPtr>    parameters;
    IndexedSemantic                 semantic        = Semantic::Undefined;  // May be undefined
    std::vector<VarDeclStmntPtr>    annotations;                            // Annotations can be ignored by analyzers and generators.
    CodeBlockPtr                    codeBlock;                              // May be null (if this AST node is a forward declaration).

    ParameterSemantics              inputSemantics;                         // Entry point input semantics.
    ParameterSemantics              outputSemantics;                        // Entry point output semantics.

    //TODO: currently unused
  //FunctionDecl*                   definitionRef   = nullptr;              // Reference to the actual function definition (only for forward declarations).
};

//TODO --> maybe separate this structure into "BufferDeclStmnt" and "BufferDecl" (like in the other declaration AST nodes)
// Uniform buffer (cbuffer, tbuffer) declaration.
struct BufferDeclStmnt : public Stmnt
{
    AST_INTERFACE(BufferDeclStmnt);

    std::string ToString() const;

    UniformBufferType               bufferType = UniformBufferType::Undefined;
    std::string                     ident;
    std::vector<RegisterPtr>        slotRegisters;
    std::vector<VarDeclStmntPtr>    members;
};

// Texture declaration.
struct TextureDeclStmnt : public Stmnt
{
    AST_INTERFACE(TextureDeclStmnt);

    BufferType                  textureType     = BufferType::Undefined;
    DataType                    colorType       = DataType::Float4;         // Sampling type. By default Float4.
    int                         numSamples      = 1;                        // Number of samples in the range [1, 128]. By default 1.
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

    bool                    isInput         = false;    // Input modifier 'in'
    bool                    isOutput        = false;    // Input modifier 'out'
    bool                    isUniform       = false;    // Input modifier 'uniform'
    std::set<StorageClass>  storageClasses;             // Storage classes (or interpolation modifiers), e.g. extern, nointerpolation, precise, etc.
    std::set<TypeModifier>  typeModifiers;              // Type modifiers, e.g. const, row_major, column_major.
    VarTypePtr              varType;
    std::vector<VarDeclPtr> varDecls;
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

    FLAG_ENUM
    {
        FLAG( isEndOfFunction, 0 ), // This return statement is at the end of its function body.
    };

    ExprPtr expr; // May be null
};

// Control transfer statement.
struct CtrlTransferStmnt : public Stmnt
{
    AST_INTERFACE(CtrlTransferStmnt);

    CtrlTransfer transfer = CtrlTransfer::Undefined; // break, continue, discard
};

/* --- Expressions --- */

// Null expression (used for dynamic array dimensions).
struct NullExpr : public Expr
{
    AST_INTERFACE(NullExpr);

    TypeDenoterPtr DeriveTypeDenoter() override;
};

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

    // Converts the data type of this literal expr. This will also modify the value string.
    void ConvertDataType(const DataType type);

    DataType        dataType    = DataType::Undefined;  // Valid data types: String, Bool, Int, UInt, Half, Float, Double
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

    ExprPtr expr; // Inner expression
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

    TypeNameExprPtr typeExpr;   // Cast type name expression
    ExprPtr         expr;       // Value expression
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
