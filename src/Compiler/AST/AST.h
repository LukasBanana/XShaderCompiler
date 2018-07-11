/*
 * AST.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
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
#include "Identifier.h"
#include "Variant.h"
#include <vector>
#include <initializer_list>
#include <string>
#include <set>
#include <map>
#include <functional>


namespace Xsc
{


/*
This file contains all node classes for the entire absract syntax tree (AST).
For simplicity only structs with public members are used here.
*/

class Visitor;

// Enumeration for expression finding predicates.
enum SearchFlags : unsigned int
{
    SearchLValue    = (1 << 0),
    SearchRValue    = (1 << 1),
    SearchAll       = (~0u),
};

// Iteration callback for VarDecl AST nodes.
using VarDeclIteratorFunctor = std::function<void(VarDeclPtr& varDecl)>;

// Iteration callback for Expr AST nodes.
using ExprIteratorFunctor = std::function<void(ExprPtr& expr)>;

// Iteration callback for argument/parameter-type associations.
using ArgumentParameterTypeFunctor = std::function<void(ExprPtr& argument, const TypeDenoter& paramTypeDen)>;

// Predicate callback to find an expression inside an expression tree.
using FindPredicateConstFunctor = std::function<bool(const Expr& expr)>;

// Function callback to merge two expressions into one.
using MergeExprFunctor = std::function<ExprPtr(const ExprPtr& expr0, const ExprPtr& expr1)>;


/* ----- Some helper macros ----- */

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
    // Types of AST classes.
    enum class Types
    {
        /* ----- Common AST classes ----- */

        Program,            // AST root node
        CodeBlock,
        Attribute,
        SwitchCase,
        SamplerValue,
        Register,           // Register qualifier (e.g. "register(b0)")
        PackOffset,         // Pack-offset qualifier (e.g. "packoffset(c0.x)")
        ArrayDimension,     // Array dimension (e.g. "[10]")
        TypeSpecifier,      // Type specifier with type denoter/modifiers/classes and optional structure (StructDecl)

        /* ----- Declaration objects that can be referenced by an 'ObjectExpr' ----- */

        VarDecl,            // Variable declaration
        BufferDecl,         // Buffer declaration (Texture- and Storage Buffers)
        SamplerDecl,        // Sampler state declaration
        StructDecl,         // Structure declaration
        AliasDecl,          // Type alias declaration
        FunctionDecl,       // Function declaration
        UniformBufferDecl,  // Uniform/constant buffer declaration

        /* ----- Declaration statements ----- */

        VarDeclStmnt,       // Variable declaration statement with several variables (VarDecl)
        BufferDeclStmnt,    // Buffer declaration statement with several buffers (BufferDecl)
        SamplerDeclStmnt,   // Sampler declaration statement with several samplers (SamplerDecl)
        AliasDeclStmnt,     // Type alias declaration statement with several types (AliasDecl)
        BasicDeclStmnt,     // Statement with a single declaration object (StructDecl, FunctionDecl, or UniformBufferDecl)

        /* ----- Common statements ----- */

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
        CtrlTransferStmnt,  // Control transfer statement (Break, Continue, Discard)
        LayoutStmnt,        // GLSL only

        /* ----- Expressions ----- */

        NullExpr,
        SequenceExpr,
        LiteralExpr,
        TypeSpecifierExpr,
        TernaryExpr,
        BinaryExpr,
        UnaryExpr,
        PostUnaryExpr,
        CallExpr,
        BracketExpr,
        ObjectExpr,
        AssignExpr,
        ArrayExpr,
        CastExpr,
        InitializerExpr,
    };

    virtual ~AST();

    // Returns the AST node type.
    virtual Types Type() const = 0;

    // Calls the respective visit-function of the specified visitor.
    virtual void Visit(Visitor* visitor, void* args = nullptr) = 0;

    FLAG_ENUM
    {
        FLAG( isReachable, 30 ), // This AST node is reachable from the main entry point.
        FLAG( isDeadCode,  29 ), // This AST node is dead code (after return path).
        FLAG( isBuiltin,   28 ), // This AST node is a built-in node (not part of the actual program source).
    };

    // Returns the AST node as the specified sub class if this AST node has the correct type. Otherwise, null is returned.
    template <typename T>
    static T* GetAs(AST* ast)
    {
        return (ast != nullptr && ast->Type() == T::classType ? static_cast<T*>(ast) : nullptr);
    }

    // Returns the AST node as the specified sub class if this AST node has the correct type. Otherwise, null is returned.
    template <typename T>
    static const T* GetAs(const AST* ast)
    {
        return (ast != nullptr && ast->Type() == T::classType ? static_cast<const T*>(ast) : nullptr);
    }

    // Returns this AST node as the specified sub class if this AST node has the correct type. Otherwise, null is returned.
    template <typename T>
    T* As()
    {
        return (Type() == T::classType ? static_cast<T*>(this) : nullptr);
    }

    // Returns this AST node as the specified sub class if this AST node has the correct type. Otherwise, null is returned.
    template <typename T>
    const T* As() const
    {
        return (Type() == T::classType ? static_cast<const T*>(this) : nullptr);
    }

    SourceArea  area;   // Source code area.
    Flags       flags;  // Flags bitmask (default 0).
};

/* ----- Global functions ----- */

// Returns true if the specified AST type denotes a "Decl" AST.
bool IsDeclAST(const AST::Types t);

// Returns true if the specified AST type denotes an "Expr" AST.
bool IsExprAST(const AST::Types t);

// Returns true if the specified AST type denotes a "Stmnt" AST.
bool IsStmntAST(const AST::Types t);

// Returns true if the specified AST type denotes a "...DeclStmnt" AST.
bool IsDeclStmntAST(const AST::Types t);

/* ----- Common AST classes ----- */

// Statement AST base class.
struct Stmnt : public AST
{
    // Collects all variable-, buffer-, and sampler AST nodes with their identifiers in the specified map.
    virtual void CollectDeclIdents(std::map<const AST*, std::string>& declASTIdents) const;

    std::string                 comment; // Optional commentary for this statement. May be a multi-line string.
    std::vector<AttributePtr>   attribs; // Attribute list. May be empty.
};

// AST base class with type denoter.
struct TypedAST : public AST
{

    public:

        // Returns a type denoter for this AST node or throws an std::runtime_error if a type denoter can not be derived.
        const TypeDenoterPtr& GetTypeDenoter(const TypeDenoter* expectedTypeDenoter = nullptr);

        // Resets the buffered type denoter.
        void ResetTypeDenoter();

    protected:

        virtual TypeDenoterPtr DeriveTypeDenoter(const TypeDenoter* expectedTypeDenoter) = 0;

    private:

        // Buffered type denoter which is stored in the "GetTypeDenoter" function and can be reset with the "ResetTypeDenoter" function.
        TypeDenoterPtr bufferedTypeDenoter_;

};

