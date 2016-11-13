/*
 * TypeDenoter.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "TypeDenoter.h"
#include "AST.h"


namespace Xsc
{


/* ----- TypeDenoter ----- */

TypeDenoter::~TypeDenoter()
{
    // dummy
}

TypeDenoter* TypeDenoter::Get()
{
    return this;
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

bool TypeDenoter::IsArray() const
{
    return (Type() == Types::Array);
}

bool TypeDenoter::IsCompatibleWith(const TypeDenoter& rhs) const
{
    return (Type() == rhs.Type());
}

bool TypeDenoter::IsCastableTo(const TypeDenoter& targetType) const
{
    return (Type() == targetType.Type());
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

bool BaseTypeDenoter::IsCastableTo(const TypeDenoter& targetType) const
{
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

TypeDenoter::Types TextureTypeDenoter::Type() const
{
    return Types::Texture;
}

std::string TextureTypeDenoter::ToString() const
{
    return "texture";
}


/* ----- SamplerTypeDenoter ----- */

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


/* ----- AliasTypeDenoter ----- */

TypeDenoter::Types AliasTypeDenoter::Type() const
{
    return Types::Alias;
}

TypeDenoter* AliasTypeDenoter::Get()
{
    return (aliasTypeRef ? aliasTypeRef : this);
}

std::string AliasTypeDenoter::ToString() const
{
    return (aliasTypeRef ? aliasTypeRef->ToString() : "undefined");
}


/* ----- ArrayTypeDenoter ----- */

TypeDenoter::Types ArrayTypeDenoter::Type() const
{
    return Types::Array;
}

std::string ArrayTypeDenoter::ToString() const
{
    std::string typeName;
    
    if (baseTypeDenoter)
        typeName = baseTypeDenoter->ToString();
    else
        typeName = "<undefined>";

    for (std::size_t i = 0; i < arrayDims.size(); ++i)
        typeName += "[]";

    return typeName;
}


} // /namespace Xsc



// ================================================================================