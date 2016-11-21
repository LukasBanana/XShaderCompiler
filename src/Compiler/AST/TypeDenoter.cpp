/*
 * TypeDenoter.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "TypeDenoter.h"
#include "Exception.h"
#include "AST.h"


namespace Xsc
{


/* ----- TypeDenoter ----- */

TypeDenoter::~TypeDenoter()
{
    // dummy
}

bool TypeDenoter::IsScalar() const
{
    return false;
}

bool TypeDenoter::IsVector() const
{
    return false;
}

bool TypeDenoter::IsMatrix() const
{
    return false;
}

bool TypeDenoter::IsVoid() const
{
    return (Type() == Types::Void);
}

bool TypeDenoter::IsBase() const
{
    return (Type() == Types::Base);
}

bool TypeDenoter::IsBuffer() const
{
    return (Type() == Types::Buffer);
}

bool TypeDenoter::IsSampler() const
{
    return (Type() == Types::Sampler);
}

bool TypeDenoter::IsTexture() const
{
    return (Type() == Types::Texture);
}

bool TypeDenoter::IsStruct() const
{
    return (Type() == Types::Struct);
}

bool TypeDenoter::IsAlias() const
{
    return (Type() == Types::Alias);
}

bool TypeDenoter::IsArray() const
{
    return (Type() == Types::Array);
}

bool TypeDenoter::Equals(const TypeDenoter& rhs) const
{
    return (GetAliased().Type() == rhs.GetAliased().Type());
}

bool TypeDenoter::IsCastableTo(const TypeDenoter& targetType) const
{
    return (GetAliased().Type() == targetType.GetAliased().Type());
}

std::string TypeDenoter::Ident() const
{
    return ""; // dummy
}

TypeDenoterPtr TypeDenoter::Get(const VarIdent* varIdent)
{
    if (varIdent)
        RuntimeErr("variable identifier could not be resolved", varIdent);
    else
        return shared_from_this();
}

TypeDenoterPtr TypeDenoter::GetFromArray(std::size_t numArrayIndices, const VarIdent* varIdent)
{
    if (numArrayIndices > 0)
        RuntimeErr("array access without array type denoter");
    else
        return Get(varIdent);
}

const TypeDenoter& TypeDenoter::GetAliased() const
{
    return *this;
}

TypeDenoterPtr TypeDenoter::AsArray(const std::vector<ExprPtr>& arrayDims)
{
    if (arrayDims.empty())
        return shared_from_this();
    else
        return std::make_shared<ArrayTypeDenoter>(shared_from_this(), arrayDims);
}


/* ----- VoidTypeDenoter ----- */

TypeDenoter::Types VoidTypeDenoter::Type() const
{
    return Types::Void;
}

std::string VoidTypeDenoter::ToString() const
{
    return "void";
}

bool VoidTypeDenoter::IsCastableTo(const TypeDenoter& targetType) const
{
    /* Void can not be casted to anything */
    return false;
}


/* ----- BaseTypeDenoter ----- */

TypeDenoter::Types BaseTypeDenoter::Type() const
{
    return Types::Base;
}

BaseTypeDenoter::BaseTypeDenoter(DataType dataType) :
    dataType{ dataType }
{
}

std::string BaseTypeDenoter::ToString() const
{
    return DataTypeToString(dataType);
}

bool BaseTypeDenoter::IsScalar() const
{
    return IsScalarType(dataType);
}

bool BaseTypeDenoter::IsVector() const
{
    return IsVectorType(dataType);
}

bool BaseTypeDenoter::IsMatrix() const
{
    return IsMatrixType(dataType);
}

bool BaseTypeDenoter::Equals(const TypeDenoter& rhs) const
{
    return (rhs.Type() == Types::Base && dataType == static_cast<const BaseTypeDenoter&>(rhs).dataType);
}