// Expression AST base class.
struct Expr : public TypedAST
{
    FLAG_ENUM
    {
        FLAG( wasConverted, 0 ), // This expression has already been converted.
    };

    // Returns the variable or null if this is not just a single variable expression.
    VarDecl* FetchVarDecl() const;

    /*
    Returns the first node in the expression tree that is an l-value (may also be constant!), or null if there is no l-value.
    If the return value is non-null, the object expression must refer to a declaration object. By default null.
    */
    virtual const ObjectExpr* FetchLValueExpr() const;

    // Returns the semantic of this expression, or Semantic::Undefined if this expression has no semantic.
    virtual IndexedSemantic FetchSemantic() const;

    // Returns the first expression for which the specified predicate returns true.
    virtual const Expr* Find(const FindPredicateConstFunctor& predicate, unsigned int flags = SearchAll) const;

    // Returns the first expression of the specified type in this expression tree.
    const Expr* FindFirstOf(const Types exprType, unsigned int flags = SearchAll) const;

    // Returns the first expression of a different type than the specified type in this expression tree.
    const Expr* FindFirstNotOf(const Types exprType, unsigned int flags = SearchAll) const;

    // Returns the first expression for which the specified predicate returns true.
    Expr* Find(const FindPredicateConstFunctor& predicate, unsigned int flags = SearchAll);

    // Returns the first expression of the specified type in this expression tree.
    Expr* FindFirstOf(const Types exprType, unsigned int flags = SearchAll);

    // Returns the first expression of a different type than the specified type in this expression tree.
    Expr* FindFirstNotOf(const Types exprType, unsigned int flags = SearchAll);
};

// Declaration AST base class.
struct Decl : public TypedAST
{
    FLAG_ENUM
    {
        FLAG( isWrittenTo, 0 ), // This declaration object is eventually written to.
        FLAG( isReadFrom,  1 ), // This declaration object is eventually read from.
    };

    // Returns a descriptive string of the type signature.
    virtual std::string ToString() const;

    // Returns the type specifier of this declaration object, or null if there is no type specifier. By default null.
    virtual TypeSpecifier* FetchTypeSpecifier() const;

    // Returns true if this is an anonymous structure.
    bool IsAnonymous() const;

    // Identifier of the declaration object (may be empty, e.g. for anonymous structures).
    Identifier ident;
};

// Program AST root.
struct Program : public AST
{
    AST_INTERFACE(Program);

    // Layout meta data for tessellation-control shaders
    struct LayoutTessControlShader
    {
        unsigned int    outputControlPoints     = 0;
        float           maxTessFactor           = 0.0f;
        FunctionDecl*   patchConstFunctionRef   = nullptr;
    };

    // Layout meta data for tessellation-evaluation shaders
    struct LayoutTessEvaluationShader
    {
        AttributeValue  domainType      = AttributeValue::Undefined;
        AttributeValue  partitioning    = AttributeValue::Undefined;
        AttributeValue  outputTopology  = AttributeValue::Undefined;
    };

    // Layout meta data for fragment shaders
    struct LayoutGeometryShader
    {
        PrimitiveType   inputPrimitive  = PrimitiveType::Undefined;
        BufferType      outputPrimitive = BufferType::Undefined;        // Must be PointStream, LineStream, or TriangleStream
        unsigned int    maxVertices     = 0;
    };

    // Layout meta data for fragment shaders
    struct LayoutFragmentShader
    {
        // True, if fragment coordinate (SV_Position) is used inside a fragment shader.
        bool fragCoordUsed      = false;

        // True, if pixel center is assumed to be integral, otherwise pixel center is assumed to have an (0.5, 0.5) offset.
        bool pixelCenterInteger = false;

        // True, if the [earlydepthstencil] attribute is specified for the fragment shader entry point.
        bool earlyDepthStencil  = false;
    };

    // Layout meta data for compute shaders
    struct LayoutComputeShader
    {
        unsigned int numThreads[3] = { 0 };
    };

    // Registers a usage of an intrinsic with the specified argument data types (only base types).
    void RegisterIntrinsicUsage(const Intrinsic intrinsic, const std::vector<DataType>& argumentDataTypes);

    // Registers a usage of an intrinsic with the specified arguments (only base types).
    void RegisterIntrinsicUsage(const Intrinsic intrinsic, const std::vector<ExprPtr>& arguments);

    // Returns a usage-container of the specified intrinsic or null if the specified intrinsic was not registered to be used.
    const IntrinsicUsage* FetchIntrinsicUsage(const Intrinsic intrinsic) const;

    std::vector<StmntPtr>               globalStmnts;               // Global declaration statements.

    std::vector<ASTPtr>                 disabledAST;                // AST nodes that have been disabled for code generation (not part of the default visitor).

    SourceCodePtr                       sourceCode;                 // Preprocessed source code.
    FunctionDecl*                       entryPointRef   = nullptr;  // Reference to the entry point function declaration.
    std::map<Intrinsic, IntrinsicUsage> usedIntrinsics;             // Set of all used intrinsic (filled by the reference analyzer).
    std::set<MatrixSubscriptUsage>      usedMatrixSubscripts;       // Set of all used matrix subscripts (filled by the reference analyzer).

    LayoutTessControlShader             layoutTessControl;          // Global program layout attributes for a tessellation-control shader.
    LayoutTessEvaluationShader          layoutTessEvaluation;       // Global program layout attributes for a tessellation-evaluation shader.
    LayoutGeometryShader                layoutGeometry;             // Global program layout attributes for a geometry shader.
    LayoutFragmentShader                layoutFragment;             // Global program layout attributes for a fragment shader.
    LayoutComputeShader                 layoutCompute;              // Global program layout attributes for a compute shader.
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

    std::string name;   // Sampler state name.
    ExprPtr     value;  // Sampler state value expression.
};

// Attribute (e.g. "[unroll]" or "[numthreads(x,y,z)]").
struct Attribute : public AST
{
    AST_INTERFACE(Attribute);

    std::string ToString() const;

    AttributeType           attributeType   = AttributeType::Undefined; // Type of this attribute. Must not be undefined.
    std::vector<ExprPtr>    arguments;                                  // Optional attribute arguments.
};

// Case block for a switch statement.
struct SwitchCase : public AST
{
    AST_INTERFACE(SwitchCase);

    // Returns true, if this is a default case (if 'expr' is null).
    bool IsDefaultCase() const;

    ExprPtr                 expr;   // Case expression; null for the default case.
    std::vector<StmntPtr>   stmnts; // Statement list (switch-case does not require a braced code block).
};

// Register (e.g. ": register(t0)").
struct Register : public AST
{
    AST_INTERFACE(Register);

    std::string ToString() const;

