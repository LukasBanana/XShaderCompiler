/*
 * TypeDenoter.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_TYPE_DENOTER_H
#define XSC_TYPE_DENOTER_H


#include "Visitor.h"
#include "ASTEnums.h"
#include "Flags.h"
#include "CiString.h"
#include <memory>
#include <string>


namespace Xsc
{


/* ----- Forward declaration ----- */

#define DECL_PTR(CLASS_NAME)                            \
    struct CLASS_NAME;                                  \
    using CLASS_NAME##Ptr = std::shared_ptr<CLASS_NAME>

DECL_PTR( TypeDenoter        );
DECL_PTR( VoidTypeDenoter    );
DECL_PTR( NullTypeDenoter    );
DECL_PTR( BaseTypeDenoter    );
DECL_PTR( BufferTypeDenoter  );
DECL_PTR( SamplerTypeDenoter );
DECL_PTR( StructTypeDenoter  );
DECL_PTR( AliasTypeDenoter   );
DECL_PTR( ArrayTypeDenoter   );

#undef DECL_PTR


/* ----- Helper classes ----- */

#ifdef XSC_ENABLE_LANGUAGE_EXT

struct VectorSpace
{
    using StringType = CiString;

    void Set(const StringType& space);
    void Set(const StringType& srcSpace, const StringType& dstSpace);

    StringType src; // Source vector space name.
    StringType dst; // Destination vector space name.
};

#endif


/* ----- Type denoter declarations ----- */

// Type denoter base class.
struct TypeDenoter : std::enable_shared_from_this<TypeDenoter>
{
    // Type denoter class types.
    enum class Types
    {
        Void,
        Null,
        Base,
        Buffer,
        Sampler,
        Struct,
        Alias,
        Array,
    };

    // Type denoter comparison flags.
    enum : unsigned int
    {
        // Ignore generic sub types in a buffer type denoter (for 'Equals' function).
        IgnoreGenericSubType = (1 << 0),
    };

    virtual ~TypeDenoter();

    // Returns the type (kind) of this type denoter.
    virtual Types Type() const = 0;

    // Returns a simple string representation of this type denoter (e.g. "scalar type").
    virtual std::string ToString() const = 0;

    // Returns a copy of this type denoter.
    virtual TypeDenoterPtr Copy() const = 0;

    // Shortcut to check if this is a BaseTypeDenoter of a scalar data type (see global function IsScalarType).
    virtual bool IsScalar() const;

    // Shortcut to check if this is a BaseTypeDenoter of a vector data type (see global function IsVectorType).
    virtual bool IsVector() const;

    // Shortcut to check if this is a BaseTypeDenoter of a matrix data type (see global function IsMatrixType).
    virtual bool IsMatrix() const;

    bool IsVoid() const;
    bool IsNull() const;
    bool IsBase() const;
    bool IsSampler() const;
    bool IsBuffer() const;
    bool IsStruct() const;
    bool IsAlias() const;
    bool IsArray() const;

    // Returns true if this (aliased) type denoter is equal to the specified (aliased) type denoter (see GetAliased).
    virtual bool Equals(const TypeDenoter& rhs, const Flags& compareFlags = 0) const;

    // Returns true if this type denoter can be casted to the specified target type denoter (special cases void and base types).
    virtual bool IsCastableTo(const TypeDenoter& targetType) const;

    // Returns the type identifier (if it has one), e.g. for structs and type aliases.
    virtual std::string Ident() const;

    // Sets the identifier of this type denoter if the aliased type is anonymous.
    virtual void SetIdentIfAnonymous(const std::string& ident);

    /*
    Returns a sub type denoter for the specified expression.
    If the input expression is null, the return value is a 'shared_from_this' object.
    Otherwise, the type denoter is derived by the expression,
    e.g. with an 'ArrayExpr' this type denoter is expected to be an 'ArrayTypeDenoter' and its base type is returned.
    */
    virtual TypeDenoterPtr GetSub(const Expr* expr = nullptr);

    // Returns a sub type denoter for the identifier of the specified object expression.
    virtual TypeDenoterPtr GetSubObject(const std::string& ident, const AST* ast = nullptr);

    // Returns a sub type denoter for the array indices of the specified array access expression.
    virtual TypeDenoterPtr GetSubArray(const std::size_t numArrayIndices, const AST* ast = nullptr);

    // Returns either this type denoter or an aliased type.
    virtual const TypeDenoter& GetAliased() const;

    // Returns the number of array dimensions. By default 0.
    virtual unsigned int NumDimensions() const;

    // Returns a pointer to the AST symbol reference or null if there is no such reference.
    virtual AST* SymbolRef() const;

    // Returns either this type denoter (if 'arrayDims' is empty), or this type denoter as array with the specified dimension expressions.
    virtual TypeDenoterPtr AsArray(const std::vector<ArrayDimensionPtr>& arrayDims);