// see https://msdn.microsoft.com/en-us/library/windows/desktop/bb172396(v=vs.85).aspx
bool BaseTypeDenoter::IsCastableTo(const TypeDenoter& targetType) const
{
    //TODO: this must be extended for a lot of casting variants!!!
    #if 0
    if (IsScalar())
        return (targetType.Type() == Types::Base || targetType.Type() == Types::Struct);
    else if (IsVector())
    {
        if (targetType.IsVector())
        {
            auto& targetBaseType = static_cast<const BaseTypeDenoter&>(targetType);
            return (targetType.IsVector() && VectorTypeDim(dataType) == VectorTypeDim(targetBaseType.dataType));
        }
    }
    else if (IsMatrix())
    {
        if (targetType.IsMatrix())
        {
            auto& targetBaseType = static_cast<const BaseTypeDenoter&>(targetType);
            return (targetType.IsVector() && MatrixTypeDim(dataType) == MatrixTypeDim(targetBaseType.dataType));
        }
    }
    return false;
    #else
    const auto& targetTypeAliased = targetType.GetAliased();
    return (targetTypeAliased.Type() == Types::Base || targetTypeAliased.Type() == Types::Struct);
    #endif
}

TypeDenoterPtr BaseTypeDenoter::Get(const VarIdent* varIdent)
{
    if (varIdent)
    {
        /* Resolve vector/matrix subscript (swizzle operator) */
        try
        {
            auto subscriptDataType = SubscriptDataType(dataType, varIdent->ident);
            auto subscriptTypeDenoter = std::make_shared<BaseTypeDenoter>(subscriptDataType);
            return subscriptTypeDenoter->Get(varIdent->next.get());
        }
        catch (const ASTRuntimeError& e)
        {
            throw e;
        }
        catch (const std::exception& e)
        {
            RuntimeErr(e.what(), varIdent);
        }
    }
    return TypeDenoter::Get(varIdent);
}

/* ----- BufferTypeDenoter ----- */

TypeDenoter::Types BufferTypeDenoter::Type() const
{
    return Types::Buffer;
}

std::string BufferTypeDenoter::ToString() const
{
    return "buffer";
}


/* ----- TextureTypeDenoter ----- */

TextureTypeDenoter::TextureTypeDenoter(BufferType textureType) :
    textureType{ textureType }
{
}

TextureTypeDenoter::TextureTypeDenoter(TextureDecl* textureDeclRef) :
    textureDeclRef{ textureDeclRef }
{
}

TypeDenoter::Types TextureTypeDenoter::Type() const
{
    return Types::Texture;
}

std::string TextureTypeDenoter::ToString() const
{
    return "texture";
}


/* ----- SamplerTypeDenoter ----- */

SamplerTypeDenoter::SamplerTypeDenoter(SamplerDecl* samplerDeclRef) :
    samplerDeclRef{ samplerDeclRef }
{
}

TypeDenoter::Types SamplerTypeDenoter::Type() const
{
    return Types::Sampler;
}

std::string SamplerTypeDenoter::ToString() const
{
    return "sampler";
}


/* ----- StructTypeDenoter ----- */

StructTypeDenoter::StructTypeDenoter(const std::string& ident) :
    ident{ ident }
{
}

StructTypeDenoter::StructTypeDenoter(StructDecl* structDeclRef) :
    ident           { structDeclRef ? structDeclRef->ident : "" },
    structDeclRef   { structDeclRef                            }
{
}

TypeDenoter::Types StructTypeDenoter::Type() const
{
    return Types::Struct;
}

std::string StructTypeDenoter::ToString() const
{
    return "struct " + (!ident.empty() ? ident : std::string("<anonymous>"));
}

std::string StructTypeDenoter::Ident() const
{
    return ident;
}

TypeDenoterPtr StructTypeDenoter::Get(const VarIdent* varIdent)
{
    if (varIdent)
    {
        if (structDeclRef)
        {
            const auto& ident = varIdent->ident;
            auto varDecl = structDeclRef->Fetch(ident);
            if (varDecl)
                return varDecl->GetTypeDenoter()->Get(varIdent->next.get());
            else
                RuntimeErr("identifier '" + ident + "' not found in 'struct " + structDeclRef->SignatureToString() + "'", varIdent);
        }
        else
            RuntimeErr("missing reference to structure declaration", varIdent);
    }
    return TypeDenoter::Get(varIdent);
}


/* ----- AliasTypeDenoter ----- */

