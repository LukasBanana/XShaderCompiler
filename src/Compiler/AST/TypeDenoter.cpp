/*
 * TypeDenoter.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "TypeDenoter.h"
#include "Exception.h"
#include "AST.h"
#include "ReportIdents.h"
#include <algorithm>


namespace Xsc
{



#ifdef XSC_ENABLE_LANGUAGE_EXT

std::string VectorSpace::ToString() const
{
    if (!IsSpecified())
        return ("<" + R_Unspecified + ">");
    else if (IsChangeOfBasis())
        return (Xsc::ToString(src) + "-to-" + Xsc::ToString(dst));
    else
        return Xsc::ToString(src);
}

bool VectorSpace::IsSpecified() const
{
    return !src.empty();
}

bool VectorSpace::IsChangeOfBasis() const
{
    return (src != dst);
}

bool VectorSpace::IsAssignableTo(const VectorSpace& rhs) const
{
    return (dst == rhs.src || !rhs.IsSpecified());
}

void VectorSpace::Set(const StringType& space)
{
    src = space;
    dst = space;
}

void VectorSpace::Set(const StringType& srcSpace, const StringType& dstSpace)
{
    src = srcSpace;
    dst = dstSpace;
}

VectorSpace VectorSpace::FindCommonVectorSpace(const std::vector<ExprPtr>& exprList, bool ignoreUnspecified, const AST* ast)
{
    if (!exprList.empty())
    {
        /* Gather base type denoters of all expressions */
        std::vector<const BaseTypeDenoter*> typeDens;
        typeDens.reserve(exprList.size());

        VectorSpace commonVectorSpace;
        const AST*  commonVectorSpaceAST    = nullptr;

        for (const auto& expr : exprList)
        {
            /* Always append type denoter to list (also null pointers!) */
            const auto& typeDen = expr->GetTypeDenoter()->GetAliased();
            if (auto baseTypeDen = typeDen.As<BaseTypeDenoter>())
            {
                typeDens.push_back(baseTypeDen);
                if (baseTypeDen->vectorSpace.IsSpecified())
                {
                    if (!commonVectorSpaceAST)
                    {
                        /* Store first specified vector space as common vector space */
                        commonVectorSpace       = baseTypeDen->vectorSpace;
                        commonVectorSpaceAST    = expr.get();
                    }
                }
            }
            else
                typeDens.push_back(nullptr);
        }

        if (commonVectorSpaceAST)
        {
            /* Validate vector space compatibility */
            for (std::size_t i = 0, n = typeDens.size(); i < n; ++i)
            {
                if (auto typeDen = typeDens[i])
                {
                    const auto& vectorSpace = typeDen->vectorSpace;
                    if ( ( vectorSpace.IsSpecified() && vectorSpace != commonVectorSpace ) ||
                         ( !vectorSpace.IsSpecified() && !ignoreUnspecified ) )
                    {
                        RuntimeErr(
                            R_InconsistVectorSpacesInTypes(commonVectorSpace.ToString(), vectorSpace.ToString()),
                            exprList[i].get(),
                            { commonVectorSpaceAST }
                        );
                    }
                }
                else if (!ignoreUnspecified)
                    RuntimeErr(R_InconsistVectorSpacesInTypes, ast);
            }

            return commonVectorSpace;
        }
    }
    return {};
}

bool operator == (const VectorSpace& lhs, const VectorSpace& rhs)
{
    return (lhs.src == rhs.src && lhs.dst == rhs.dst);
}

bool operator != (const VectorSpace& lhs, const VectorSpace& rhs)
{
    return !(lhs == rhs);
}

#endif

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

bool TypeDenoter::IsNull() const
{
    return (Type() == Types::Null);
}

bool TypeDenoter::IsBase() const
{
    return (Type() == Types::Base);
}

bool TypeDenoter::IsSampler() const
{
    return (Type() == Types::Sampler);
}