    // Returns the first slot register for the specified shader target or null, if there is no register.
    static Register* GetForTarget(const std::vector<RegisterPtr>& registers, const ShaderTarget shaderTarget);

    ShaderTarget    shaderTarget    = ShaderTarget::Undefined;  // Shader target (or profile). Undefined means all targets are affected.
    RegisterType    registerType    = RegisterType::Undefined;  // Type of the register. Must not be undefined.
    int             slot            = 0;                        // Zero-based register slot index. By default 0.
};

// Pack offset.
struct PackOffset : public AST
{
    AST_INTERFACE(PackOffset);

    std::string ToString() const;

    //TODO: change this to an enumeration (like 'RegisterType').
    std::string registerName;
    std::string vectorComponent;    // Vector component. May be empty.
};

// Array dimension with bufferd expression evaluation.
struct ArrayDimension : public TypedAST
{
    AST_INTERFACE(ArrayDimension);

    std::string ToString() const;

    TypeDenoterPtr DeriveTypeDenoter(const TypeDenoter* expectedTypeDenoter) override;

    // Returns true if this array dimension has a dynamic size (i.e. size == 0).
    bool HasDynamicSize() const;

    // Validates the boundary of the specified index, if this array dimension has a fixed size. Throws a runtime error on failure.
    void ValidateIndexBoundary(int idx) const;

    #if 0 //UNUSED
    // Returns the array dimension sizes as integral vector.
    static std::vector<int> GetArrayDimensionSizes(const std::vector<ArrayDimensionPtr>& arrayDims);
    #endif

    ExprPtr expr;           // Array dimension expression. Must be a constant integer expression.

    int     size    = 0;    // Evaluated array dimension size. Zero for dynamic array dimension.
};

// Type specifier with optional structure declaration.
struct TypeSpecifier : public TypedAST
{
    AST_INTERFACE(TypeSpecifier);

    // Returns the name of this type and all modifiers.
    std::string ToString() const;

    TypeDenoterPtr DeriveTypeDenoter(const TypeDenoter* expectedTypeDenoter) override;

    // Returns the StructDecl reference of this type denoter or null if there is no such reference.
    StructDecl* GetStructDeclRef();

    // Returns true if this is an input parameter.
    bool IsInput() const;

    // Returns true if this is an output parameter.
    bool IsOutput() const;

    // Returns true if the 'const' type modifier is set.
    bool IsConst() const;

    // Returns true if the 'const' type modifier or the 'uniform' input modifier is set.
    bool IsConstOrUniform() const;

    // Inserts the specified type modifier. Overlapping matrix packings will be removed.
    void SetTypeModifier(const TypeModifier modifier);

    // Returns true if any of the specified type modifiers is contained.
    bool HasAnyTypeModifierOf(const std::initializer_list<TypeModifier>& modifiers) const;

    // Returns true if any of the specified storage classes is contained.
    bool HasAnyStorageClassOf(const std::initializer_list<StorageClass>& modifiers) const;

    // Swaps the 'row_major' with 'column_major' storage layout, and inserts the specified default layout if none of these are set.
    void SwapMatrixStorageLayout(const TypeModifier defaultStorgeLayout);

    bool                        isInput         = false;                    // Input modifier 'in'.
    bool                        isOutput        = false;                    // Input modifier 'out'.
    bool                        isUniform       = false;                    // Input modifier 'uniform'.

    std::set<StorageClass>      storageClasses;                             // Storage classes, e.g. extern, precise, etc.
    std::set<InterpModifier>    interpModifiers;                            // Interpolation modifiers, e.g. nointerpolation, linear, centroid etc.
    std::set<TypeModifier>      typeModifiers;                              // Type modifiers, e.g. const, row_major, column_major (also 'snorm' and 'unorm' for floats).
    PrimitiveType               primitiveType   = PrimitiveType::Undefined; // Primitive type for geometry entry pointer parameters.
    StructDeclPtr               structDecl;                                 // Optional structure declaration.

    TypeDenoterPtr              typeDenoter;                                // Own type denoter.
};

/* ----- Declaration objects ----- */

// Variable declaration.
struct VarDecl : public Decl
{
    AST_INTERFACE(VarDecl);

    FLAG_ENUM
    {
        FLAG( isShaderInput,        2 ), // This variable is used as shader input.
        FLAG( isShaderOutput,       3 ), // This variable is used as shader output.
        FLAG( isSystemValue,        4 ), // This variable is a system value (e.g. SV_Position/ gl_Position).
        FLAG( isDynamicArray,       5 ), // This variable is a dynamic array (for input/output semantics).
        FLAG( isEntryPointOutput,   6 ), // This variable is used as entry point output (return value, output parameter, stream output).
        FLAG( isEntryPointLocal,    7 ), // This variable is a local variable of the entry point.

        isShaderInputSV     = (isShaderInput  | isSystemValue), // This variable is used as shader input, and it is a system value.
        isShaderOutputSV    = (isShaderOutput | isSystemValue), // This variable is used as shader output, and it is a system value.
    };

    // Returns the variable declaration as string.
    std::string ToString() const override;

    // Returns a type denoter for this variable declaration or throws an std::runtime_error if the type can not be derived.
    TypeDenoterPtr DeriveTypeDenoter(const TypeDenoter* expectedTypeDenoter) override;

    // Returns the type specifier of the declaration statemnt reference (if set).
    TypeSpecifier* FetchTypeSpecifier() const override;

    // Returns the reference to the static member variable declaration, or null if there is no such declaration (see staticMemberVarRef).
    VarDecl* FetchStaticVarDeclRef() const;

    // Returns the reference to the static member variable definition, or null if there is no such definition (see staticMemberVarRef).
    VarDecl* FetchStaticVarDefRef() const;

    // Returns true if this variable is declared as static.
    bool IsStatic() const;

    // Returns true if this variable is a function parameter.
    bool IsParameter() const;

    // Returns true if this is a non-parameter local variable with a constant initializer.
    bool HasStaticConstInitializer() const;

    // Sets a custom type denoter, or the default type denoter if the parameter is null.
    void SetCustomTypeDenoter(const TypeDenoterPtr& typeDenoter);

    // Adds the specified flag to this variable and all members and sub members of the structure, if the variable has a structure type.
    void AddFlagsRecursive(unsigned int varFlags);

    ObjectExprPtr                   namespaceExpr;                  // Optional namespace expression. May be null.
    std::vector<ArrayDimensionPtr>  arrayDims;                      // Array dimension list. May be empty.
    IndexedSemantic                 semantic;                       // Variable semantic. May be invalid.
    PackOffsetPtr                   packOffset;                     // Optional pack offset. May be null.
    std::vector<VarDeclStmntPtr>    annotations;                    // Annotations can be ignored by analyzers and generators.
    ExprPtr                         initializer;                    // Optional initializer expression. May be null.