AliasTypeDenoter::AliasTypeDenoter(const std::string& ident) :
    ident{ ident }
{
}

TypeDenoter::Types AliasTypeDenoter::Type() const
{
    return Types::Alias;
}

std::string AliasTypeDenoter::ToString() const
{
    return ident;
}

std::string AliasTypeDenoter::Ident() const
{
    return ident;
}

TypeDenoterPtr AliasTypeDenoter::Get(const VarIdent* varIdent)
{
    if (aliasDeclRef)
        return aliasDeclRef->GetTypeDenoter()->Get(varIdent);
    RuntimeErr("missing reference to alias declaration", varIdent);
}

TypeDenoterPtr AliasTypeDenoter::GetFromArray(std::size_t numArrayIndices, const VarIdent* varIdent)
{
    return Get()->GetFromArray(numArrayIndices, varIdent);
}

const TypeDenoter& AliasTypeDenoter::GetAliased() const
{
    if (aliasDeclRef)
        return *(aliasDeclRef->GetTypeDenoter());
    RuntimeErr("missing reference to alias declaration");
}


/* ----- ArrayTypeDenoter ----- */

ArrayTypeDenoter::ArrayTypeDenoter(const TypeDenoterPtr& baseTypeDenoter) :
    baseTypeDenoter{ baseTypeDenoter }
{
}

ArrayTypeDenoter::ArrayTypeDenoter(const TypeDenoterPtr& baseTypeDenoter, const std::vector<ExprPtr>& arrayDims) :
    baseTypeDenoter { baseTypeDenoter },
    arrayDims       { arrayDims       }
{
}

TypeDenoter::Types ArrayTypeDenoter::Type() const
{
    return Types::Array;
}

std::string ArrayTypeDenoter::ToString() const
{
    if (!baseTypeDenoter)
        throw std::runtime_error("missing base type in array type denoter");

    auto typeName = baseTypeDenoter->ToString();

    for (std::size_t i = 0; i < arrayDims.size(); ++i)
        typeName += "[]";

    return typeName;
}

TypeDenoterPtr ArrayTypeDenoter::Get(const VarIdent* varIdent)
{
    if (varIdent)
    {
        /* Validate array dimensions */
        ValidateArrayIndices(varIdent->arrayIndices.size(), varIdent);

        /* Get base type denoter with next identifier */
        return baseTypeDenoter->Get(varIdent->next.get());
    }
    return TypeDenoter::Get(varIdent);
}

TypeDenoterPtr ArrayTypeDenoter::GetFromArray(std::size_t numArrayIndices, const VarIdent* varIdent)
{
    /* Validate array dimensions */
    ValidateArrayIndices(numArrayIndices);
    return baseTypeDenoter->Get(varIdent);
}

bool ArrayTypeDenoter::Equals(const TypeDenoter& rhs) const
{
    const auto& rhsAliased = rhs.GetAliased();
    if (rhsAliased.Type() == Types::Array)
    {
        const auto& rhsArray = static_cast<const ArrayTypeDenoter&>(rhsAliased);
        if (baseTypeDenoter && rhsArray.baseTypeDenoter)
            return baseTypeDenoter->Equals(*rhsArray.baseTypeDenoter);
    }
    return false;
}

bool ArrayTypeDenoter::IsCastableTo(const TypeDenoter& targetType) const
{
    if (Equals(targetType))
        return true;
    if (baseTypeDenoter)
    {
        const auto& baseAliased = baseTypeDenoter->GetAliased();
        return (!baseAliased.IsArray() && baseAliased.IsCastableTo(targetType));
    }
    return false;
}

void ArrayTypeDenoter::ValidateArrayIndices(std::size_t numArrayIndices, const AST* ast) const
{
    /* Validate array dimensions for specified indices */
    auto numDims = arrayDims.size();

    if (numArrayIndices != numDims)
    {
        std::string err;
            
        if (numArrayIndices < numDims)
            err = "not enough";
        else if (numArrayIndices > numDims)
            err = "too many";

        RuntimeErr(err + " array indices (expected " + std::to_string(numDims) + " but got " + std::to_string(numArrayIndices) + ")", ast);
    }
}


} // /namespace Xsc



// ================================================================================