bool TypeDenoter::IsBuffer() const
{
    return (Type() == Types::Buffer);
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

bool TypeDenoter::Equals(const TypeDenoter& rhs, const Flags& /*compareFlags*/) const
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

void TypeDenoter::SetIdentIfAnonymous(const std::string& ident)
{
    // dummy
}

TypeDenoterPtr TypeDenoter::GetSub(const Expr* expr)
{
    if (expr)
    {
        if (auto objExpr = expr->As<ObjectExpr>())
            return GetSubObject(objExpr->ident, expr);
        if (auto arrayExpr = expr->As<ArrayExpr>())
            return GetSubArray(arrayExpr->NumIndices(), expr);
        RuntimeErr(R_InvalidExprForSubTypeDen(ToString()), expr);
    }
    return shared_from_this();
}

TypeDenoterPtr TypeDenoter::GetSubObject(const std::string& ident, const AST* ast)
{
    RuntimeErr(R_TypeHasNoSuchObject(ToString(), ident), ast);
}

TypeDenoterPtr TypeDenoter::GetSubArray(const std::size_t numArrayIndices, const AST* ast)
{
    if (numArrayIndices > 0)
        RuntimeErr(R_IllegalArrayAccess(ToString()), ast);
    else
        return shared_from_this();
}

const TypeDenoter& TypeDenoter::GetAliased() const
{
    return *this;
}

unsigned int TypeDenoter::NumDimensions() const
{
    return 0;
}

AST* TypeDenoter::SymbolRef() const
{
    return nullptr;
}

TypeDenoterPtr TypeDenoter::AsArray(const std::vector<ArrayDimensionPtr>& arrayDims)
{
    if (arrayDims.empty())
        return shared_from_this();
    else
        return std::make_shared<ArrayTypeDenoter>(shared_from_this(), arrayDims);
}

static DataType HighestOrderDataType(DataType lhs, DataType rhs, DataType highestType = DataType::Float) //Double//Float
{
    /*
    Return data type with highest order of both types: max{ lhs, rhs },
    where order is the integral enum value (bool < int < uint < float ...)
    */
    auto highestOrder = std::max({ static_cast<int>(lhs), static_cast<int>(rhs) });
    auto clampedOrder = std::min({ highestOrder, static_cast<int>(highestType) });
    return static_cast<DataType>(clampedOrder);
}

static TypeDenoterPtr FindCommonTypeDenoterScalarAndScalar(BaseTypeDenoter* lhsTypeDen, BaseTypeDenoter* rhsTypeDen)
{
    /* Return scalar type with highest order data type */
    auto commonType = HighestOrderDataType(lhsTypeDen->dataType, rhsTypeDen->dataType);
    return std::make_shared<BaseTypeDenoter>(commonType);
}

static TypeDenoterPtr FindCommonTypeDenoterScalarAndVector(BaseTypeDenoter* lhsTypeDen, BaseTypeDenoter* rhsTypeDen, bool useMinDimension)
{
    auto commonType = HighestOrderDataType(lhsTypeDen->dataType, BaseDataType(rhsTypeDen->dataType));
    if (useMinDimension)
    {
        /* Return scalar type (minimal dimension) */
        return std::make_shared<BaseTypeDenoter>(commonType);
    }
    else
    {
        /* Return vector type */
        auto rhsDim = VectorTypeDim(rhsTypeDen->dataType);
        return std::make_shared<BaseTypeDenoter>(VectorDataType(commonType, rhsDim));
    }
}

static TypeDenoterPtr FindCommonTypeDenoterVectorAndVector(BaseTypeDenoter* lhsTypeDen, BaseTypeDenoter* rhsTypeDen, bool useMinDimension)
{
    auto commonType = HighestOrderDataType(BaseDataType(lhsTypeDen->dataType), BaseDataType(rhsTypeDen->dataType));
    
    /* Return either highest or lowest dimension */
    auto lhsDim = VectorTypeDim(lhsTypeDen->dataType);
    auto rhsDim = VectorTypeDim(rhsTypeDen->dataType);
    auto commonDim = (useMinDimension ? std::min(lhsDim, rhsDim) : std::max(lhsDim, rhsDim));

    return std::make_shared<BaseTypeDenoter>(VectorDataType(commonType, commonDim));
}

static TypeDenoterPtr FindCommonTypeDenoterAnyAndAny(TypeDenoter* lhsTypeDen, TypeDenoter* rhsTypeDen)
{
    /* Always use type of left hand side */
    return lhsTypeDen->GetSub();
}

TypeDenoterPtr TypeDenoter::FindCommonTypeDenoter(const TypeDenoterPtr& lhsTypeDen, const TypeDenoterPtr& rhsTypeDen, bool useMinDimension)
{
    /* Scalar and Scalar */
    if (lhsTypeDen->IsScalar() && rhsTypeDen->IsScalar())
        return FindCommonTypeDenoterScalarAndScalar(lhsTypeDen->As<BaseTypeDenoter>(), rhsTypeDen->As<BaseTypeDenoter>());

    /* Scalar and Vector */
    if (lhsTypeDen->IsScalar() && rhsTypeDen->IsVector())
        return FindCommonTypeDenoterScalarAndVector(lhsTypeDen->As<BaseTypeDenoter>(), rhsTypeDen->As<BaseTypeDenoter>(), useMinDimension);

    /* Vector and Scalar */
    if (lhsTypeDen->IsVector() && rhsTypeDen->IsScalar())
        return FindCommonTypeDenoterScalarAndVector(rhsTypeDen->As<BaseTypeDenoter>(), lhsTypeDen->As<BaseTypeDenoter>(), useMinDimension);

    /* Vector and Vector (Note: always use minimal dimension here!) */
    if (lhsTypeDen->IsVector() && rhsTypeDen->IsVector())
        return FindCommonTypeDenoterVectorAndVector(lhsTypeDen->As<BaseTypeDenoter>(), rhsTypeDen->As<BaseTypeDenoter>(), true);

    /* Default type */
    return FindCommonTypeDenoterAnyAndAny(lhsTypeDen.get(), rhsTypeDen.get());
}

TypeDenoterPtr TypeDenoter::FindCommonTypeDenoterFrom(const ExprPtr& lhsExpr, const ExprPtr& rhsExpr, bool useMinDimension, const AST* ast)
{
    auto lhsTypeDen = lhsExpr->GetTypeDenoter();
    auto rhsTypeDen = rhsExpr->GetTypeDenoter();

    auto commonTypeDenoter = TypeDenoter::FindCommonTypeDenoter(lhsTypeDen, rhsTypeDen, useMinDimension);

    #ifdef XSC_ENABLE_LANGUAGE_EXT

    if (auto baseTypeDen = commonTypeDenoter->As<BaseTypeDenoter>())
    {
        baseTypeDen->vectorSpace = VectorSpace::FindCommonVectorSpace(
            std::vector<ExprPtr>{ lhsExpr, rhsExpr },
            true,
            ast
        );
    }

    #endif

    return commonTypeDenoter;
}

BaseTypeDenoterPtr TypeDenoter::MakeBoolTypeWithDimensionOf(const TypeDenoter& typeDen)
{
    if (auto baseTypeDen = typeDen.GetAliased().As<BaseTypeDenoter>())
    {
        /* Make vector boolean type denoter with dimension of the specified type denoter */
        auto vecBoolType = VectorDataType(DataType::Bool, VectorTypeDim(baseTypeDen->dataType));
        return std::make_shared<BaseTypeDenoter>(vecBoolType);
    }
    else
    {
        /* Make single boolean type denoter */
        return std::make_shared<BaseTypeDenoter>(DataType::Bool);
    }
}

bool TypeDenoter::HasVectorTruncation(
    const TypeDenoter& sourceTypeDen, const TypeDenoter& destTypeDen, int& sourceVecSize, int& destVecSize)
{
    /* Are both types base type denoters? */
    if (auto sourceBaseTypeDen = sourceTypeDen.As<BaseTypeDenoter>())
    {
        if (auto destBaseTypeDen = destTypeDen.As<BaseTypeDenoter>())
        {
            /* Get data types from type denoters */
            sourceVecSize = VectorTypeDim(sourceBaseTypeDen->dataType);
            destVecSize = VectorTypeDim(destBaseTypeDen->dataType);
            return (destVecSize < sourceVecSize);
        }
    }
    return false;
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

TypeDenoterPtr VoidTypeDenoter::Copy() const
{
    return std::make_shared<VoidTypeDenoter>();
}

bool VoidTypeDenoter::IsCastableTo(const TypeDenoter& targetType) const
{
    /* Void can not be casted to anything */
    return false;
}


/* ----- NullTypeDenoter ----- */

TypeDenoter::Types NullTypeDenoter::Type() const
{
    return Types::Null;
}

std::string NullTypeDenoter::ToString() const
{
    return "NULL";
}

TypeDenoterPtr NullTypeDenoter::Copy() const
{
    return std::make_shared<NullTypeDenoter>();
}

bool NullTypeDenoter::IsCastableTo(const TypeDenoter& targetType) const
{
    /* Null literal is castable to all object types */
    auto target = targetType.GetAliased().Type();
    return (target == TypeDenoter::Types::Buffer || target == TypeDenoter::Types::Sampler);
}


/* ----- BaseTypeDenoter ----- */

TypeDenoter::Types BaseTypeDenoter::Type() const
{
    return Types::Base;
}

BaseTypeDenoter::BaseTypeDenoter(const DataType dataType) :
    dataType { dataType }
{
}

std::string BaseTypeDenoter::ToString() const
{
    return DataTypeToString(dataType);
}

TypeDenoterPtr BaseTypeDenoter::Copy() const
{
    return std::make_shared<BaseTypeDenoter>(dataType);
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

bool BaseTypeDenoter::Equals(const TypeDenoter& rhs, const Flags& /*compareFlags*/) const
{
    /* Compare data types of both type denoters */
    if (auto rhsBaseTypeDen = rhs.As<BaseTypeDenoter>())
        return (dataType == rhsBaseTypeDen->dataType);
    else
        return false;
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

TypeDenoterPtr BaseTypeDenoter::GetSubObject(const std::string& ident, const AST* ast)
{
    /* Resolve vector/matrix subscript (swizzle operator) */
    try
    {
        auto subscriptDataType = SubscriptDataType(dataType, ident);
        auto subTypeDen = std::make_shared<BaseTypeDenoter>(subscriptDataType);

        #ifdef XSC_ENABLE_LANGUAGE_EXT
        subTypeDen->vectorSpace = vectorSpace;
        #endif

        return subTypeDen;
    }
    catch (const std::exception& e)
    {
        RuntimeErr(e.what(), ast);
    }
}

TypeDenoterPtr BaseTypeDenoter::GetSubArray(const std::size_t numArrayIndices, const AST* ast)
{
    if (numArrayIndices > 0)
    {
        /* Convert vector or matrix type for array access */
        if (IsVectorType(dataType))
        {
            /* Return scalar type */
            if (numArrayIndices > 1)
                RuntimeErr(R_TooManyArrayDimensions(R_VectorTypeDen), ast);
            else
                return std::make_shared<BaseTypeDenoter>(BaseDataType(dataType));
        }
        else if (IsMatrixType(dataType))
        {
            /* Return scalar or vector type */
            if (numArrayIndices == 1)
            {
                auto matrixDim = MatrixTypeDim(dataType);
                return std::make_shared<BaseTypeDenoter>(VectorDataType(BaseDataType(dataType), matrixDim.second));
            }
            else if (numArrayIndices == 2)
                return std::make_shared<BaseTypeDenoter>(BaseDataType(dataType));
            else if (numArrayIndices > 2)
                RuntimeErr(R_TooManyArrayDimensions(R_MatrixTypeDen), ast);
        }
        else
            return TypeDenoter::GetSubArray(numArrayIndices, ast);
    }

    return GetSub();
}


/* ----- BufferTypeDenoter ----- */

BufferTypeDenoter::BufferTypeDenoter(const BufferType bufferType) :
    bufferType{ bufferType }
{
}

BufferTypeDenoter::BufferTypeDenoter(BufferDecl* bufferDeclRef) :
    bufferDeclRef{ bufferDeclRef }
{
    if (bufferDeclRef && bufferDeclRef->declStmntRef)
    {
        auto sourceTypeDen = bufferDeclRef->declStmntRef->typeDenoter.get();
        bufferType          = sourceTypeDen->bufferType;
        genericTypeDenoter  = sourceTypeDen->genericTypeDenoter;
        genericSize         = sourceTypeDen->genericSize;
    }
}

TypeDenoter::Types BufferTypeDenoter::Type() const
{
    return Types::Buffer;
}

std::string BufferTypeDenoter::ToString() const
{
    auto s = BufferTypeToString(bufferType);

    if (genericTypeDenoter)
    {
        s += '<';
        s += genericTypeDenoter->ToString();
        s += '>';
    }

    return s;
}

TypeDenoterPtr BufferTypeDenoter::Copy() const
{
    auto copy = std::make_shared<BufferTypeDenoter>();
    {
        copy->bufferType            = bufferType;
        copy->genericTypeDenoter    = genericTypeDenoter;
        copy->genericSize           = genericSize;
        copy->bufferDeclRef         = bufferDeclRef;
    }
    return copy;
}

bool BufferTypeDenoter::Equals(const TypeDenoter& rhs, const Flags& compareFlags) const
{
    if (auto rhsBufferTypeDen = rhs.GetAliased().As<BufferTypeDenoter>())
    {
        if (bufferType == rhsBufferTypeDen->bufferType)
        {
            if (!compareFlags(IgnoreGenericSubType))
            {
                /* Compare generic sub type denoters */
                if (genericTypeDenoter && rhsBufferTypeDen->genericTypeDenoter)
                    return genericTypeDenoter->Equals(*rhsBufferTypeDen->genericTypeDenoter, compareFlags);
                if (!genericTypeDenoter && !rhsBufferTypeDen->genericTypeDenoter)
                    return true;
            }
            else
                return true;
        }
    }
    return false;
}

TypeDenoterPtr BufferTypeDenoter::GetSubObject(const std::string& ident, const AST* ast)
{
    //TODO: current not supported
    //TODO: must be abstracted for different frontends!
    if (ident == "mips")
    {
        RuntimeErr(R_NotImplementedYet(ToString() + ".mips", __FUNCTION__), ast);
    }
    return TypeDenoter::GetSubObject(ident, ast);
}

TypeDenoterPtr BufferTypeDenoter::GetSubArray(const std::size_t numArrayIndices, const AST* ast)
{
    if (numArrayIndices > 0)
        return GetGenericTypeDenoter()->GetSubArray(numArrayIndices - 1, ast);
    else
        return shared_from_this();
}

TypeDenoterPtr BufferTypeDenoter::GetGenericTypeDenoter() const
{
    return (genericTypeDenoter ? genericTypeDenoter : std::make_shared<BaseTypeDenoter>(DataType::Float4));
}

AST* BufferTypeDenoter::SymbolRef() const
{
    return bufferDeclRef;
}


/* ----- SamplerTypeDenoter ----- */

SamplerTypeDenoter::SamplerTypeDenoter(const SamplerType samplerType) :
    samplerType { samplerType }
{
}

SamplerTypeDenoter::SamplerTypeDenoter(SamplerDecl* samplerDeclRef) :
    samplerDeclRef { samplerDeclRef }
{
    if (samplerDeclRef)
        samplerType = samplerDeclRef->GetSamplerType();
}

TypeDenoter::Types SamplerTypeDenoter::Type() const
{
    return Types::Sampler;
}

std::string SamplerTypeDenoter::ToString() const
{
    return (IsSamplerStateType(samplerType) ? "SamplerState" : "Sampler");
}

TypeDenoterPtr SamplerTypeDenoter::Copy() const
{
    auto copy = std::make_shared<SamplerTypeDenoter>();
    {
        copy->samplerType       = samplerType;
        copy->samplerDeclRef    = samplerDeclRef;
    }
    return copy;
}

bool SamplerTypeDenoter::Equals(const TypeDenoter& rhs, const Flags& compareFlags) const
{
    /* Compare sampler types */
    if (auto rhsSamplerTypeDen = rhs.GetAliased().As<SamplerTypeDenoter>())
        return (samplerType == rhsSamplerTypeDen->samplerType);
    else
        return false;
}

AST* SamplerTypeDenoter::SymbolRef() const
{
    return samplerDeclRef;
}


/* ----- StructTypeDenoter ----- */

StructTypeDenoter::StructTypeDenoter(const std::string& ident) :
    ident{ ident }
{
}

StructTypeDenoter::StructTypeDenoter(StructDecl* structDeclRef) :
    ident           { structDeclRef ? structDeclRef->ident.Original() : "" },
    structDeclRef   { structDeclRef                                        }
{
}

TypeDenoter::Types StructTypeDenoter::Type() const
{
    return Types::Struct;
}

std::string StructTypeDenoter::ToString() const
{
    return (structDeclRef ? structDeclRef->ToString() : "struct <unknown>");
}

TypeDenoterPtr StructTypeDenoter::Copy() const
{
    auto copy = std::make_shared<StructTypeDenoter>();
    {
        copy->ident         = ident;
        copy->structDeclRef = structDeclRef;
    }
    return copy;
}

void StructTypeDenoter::SetIdentIfAnonymous(const std::string& ident)
{
    if (this->ident.empty())
        this->ident = ident;
}

bool StructTypeDenoter::Equals(const TypeDenoter& rhs, const Flags& compareFlags) const
{
    if (auto rhsStructTypeDen = rhs.GetAliased().As<StructTypeDenoter>())
    {
        /* Get structure declarations from type denoters */
        if (auto lhsStructDecl = structDeclRef)
        {
            /* Compare this structure type with another structure type */
            if (auto rhsStructDecl = rhsStructTypeDen->structDeclRef)
                return lhsStructDecl->EqualsMembers(*rhsStructDecl, compareFlags);
            else
                RuntimeErr(R_MissingRefToStructDecl(rhsStructTypeDen->ident));
        }
        else
            RuntimeErr(R_MissingRefToStructDecl(ident));
    }
    return false;
}

bool StructTypeDenoter::IsCastableTo(const TypeDenoter& targetType) const
{
    /* Get structure declarations from type denoters */
    if (auto structDecl = structDeclRef)
    {
        const auto& targetAliasedType = targetType.GetAliased();
        if (auto targetStructTypeDen = targetAliasedType.As<StructTypeDenoter>())
        {
            /* Compare this structure type with another structure type */
            if (auto targetStructDecl = targetStructTypeDen->structDeclRef)
                return structDecl->EqualsMembers(*targetStructDecl);
            else
                RuntimeErr(R_MissingRefToStructDecl(targetStructDecl->ident));
        }
        else if (auto targetBaseTypeDen = targetAliasedType.As<BaseTypeDenoter>())
        {
            /* Compare this structure type with target base type */
            return structDecl->IsCastableTo(*targetBaseTypeDen);
        }
    }
    else
        RuntimeErr(R_MissingRefToStructDecl(ident));
    return false;
}

std::string StructTypeDenoter::Ident() const
{
    return ident;
}

AST* StructTypeDenoter::SymbolRef() const
{
    return structDeclRef;
}

TypeDenoterPtr StructTypeDenoter::GetSubObject(const std::string& ident, const AST* ast)
{
    if (structDeclRef)
    {
        if (auto varDecl = structDeclRef->FetchVarDecl(ident))
        {
            /* Return type of variable declaration in structure */
            return varDecl->GetTypeDenoter();
        }
        else
        {
            RuntimeErr(
                R_UndeclaredIdent(ident, structDeclRef->ToString(), structDeclRef->FetchSimilar(ident)),
                ast
            );
        }
    }
    RuntimeErr(R_MissingRefToStructDecl(ident), ast);
}


/* ----- AliasTypeDenoter ----- */

AliasTypeDenoter::AliasTypeDenoter(const std::string& ident) :
    ident{ ident }
{
}

AliasTypeDenoter::AliasTypeDenoter(AliasDecl* aliasDeclRef) :
    ident       { aliasDeclRef ? aliasDeclRef->ident.Original() : "" },
    aliasDeclRef{ aliasDeclRef                                       }
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

TypeDenoterPtr AliasTypeDenoter::Copy() const
{
    auto copy = std::make_shared<AliasTypeDenoter>();
    {
        copy->ident         = ident;
        copy->aliasDeclRef  = aliasDeclRef;
    }
    return copy;
}

void AliasTypeDenoter::SetIdentIfAnonymous(const std::string& ident)
{
    if (this->ident.empty())
        this->ident = ident;
}

bool AliasTypeDenoter::Equals(const TypeDenoter& rhs, const Flags& compareFlags) const
{
    return GetAliasedTypeOrThrow()->Equals(rhs, compareFlags);
}

bool AliasTypeDenoter::IsCastableTo(const TypeDenoter& targetType) const
{
    return GetAliasedTypeOrThrow()->IsCastableTo(targetType);
}

std::string AliasTypeDenoter::Ident() const
{
    return ident;
}

TypeDenoterPtr AliasTypeDenoter::GetSub(const Expr* expr)
{
    return GetAliasedTypeOrThrow(expr)->GetSub(expr);
}

TypeDenoterPtr AliasTypeDenoter::GetSubObject(const std::string& ident, const AST* ast)
{
    return GetAliasedTypeOrThrow(ast)->GetSubObject(ident, ast);
}

TypeDenoterPtr AliasTypeDenoter::GetSubArray(const std::size_t numArrayIndices, const AST* ast)
{
    return GetAliasedTypeOrThrow(ast)->GetSubArray(numArrayIndices, ast);
}

const TypeDenoter& AliasTypeDenoter::GetAliased() const
{
    return GetAliasedTypeOrThrow()->GetAliased();
}

const TypeDenoterPtr& AliasTypeDenoter::GetAliasedTypeOrThrow(const AST* ast) const
{
    if (aliasDeclRef)
        return aliasDeclRef->GetTypeDenoter();
    else
        RuntimeErr(R_MissingRefToAliasDecl(ident), ast);
}

unsigned int AliasTypeDenoter::NumDimensions() const
{
    return GetAliasedTypeOrThrow()->NumDimensions();
}

AST* AliasTypeDenoter::SymbolRef() const
{
    return aliasDeclRef;
}


/* ----- ArrayTypeDenoter ----- */

ArrayTypeDenoter::ArrayTypeDenoter(const TypeDenoterPtr& subTypeDenoter) :
    subTypeDenoter { subTypeDenoter }
{
}

ArrayTypeDenoter::ArrayTypeDenoter(const TypeDenoterPtr& subTypeDenoter, const std::vector<ArrayDimensionPtr>& arrayDims) :
    subTypeDenoter { subTypeDenoter },
    arrayDims      { arrayDims      }
{
}

ArrayTypeDenoter::ArrayTypeDenoter(
    const TypeDenoterPtr& subTypeDenoter,
    const std::vector<ArrayDimensionPtr>& baseArrayDims,
    const std::vector<ArrayDimensionPtr>& subArrayDims) :
        subTypeDenoter { subTypeDenoter },
        arrayDims      { baseArrayDims  }
{
    arrayDims.insert(arrayDims.end(), subArrayDims.begin(), subArrayDims.end());
}

TypeDenoter::Types ArrayTypeDenoter::Type() const
{
    return Types::Array;
}

std::string ArrayTypeDenoter::ToString() const
{
    if (!subTypeDenoter)
        throw std::runtime_error(R_MissingBaseTypeInArray);

    auto typeName = subTypeDenoter->ToString();

    for (const auto& dim : arrayDims)
        typeName += dim->ToString();

    return typeName;
}

TypeDenoterPtr ArrayTypeDenoter::Copy() const
{
    auto copy = std::make_shared<ArrayTypeDenoter>();
    {
        copy->subTypeDenoter    = subTypeDenoter;
        copy->arrayDims         = arrayDims;
    }
    return copy;
}

TypeDenoterPtr ArrayTypeDenoter::GetSubArray(const std::size_t numArrayIndices, const AST* ast)
{
    const auto numDims = arrayDims.size();
    
    if (numArrayIndices == 0)
    {
        /* Just return this array type denoter */
        return shared_from_this();
    }

    if (numArrayIndices < numDims)
    {
        /* Make new array type denoter with less dimensions */
        auto subArrayDims = arrayDims;
        subArrayDims.resize(numDims - numArrayIndices);
        return std::make_shared<ArrayTypeDenoter>(subTypeDenoter, subArrayDims);
    }

    /* Get sub type denoter with next array index */
    return subTypeDenoter->GetSubArray(numArrayIndices - numDims, ast);
}

bool ArrayTypeDenoter::Equals(const TypeDenoter& rhs, const Flags& compareFlags) const
{
    if (auto rhsArrayTypeDen = rhs.GetAliased().As<ArrayTypeDenoter>())
    {
        /* Compare sub type denoters */
        if (subTypeDenoter && rhsArrayTypeDen->subTypeDenoter && EqualsDimensions(*rhsArrayTypeDen))
            return subTypeDenoter->Equals(*rhsArrayTypeDen->subTypeDenoter, compareFlags);
    }
    return false;
}

bool ArrayTypeDenoter::IsCastableTo(const TypeDenoter& targetType) const
{
    if (auto targetArrayTypeDen = targetType.GetAliased().As<ArrayTypeDenoter>())
    {
        /* Compare sub type denoters */
        if (subTypeDenoter && targetArrayTypeDen->subTypeDenoter && EqualsDimensions(*targetArrayTypeDen))
            return subTypeDenoter->IsCastableTo(*targetArrayTypeDen->subTypeDenoter);
    }
    return false;
}

unsigned int ArrayTypeDenoter::NumDimensions() const
{
    return (static_cast<unsigned int>(arrayDims.size()) + subTypeDenoter->NumDimensions());
}

AST* ArrayTypeDenoter::SymbolRef() const
{
    return (subTypeDenoter ? subTypeDenoter->SymbolRef() : nullptr);
}

bool ArrayTypeDenoter::EqualsDimensions(const ArrayTypeDenoter& rhs) const
{
    /* Compare dimension sizes */
    if (arrayDims.size() == rhs.arrayDims.size())
    {
        for (std::size_t i = 0, n = arrayDims.size(); i < n; ++i)
        {
            if (arrayDims[i]->size != rhs.arrayDims[i]->size)
                return false;
        }
        return true;
    }
    return false;
}

TypeDenoterPtr ArrayTypeDenoter::AsArray(const std::vector<ArrayDimensionPtr>& subArrayDims)
{
    if (subArrayDims.empty())
        return shared_from_this();
    else
        return std::make_shared<ArrayTypeDenoter>(subTypeDenoter, arrayDims, subArrayDims);
}

void ArrayTypeDenoter::InsertSubArray(const ArrayTypeDenoter& subArrayTypeDenoter)
{
    /* Move array dimensions into final array type */
    arrayDims.insert(
        arrayDims.end(),
        subArrayTypeDenoter.arrayDims.begin(),
        subArrayTypeDenoter.arrayDims.end()
    );

    /* Replace sub type denoter */
    subTypeDenoter = subArrayTypeDenoter.subTypeDenoter;
}

std::vector<int> ArrayTypeDenoter::GetDimensionSizes() const
{
    std::vector<int> sizes;

    for (const auto& dim : arrayDims)
        sizes.push_back(dim != nullptr ? dim->size : -1);

    return sizes;
}


} // /namespace Xsc



// ================================================================================
