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


struct TypeDenoter;
using TypeDenoterPtr = std::shared_ptr<TypeDenoter>;

// Type denoter base class.
struct TypeDenoter : std::enable_shared_from_this<TypeDenoter>
{
    enum class Types
    {
        Void,
        Base,
        Buffer,
        Texture,
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

    virtual bool IsScalar() const;
    virtual bool IsVector() const;
    virtual bool IsMatrix() const;

    bool IsVoid() const;
    bool IsBase() const;
    bool IsBuffer() const;
    bool IsSampler() const;
    bool IsTexture() const;
    bool IsStruct() const;
    bool IsAlias() const;
    bool IsArray() const;

    // Returns true if this type denoter is equal to the specified type denoter.
    virtual bool Equals(const TypeDenoter& rhs) const;

    // Returns true if this type denoter can be casted to the specified target type denoter (special cases void and base types).
    virtual bool IsCastableTo(const TypeDenoter& targetType) const;

    // Returns the type identifier (if it has one), e.g. for structs and type aliases.
    virtual std::string Ident() const;

    // Returns a type denoter for the specified full variable identifier. Throws ASTRuntimeError on failure.
    virtual TypeDenoterPtr Get(const VarIdent* varIdent = nullptr);

    // Returns a type denoter for the specified array access and full variable identifier. Throws ASTRuntimeError on failure.
    virtual TypeDenoterPtr GetFromArray(std::size_t numArrayIndices, const VarIdent* varIdent = nullptr);

    // Returns either this type denoter or an aliased type.
    virtual const TypeDenoter& GetAliased() const;
};

// Void type denoter.
struct VoidTypeDenoter : public TypeDenoter
{
    Types Type() const override;
    std::string ToString() const override;

    // Returns always false, since void type can not be casted to anything.
    bool IsCastableTo(const TypeDenoter& targetType) const override;
};

using VoidTypeDenoterPtr = std::shared_ptr<VoidTypeDenoter>;

// Base type denoter.
struct BaseTypeDenoter : public TypeDenoter
{
    BaseTypeDenoter() = default;
    BaseTypeDenoter(DataType dataType);

    Types Type() const override;
    std::string ToString() const override;

    bool IsScalar() const override;
    bool IsVector() const override;
    bool IsMatrix() const override;

    bool Equals(const TypeDenoter& rhs) const override;
    bool IsCastableTo(const TypeDenoter& targetType) const override;

    TypeDenoterPtr Get(const VarIdent* varIdent) override;

    DataType dataType = DataType::Undefined;
};

using BaseTypeDenoterPtr = std::shared_ptr<BaseTypeDenoter>;

// Buffer type denoter.
struct BufferTypeDenoter : public TypeDenoter
{
    Types Type() const override;
    std::string ToString() const override;

    BufferDeclStmnt* bufferDeclRef = nullptr;
};

using BufferTypeDenoterPtr = std::shared_ptr<BufferTypeDenoter>;

// Texture type denoter.
struct TextureTypeDenoter : public TypeDenoter
{
    TextureTypeDenoter() = default;
    TextureTypeDenoter(TextureDecl* textureDeclRef);

    Types Type() const override;
    std::string ToString() const override;

  //TextureType     textureType;
    TextureDecl*    textureDeclRef = nullptr;
};

using TextureTypeDenoterPtr = std::shared_ptr<TextureTypeDenoter>;

// Sampler type denoter.
struct SamplerTypeDenoter : public TypeDenoter
{
    SamplerTypeDenoter() = default;
    SamplerTypeDenoter(SamplerDecl* samplerDeclRef);

    Types Type() const override;
    std::string ToString() const override;

    SamplerDecl* samplerDeclRef = nullptr;
};

using SamplerTypeDenoterPtr = std::shared_ptr<SamplerTypeDenoter>;

// Struct type denoter.
struct StructTypeDenoter : public TypeDenoter
{
    StructTypeDenoter() = default;
    StructTypeDenoter(const std::string& ident);
    StructTypeDenoter(StructDecl* structDeclRef);

    Types Type() const override;
    std::string ToString() const override;
    std::string Ident() const override;

    TypeDenoterPtr Get(const VarIdent* varIdent = nullptr) override;

    std::string     ident;
    StructDecl*     structDeclRef = nullptr;    // Reference to the StructDecl AST node
};

using StructTypeDenoterPtr = std::shared_ptr<StructTypeDenoter>;

// Alias type denoter.
struct AliasTypeDenoter : public TypeDenoter
{
    AliasTypeDenoter() = default;
    AliasTypeDenoter(const std::string& ident);

    Types Type() const override;
    std::string ToString() const override;
    std::string Ident() const override;

    TypeDenoterPtr Get(const VarIdent* varIdent = nullptr) override;
    TypeDenoterPtr GetFromArray(std::size_t numArrayIndices, const VarIdent* varIdent = nullptr) override;

    const TypeDenoter& GetAliased() const override;

    std::string     ident;                      // Type identifier
    AliasDecl*      aliasDeclRef    = nullptr;  // Reference to the AliasDecl AST node.
};

using AliasTypeDenoterPtr = std::shared_ptr<AliasTypeDenoter>;

// Array type denoter.
struct ArrayTypeDenoter : public TypeDenoter
{
    ArrayTypeDenoter() = default;
    ArrayTypeDenoter(const TypeDenoterPtr& baseTypeDenoter);
    ArrayTypeDenoter(const TypeDenoterPtr& baseTypeDenoter, const std::vector<ExprPtr>& arrayDims);

    Types Type() const override;

    // Throws std::runtime_error if 'baseTypeDenoter' is null.
    std::string ToString() const override;

    TypeDenoterPtr Get(const VarIdent* varIdent = nullptr) override;

    TypeDenoterPtr GetFromArray(std::size_t numArrayIndices, const VarIdent* varIdent = nullptr) override;

    // Validates the number of array indices for this array type denoter.
    void ValidateArrayIndices(std::size_t numArrayIndices, const AST* ast = nullptr) const;

    TypeDenoterPtr          baseTypeDenoter;
    std::vector<ExprPtr>    arrayDims;          // May be null
};

using ArrayTypeDenoterPtr = std::shared_ptr<ArrayTypeDenoter>;



} // /namespace Xsc


#endif



// ================================================================================