    TypeDenoterPtr                  customTypeDenoter;              // Optional type denoter which can be different from the type of its declaration statement.
    Variant                         initializerValue;               // Optional variant of the initializer value (if the initializer is a constant expression).

    VarDeclStmnt*                   declStmntRef        = nullptr;  // Reference to its declaration statement (parent node). May be null.
    UniformBufferDecl*              bufferDeclRef       = nullptr;  // Reference to its uniform buffer declaration (optional parent-parent-node). May be null.
    StructDecl*                     structDeclRef       = nullptr;  // Reference to its owner structure declaration (optional parent-parent-node). May be null.
    VarDecl*                        staticMemberVarRef  = nullptr;  // Bi-directional reference to its static variable declaration or definition. May be null.
};

// Buffer declaration.
struct BufferDecl : public Decl
{
    AST_INTERFACE(BufferDecl);

    FLAG_ENUM
    {
        FLAG( isUsedForCompare,     2 ), // This buffer is used in a texture compare operation.
        FLAG( isUsedForImageRead,   3 ), // This is a buffer used in an image load or image atomic operation.
    };

    TypeDenoterPtr DeriveTypeDenoter(const TypeDenoter* expectedTypeDenoter) override;

    // Returns the buffer type of the parent's node type denoter.
    BufferType GetBufferType() const;

    std::vector<ArrayDimensionPtr>  arrayDims;                  // Array dimension list. May be empty.
    std::vector<RegisterPtr>        slotRegisters;              // Slot register list. May be empty.
    std::vector<VarDeclStmntPtr>    annotations;                // Annotations can be ignored by analyzers and generators.

    BufferDeclStmnt*                declStmntRef    = nullptr;  // Reference to its declaration statement (parent node).
};

// Sampler state declaration.
struct SamplerDecl : public Decl
{
    AST_INTERFACE(SamplerDecl);

    TypeDenoterPtr DeriveTypeDenoter(const TypeDenoter* expectedTypeDenoter) override;

    // Returns the sampler type of the parent's node type denoter.
    SamplerType GetSamplerType() const;

    std::vector<ArrayDimensionPtr>  arrayDims;                  // Array dimension list. May be empty.
    std::vector<RegisterPtr>        slotRegisters;              // Slot register list. May be empty.
    std::string                     textureIdent;               // Optional variable identifier of the texture object (for DX9 effect files).
    std::vector<SamplerValuePtr>    samplerValues;              // State values for a sampler decl-ident.

    SamplerDeclStmnt*               declStmntRef    = nullptr;  // Reference to its declaration statmenet (parent node).
};

// StructDecl object.
struct StructDecl : public Decl
{
    AST_INTERFACE(StructDecl);

    FLAG_ENUM
    {
        FLAG( isShaderInput,        2 ), // This structure is used as shader input.
        FLAG( isShaderOutput,       3 ), // This structure is used as shader output.
        FLAG( isNestedStruct,       4 ), // This is a nested structure within another structure.

        //TODO: rename this flag, since it's meaning is confusing
        FLAG( isNonEntryPointParam, 5 ), // This structure is eventually used as variable or parameter type of function other than the entry point.
    };

    // Returns a descriptive string of the structure signature (e.g. "struct s" or "struct <anonymous>").
    std::string ToString() const override;

    // Returns true if the specified structure declaration has the same member types as this structure (see 'TypeDenoter' for valid compare flags).
    bool EqualsMemberTypes(const StructDecl& rhs, const Flags& compareFlags = 0) const;

    // Returns true if this structure type is castable to the specified base type denoter.
    bool IsCastableTo(const BaseTypeDenoter& rhs) const;

    // Returns the VarDecl AST node inside this struct decl for the specified identifier, or null if there is no such VarDecl.
    VarDecl* FetchVarDecl(const std::string& ident, const StructDecl** owner = nullptr) const;

    // Returns the VarDecl AST node of the 'base' member variable, or null if there is no such VarDecl.
    VarDecl* FetchBaseMember() const;

    // Returns the StructDecl AST node with the specified identifier, that is a base of this structure (or the structure itself), or null if there is no such StructDecl.
    StructDecl* FetchBaseStructDecl(const std::string& ident);

    // Returns the FunctionDecl AST node for the specified argument type denoter list (used to derive the overloaded function).
    FunctionDecl* FetchFunctionDecl(
        const std::string& ident, const std::vector<TypeDenoterPtr>& argTypeDenoters,
        const StructDecl** owner = nullptr, bool throwErrorIfNoMatch = false
    ) const;

    // Returns an identifier that is similar to the specified identifier (for suggestions of typos)
    std::string FetchSimilar(const std::string& ident);

    // Returns a type denoter for this structure.
    TypeDenoterPtr DeriveTypeDenoter(const TypeDenoter* expectedTypeDenoter) override;

    // Returns true if this structure has at least one member that is not a system value.
    bool HasNonSystemValueMembers() const;

    // Returns the total number of member variables (including all base structures).
    std::size_t NumMemberVariables(bool onlyNonStaticMembers = false) const;

    // Returns the total number of member functions (including all base structures).
    std::size_t NumMemberFunctions(bool onlyNonStaticMembers = false) const;

    // Returns a list with the type denoters of all members (including all base structures, if enabled).
    void CollectMemberTypeDenoters(std::vector<TypeDenoterPtr>& memberTypeDens, bool includeBaseStructs = true) const;

    // Iterates over each VarDecl AST node (included nested structures, and members in base structures).
    void ForEachVarDecl(const VarDeclIteratorFunctor& iterator, bool includeBaseStructs = true);

    // Adds the specified variable as an instance of this structure, that is used as shader output (see HasMultipleShaderOutputInstances).
    void AddShaderOutputInstance(VarDecl* varDecl);

    // Returns true if this structure is used more than once as entry point output (either through variable arrays or multiple variable declarations).
    bool HasMultipleShaderOutputInstances() const;

    // Returns true if this structure is a base of the specified sub structure.
    bool IsBaseOf(const StructDecl* subStructDecl, bool includeSelf = false) const;

    // Adds the specified flags to this structure, all base structures, all nested structures, and all structures of its member variables.
    void AddFlagsRecursive(unsigned int structFlags);

    // Adds the specified flags to this structure, and all parent structures (i.e. all structures that have a member variable with this structure type).
    void AddFlagsRecursiveParents(unsigned int structFlags);

    // Returns the zero-based index of the specified member variable, or ~0 on failure.
    std::size_t MemberVarToIndex(const VarDecl* varDecl, bool includeBaseStructs = true) const;

    // Returns the member variable of the specified zero-based index, null on failure.
    VarDecl* IndexToMemberVar(std::size_t idx, bool includeBaseStructs = true) const;

