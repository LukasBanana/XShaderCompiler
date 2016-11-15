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
    return (Type() == rhs.Type());
}

bool TypeDenoter::IsCastableTo(const TypeDenoter& targetType) const
{
    return (Type() == targetType.Type());
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
    if (IsScalar())
        return "scalar";
    if (IsVector())
        return "vector";
    if (IsMatrix())
        return "matrix";
    return "<undefined>";
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
    return (targetType.Type() == Types::Base || targetType.Type() == Types::Struct);
    #endif
}

TypeDenoterPtr BaseTypeDenoter::Get(const VarIdent* varIdent)
{
    if (varIdent)
    {
        if (varIdent->next)
            RuntimeErr("vector subscript '" + varIdent->ident + "' can not have further suffixes", varIdent->next.get());

        /* Check if data type can be vector subscript */
        auto baseDataType = BaseDataType(dataType);

        if (!IsVectorType(baseDataType))
            RuntimeErr("non-vector types can not have vector subscripts", varIdent);

        /* Resolve vector subscript (swizzle operator) */
        try
        {
            auto subscriptDataType = VectorSubscriptDataType(baseDataType, varIdent->ident);
            return std::make_shared<BaseTypeDenoter>(subscriptDataType);
        }
        catch (const std::exception& e)
        {
            RuntimeErr(e.what(), varIdent);
        }
    }

    /* Default getter */
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
    ident           { structDeclRef ? structDeclRef->name : "" },
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


/* ----- ArrayTypeDenoter ----- */

ArrayTypeDenoter::ArrayTypeDenoter(const TypeDenoterPtr& baseTypeDenoter) :
    baseTypeDenoter{ baseTypeDenoter }
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


} // /namespace Xsc



// ================================================================================