    // Returns this type denoter as the specified sub class if this type denoter has the correct type. Otherwise, null is returned.
    template <typename T>
    T* As()
    {
        return (Type() == T::classType ? static_cast<T*>(this) : nullptr);
    }

    // Returns this constant type denoter as the specified sub class if this type denoter has the correct type. Otherwise, null is returned.
    template <typename T>
    const T* As() const
    {
        return (Type() == T::classType ? static_cast<const T*>(this) : nullptr);
    }

    // Find the best suitable common type denoter for both left and right hand side type denoters.
    static TypeDenoterPtr FindCommonTypeDenoter(const TypeDenoterPtr& lhsTypeDen, const TypeDenoterPtr& rhsTypeDen, bool useMinDimension = false);

    // Makes a boolean type denoter with the dimension of the specified type denoter.
    static BaseTypeDenoterPtr MakeBoolTypeWithDimensionOf(const TypeDenoter& typeDen);

    // Returns true if the specified types have an implicit vector truncation (i.e. .
    static bool HasVectorTruncation(
        const TypeDenoter& sourceTypeDen, const TypeDenoter& destTypeDen,
        int& sourceVecSize, int& destVecSize
    );

    #ifdef XSC_ENABLE_LANGUAGE_EXT
    VectorSpace vectorSpace; // Vector space of this type denoter.
    #endif
};

// Void type denoter.
struct VoidTypeDenoter : public TypeDenoter
{
    static const Types classType = Types::Void;

    Types Type() const override;
    std::string ToString() const override;
    TypeDenoterPtr Copy() const override;

    // Returns always false, since void type can not be casted to anything.
    bool IsCastableTo(const TypeDenoter& targetType) const override;
};

// Null type denoter.
struct NullTypeDenoter : public TypeDenoter
{
    static const Types classType = Types::Null;

    Types Type() const override;
    std::string ToString() const override;
    TypeDenoterPtr Copy() const override;

    bool IsCastableTo(const TypeDenoter& targetType) const override;
};

// Base type denoter.
struct BaseTypeDenoter : public TypeDenoter
{
    static const Types classType = Types::Base;

    BaseTypeDenoter() = default;
    BaseTypeDenoter(DataType dataType);

    Types Type() const override;
    std::string ToString() const override;
    TypeDenoterPtr Copy() const override;

    bool IsScalar() const override;
    bool IsVector() const override;
    bool IsMatrix() const override;

    bool Equals(const TypeDenoter& rhs, const Flags& compareFlags = 0) const override;
    bool IsCastableTo(const TypeDenoter& targetType) const override;

    TypeDenoterPtr GetSubObject(const std::string& ident, const AST* ast = nullptr) override;
    TypeDenoterPtr GetSubArray(const std::size_t numArrayIndices, const AST* ast = nullptr) override;

    DataType dataType = DataType::Undefined;    // Data type of this base type denoter. By default DataType::Undefined.
};

/*
Buffer type denoter with generic sub type and generic size. This type denoter has multiple usages:
Read-Only Buffers (e.g. "StructuredBuffer"),
Read/Write Buffers (e.g. "RWStructuredBuffer"),
Textures (e.g. "Texture2D", "Texture2DMS<int, 4>", or "RWTexture3D<int>"),
Input/Output Patches (e.g. "InputPatch<VertexInput, 4>"),
Primitive Streams (e.g. "TriangleStream<VertexInput>").
*/
struct BufferTypeDenoter : public TypeDenoter
{
    static const Types classType = Types::Buffer;

    BufferTypeDenoter() = default;
    BufferTypeDenoter(const BufferType bufferType);
    BufferTypeDenoter(BufferDecl* bufferDeclRef);

    Types Type() const override;
    std::string ToString() const override;
    TypeDenoterPtr Copy() const override;

    bool Equals(const TypeDenoter& rhs, const Flags& compareFlags = 0) const override;

    TypeDenoterPtr GetSubObject(const std::string& ident, const AST* ast = nullptr) override;
    TypeDenoterPtr GetSubArray(const std::size_t numArrayIndices, const AST* ast = nullptr) override;

    AST* SymbolRef() const override;

    // Always returns a valid generic type denoter. By default BaseTypeDenoter(Float4).
    TypeDenoterPtr GetGenericTypeDenoter() const;

    BufferType      bufferType          = BufferType::Undefined;
    TypeDenoterPtr  genericTypeDenoter;                             // May be null
    int             genericSize         = 1;                        // Either number of samples in [1, 128) (for multi-sampled textures), or patch size. By default 1.

    BufferDecl*     bufferDeclRef       = nullptr;
};

// Sampler type denoter.
struct SamplerTypeDenoter : public TypeDenoter
{
    static const Types classType = Types::Sampler;