    bool                            isClass                 = false;    // This struct was declared as 'class'.
    std::string                     baseStructName;                     // May be empty (if no inheritance is used).
    std::vector<StmntPtr>           localStmnts;                        // Local declaration statements.

    //TODO: maybe replace "VarDeclStmntPtr" by "VarDeclPtr" here.
    std::vector<VarDeclStmntPtr>    varMembers;                         // List of all member variable declaration statements.
    std::vector<FunctionDeclPtr>    funcMembers;                        // List of all member function declarations.

    BasicDeclStmnt*                 declStmntRef            = nullptr;  // Reference to its declaration statement (parent node).
    StructDecl*                     baseStructRef           = nullptr;  // Optional reference to base struct.
    StructDecl*                     compatibleStructRef     = nullptr;  // Optional reference to a type compatible struct (only for anonymous structs).
    std::map<std::string, VarDecl*> systemValuesRef;                    // List of members with system value semantic (SV_...).
    std::set<StructDecl*>           parentStructDeclRefs;               // References to all structures that have a member variable with this structure type.
    std::set<VarDecl*>              shaderOutputVarDeclRefs;            // References to all variables from this structure that are used as entry point outputs.
};

// Type alias declaration.
struct AliasDecl : public Decl
{
    AST_INTERFACE(AliasDecl);

    TypeDenoterPtr DeriveTypeDenoter(const TypeDenoter* expectedTypeDenoter) override;

    TypeDenoterPtr  typeDenoter;            // Type denoter of the aliased type.

    AliasDeclStmnt* declStmntRef = nullptr; // Reference to its declaration statement (parent node).
};

// Function declaration.
struct FunctionDecl : public Decl
{
    AST_INTERFACE(FunctionDecl);

    struct ParameterSemantics
    {
        using IteratorFunc = std::function<void(VarDecl* varDecl)>;

        void Add(VarDecl* varDecl);
        bool Contains(VarDecl* varDecl) const;
        void ForEach(const IteratorFunc& iterator);

        // Returns true if both lists are empty.
        bool Empty() const;

        // Updates the distribution of system-value and non-system-value semantics.
        void UpdateDistribution();

        std::vector<VarDecl*> varDeclRefs;      // References to all variable declarations of the user defined semantics
        std::vector<VarDecl*> varDeclRefsSV;    // References to all variable declarations of the system value semantics
    };

    struct ParameterStructure
    {
        Expr*       expr;       // Either this is used ...
        VarDecl*    varDecl;    // ... or this
        StructDecl* structDecl;
    };

    FLAG_ENUM
    {
        FLAG( isEntryPoint,            0 ), // This function is the main entry point.
        FLAG( isSecondaryEntryPoint,   1 ), // This function is a secondary entry point (e.g. patch constant function).
        FLAG( hasNonReturnControlPath, 2 ), // At least one control path does not return a value.
    };

    TypeDenoterPtr DeriveTypeDenoter(const TypeDenoter* expectedTypeDenoter) override;

    // Returns true if this function declaration is just a forward declaration (without function body).
    bool IsForwardDecl() const;

    // Returns true if this function has a void return type.
    bool HasVoidReturnType() const;

    // Returns true if this is a member function (member of a structure).
    bool IsMemberFunction() const;

    // Returns true if this is a static function (see TypeSpecifier::HasAnyStorageClassOf).
    bool IsStatic() const;

    // Returns a descriptive string of the function signature (e.g. "void f(int x)").
    std::string ToString() const override;

    // Returns a descriptive string of the function signature (e.g. "void f(int x)").
    std::string ToString(bool useParamNames) const;

    // Returns a descriptive strinf of the type of this function object (e.g. "void(int)").
    std::string ToTypeDenoterString() const;

    // Returns true if the specified function declaration has the same signature as this function (see 'TypeDenoter' for valid compare flags).
    bool EqualsSignature(const FunctionDecl& rhs, const Flags& compareFlags = 0) const;

    // Returns the minimal number of arguments for a call to this function.
    std::size_t NumMinArgs() const;

    // Returns the maximal number of arguments for a call to this function (this is merely: parameters.size()).
    std::size_t NumMaxArgs() const;

    // Sets the specified function AST node as the implementation of this forward declaration.
    void SetFuncImplRef(FunctionDecl* funcDecl);

    // Returns true if the specified type denoter matches the parameter.
    bool MatchParameterWithTypeDenoter(std::size_t paramIndex, const TypeDenoter& argType, bool implicitConversion) const;

    // Fetches the function declaration from the list that matches the specified argument types.
    static FunctionDecl* FetchFunctionDeclFromList(
        const std::vector<FunctionDecl*>& funcDeclList,
        const std::string& ident, const std::vector<TypeDenoterPtr>& argTypeDenoters,
        bool throwErrorIfNoMatch = true
    );

    TypeSpecifierPtr                returnType;                                 // Function return type (TypeSpecifier).
    std::vector<VarDeclStmntPtr>    parameters;                                 // Function parameter list.
    IndexedSemantic                 semantic            = Semantic::Undefined;  // Function return semantic; may be undefined.
    std::vector<VarDeclStmntPtr>    annotations;                                // Annotations can be ignored by analyzers and generators.
    CodeBlockPtr                    codeBlock;                                  // May be null (if this AST node is a forward declaration).

    ParameterSemantics              inputSemantics;                             // Entry point input semantics.
    ParameterSemantics              outputSemantics;                            // Entry point output semantics.

    BasicDeclStmnt*                 declStmntRef        = nullptr;              // Reference to its declaration statement (parent node). Must not be null.
    FunctionDecl*                   funcImplRef         = nullptr;              // Reference to the function implementation (only for forward declarations).
    std::vector<FunctionDecl*>      funcForwardDeclRefs;                        // Reference to all forward declarations (only for implementations).
    StructDecl*                     structDeclRef       = nullptr;              // Structure declaration reference if this is a member function; may be null

    std::vector<ParameterStructure> paramStructs;                               // Parameter with structure type (only for entry point).
};

// Uniform buffer (cbuffer, tbuffer) declaration.
struct UniformBufferDecl : public Decl
{
    AST_INTERFACE(UniformBufferDecl);

    TypeDenoterPtr DeriveTypeDenoter(const TypeDenoter* expectedTypeDenoter) override;

    std::string ToString() const override;

    // Derives the common storage layout type modifier of all variable members (see 'commonStorageLayout' member).
    TypeModifier DeriveCommonStorageLayout(const TypeModifier defaultStorgeLayout = TypeModifier::Undefined);

    UniformBufferType               bufferType          = UniformBufferType::Undefined; // Type of this uniform buffer. Must not be undefined.
    std::vector<RegisterPtr>        slotRegisters;                                      // Slot register list. May be empty.
    std::vector<StmntPtr>           localStmnts;                                        // Local declaration statements.

