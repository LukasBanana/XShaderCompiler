/*
 * TypeDenoter.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_TYPE_DENOTER_H
#define XSC_TYPE_DENOTER_H


#include "Visitor.h"
#include "ASTEnums.h"
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
DECL_PTR( BaseTypeDenoter    );
DECL_PTR( BufferTypeDenoter  );
DECL_PTR( SamplerTypeDenoter );
DECL_PTR( StructTypeDenoter  );
DECL_PTR( AliasTypeDenoter   );
DECL_PTR( ArrayTypeDenoter   );

#undef DECL_PTR


/* ----- Type denoter declarations ----- */

// Type denoter base class.
struct TypeDenoter : std::enable_shared_from_this<TypeDenoter>
{
    enum class Types
    {
        Void,
        Base,
        Buffer,
        Sampler,
        Struct,
        Alias,
        Array,
    };

    virtual ~TypeDenoter();

    // Returns the type (kind) of this type denoter.
    virtual Types Type() const = 0;

    // Returns a simple string representation of this type denoter (e.g. "scalar type").
    virtual std::string ToString() const = 0;

    // Returns a copy of this type denoter.
    //virtual TypeDenoterPtr Copy() const = 0;

    virtual bool IsScalar() const;
    virtual bool IsVector() const;
    virtual bool IsMatrix() const;

    bool IsVoid() const;
    bool IsBase() const;
    bool IsSampler() const;
    bool IsBuffer() const;
    bool IsStruct() const;
    bool IsAlias() const;
    bool IsArray() const;

    // Returns true if this type denoter is equal to the specified type denoter.
    virtual bool Equals(const TypeDenoter& rhs) const;

    // Returns true if this type denoter can be casted to the specified target type denoter (special cases void and base types).
    virtual bool IsCastableTo(const TypeDenoter& targetType) const;

    // Returns the type identifier (if it has one), e.g. for structs and type aliases.
    virtual std::string Ident() const;

    // Sets the identifier of this type denoter if the aliased type is anonymous.
    virtual void SetIdentIfAnonymous(const std::string& ident);

    // Returns a type denoter for the specified full variable identifier. Throws ASTRuntimeError on failure.
    virtual TypeDenoterPtr Get(const VarIdent* varIdent = nullptr);

    // Returns a type denoter for the specified array access and full variable identifier. Throws ASTRuntimeError on failure.
    virtual TypeDenoterPtr GetFromArray(std::size_t numArrayIndices, const VarIdent* varIdent = nullptr);

    // Returns either this type denoter or an aliased type.
    virtual const TypeDenoter& GetAliased() const;

    // Returns the number of array dimensions. By default 0.
    virtual unsigned int NumDimensions() const;

    // Returns a pointer to the AST symbol reference or null if there is no such reference.
    virtual AST* SymbolRef() const;

    // Returns either this type denoter (if 'arrayDims' is empty), or this type denoter as array with the specified dimension expressions.
    TypeDenoterPtr AsArray(const std::vector<ArrayDimensionPtr>& arrayDims);

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
};

// Void type denoter.
struct VoidTypeDenoter : public TypeDenoter
{
    static const Types classType = Types::Void;

    Types Type() const override;
    std::string ToString() const override;

    // Returns always false, since void type can not be casted to anything.
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

    bool IsScalar() const override;
    bool IsVector() const override;
    bool IsMatrix() const override;

    bool Equals(const TypeDenoter& rhs) const override;
    bool IsCastableTo(const TypeDenoter& targetType) const override;

    TypeDenoterPtr Get(const VarIdent* varIdent = nullptr) override;
    TypeDenoterPtr GetFromArray(std::size_t numArrayIndices, const VarIdent* varIdent = nullptr) override;

    DataType dataType = DataType::Undefined;
};

// Buffer type denoter.
struct BufferTypeDenoter : public TypeDenoter
{
    static const Types classType = Types::Buffer;

    BufferTypeDenoter() = default;
    BufferTypeDenoter(const BufferType bufferType);
    BufferTypeDenoter(BufferDecl* bufferDeclRef);

    Types Type() const override;
    std::string ToString() const override;

    TypeDenoterPtr GetFromArray(std::size_t numArrayIndices, const VarIdent* varIdent = nullptr) override;

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

    std::string Ident() const override;
    void SetIdentIfAnonymous(const std::string& ident) override;

    AST* SymbolRef() const override;

    TypeDenoterPtr Get(const VarIdent* varIdent = nullptr) override;

    std::string     ident;

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

    std::string Ident() const override;
    void SetIdentIfAnonymous(const std::string& ident) override;

    TypeDenoterPtr Get(const VarIdent* varIdent = nullptr) override;
    TypeDenoterPtr GetFromArray(std::size_t numArrayIndices, const VarIdent* varIdent = nullptr) override;

    const TypeDenoter& GetAliased() const override;

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
    ArrayTypeDenoter(const TypeDenoterPtr& baseTypeDenoter);
    ArrayTypeDenoter(const TypeDenoterPtr& baseTypeDenoter, const std::vector<ArrayDimensionPtr>& arrayDims);

    Types Type() const override;

    // Throws std::runtime_error if 'baseTypeDenoter' is null.
    std::string ToString() const override;

    TypeDenoterPtr Get(const VarIdent* varIdent = nullptr) override;
    TypeDenoterPtr GetFromArray(std::size_t numArrayIndices, const VarIdent* varIdent = nullptr) override;

    bool Equals(const TypeDenoter& rhs) const override;
    bool IsCastableTo(const TypeDenoter& targetType) const override;

    // Returns the base type denoter or a new array type denoter with (|arrayDims| - |numArrayIndices|) dimensions.
    TypeDenoterPtr GetWithIndices(std::size_t numArrayIndices, const VarIdent* varIdent);

    unsigned int NumDimensions() const override;

    AST* SymbolRef() const override;

    // Inserts the specified sub array type denoter to this type denoter, with all its array dimension, and replaces the base type denoter.
    void InsertSubArray(const ArrayTypeDenoter& subArrayTypeDenoter);

    // Returns the array dimension sizes.
    std::vector<int> GetDimensionSizes() const;

    TypeDenoterPtr                  baseTypeDenoter;
    std::vector<ArrayDimensionPtr>  arrayDims;          // Entries may be null
};


} // /namespace Xsc


#endif



// ================================================================================