    SamplerTypeDenoter() = default;
    SamplerTypeDenoter(const SamplerType samplerType);
    SamplerTypeDenoter(SamplerDecl* samplerDeclRef);

    Types Type() const override;
    std::string ToString() const override;
    TypeDenoterPtr Copy() const override;

    bool Equals(const TypeDenoter& rhs, const Flags& compareFlags = 0) const override;

    AST* SymbolRef() const override;

    SamplerType     samplerType     = SamplerType::Undefined;
    SamplerDecl*    samplerDeclRef  = nullptr;
};

// Struct type denoter.
struct StructTypeDenoter : public TypeDenoter
{
    static const Types classType = Types::Struct;

    StructTypeDenoter() = default;
    StructTypeDenoter(const std::string& ident);
    StructTypeDenoter(StructDecl* structDeclRef);

    Types Type() const override;
    std::string ToString() const override;
    TypeDenoterPtr Copy() const override;

    void SetIdentIfAnonymous(const std::string& ident) override;

    bool Equals(const TypeDenoter& rhs, const Flags& compareFlags = 0) const override;
    bool IsCastableTo(const TypeDenoter& targetType) const override;

    std::string Ident() const override;

    AST* SymbolRef() const override;

    TypeDenoterPtr GetSubObject(const std::string& ident, const AST* ast = nullptr) override;

    std::string     ident;                      // Type identifier

    StructDecl*     structDeclRef = nullptr;    // Reference to the StructDecl AST node
};

// Alias type denoter.
struct AliasTypeDenoter : public TypeDenoter
{
    static const Types classType = Types::Alias;

    AliasTypeDenoter() = default;
    AliasTypeDenoter(const std::string& ident);
    AliasTypeDenoter(AliasDecl* aliasDeclRef);

    Types Type() const override;
    std::string ToString() const override;
    TypeDenoterPtr Copy() const override;

    void SetIdentIfAnonymous(const std::string& ident) override;

    bool Equals(const TypeDenoter& rhs, const Flags& compareFlags = 0) const override;
    bool IsCastableTo(const TypeDenoter& targetType) const override;
    
    std::string Ident() const override;

    TypeDenoterPtr GetSub(const Expr* expr = nullptr) override;
    TypeDenoterPtr GetSubObject(const std::string& ident, const AST* ast = nullptr) override;
    TypeDenoterPtr GetSubArray(const std::size_t numArrayIndices, const AST* ast = nullptr) override;

    const TypeDenoter& GetAliased() const override;

    const TypeDenoterPtr& GetAliasedTypeOrThrow(const AST* ast = nullptr) const;

    unsigned int NumDimensions() const override;

    AST* SymbolRef() const override;

    std::string     ident;                      // Type identifier

    AliasDecl*      aliasDeclRef    = nullptr;  // Reference to the AliasDecl AST node.
};

// Array type denoter.
struct ArrayTypeDenoter : public TypeDenoter
{
    static const Types classType = Types::Array;

    ArrayTypeDenoter() = default;
    
    ArrayTypeDenoter(const TypeDenoterPtr& subTypeDenoter);
    ArrayTypeDenoter(const TypeDenoterPtr& subTypeDenoter, const std::vector<ArrayDimensionPtr>& arrayDims);

    ArrayTypeDenoter(
        const TypeDenoterPtr& subTypeDenoter,
        const std::vector<ArrayDimensionPtr>& baseArrayDims,
        const std::vector<ArrayDimensionPtr>& subArrayDims
    );

    Types Type() const override;

    // Throws std::runtime_error if 'subTypeDenoter' is null.
    std::string ToString() const override;

    TypeDenoterPtr Copy() const override;

    TypeDenoterPtr GetSubArray(const std::size_t numArrayIndices, const AST* ast = nullptr) override;

    bool Equals(const TypeDenoter& rhs, const Flags& compareFlags = 0) const override;
    bool IsCastableTo(const TypeDenoter& targetType) const override;

    unsigned int NumDimensions() const override;

    AST* SymbolRef() const override;
    
    // Returns true if the dimensions of the specified array type denoter are equal to the dimension of this array type denoter.
    bool EqualsDimensions(const ArrayTypeDenoter& rhs) const;

    // Returns a copy of this type denoter with the accumulated array dimensions.
    TypeDenoterPtr AsArray(const std::vector<ArrayDimensionPtr>& subArrayDims) override;

    // Inserts the specified sub array type denoter to this type denoter, with all its array dimension, and replaces the base type denoter.
    void InsertSubArray(const ArrayTypeDenoter& subArrayTypeDenoter);

    // Returns the array dimension sizes.
    std::vector<int> GetDimensionSizes() const;

    TypeDenoterPtr                  subTypeDenoter; // Sub type denoter
    std::vector<ArrayDimensionPtr>  arrayDims;      // Entries may be null
};


} // /namespace Xsc


#endif



// ================================================================================