    std::vector<VarDeclStmntPtr>    varMembers;                                         // List of all member variable declaration statements.
    TypeModifier                    commonStorageLayout = TypeModifier::ColumnMajor;    // Type modifier of the common matrix/vector storage.

    BasicDeclStmnt*                 declStmntRef        = nullptr;                      // Reference to its declaration statement (parent node). Must not be null.
};

/* ----- Declaration statements ----- */

// Buffer (and texture) declaration.
struct BufferDeclStmnt : public Stmnt
{
    AST_INTERFACE(BufferDeclStmnt);

    // Implements Stmnt::CollectDeclIdents
    void CollectDeclIdents(std::map<const AST*, std::string>& declASTIdents) const override;

    BufferTypeDenoterPtr        typeDenoter;    // Own type denoter.
    std::vector<BufferDeclPtr>  bufferDecls;    // Buffer declaration list.
};

// Sampler declaration.
struct SamplerDeclStmnt : public Stmnt
{
    AST_INTERFACE(SamplerDeclStmnt);

    // Implements Stmnt::CollectDeclIdents
    void CollectDeclIdents(std::map<const AST*, std::string>& declASTIdents) const override;

    SamplerTypeDenoterPtr       typeDenoter;    // Own type denoter.
    std::vector<SamplerDeclPtr> samplerDecls;   // Sampler declaration list.
};

// Basic declaration statement.
struct BasicDeclStmnt : public Stmnt
{
    AST_INTERFACE(BasicDeclStmnt);

    DeclPtr declObject;   // Declaration object.
};

// Variable declaration statement.
struct VarDeclStmnt : public Stmnt
{
    AST_INTERFACE(VarDeclStmnt);

    FLAG_ENUM
    {
        FLAG( isShaderInput,    0 ), // This variable is used as shader input.
        FLAG( isShaderOutput,   1 ), // This variable is used as shader output.
        FLAG( isParameter,      2 ), // This variable is a function parameter (flag should be set during parsing).
        FLAG( isSelfParameter,  3 ), // This variable is the 'self' parameter of a member function.
        FLAG( isBaseMember,     4 ), // This variable is the 'base' member of a structure with inheritance.
        FLAG( isImplicitConst,  5 ), // This variable is implicitly declared as constant.
    };

    // Implements Stmnt::CollectDeclIdents
    void CollectDeclIdents(std::map<const AST*, std::string>& declASTIdents) const override;

    // Returns the var-decl statement as string.
    std::string ToString(bool useVarNames = true) const;

    // Returns the VarDecl AST node inside this var-decl statement for the specified identifier, or null if there is no such VarDecl.
    VarDecl* FetchVarDecl(const std::string& ident) const;

    // Returns true if this is an input parameter.
    bool IsInput() const;

    // Returns true if this is an output parameter.
    bool IsOutput() const;

    // Returns true if this is a uniform.
    bool IsUniform() const;

    // Returns true if the 'const' type modifier or the 'uniform' input modifier is set.
    bool IsConstOrUniform() const;

    // Inserts the specified type modifier. Overlapping matrix packings will be removed.
    void SetTypeModifier(const TypeModifier modifier);

    // Returns true if any of the specified type modifiers is contained.
    bool HasAnyTypeModifierOf(const std::initializer_list<TypeModifier>& modifiers) const;

    // Iterates over each VarDecl AST node.
    void ForEachVarDecl(const VarDeclIteratorFunctor& iterator);

    // Makes this var-decl statement implicitly constant, iff not explicitly declared as constant (see 'isUniform' and 'isImplicitlyConst').
    void MakeImplicitConst();

    // Returns the reference to the owner structure from the first variable entry, or null if there is no such owner structure.
    StructDecl* FetchStructDeclRef() const;

    TypeSpecifierPtr        typeSpecifier;  // Type basis for all variables (can be extended by array indices in each individual variable).
    std::vector<VarDeclPtr> varDecls;       // Variable declaration list.
};

// Type alias declaration statement.
struct AliasDeclStmnt : public Stmnt
{
    AST_INTERFACE(AliasDeclStmnt);

    StructDeclPtr               structDecl; // Optional structure declaration. May be null.
    std::vector<AliasDeclPtr>   aliasDecls; // Alias declaration list.
};

/* ----- Statements ----- */

// Null statement.
struct NullStmnt : public Stmnt
{
    AST_INTERFACE(NullStmnt);
};

// Code block statement.
struct CodeBlockStmnt : public Stmnt
{
    AST_INTERFACE(CodeBlockStmnt);

    CodeBlockPtr codeBlock; // Code block.
};

// 'for'-loop statemnet.
struct ForLoopStmnt : public Stmnt
{
    AST_INTERFACE(ForLoopStmnt);

    StmntPtr    initStmnt;  // Initializer statement. Must not be null, but can be an instance of 'NullStmnt'.
    ExprPtr     condition;  // Condition expresion. May be null.
    ExprPtr     iteration;  // Loop iteration expression. May be null.
    StmntPtr    bodyStmnt;  // Loop body statement.
};

// 'while'-loop statement.
struct WhileLoopStmnt : public Stmnt
{
    AST_INTERFACE(WhileLoopStmnt);

    ExprPtr     condition;  // Condition expression.
    StmntPtr    bodyStmnt;  // Loop body statement.
};

// 'do/while'-loop statement.
struct DoWhileLoopStmnt : public Stmnt
{
    AST_INTERFACE(DoWhileLoopStmnt);

    StmntPtr    bodyStmnt;  // Loop body statement.
    ExprPtr     condition;  // Condition expression.
};

// 'if' statement.
struct IfStmnt : public Stmnt
{
    AST_INTERFACE(IfStmnt);

    ExprPtr         condition;  // Condition expression.
    StmntPtr        bodyStmnt;  // 'then'-branch body statement.
    ElseStmntPtr    elseStmnt;  // 'else'-branch statement. May be null.
};

// 'else' statement.
struct ElseStmnt : public Stmnt
{
    AST_INTERFACE(ElseStmnt);

    StmntPtr bodyStmnt; // 'else'-branch body statement.
};

// 'switch' statement.
struct SwitchStmnt : public Stmnt
{
    AST_INTERFACE(SwitchStmnt);

    ExprPtr                     selector;   // Switch selector expression.
    std::vector<SwitchCasePtr>  cases;      // Switch case list.
};

// Arbitrary expression statement.
struct ExprStmnt : public Stmnt
{
    AST_INTERFACE(ExprStmnt);

    ExprPtr expr; // Common expression.
};

// Returns statement.
struct ReturnStmnt : public Stmnt
{
    AST_INTERFACE(ReturnStmnt);

    FLAG_ENUM
    {
        FLAG( isEndOfFunction, 0 ), // This return statement is at the end of its function body.
    };

    ExprPtr expr; // Return statement expression. May be null.
};

// Control transfer statement.
struct CtrlTransferStmnt : public Stmnt
{
    AST_INTERFACE(CtrlTransferStmnt);

    CtrlTransfer transfer = CtrlTransfer::Undefined; // Control transfer type (break, continue, discard). Must not be undefined.
};

struct LayoutStmnt : public Stmnt
{
    AST_INTERFACE(LayoutStmnt);

    bool isInput    = false; // Input modifier 'in'.
    bool isOutput   = false; // Input modifier 'out'.
};

/* ----- Expressions ----- */

// Null expression (used for dynamic array dimensions).
struct NullExpr : public Expr
{
    AST_INTERFACE(NullExpr);

    TypeDenoterPtr DeriveTypeDenoter(const TypeDenoter* expectedTypeDenoter) override;
};

// List expression ( expr (',' expr)+ ).
struct SequenceExpr : public Expr
{
    AST_INTERFACE(SequenceExpr);

    TypeDenoterPtr DeriveTypeDenoter(const TypeDenoter* expectedTypeDenoter) override;

    const Expr* Find(const FindPredicateConstFunctor& predicate, unsigned int flags = SearchAll) const override;

    // Appens the specified expression to the sub expressions ('SequenceExpr' will be flattened).
    void Append(const ExprPtr& expr);

    std::vector<ExprPtr> exprs; // Sub expression list. Must have at least two elements.
};

// Literal expression.
struct LiteralExpr : public Expr
{
    AST_INTERFACE(LiteralExpr);

    TypeDenoterPtr DeriveTypeDenoter(const TypeDenoter* expectedTypeDenoter) override;

    // Converts the data type of this literal expr, resets the buffered type denoter (see ResetTypeDenoter), and modifies the value string.
    void ConvertDataType(const DataType type);

    // Returns the value of this literal if it is a string literal (excluding the quotation marks). Otherwise an empty string is returned.
    std::string GetStringValue() const;

    // Returns true if this is a NULL literal.
    bool IsNull() const;

    // Returns true if this literal needs a space after the literal, when a vector subscript is used as postfix.
    bool IsSpaceRequiredForSubscript() const;

    std::string     value;                              // Literal expression value.

    DataType        dataType    = DataType::Undefined;  // Valid data types: String, Bool, Int, UInt, Half, Float, Double. Undefined for 'NULL'.
};

// Type name expression (used for simpler cast-expression parsing).
struct TypeSpecifierExpr : public Expr
{
    AST_INTERFACE(TypeSpecifierExpr);

    TypeDenoterPtr DeriveTypeDenoter(const TypeDenoter* expectedTypeDenoter) override;

    TypeSpecifierPtr typeSpecifier; // Type specifier.
};

// Ternary expression.
struct TernaryExpr : public Expr
{
    AST_INTERFACE(TernaryExpr);

    TypeDenoterPtr DeriveTypeDenoter(const TypeDenoter* expectedTypeDenoter) override;

    const Expr* Find(const FindPredicateConstFunctor& predicate, unsigned int flags = SearchAll) const override;

    // Returns true if the conditional expression is a vector type.
    bool IsVectorCondition() const;

    ExprPtr condExpr; // Ternary condition expression.
    ExprPtr thenExpr; // 'then'-branch expression.
    ExprPtr elseExpr; // 'else'-branch expression.
};

// Binary expression.
struct BinaryExpr : public Expr
{
    AST_INTERFACE(BinaryExpr);

    TypeDenoterPtr DeriveTypeDenoter(const TypeDenoter* expectedTypeDenoter) override;

    const Expr* Find(const FindPredicateConstFunctor& predicate, unsigned int flags = SearchAll) const override;

    ExprPtr     lhsExpr;                        // Left-hand-side expression.
    BinaryOp    op      = BinaryOp::Undefined;  // Binary operator. Must not be undefined.
    ExprPtr     rhsExpr;                        // Right-hand-side expression.
};

// (Pre-) Unary expression.
struct UnaryExpr : public Expr
{
    AST_INTERFACE(UnaryExpr);

    TypeDenoterPtr DeriveTypeDenoter(const TypeDenoter* expectedTypeDenoter) override;

    const Expr* Find(const FindPredicateConstFunctor& predicate, unsigned int flags = SearchAll) const override;

    const ObjectExpr* FetchLValueExpr() const override;

    UnaryOp op      = UnaryOp::Undefined;   // Unary operator. Must not be undefined.
    ExprPtr expr;                           // Right-hand-side expression.
};

// Post unary expression (e.g. x++, x--)
struct PostUnaryExpr : public Expr
{
    AST_INTERFACE(PostUnaryExpr);

    TypeDenoterPtr DeriveTypeDenoter(const TypeDenoter* expectedTypeDenoter) override;

    const Expr* Find(const FindPredicateConstFunctor& predicate, unsigned int flags = SearchAll) const override;

    ExprPtr expr;                           // Left-hand-side expression.
    UnaryOp op      = UnaryOp::Undefined;   // Unary operator. Must not be undefined.
};

// Function call expression (e.g. "foo()" or "foo().bar()" or "foo()[0].bar()").
struct CallExpr : public Expr
{
    AST_INTERFACE(CallExpr);

    FLAG_ENUM
    {
        // If this function call is an intrinsic, it's wrapper function can be inlined (i.e. no wrapper function must be generated)
        // e.g. "clip(a), clip(b);" can not be inlined, due to the list expression.
        FLAG( canInlineIntrinsicWrapper, 0 ),

        // This is a call expression to a wrapper function (use expression identifier, not from a function reference).
        FLAG( isWrapperCall,             1 ),
    };

    TypeDenoterPtr DeriveTypeDenoter(const TypeDenoter* expectedTypeDenoter) override;

    const Expr* Find(const FindPredicateConstFunctor& predicate, unsigned int flags = SearchAll) const override;

    IndexedSemantic FetchSemantic() const override;

    // Returns a list of all argument expressions (including the default parameters).
    std::vector<Expr*> GetArguments() const;

    // Returns the reference to the function declaration of this call expression, or null if not set.
    FunctionDecl* GetFunctionDecl() const;

    // Returns the reference to the function implementation of this call expression, or null if not set.
    FunctionDecl* GetFunctionImpl() const;

    // Iterates over each argument expression that is assigned to an output parameter.
    void ForEachOutputArgument(const ExprIteratorFunctor& iterator);

    // Iterates over each argument expression together with its associated parameter.
    void ForEachArgumentWithParameterType(const ArgumentParameterTypeFunctor& iterator);

    // Inserts the specified argument expression at the front of the argument list.
    void PushArgumentFront(const ExprPtr& expr);

    // Moves the prefix expression into the argument list as first argument.
    bool PushPrefixToArguments();

    // Moves the first argument out of the argument list as prefix expression.
    bool PopPrefixFromArguments();

    // Merges the argument with index 'firstArgIndex' and the next argument with the merge function callback.
    bool MergeArguments(std::size_t firstArgIndex, const MergeExprFunctor& mergeFunctor);

    // Returns the expression which denotes the member function class instance (either the prefix expression or the first argument); see PushPrefixToArguments.
    Expr* GetMemberFuncObjectExpr() const;

    ExprPtr                 prefixExpr;                                 // Optional prefix expression. May be null.
    bool                    isStatic            = false;                // Specifies whether this function is a static member.
    std::string             ident;                                      // Function name identifier. Empty for type constructors.
    TypeDenoterPtr          typeDenoter;                                // Null, if the function call is NOT a type constructor (e.g. "float2(0, 0)").
    std::vector<ExprPtr>    arguments;                                  // Argument expression list.

    FunctionDecl*           funcDeclRef         = nullptr;              // Reference to the function declaration. May be null.
    Intrinsic               intrinsic           = Intrinsic::Undefined; // Intrinsic ID (if this is an intrinsic).
    std::vector<Expr*>      defaultArgumentRefs;                        // Reference to default argument expressions of all remaining parameters.
};

// Bracket expression.
struct BracketExpr : public Expr
{
    AST_INTERFACE(BracketExpr);

    TypeDenoterPtr DeriveTypeDenoter(const TypeDenoter* expectedTypeDenoter) override;

    const Expr* Find(const FindPredicateConstFunctor& predicate, unsigned int flags = SearchAll) const override;

    const ObjectExpr* FetchLValueExpr() const override;

    IndexedSemantic FetchSemantic() const override;

    ExprPtr expr; // Inner expression.
};

// Assignment expression.
struct AssignExpr : public Expr
{
    AST_INTERFACE(AssignExpr);

    TypeDenoterPtr DeriveTypeDenoter(const TypeDenoter* expectedTypeDenoter) override;

    const Expr* Find(const FindPredicateConstFunctor& predicate, unsigned int flags = SearchAll) const override;

    const ObjectExpr* FetchLValueExpr() const override;

    ExprPtr     lvalueExpr;                         // L-value expression.
    AssignOp    op          = AssignOp::Undefined;  // Assignment operator. Must not be undefined.
    ExprPtr     rvalueExpr;                         // R-value expression.
};

// Object access expression.
struct ObjectExpr : public Expr
{
    AST_INTERFACE(ObjectExpr);

    FLAG_ENUM
    {
        FLAG( isImmutable,           1 ), // This object identifier must be written out as it is.
        FLAG( isBaseStructNamespace, 2 ), // This expression is a base structure namespace expression.
    };

    TypeDenoterPtr DeriveTypeDenoter(const TypeDenoter* expectedTypeDenoter) override;

    const Expr* Find(const FindPredicateConstFunctor& predicate, unsigned int flags = SearchAll) const override;

    const ObjectExpr* FetchLValueExpr() const override;

    IndexedSemantic FetchSemantic() const override;

    // Returns a type denoter for the vector subscript of this object expression or throws an runtime error on failure.
    BaseTypeDenoterPtr GetTypeDenoterFromSubscript() const;

    // Returns this object expression as a static namespace, i.e. only for prefix expression that are also from type ObjectExpr.
    std::string ToStringAsNamespace() const;

    // Replaces the old symbol and identifier by the new symbol.
    void ReplaceSymbol(Decl* symbol);

    // Returns the specified type of AST node from the symbol (if the symbol refers to one).
    template <typename T>
    T* FetchSymbol() const
    {
        if (symbolRef)
        {
            if (auto ast = symbolRef->As<T>())
                return ast;
        }
        return nullptr;
    }

    // Returns the variable AST node (if the symbol refers to one).
    VarDecl* FetchVarDecl() const;

    ExprPtr     prefixExpr;             // Optional prefix expression. May be null.
    bool        isStatic    = false;    // Specifies whether this object is a static member.
    std::string ident;                  // Object identifier.

    Decl*       symbolRef   = nullptr;  // Optional symbol reference to the object declaration. May be null (e.g. for vector subscripts).
};

// Array-access expression (e.g. "foo()[arrayAccess]").
struct ArrayExpr : public Expr
{
    AST_INTERFACE(ArrayExpr);

    TypeDenoterPtr DeriveTypeDenoter(const TypeDenoter* expectedTypeDenoter) override;

    const Expr* Find(const FindPredicateConstFunctor& predicate, unsigned int flags = SearchAll) const override;

    const ObjectExpr* FetchLValueExpr() const override;

    // Returns the number of array indices (shortcut for "arrayIndices.size()").
    std::size_t NumIndices() const;

    ExprPtr                 prefixExpr;     // Prefix expression.
    std::vector<ExprPtr>    arrayIndices;   // Array index expression list (right hand side).
};

// Cast expression.
struct CastExpr : public Expr
{
    AST_INTERFACE(CastExpr);

    TypeDenoterPtr DeriveTypeDenoter(const TypeDenoter* expectedTypeDenoter) override;

    const Expr* Find(const FindPredicateConstFunctor& predicate, unsigned int flags = SearchAll) const override;

    TypeSpecifierPtr    typeSpecifier;  // Destination type specifier.
    ExprPtr             expr;           // Source value expression.
};

// Initializer list expression.
struct InitializerExpr : public Expr
{
    AST_INTERFACE(InitializerExpr);

    TypeDenoterPtr DeriveTypeDenoter(const TypeDenoter* expectedTypeDenoter) override;

    const Expr* Find(const FindPredicateConstFunctor& predicate, unsigned int flags = SearchAll) const override;

    // Returns the number of elements with all sub initializers being unrollwed (e.g. { 1, { 2, 3 } } has 3 'unrolled' elements just like { 1, 2, 3 }).
    std::size_t NumElementsUnrolled() const;

    // Collects all elements from the sub expressions without initializer lists (e.g. { 1, { 2, 3 } } --> { 1, 2, 3 }).
    void CollectElements(std::vector<ExprPtr>& elements) const;

    // Resolves all initializer lists in the sub expression.
    void UnrollElements();

    // Fetches the sub expression with the specified array indices and throws an ASTRuntimeError on failure.
    ExprPtr FetchSubExpr(const std::vector<int>& arrayIndices) const;

    // Returns the next array indices for a sub expression.
    bool NextArrayIndices(std::vector<int>& arrayIndices) const;

    std::vector<ExprPtr> exprs; // Sub expression list.
};


#undef AST_INTERFACE
#undef DECL_AST_ALIAS
#undef FLAG_ENUM
#undef FLAG


} // /namespace Xsc


#endif



// ================================================================================
