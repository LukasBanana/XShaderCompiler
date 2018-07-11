/*
 * AST.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "AST.h"
#include "ASTFactory.h"
#include "Exception.h"
#include "IntrinsicAdept.h"
#include "Variant.h"
#include "SymbolTable.h"
#include "ReportHandler.h"
#include "ReportIdents.h"
#include "HLSLKeywords.h"
#include "GLSLKeywords.h"
#include <algorithm>
#include <cctype>


namespace Xsc
{


#define CALL_EXPR_FIND_PREDICATE(PREDICATE) \
    if (PREDICATE(*this))                   \
        return this


/* ----- Global functions ----- */

bool IsDeclAST(const AST::Types t)
{
    return (t >= AST::Types::VarDecl && t <= AST::Types::UniformBufferDecl);
}

bool IsExprAST(const AST::Types t)
{
    return (t >= AST::Types::NullExpr && t <= AST::Types::InitializerExpr);
}

bool IsStmntAST(const AST::Types t)
{
    return (t >= AST::Types::VarDeclStmnt && t <= AST::Types::CtrlTransferStmnt);
}

bool IsDeclStmntAST(const AST::Types t)
{
    return (t >= AST::Types::VarDeclStmnt && t <= AST::Types::BasicDeclStmnt);
}


/* ----- AST ----- */

AST::~AST()
{
    // dummy
}


/* ----- Stmnt ----- */

void Stmnt::CollectDeclIdents(std::map<const AST*, std::string>& declASTIdents) const
{
    // dummy
}


/* ----- TypedAST ----- */

const TypeDenoterPtr& TypedAST::GetTypeDenoter(const TypeDenoter* expectedTypeDenoter)
{
    if (!bufferedTypeDenoter_ || expectedTypeDenoter)
        bufferedTypeDenoter_ = DeriveTypeDenoter(expectedTypeDenoter);
    return bufferedTypeDenoter_;
}

void TypedAST::ResetTypeDenoter()
{
    bufferedTypeDenoter_.reset();
}


/* ----- Expr ----- */

VarDecl* Expr::FetchVarDecl() const
{
    if (auto lvalueExpr = FetchLValueExpr())
        return lvalueExpr->FetchVarDecl();
    else
        return nullptr;
}

const ObjectExpr* Expr::FetchLValueExpr() const
{
    return nullptr;
}

IndexedSemantic Expr::FetchSemantic() const
{
    return Semantic::Undefined;
}

const Expr* Expr::Find(const FindPredicateConstFunctor& predicate, unsigned int flags) const
{
    if (predicate && predicate(*this))
        return this;
    else
        return nullptr;
}

const Expr* Expr::FindFirstOf(const Types exprType, unsigned int flags) const
{
    return Find(
        [exprType](const Expr& expr)
        {
            return (expr.Type() == exprType);
        },
        flags
    );
}

const Expr* Expr::FindFirstNotOf(const Types exprType, unsigned int flags) const
{
    return Find(
        [exprType](const Expr& expr)
        {
            return (expr.Type() != exprType);
        },
        flags
    );
}

Expr* Expr::Find(const FindPredicateConstFunctor& predicate, unsigned int flags)
{
    /*
    Use const function and cast the constness away,
    which is allowed here, since this is a non-const member function
    */
    return const_cast<Expr*>(static_cast<const Expr*>(this)->Find(predicate, flags));
}

Expr* Expr::FindFirstOf(const Types exprType, unsigned int flags)
{
    /*
    Use const function and cast the constness away,
    which is allowed here, since this is a non-const member function
    */
    return const_cast<Expr*>(static_cast<const Expr*>(this)->FindFirstOf(exprType, flags));
}

Expr* Expr::FindFirstNotOf(const Types exprType, unsigned int flags)
{
    /*
    Use const function and cast the constness away,
    which is allowed here, since this is a non-const member function
    */
    return const_cast<Expr*>(static_cast<const Expr*>(this)->FindFirstNotOf(exprType, flags));
}


/* ----- Decl ----- */

std::string Decl::ToString() const
{
    return ident.Original();
}

TypeSpecifier* Decl::FetchTypeSpecifier() const
{
    return nullptr;
}

bool Decl::IsAnonymous() const
{
    return ident.Empty();
}


/* ----- Program ----- */

void Program::RegisterIntrinsicUsage(const Intrinsic intrinsic, const std::vector<DataType>& argumentDataTypes)
{
    /* Insert argument types (only base types) into usage list */
    IntrinsicUsage::ArgumentList argList;
    {
        argList.argTypes.reserve(argumentDataTypes.size());
        for (auto dataType : argumentDataTypes)
            argList.argTypes.push_back(dataType);
    }
    usedIntrinsics[intrinsic].argLists.insert(argList);
}

void Program::RegisterIntrinsicUsage(const Intrinsic intrinsic, const std::vector<ExprPtr>& arguments)
{
    /* Insert argument types (only base types) into usage list */
    IntrinsicUsage::ArgumentList argList;
    {
        argList.argTypes.reserve(arguments.size());
        for (auto& arg : arguments)
        {
            const auto& typeDen = arg->GetTypeDenoter()->GetAliased();
            if (auto baseTypeDen = typeDen.As<BaseTypeDenoter>())
                argList.argTypes.push_back(baseTypeDen->dataType);
        }
    }
    usedIntrinsics[intrinsic].argLists.insert(argList);
}

const IntrinsicUsage* Program::FetchIntrinsicUsage(const Intrinsic intrinsic) const
{
    auto it = usedIntrinsics.find(intrinsic);
    return (it != usedIntrinsics.end() ? &(it->second) : nullptr);
}


/* ----- Attribute ----- */

static const std::string* AttributeTypeToString(const AttributeType t)
{
    if (IsHLSLAttributeType(t))
        return AttributeTypeToHLSLKeyword(t);
    if (IsGLSLAttributeType(t))
        return AttributeTypeToGLSLKeyword(t);
    return nullptr;
}

std::string Attribute::ToString() const
{
    if (auto s = AttributeTypeToString(attributeType))
        return *s;
    else
        return R_Undefined;
}


/* ----- SwitchCase ----- */

bool SwitchCase::IsDefaultCase() const
{
    return (expr == nullptr);
}


/* ----- Register ----- */

std::string Register::ToString() const
{
    std::string s;

    s += "Register(";

    if (registerType == RegisterType::Undefined)
        s += R_Undefined;
    else
        s += RegisterTypeToString(registerType);

    s += "[" + std::to_string(slot) + "])";

    return s;
}

Register* Register::GetForTarget(const std::vector<RegisterPtr>& registers, const ShaderTarget shaderTarget)
{
    for (const auto& slotRegister : registers)
    {
        if (slotRegister->shaderTarget == ShaderTarget::Undefined || slotRegister->shaderTarget == shaderTarget)
            return slotRegister.get();
    }
    return nullptr;
}


/* ----- PackOffset ----- */

std::string PackOffset::ToString() const
{
    std::string s;

    s += "PackOffset(";
    s += registerName;

    if (!vectorComponent.empty())
    {
        s += '.';
        s += vectorComponent;
    }

    s += ')';

    return s;
}


/* ----- ArrayDimension ----- */

std::string ArrayDimension::ToString() const
{
    std::string s;

    s += '[';
    if (size > 0)
        s += std::to_string(size);
    s += ']';

    return s;
}

TypeDenoterPtr ArrayDimension::DeriveTypeDenoter(const TypeDenoter* /*expectedTypeDenoter*/)
{
    return expr->GetTypeDenoter();
}

bool ArrayDimension::HasDynamicSize() const
{
    return (size == 0);
}

void ArrayDimension::ValidateIndexBoundary(int idx) const
{
    if (size > 0)
    {
        if (idx < 0 || idx >= size)
            RuntimeErr(R_ArrayIndexOutOfBounds(idx, size));
    }
}

#if 0 //UNUSED
std::vector<int> ArrayDimension::GetArrayDimensionSizes(const std::vector<ArrayDimensionPtr>& arrayDims)
{
    std::vector<int> sizes;
    sizes.reserve(arrayDims.size());

    for (const auto& dim : arrayDims)
        sizes.push_back(dim->size);

    return sizes;
}
#endif


/* ----- TypeSpecifier ----- */

std::string TypeSpecifier::ToString() const
{
    std::string s;

    /*
    Use members 'isInput' and 'isOutput' instead of functions 'IsInput()' and 'IsOutput()',
    to only reflect the explicitly written type specifier!
    */
    if (isInput && isOutput)
        s += "inout ";
    else if (isInput)
        s += "in ";
    else if (isOutput)
        s += "out ";

    if (isUniform)
        s += "uniform ";

    if (IsConst())
        s += "const ";

    /* Append type denoter */
    s += typeDenoter->ToString();

    return s;
}

TypeDenoterPtr TypeSpecifier::DeriveTypeDenoter(const TypeDenoter* /*expectedTypeDenoter*/)
{
    return typeDenoter;
}

StructDecl* TypeSpecifier::GetStructDeclRef()
{
    const auto& typeDen = typeDenoter->GetAliased();
    if (auto structTypeDen = typeDen.As<StructTypeDenoter>())
        return structTypeDen->structDeclRef;
    else
        return nullptr;
}

bool TypeSpecifier::IsInput() const
{
    return (isInput || !isOutput);
}

bool TypeSpecifier::IsOutput() const
{
    return isOutput;
}

bool TypeSpecifier::IsConst() const
{
    return (typeModifiers.find(TypeModifier::Const) != typeModifiers.end());
}

bool TypeSpecifier::IsConstOrUniform() const
{
    return (IsConst() || isUniform);
}

void TypeSpecifier::SetTypeModifier(const TypeModifier modifier)
{
    /* Remove overlapping modifier first */
    if (modifier == TypeModifier::RowMajor)
        typeModifiers.erase(TypeModifier::ColumnMajor);
    else if (modifier == TypeModifier::ColumnMajor)
        typeModifiers.erase(TypeModifier::RowMajor);

    /* Insert new modifier */
    typeModifiers.insert(modifier);
}

bool TypeSpecifier::HasAnyTypeModifierOf(const std::initializer_list<TypeModifier>& modifiers) const
{
    for (auto mod : modifiers)
    {
        if (typeModifiers.find(mod) != typeModifiers.end())
            return true;
    }
    return false;
}

bool TypeSpecifier::HasAnyStorageClassOf(const std::initializer_list<StorageClass>& modifiers) const
{
    for (auto mod : modifiers)
    {
        if (storageClasses.find(mod) != storageClasses.end())
            return true;
    }
    return false;
}

void TypeSpecifier::SwapMatrixStorageLayout(const TypeModifier defaultStorgeLayout)
{
    bool found = false;

    /* Construct new type modifier set */
    std::set<TypeModifier> modifiers;

    for (auto mod : typeModifiers)
    {
        if (mod == TypeModifier::RowMajor)
        {
            modifiers.insert(TypeModifier::ColumnMajor);
            found = true;
        }
        else if (mod == TypeModifier::ColumnMajor)
        {
            modifiers.insert(TypeModifier::RowMajor);
            found = true;
        }
        else
            modifiers.insert(mod);
    }

    /* If no matrix storage layout has been found, insert the default layout */
    if (!found)
        modifiers.insert(defaultStorgeLayout);

    /* Take new type modifier set */
    typeModifiers = std::move(modifiers);
}


/* ----- VarDecl ----- */

std::string VarDecl::ToString() const
{
    std::string s;

    /* Append static namespace */
    if (namespaceExpr)
    {
        s += namespaceExpr->ToStringAsNamespace();
        s += "::";
    }

    /* Append variable identifier */
    s += ident.Original();

    /* Append array dimensions */
    for (const auto& dim : arrayDims)
    {
        s += '[';
        if (dim->size > 0)
            s += std::to_string(dim->size);
        s += ']';
    }

    /* Append semantic */
    if (semantic != Semantic::Undefined)
    {
        s += " : ";
        s += semantic.ToString();
    }

    return s;
}

TypeDenoterPtr VarDecl::DeriveTypeDenoter(const TypeDenoter* /*expectedTypeDenoter*/)
{
    if (customTypeDenoter)
    {
        /* Return custom type denoter */
        return customTypeDenoter;
    }
    else
    {
        if (declStmntRef)
        {
            /* Get base type denoter from declaration statement */
            return declStmntRef->typeSpecifier->typeDenoter->AsArray(arrayDims);
        }
        RuntimeErr(R_MissingDeclStmntRefToDeriveType(ident), this);
    }
}

TypeSpecifier* VarDecl::FetchTypeSpecifier() const
{
    return (declStmntRef != nullptr ? declStmntRef->typeSpecifier.get() : nullptr);
}

VarDecl* VarDecl::FetchStaticVarDeclRef() const
{
    return (namespaceExpr != nullptr ? staticMemberVarRef : nullptr);
}

VarDecl* VarDecl::FetchStaticVarDefRef() const
{
    return (namespaceExpr == nullptr ? staticMemberVarRef : nullptr);
}

bool VarDecl::IsStatic() const
{
    if (auto typeSpecifier = FetchTypeSpecifier())
        return typeSpecifier->HasAnyStorageClassOf({ StorageClass::Static });
    else
        return false;
}

bool VarDecl::IsParameter() const
{
    return (declStmntRef != nullptr && declStmntRef->flags(VarDeclStmnt::isParameter));
}

bool VarDecl::HasStaticConstInitializer() const
{
    return
    (
        declStmntRef != nullptr &&
        declStmntRef->IsConstOrUniform() &&
        !declStmntRef->flags(VarDeclStmnt::isParameter) &&
        initializerValue
    );
}

void VarDecl::SetCustomTypeDenoter(const TypeDenoterPtr& typeDenoter)
{
    customTypeDenoter = typeDenoter;
    ResetTypeDenoter();
}

void VarDecl::AddFlagsRecursive(unsigned int varFlags)
{
    if (!flags(varFlags))
    {
        /* Add flags to this variable */
        flags << varFlags;

        /* Has this variable a structure type? */
        const auto& typeDen = GetTypeDenoter()->GetAliased();
        if (auto structTypeDen = typeDen.As<StructTypeDenoter>())
        {
            if (auto structDecl = structTypeDen->structDeclRef)
            {
                /* Add flags to all structure member variables */
                structDecl->ForEachVarDecl(
                    [varFlags](VarDeclPtr& varDecl)
                    {
                        varDecl->AddFlagsRecursive(varFlags);
                    }
                );
            }
        }
    }
}


/* ----- BufferDecl ----- */

TypeDenoterPtr BufferDecl::DeriveTypeDenoter(const TypeDenoter* /*expectedTypeDenoter*/)
{
    return std::make_shared<BufferTypeDenoter>(this)->AsArray(arrayDims);
}

BufferType BufferDecl::GetBufferType() const
{
    return (declStmntRef ? declStmntRef->typeDenoter->bufferType : BufferType::Undefined);
}


/* ----- SamplerDecl ----- */

TypeDenoterPtr SamplerDecl::DeriveTypeDenoter(const TypeDenoter* /*expectedTypeDenoter*/)
{
    return std::make_shared<SamplerTypeDenoter>(this)->AsArray(arrayDims);
}

SamplerType SamplerDecl::GetSamplerType() const
{
    return (declStmntRef ? declStmntRef->typeDenoter->samplerType : SamplerType::Undefined);
}


/* ----- StructDecl ----- */

std::string StructDecl::ToString() const
{
    std::string s;

    if (isClass)
        s += "class";
    else
        s += "struct";

    s += ' ';

    if (IsAnonymous())
        s += ('<' + R_Anonymous() + '>');
    else
        s += ident.Original();

    return s;
}

bool StructDecl::EqualsMemberTypes(const StructDecl& rhs, const Flags& compareFlags) const
{
    /* Collect member type denoters from this structure */
    std::vector<TypeDenoterPtr> lhsMemberTypeDens;
    CollectMemberTypeDenoters(lhsMemberTypeDens);

    /* Collect member type denoters from structure to compare with */
    std::vector<TypeDenoterPtr> rhsMemberTypeDens;
    rhs.CollectMemberTypeDenoters(rhsMemberTypeDens);

    /* Compare number of members */
    if (lhsMemberTypeDens.size() != rhsMemberTypeDens.size())
        return false;

    /* Compare member type denoters */
    for (std::size_t i = 0, n = lhsMemberTypeDens.size(); i < n; ++i)
    {
        if (!lhsMemberTypeDens[i]->Equals(*rhsMemberTypeDens[i], compareFlags))
            return false;
    }

    return true;
}

bool StructDecl::IsCastableTo(const BaseTypeDenoter& rhs) const
{
    //TODO ...
    return true;
}

VarDecl* StructDecl::FetchVarDecl(const std::string& ident, const StructDecl** owner) const
{
    /* Fetch symbol from members first */
    for (const auto& varDeclStmnt : varMembers)
    {
        if (auto symbol = varDeclStmnt->FetchVarDecl(ident))
        {
            if (owner)
                *owner = this;
            return symbol;
        }
    }

    /* Now fetch symbol from base struct */
    if (baseStructRef)
    {
        if (auto symbol = baseStructRef->FetchVarDecl(ident, owner))
            return symbol;
    }

    return nullptr;
}

VarDecl* StructDecl::FetchBaseMember() const
{
    if (!varMembers.empty() && varMembers.front()->flags(VarDeclStmnt::isBaseMember))
        return varMembers.front()->varDecls.front().get();
    else
        return nullptr;
}

StructDecl* StructDecl::FetchBaseStructDecl(const std::string& ident)
{
    if (this->ident == ident)
        return this;
    else if (baseStructRef)
        return baseStructRef->FetchBaseStructDecl(ident);
    else
        return nullptr;
}

FunctionDecl* StructDecl::FetchFunctionDecl(
    const std::string& ident, const std::vector<TypeDenoterPtr>& argTypeDenoters, const StructDecl** owner, bool throwErrorIfNoMatch) const
{
    /* Fetch symbol from base struct first */
    if (baseStructRef)
    {
        if (auto symbol = baseStructRef->FetchFunctionDecl(ident, argTypeDenoters, owner))
            return symbol;
    }

    /* Now fetch symbol from members */
    std::vector<FunctionDecl*> funcDeclList;
    funcDeclList.reserve(funcMembers.size());

    for (const auto& funcDecl : funcMembers)
    {
        if (funcDecl->ident.Original() == ident)
            funcDeclList.push_back(funcDecl.get());
    }

    if (funcDeclList.empty())
        return nullptr;

    if (owner)
        *owner = this;

    return FunctionDecl::FetchFunctionDeclFromList(funcDeclList, ident, argTypeDenoters, throwErrorIfNoMatch);
}

std::string StructDecl::FetchSimilar(const std::string& ident)
{
    /* Collect identifiers of all structure members */
    std::vector<std::string> similarIdents;

    ForEachVarDecl(
        [&similarIdents](VarDeclPtr& varDecl)
        {
            similarIdents.push_back(varDecl->ident.Original());
        }
    );

    /* Find similar identifiers */
    const std::string* similar = nullptr;
    unsigned int dist = ~0;

    for (const auto& symbol : similarIdents)
    {
        auto d = StringDistance(ident, symbol);
        if (d < dist)
        {
            similar = (&symbol);
            dist = d;
        }
    }

    /* Check if the distance is not too large */
    if (similar != nullptr && dist < ident.size())
        return *similar;

    /* No similarities found */
    return "";
}

TypeDenoterPtr StructDecl::DeriveTypeDenoter(const TypeDenoter* /*expectedTypeDenoter*/)
{
    return std::make_shared<StructTypeDenoter>(this);
}

bool StructDecl::HasNonSystemValueMembers() const
{
    /* Check if base structure has any non-system-value members */
    if (baseStructRef && baseStructRef->HasNonSystemValueMembers())
        return true;

    /* Search for non-system-value member */
    for (const auto& member : varMembers)
    {
        for (const auto& varDecl : member->varDecls)
        {
            if (!varDecl->semantic.IsSystemValue())
                return true;
        }
    }

    /* No non-system-value member found */
    return false;
}

std::size_t StructDecl::NumMemberVariables(bool onlyNonStaticMembers) const
{
    std::size_t n = 0;

    if (baseStructRef)
        n += baseStructRef->NumMemberVariables(onlyNonStaticMembers);

    for (const auto& member : varMembers)
    {
        if (!onlyNonStaticMembers || !member->typeSpecifier->HasAnyStorageClassOf({ StorageClass::Static }))
            n += member->varDecls.size();
    }

    return n;
}

std::size_t StructDecl::NumMemberFunctions(bool onlyNonStaticMembers) const
{
    std::size_t n = 0;

    if (baseStructRef)
        n += baseStructRef->NumMemberFunctions(onlyNonStaticMembers);

    if (onlyNonStaticMembers)
    {
        for (const auto& member : funcMembers)
        {
            if (!member->IsStatic())
                ++n;
        }
    }
    else
        n += funcMembers.size();

    return n;
}

void StructDecl::CollectMemberTypeDenoters(std::vector<TypeDenoterPtr>& memberTypeDens, bool includeBaseStructs) const
{
    /* First collect type denoters from base structure */
    if (baseStructRef && includeBaseStructs)
        baseStructRef->CollectMemberTypeDenoters(memberTypeDens, includeBaseStructs);

    /* Collect type denoters from this structure */
    for (const auto& member : varMembers)
    {
        /* Add type denoter of each member variable */
        for (const auto& varDecl : member->varDecls)
            memberTypeDens.push_back(varDecl->GetTypeDenoter());
    }
}

void StructDecl::ForEachVarDecl(const VarDeclIteratorFunctor& iterator, bool includeBaseStructs)
{
    /* Iterate over all base struct members */
    if (baseStructRef && includeBaseStructs)
        baseStructRef->ForEachVarDecl(iterator, includeBaseStructs);

    for (auto& member : varMembers)
    {
        /* Iterate over all sub-struct members */
        const auto& typeDen = member->typeSpecifier->GetTypeDenoter()->GetAliased();
        if (auto structTypeDen = typeDen.As<StructTypeDenoter>())
        {
            if (auto structDecl = structTypeDen->structDeclRef)
                structDecl->ForEachVarDecl(iterator);
        }

        /* Iterate over all variables for current member */
        member->ForEachVarDecl(iterator);
    }
}

void StructDecl::AddShaderOutputInstance(VarDecl* varDecl)
{
    if (varDecl)
        shaderOutputVarDeclRefs.insert(varDecl);
}

bool StructDecl::HasMultipleShaderOutputInstances() const
{
    auto numInstances = shaderOutputVarDeclRefs.size();
    if (numInstances == 1)
    {
        auto varDecl = *shaderOutputVarDeclRefs.begin();
        return (!varDecl->arrayDims.empty());
    }
    return (numInstances > 1);
}

bool StructDecl::IsBaseOf(const StructDecl* subStructDecl, bool includeSelf) const
{
    if (subStructDecl)
    {
        if (includeSelf && subStructDecl == this)
            return true;

        if (auto baseStructDecl = subStructDecl->baseStructRef)
        {
            /* Check if this structure is a direct base of the specified structure */
            if (baseStructDecl == this)
                return true;
            else
            {
                /* Otherwise, repeat the check for the base of the specified structure */
                return IsBaseOf(baseStructDecl);
            }
        }
    }
    return false;
}

void StructDecl::AddFlagsRecursive(unsigned int structFlags)
{
    /* Add flags to this structure */
    if (flags.SetOnce(structFlags))
    {
        /* Add flags to base structure */
        if (baseStructRef)
            baseStructRef->AddFlagsRecursive(structFlags);

        /* Add flags to nested structures */
        for (auto& member : varMembers)
        {
            /* Iterate over all sub-struct members */
            const auto& typeDen = member->typeSpecifier->GetTypeDenoter()->GetAliased();
            if (auto structTypeDen = typeDen.As<StructTypeDenoter>())
            {
                if (auto structDecl = structTypeDen->structDeclRef)
                    structDecl->AddFlagsRecursive(structFlags);
            }
        }
    }
}

void StructDecl::AddFlagsRecursiveParents(unsigned int structFlags)
{
    /* Add flags to this structure */
    if (flags.SetOnce(structFlags))
    {
        /* Add flags to parent structures, i.e. to all structures that have a member variable with this structure type */
        for (const auto parent : parentStructDeclRefs)
            parent->AddFlagsRecursiveParents(structFlags);
    }
}

std::size_t StructDecl::MemberVarToIndex(const VarDecl* varDecl, bool includeBaseStructs) const
{
    static const std::size_t invalidIdx = ~0;

    std::size_t idx = 0;

    /* Search member variable in base struct */
    if (baseStructRef && includeBaseStructs)
    {
        auto baseIdx = baseStructRef->MemberVarToIndex(varDecl, includeBaseStructs);
        if (baseIdx != invalidIdx)
            idx = baseIdx;
    }

    /* Search member variable in this struct */
    for (const auto& member : varMembers)
    {
        for (const auto& var : member->varDecls)
        {
            if (varDecl == var.get())
                return idx;
            ++idx;
        }
    }

    return invalidIdx;
}

static VarDecl* FindIndexOfStructMemberVar(const StructDecl& structDecl, std::size_t& idx, bool includeBaseStructs)
{
    /* Search member variable in base struct */
    if (structDecl.baseStructRef && includeBaseStructs)
    {
        if (auto varDecl = FindIndexOfStructMemberVar(*structDecl.baseStructRef, idx, includeBaseStructs))
            return varDecl;
    }

    /* Search member variable in this struct */
    for (const auto& member : structDecl.varMembers)
    {
        if (idx < member->varDecls.size())
            return member->varDecls[idx].get();
        else
            idx -= member->varDecls.size();
    }

    return nullptr;
}

VarDecl* StructDecl::IndexToMemberVar(std::size_t idx, bool includeBaseStructs) const
{
    return FindIndexOfStructMemberVar(*this, idx, includeBaseStructs);
}


/* ----- AliasDecl ----- */

TypeDenoterPtr AliasDecl::DeriveTypeDenoter(const TypeDenoter* /*expectedTypeDenoter*/)
{
    return typeDenoter;
}


/* ----- FunctionDecl ----- */

void FunctionDecl::ParameterSemantics::Add(VarDecl* varDecl)
{
    if (varDecl)
    {
        if (varDecl->flags(VarDecl::isSystemValue))
            varDeclRefsSV.push_back(varDecl);
        else
            varDeclRefs.push_back(varDecl);
    }
}

bool FunctionDecl::ParameterSemantics::Contains(VarDecl* varDecl) const
{
    return
    (
        ( std::find(varDeclRefs  .begin(), varDeclRefs  .end(), varDecl) != varDeclRefs  .end() ) ||
        ( std::find(varDeclRefsSV.begin(), varDeclRefsSV.end(), varDecl) != varDeclRefsSV.end() )
    );
}

void FunctionDecl::ParameterSemantics::ForEach(const IteratorFunc& iterator)
{
    std::for_each(varDeclRefs.begin(), varDeclRefs.end(), iterator);
    std::for_each(varDeclRefsSV.begin(), varDeclRefsSV.end(), iterator);
}

bool FunctionDecl::ParameterSemantics::Empty() const
{
    return (varDeclRefs.empty() && varDeclRefsSV.empty());
}

void FunctionDecl::ParameterSemantics::UpdateDistribution()
{
    /* Move system-value semantics */
    for (auto it = varDeclRefs.begin(); it != varDeclRefs.end();)
    {
        if ((*it)->semantic.IsSystemValue())
        {
            varDeclRefsSV.push_back(*it);
            it = varDeclRefs.erase(it);
        }
        else
            ++it;
    }

    /* Move non-system-value semantics */
    for (auto it = varDeclRefsSV.begin(); it != varDeclRefsSV.end();)
    {
        if (!(*it)->semantic.IsSystemValue())
        {
            varDeclRefs.push_back(*it);
            it = varDeclRefsSV.erase(it);
        }
        else
            ++it;
    }
}

TypeDenoterPtr FunctionDecl::DeriveTypeDenoter(const TypeDenoter* /*expectedTypeDenoter*/)
{
    //RuntimeErr(R_CantDeriveTypeOfFunction, this);
    return std::make_shared<FunctionTypeDenoter>(this);
}

bool FunctionDecl::IsForwardDecl() const
{
    return (codeBlock == nullptr);
}

bool FunctionDecl::HasVoidReturnType() const
{
    return returnType->typeDenoter->IsVoid();
}

bool FunctionDecl::IsMemberFunction() const
{
    return (structDeclRef != nullptr);
}

bool FunctionDecl::IsStatic() const
{
    return returnType->HasAnyStorageClassOf({ StorageClass::Static });
}

std::string FunctionDecl::ToString() const
{
    return ToString(true);
}

std::string FunctionDecl::ToString(bool useParamNames) const
{
    std::string s;

    /* Append return type */
    s += returnType->ToString();
    s += ' ';

    /* Append optional owner structure */
    if (structDeclRef)
    {
        s += structDeclRef->ident.Original();
        s += "::";
    }

    /* Append identifier */
    s += ident.Original();

    /* Append parameter types */
    s += '(';

    for (std::size_t i = 0; i < parameters.size(); ++i)
    {
        if (!parameters[i]->flags(VarDeclStmnt::isSelfParameter))
        {
            s += parameters[i]->ToString(useParamNames);
            if (i + 1 < parameters.size())
                s += ", ";
        }
    }

    s += ')';

    return s;
}

std::string FunctionDecl::ToTypeDenoterString() const
{
    std::string s;

    /* Append function return type */
    s += returnType->ToString();

    /* Append parameter types */
    s += '(';

    for (std::size_t i = 0; i < parameters.size(); ++i)
    {
        if (!parameters[i]->flags(VarDeclStmnt::isSelfParameter))
        {
            s += parameters[i]->ToString(false);
            if (i + 1 < parameters.size())
                s += ", ";
        }
    }

    s += ')';

    return s;
}

bool FunctionDecl::EqualsSignature(const FunctionDecl& rhs, const Flags& compareFlags) const
{
    /* Compare parameter count */
    if (parameters.size() != rhs.parameters.size())
        return false;

    /* Compare parameter type denoters */
    for (std::size_t i = 0; i < parameters.size(); ++i)
    {
        auto lhsTypeDen = parameters[i]->typeSpecifier->typeDenoter.get();
        auto rhsTypeDen = rhs.parameters[i]->typeSpecifier->typeDenoter.get();

        if (!lhsTypeDen->Equals(*rhsTypeDen, compareFlags))
            return false;
    }

    return true;
}

std::size_t FunctionDecl::NumMinArgs() const
{
    std::size_t n = 0;

    for (const auto& param : parameters)
    {
        if (!param->varDecls.empty() && param->varDecls.front()->initializer != nullptr)
            break;
        ++n;
    }

    return n;
}

std::size_t FunctionDecl::NumMaxArgs() const
{
    return parameters.size();
}

void FunctionDecl::SetFuncImplRef(FunctionDecl* funcDecl)
{
    if (funcDecl && !funcDecl->IsForwardDecl() && IsForwardDecl())
    {
        funcImplRef = funcDecl;
        funcDecl->funcForwardDeclRefs.push_back(this);
    }
}

bool FunctionDecl::MatchParameterWithTypeDenoter(std::size_t paramIndex, const TypeDenoter& argType, bool implicitConversion) const
{
    /* Get parameter by index */
    if (paramIndex >= parameters.size())
        return false;

    const auto& param = parameters[paramIndex];
    if (param->varDecls.size() != 1)
        return false;

    const auto& paramVar = param->varDecls.front();

    /* Get type denoters to compare */
    const auto& paramTypeDen = paramVar->GetTypeDenoter();

    /* Check for explicit compatability: are they equal? */
    if (!argType.Equals(*paramTypeDen))
    {
        if (implicitConversion)
        {
            /* Check for implicit compatability: is it castable? */
            if (!argType.IsCastableTo(*paramTypeDen))
                return false;
        }
        else
            return false;
    }

    return true;
}

static bool MatchNumArgsToFunctionDecls(const std::vector<FunctionDecl*>& funcDeclList, std::size_t numArgs)
{
    for (auto funcDecl : funcDeclList)
    {
        /* Are the number of arguments sufficient? */
        if (numArgs >= funcDecl->NumMinArgs() && numArgs <= funcDecl->NumMaxArgs())
            return true;
    }
    return false;
}

static bool MatchFunctionDeclWithArgs(
    FunctionDecl& funcDecl, const std::vector<TypeDenoterPtr>& typeDens, bool implicitTypeConversion)
{
    auto numArgs = typeDens.size();
    if (numArgs >= funcDecl.NumMinArgs() && numArgs <= funcDecl.NumMaxArgs())
    {
        for (std::size_t i = 0, n = std::min(typeDens.size(), funcDecl.parameters.size()); i < n; ++i)
        {
            /* Match argument type denoter to parameter */
            if (!funcDecl.MatchParameterWithTypeDenoter(i, *typeDens[i], implicitTypeConversion))
                return false;
        }
        return true;
    }
    return false;
}

static void ListAllFuncCandidates(const std::vector<FunctionDecl*>& candidates)
{
    ReportHandler::HintForNextReport(R_CandidatesAre + ":");
    for (auto funcDecl : candidates)
        ReportHandler::HintForNextReport("  '" + funcDecl->ToString(false) + "' (" + funcDecl->area.Pos().ToString() + ")");
};

FunctionDecl* FunctionDecl::FetchFunctionDeclFromList(
    const std::vector<FunctionDecl*>& funcDeclList, const std::string& ident,
    const std::vector<TypeDenoterPtr>& argTypeDenoters, bool throwErrorIfNoMatch)
{
    if (funcDeclList.empty())
    {
        if (throwErrorIfNoMatch)
            RuntimeErr(R_UndefinedSymbol(ident));
        else
            return nullptr;
    }

    /* Match number of arguments to function declarations */
    const auto numArgs = argTypeDenoters.size();

    if (!MatchNumArgsToFunctionDecls(funcDeclList, numArgs))
    {
        if (throwErrorIfNoMatch)
        {
            /* Add candidate signatures to report hints */
            ListAllFuncCandidates(funcDeclList);

            /* Throw runtime error */
            if (numArgs == 1)
                RuntimeErr(R_FuncDoesntTake1Param(ident, numArgs));
            else
                RuntimeErr(R_FuncDoesntTakeNParams(ident, numArgs));
        }
        else
            return nullptr;
    }

    /* Find best fit with explicit argument types */
    std::vector<FunctionDecl*> funcDeclCandidates;

    for (auto funcDecl : funcDeclList)
    {
        if (MatchFunctionDeclWithArgs(*funcDecl, argTypeDenoters, false))
            funcDeclCandidates.push_back(funcDecl);
    }

    /* Nothing found? -> find first fit with implicit argument types */
    if (funcDeclCandidates.empty())
    {
        for (auto funcDecl : funcDeclList)
        {
            if (MatchFunctionDeclWithArgs(*funcDecl, argTypeDenoters, true))
                funcDeclCandidates.push_back(funcDecl);
        }
    }

    /* Check for ambiguous function call */
    if (funcDeclCandidates.size() != 1)
    {
        if (funcDeclCandidates.empty() && !throwErrorIfNoMatch)
            return nullptr;

        /* Construct descriptive string for argument type names */
        std::string argTypeNames;

        if (numArgs > 0)
        {
            for (std::size_t i = 0; i < numArgs; ++i)
            {
                argTypeNames += argTypeDenoters[i]->ToString();
                if (i + 1 < numArgs)
                    argTypeNames += ", ";
            }
        }
        else
            argTypeNames = "void";

        /* Add candidate signatures to report hints */
        if (funcDeclCandidates.empty())
            ListAllFuncCandidates(funcDeclList);
        else
            ListAllFuncCandidates(funcDeclCandidates);

        /* Throw runtime error */
        RuntimeErr(R_AmbiguousFuncCall(ident, argTypeNames));
    }

    return funcDeclCandidates.front();
}


/* ----- UniformBufferDecl ----- */

TypeDenoterPtr UniformBufferDecl::DeriveTypeDenoter(const TypeDenoter* /*expectedTypeDenoter*/)
{
    RuntimeErr(R_CantDeriveTypeOfConstBuffer, this);
}

std::string UniformBufferDecl::ToString() const
{
    std::string s;

    switch (bufferType)
    {
        case UniformBufferType::Undefined:
            s = R_Undefined;
            break;
        case UniformBufferType::ConstantBuffer:
            s = "cbuffer";
            break;
        case UniformBufferType::TextureBuffer:
            s = "tbuffer";
            break;
    }

    s += ' ';
    s += ident;

    return s;
}

TypeModifier UniformBufferDecl::DeriveCommonStorageLayout(const TypeModifier defaultStorgeLayout)
{
    /* Count number of row- and column major storage layouts */
    std::size_t numRowMajors = 0,
                numColMajors = 0;

    for (const auto& varDeclStmnt : varMembers)
    {
        const auto& typeModifers = varDeclStmnt->typeSpecifier->typeModifiers;
        if (typeModifers.find(TypeModifier::RowMajor) != typeModifers.end())
            ++numRowMajors;
        else if (typeModifers.find(TypeModifier::ColumnMajor) != typeModifers.end())
            ++numColMajors;
    }

    /* Set storage layout that is used the most */
    if (numRowMajors > 0 && numRowMajors >= numColMajors)
        commonStorageLayout = TypeModifier::RowMajor;
    else if (numColMajors > 0)
        commonStorageLayout = TypeModifier::ColumnMajor;
    else if (defaultStorgeLayout != TypeModifier::Undefined)
        commonStorageLayout = defaultStorgeLayout;

    return commonStorageLayout;
}


/* ----- BufferDeclStmnt ----- */

void BufferDeclStmnt::CollectDeclIdents(std::map<const AST*, std::string>& declASTIdents) const
{
    for (const auto& ast : bufferDecls)
        declASTIdents[ast.get()] = ast->ident;
}


/* ----- SamplerDeclStmnt ----- */

void SamplerDeclStmnt::CollectDeclIdents(std::map<const AST*, std::string>& declASTIdents) const
{
    for (const auto& ast : samplerDecls)
        declASTIdents[ast.get()] = ast->ident;
}


/* ----- VarDelcStmnt ----- */

void VarDeclStmnt::CollectDeclIdents(std::map<const AST*, std::string>& declASTIdents) const
{
    for (const auto& ast : varDecls)
        declASTIdents[ast.get()] = ast->ident;
}

std::string VarDeclStmnt::ToString(bool useVarNames) const
{
    auto s = typeSpecifier->ToString();

    if (useVarNames)
    {
        /* Append variable names with their array dimensions */
        for (std::size_t i = 0; i < varDecls.size(); ++i)
        {
            s += ' ';
            s += varDecls[i]->ToString();
            if (i + 1 < varDecls.size())
                s += ',';
        }
    }
    else if (varDecls.size() == 1)
    {
        /* Append only array dimension of single variable */
        for (const auto& dim : varDecls.front()->arrayDims)
        {
            s += '[';
            if (dim->size > 0)
                s += std::to_string(dim->size);
            s += ']';
        }
    }

    /* Append indicator for optional parameters */
    if (flags(VarDeclStmnt::isParameter) && !varDecls.empty() && varDecls.front()->initializer)
        return '[' + s + ']';

    return s;
}

VarDecl* VarDeclStmnt::FetchVarDecl(const std::string& ident) const
{
    for (const auto& var : varDecls)
    {
        if (var->ident.Original() == ident)
            return var.get();
    }
    return nullptr;
}

bool VarDeclStmnt::IsInput() const
{
    return typeSpecifier->IsInput();
}

bool VarDeclStmnt::IsOutput() const
{
    return typeSpecifier->IsOutput();
}

bool VarDeclStmnt::IsUniform() const
{
    return typeSpecifier->isUniform;
}

bool VarDeclStmnt::IsConstOrUniform() const
{
    return typeSpecifier->IsConstOrUniform();
}

void VarDeclStmnt::SetTypeModifier(const TypeModifier modifier)
{
    typeSpecifier->SetTypeModifier(modifier);
}

bool VarDeclStmnt::HasAnyTypeModifierOf(const std::initializer_list<TypeModifier>& modifiers) const
{
    return typeSpecifier->HasAnyTypeModifierOf(modifiers);
}

void VarDeclStmnt::ForEachVarDecl(const VarDeclIteratorFunctor& iterator)
{
    if (iterator)
    {
        for (auto& varDecl : varDecls)
            iterator(varDecl);
    }
}

void VarDeclStmnt::MakeImplicitConst()
{
    /* Is this variable type currenlty not constant, uniform, static, or shared? */
    if ( !IsConstOrUniform() &&
         !typeSpecifier->HasAnyStorageClassOf({ StorageClass::Static, StorageClass::GroupShared }) )
    {
        /* Is variable declaration a static structure member? */
        for (const auto& varDecl : varDecls)
        {
            if (varDecl->namespaceExpr != nullptr)
                return;
        }

        /* Mark as implicitly constant */
        flags << VarDeclStmnt::isImplicitConst;
        typeSpecifier->isUniform = true;
    }
}

StructDecl* VarDeclStmnt::FetchStructDeclRef() const
{
    if (varDecls.empty())
        return nullptr;
    else
        return varDecls.front()->structDeclRef;
}


/* ----- NullExpr ----- */

TypeDenoterPtr NullExpr::DeriveTypeDenoter(const TypeDenoter* /*expectedTypeDenoter*/)
{
    /*
    Return 'int' as type, because null expressions are only
    used as dynamic array dimensions (which must be integral types)
    */
    return std::make_shared<BaseTypeDenoter>(DataType::Int);
}


/* ----- SequenceExpr ----- */

TypeDenoterPtr SequenceExpr::DeriveTypeDenoter(const TypeDenoter* /*expectedTypeDenoter*/)
{
    /* Only return type denoter of first sub expression */
    return exprs.front()->GetTypeDenoter();
}

const Expr* SequenceExpr::Find(const FindPredicateConstFunctor& predicate, unsigned int flags) const
{
    if (predicate)
    {
        /* Call predicate for this expression */
        CALL_EXPR_FIND_PREDICATE(predicate);

        /* Search in sub expressions */
        for (const auto& subExpr : exprs)
        {
            if (auto e = subExpr->Find(predicate, flags))
                return e;
        }
    }
    return nullptr;
}

void SequenceExpr::Append(const ExprPtr& expr)
{
    if (auto listExpr = expr->As<SequenceExpr>())
    {
        /* Copy all sub expressions of the 'listExpr' into this expression */
        exprs.insert(exprs.end(), listExpr->exprs.begin(), listExpr->exprs.end());
    }
    else
    {
        /* Simply add expression to list */
        exprs.push_back(expr);
    }
}


/* ----- LiteralExpr ----- */

TypeDenoterPtr LiteralExpr::DeriveTypeDenoter(const TypeDenoter* /*expectedTypeDenoter*/)
{
    if (IsNull())
        return std::make_shared<NullTypeDenoter>();
    else
        return std::make_shared<BaseTypeDenoter>(dataType);
}

void LiteralExpr::ConvertDataType(const DataType type)
{
    if (dataType != type)
    {
        /* Parse variant from value string */
        auto variant = Variant::ParseFrom(value);

        switch (type)
        {
            case DataType::Bool:
                variant.ToBool();
                value = variant.ToString();
                break;

            case DataType::Int:
                variant.ToInt();
                value = variant.ToString();
                break;

            case DataType::UInt:
                variant.ToInt();
                value = variant.ToString() + "u";
                break;

            case DataType::Half:
                if (dataType != DataType::Double)
                {
                    variant.ToReal();
                    value = variant.ToString();
                }
                break;

            case DataType::Float:
                if (dataType != DataType::Double)
                {
                    variant.ToReal();
                    value = variant.ToString();
                }
                value.push_back('f');
                break;

            case DataType::Double:
                variant.ToReal();
                value = variant.ToString();
                break;

            default:
                break;
        }

        /* Set new data type and reset buffered type dentoer */
        dataType = type;
        ResetTypeDenoter();
    }
}

std::string LiteralExpr::GetStringValue() const
{
    /* Return string literal content (without the quotation marks) */
    if (dataType == DataType::String && value.size() >= 2 && value.front() == '\"' && value.back() == '\"')
        return value.substr(1, value.size() - 2);
    else
        return "";
}

bool LiteralExpr::IsNull() const
{
    return (dataType == DataType::Undefined && value == "NULL");
}

bool LiteralExpr::IsSpaceRequiredForSubscript() const
{
    /* Check if a space between this literal and a '.' swizzle operator is required (e.g. "1.xx" --> "1 .xx") */
    return (!value.empty() && value.find('.') == std::string::npos && std::isdigit(static_cast<int>(value.back())));
}


/* ----- TypeSpecifierExpr ----- */

TypeDenoterPtr TypeSpecifierExpr::DeriveTypeDenoter(const TypeDenoter* /*expectedTypeDenoter*/)
{
    return typeSpecifier->GetTypeDenoter();
}


/* ----- TernaryExpr ----- */

TypeDenoterPtr TernaryExpr::DeriveTypeDenoter(const TypeDenoter* /*expectedTypeDenoter*/)
{
    /* Check if conditional expression is compatible to a boolean */
    const auto& condTypeDen = condExpr->GetTypeDenoter();
    const BaseTypeDenoter boolTypeDen(DataType::Bool);

    if (!condTypeDen->IsCastableTo(boolTypeDen))
        RuntimeErr(R_IllegalCast(condTypeDen->ToString(), boolTypeDen.ToString(), R_ConditionOfTernaryExpr), condExpr.get());

    /* Find common type denoter for both sub expressions */
    const auto& thenTypeDen = thenExpr->GetTypeDenoter()->GetAliased();
    const auto& elseTypeDen = elseExpr->GetTypeDenoter()->GetAliased();

    if (!elseTypeDen.IsCastableTo(thenTypeDen))
        RuntimeErr(R_IllegalCast(elseTypeDen.ToString(), thenTypeDen.ToString(), R_TernaryExpr), this);

    auto commonTypeDen = TypeDenoter::FindCommonTypeDenoterFrom(thenExpr, elseExpr, false, this);

    /* Get common boolean type denoter from condition expression */
    const auto& condTypeDenAliased = condExpr->GetTypeDenoter()->GetAliased();

    if (auto baseTypeDen = condTypeDenAliased.As<BaseTypeDenoter>())
    {
        /* Is the condition a boolean vector type? */
        const auto condVecSize = VectorTypeDim(baseTypeDen->dataType);
        if (condVecSize > 1)
        {
            /* Is the sub type denoter a base type denoter? */
            if (auto baseSubTypeDen = commonTypeDen->As<BaseTypeDenoter>())
            {
                /* Return common type denoter, based on conditional expression type dimension */
                const auto subDataType = VectorDataType(baseSubTypeDen->dataType, condVecSize);
                return std::make_shared<BaseTypeDenoter>(subDataType);
            }
        }
    }

    return commonTypeDen;
}

const Expr* TernaryExpr::Find(const FindPredicateConstFunctor& predicate, unsigned int flags) const
{
    if (predicate)
    {
        /* Call predicate for this expression */
        CALL_EXPR_FIND_PREDICATE(predicate);

        /* Search in sub expressions */
        if ((flags & SearchRValue) != 0)
        {
            if (auto e = condExpr->Find(predicate, flags))
                return e;
            if (auto e = thenExpr->Find(predicate, flags))
                return e;
            if (auto e = elseExpr->Find(predicate, flags))
                return e;
        }
    }
    return nullptr;
}

bool TernaryExpr::IsVectorCondition() const
{
    const auto& condTypeDen = condExpr->GetTypeDenoter()->GetAliased();

    if (auto baseTypeDen = condTypeDen.As<BaseTypeDenoter>())
    {
        /* Is the condition a boolean vector type? */
        const auto condVecSize = VectorTypeDim(baseTypeDen->dataType);
        return (condVecSize > 1);
    }

    return false;
}


/* ----- BinaryExpr ----- */

TypeDenoterPtr BinaryExpr::DeriveTypeDenoter(const TypeDenoter* /*expectedTypeDenoter*/)
{
    /* Return type of left-hand-side sub expresion if the types are compatible */
    const auto& lhsTypeDen = lhsExpr->GetTypeDenoter()->GetAliased();
    const auto& rhsTypeDen = rhsExpr->GetTypeDenoter()->GetAliased();

    if (!rhsTypeDen.IsCastableTo(lhsTypeDen) || !lhsTypeDen.IsCastableTo(rhsTypeDen))
        RuntimeErr(R_IllegalCast(rhsTypeDen.ToString(), lhsTypeDen.ToString(), R_BinaryExpr(BinaryOpToString(op))), this);

    /* Find common type denoter of left and right sub expressions */
    if (auto commonTypeDen = TypeDenoter::FindCommonTypeDenoterFrom(lhsExpr, rhsExpr, false, this))
    {
        /* Throw error if any expression has not a base type */
        if (!lhsTypeDen.IsBase())
            RuntimeErr(R_OnlyBaseTypeAllowed(R_BinaryExpr(BinaryOpToString(op)), lhsTypeDen.ToString()), this);
        if (!rhsTypeDen.IsBase())
            RuntimeErr(R_OnlyBaseTypeAllowed(R_BinaryExpr(BinaryOpToString(op)), rhsTypeDen.ToString()), this);

        if (IsBooleanOp(op))
            return TypeDenoter::MakeBoolTypeWithDimensionOf(*commonTypeDen);
        else
            return commonTypeDen;
    }

    return nullptr;
}

const Expr* BinaryExpr::Find(const FindPredicateConstFunctor& predicate, unsigned int flags) const
{
    if (predicate)
    {
        /* Call predicate for this expression */
        CALL_EXPR_FIND_PREDICATE(predicate);

        /* Search in sub expressions */
        if ((flags & SearchRValue) != 0)
        {
            if (auto e = lhsExpr->Find(predicate, flags))
                return e;
            if (auto e = rhsExpr->Find(predicate, flags))
                return e;
        }
    }
    return nullptr;
}


/* ----- UnaryExpr ----- */

TypeDenoterPtr UnaryExpr::DeriveTypeDenoter(const TypeDenoter* /*expectedTypeDenoter*/)
{
    const auto& typeDen = expr->GetTypeDenoter();

    if (IsLogicalOp(op))
        return TypeDenoter::MakeBoolTypeWithDimensionOf(*typeDen);
    else
        return typeDen;
}

const Expr* UnaryExpr::Find(const FindPredicateConstFunctor& predicate, unsigned int flags) const
{
    if (predicate)
    {
        /* Call predicate for this expression */
        CALL_EXPR_FIND_PREDICATE(predicate);

        /* Search in sub expression */
        if ( ( IsLValueOp(op) && ((flags & SearchLValue) != 0) ) ||
             ( !IsLValueOp(op) && ((flags & SearchRValue) != 0) ) )
        {
            if (auto e = expr->Find(predicate, flags))
                return e;
        }
    }
    return nullptr;
}

const ObjectExpr* UnaryExpr::FetchLValueExpr() const
{
    if (IsLValueOp(op))
        return expr->FetchLValueExpr();
    else
        return nullptr;
}


/* ----- PostUnaryExpr ----- */

TypeDenoterPtr PostUnaryExpr::DeriveTypeDenoter(const TypeDenoter* /*expectedTypeDenoter*/)
{
    return expr->GetTypeDenoter();
}

const Expr* PostUnaryExpr::Find(const FindPredicateConstFunctor& predicate, unsigned int flags) const
{
    if (predicate)
    {
        /* Call predicate for this expression */
        CALL_EXPR_FIND_PREDICATE(predicate);

        /* Search in sub expression */
        if ((flags & SearchRValue) != 0)
        {
            if (auto e = expr->Find(predicate, flags))
                return e;
        }
    }
    return nullptr;
}


/* ----- CallExpr ----- */

TypeDenoterPtr CallExpr::DeriveTypeDenoter(const TypeDenoter* /*expectedTypeDenoter*/)
{
    if (auto funcDecl = GetFunctionDecl())
    {
        /* Return type denoter of associated function declaration */
        return funcDecl->returnType->typeDenoter;
    }
    else if (typeDenoter)
    {
        #ifdef XSC_ENABLE_LANGUAGE_EXT

        /* Derive type denoter with optional vector-space */
        if (auto baseTypeDen = typeDenoter->GetAliased().As<BaseTypeDenoter>())
        {
            /* Verify that all arguments have the same vector space */
            auto vectorSpace = VectorSpace::FindCommonVectorSpace(arguments, true, this);
            if (vectorSpace.IsSpecified())
            {
                /* Return copy of type denoter with derived vector-space */
                auto typeDen = baseTypeDen->Copy();
                typeDen->As<BaseTypeDenoter>()->vectorSpace = vectorSpace;
                return typeDen;
            }
        }

        #endif

        /* Return type denoter of type constructor */
        return typeDenoter;
    }
    else if (intrinsic != Intrinsic::Undefined)
    {
        try
        {
            /* Get object expression if this is a member function */
            auto memberFuncObjExpr = GetMemberFuncObjectExpr();

            /* Return type denoter of associated intrinsic */
            return IntrinsicAdept::Get().GetIntrinsicReturnType(
                intrinsic,
                arguments,
                (memberFuncObjExpr != nullptr ? memberFuncObjExpr->GetTypeDenoter() : nullptr)
            );
        }
        catch (const std::exception& e)
        {
            RuntimeErr(e.what(), this);
        }
    }
    else
        RuntimeErr(R_MissingFuncRefToDeriveExprType, this);
}

const Expr* CallExpr::Find(const FindPredicateConstFunctor& predicate, unsigned int flags) const
{
    if (predicate)
    {
        /* Call predicate for this expression */
        CALL_EXPR_FIND_PREDICATE(predicate);

        /* Search in sub expression */
        if ((flags & SearchRValue) != 0 && prefixExpr)
        {
            if (auto e = prefixExpr->Find(predicate, flags))
                return e;
        }
    }
    return nullptr;
}

IndexedSemantic CallExpr::FetchSemantic() const
{
    /* Return semantic of function declaration */
    if (auto funcDecl = GetFunctionDecl())
        return funcDecl->semantic;
    else
        return Semantic::Undefined;
}

std::vector<Expr*> CallExpr::GetArguments() const
{
    std::vector<Expr*> args;
    args.reserve(arguments.size() + defaultArgumentRefs.size());

    /* Add explicit arguments */
    for (const auto& arg : arguments)
        args.push_back(arg.get());

    /* Add remaining default arguments */
    for (auto arg : defaultArgumentRefs)
        args.push_back(arg);

    return args;
}

FunctionDecl* CallExpr::GetFunctionDecl() const
{
    return funcDeclRef;
}

FunctionDecl* CallExpr::GetFunctionImpl() const
{
    if (auto funcDecl = GetFunctionDecl())
    {
        if (funcDecl->funcImplRef)
            return funcDecl->funcImplRef;
        else
            return funcDecl;
    }
    return nullptr;
}

void CallExpr::ForEachOutputArgument(const ExprIteratorFunctor& iterator)
{
    if (iterator)
    {
        if (auto funcDecl = GetFunctionDecl())
        {
            /* Get output parameters from associated function declaration */
            const auto& parameters = funcDecl->parameters;
            for (std::size_t i = 0, n = std::min(arguments.size(), parameters.size()); i < n; ++i)
            {
                if (parameters[i]->IsOutput())
                    iterator(arguments[i]);
            }
        }
        else if (intrinsic != Intrinsic::Undefined)
        {
            /* Get output parameters from associated intrinsic */
            const auto outputParamIndices = IntrinsicAdept::Get().GetIntrinsicOutputParameterIndices(intrinsic);
            for (auto paramIndex : outputParamIndices)
            {
                if (paramIndex < arguments.size())
                    iterator(arguments[paramIndex]);
            }
        }
    }
}

void CallExpr::ForEachArgumentWithParameterType(const ArgumentParameterTypeFunctor& iterator)
{
    if (iterator)
    {
        if (auto funcDecl = GetFunctionDecl())
        {
            /* Get parameter type denoters from associated function declaration */
            const auto& parameters = funcDecl->parameters;
            for (std::size_t i = 0, n = std::min(arguments.size(), parameters.size()); i < n; ++i)
            {
                auto param = parameters[i]->varDecls.front();
                const auto& paramTypeDen = param->GetTypeDenoter()->GetAliased();
                iterator(arguments[i], paramTypeDen);
            }
        }
        else if (intrinsic != Intrinsic::Undefined)
        {
            /* Get parameter type denoters from associated intrinsic */
            const auto paramTypeDenoters = IntrinsicAdept::Get().GetIntrinsicParameterTypes(intrinsic, arguments);
            for (std::size_t i = 0, n = std::min(arguments.size(), paramTypeDenoters.size()); i < n; ++i)
                iterator(arguments[i], *paramTypeDenoters[i]);
        }
    }
}

void CallExpr::PushArgumentFront(const ExprPtr& expr)
{
    arguments.insert(arguments.begin(), expr);
}

bool CallExpr::PushPrefixToArguments()
{
    if (prefixExpr)
    {
        arguments.insert(arguments.begin(), std::move(prefixExpr));
        return true;
    }
    return false;
}

bool CallExpr::PopPrefixFromArguments()
{
    if (!prefixExpr && !arguments.empty())
    {
        prefixExpr = std::move(arguments.front());
        arguments.erase(arguments.begin());
        return true;
    }
    return false;
}

bool CallExpr::MergeArguments(std::size_t firstArgIndex, const MergeExprFunctor& mergeFunctor)
{
    if (firstArgIndex + 1 < arguments.size())
    {
        arguments[firstArgIndex] = mergeFunctor(arguments[firstArgIndex], arguments[firstArgIndex + 1]);
        arguments.erase(arguments.begin() + firstArgIndex + 1);
        return true;
    }
    return false;
}

Expr* CallExpr::GetMemberFuncObjectExpr() const
{
    if (prefixExpr)
        return prefixExpr.get();
    if (!arguments.empty())
        return arguments.front().get();
    return nullptr;
}


/* ----- BracketExpr ----- */

TypeDenoterPtr BracketExpr::DeriveTypeDenoter(const TypeDenoter* /*expectedTypeDenoter*/)
{
    return expr->GetTypeDenoter();
}

const Expr* BracketExpr::Find(const FindPredicateConstFunctor& predicate, unsigned int flags) const
{
    if (predicate)
    {
        /* Call predicate for this expression */
        CALL_EXPR_FIND_PREDICATE(predicate);

        /* Search in sub expression */
        if (auto e = expr->Find(predicate, flags))
            return e;
    }
    return nullptr;
}

const ObjectExpr* BracketExpr::FetchLValueExpr() const
{
    return expr->FetchLValueExpr();
}

IndexedSemantic BracketExpr::FetchSemantic() const
{
    return expr->FetchSemantic();
}


/* ----- AssignExpr ----- */

TypeDenoterPtr AssignExpr::DeriveTypeDenoter(const TypeDenoter* /*expectedTypeDenoter*/)
{
    return lvalueExpr->GetTypeDenoter();
}

const Expr* AssignExpr::Find(const FindPredicateConstFunctor& predicate, unsigned int flags) const
{
    if (predicate)
    {
        /* Call predicate for this expression */
        CALL_EXPR_FIND_PREDICATE(predicate);

        /* Search in sub expressions */
        if ((flags & SearchLValue) != 0)
        {
            if (auto e = lvalueExpr->Find(predicate, flags))
                return e;
        }
        if ((flags & SearchRValue) != 0)
        {
            if (auto e = rvalueExpr->Find(predicate, flags))
                return e;
        }
    }
    return nullptr;
}

const ObjectExpr* AssignExpr::FetchLValueExpr() const
{
    return lvalueExpr->FetchLValueExpr();
}


/* ----- ObjectExpr ----- */

TypeDenoterPtr ObjectExpr::DeriveTypeDenoter(const TypeDenoter* /*expectedTypeDenoter*/)
{
    if (symbolRef)
    {
        /* Derive type denoter from symbol reference */
        return symbolRef->GetTypeDenoter();
    }
    else if (prefixExpr)
    {
        /* Get type denoter of vector subscript, or report error if static access is used */
        if (isStatic)
            RuntimeErr(R_IllegalStaticAccessForSubscript(ident), this);
        else
            return prefixExpr->GetTypeDenoter()->GetSub(this);
    }

    RuntimeErr(R_UnknownTypeOfObjectIdentSymbolRef(ident), this);
}

const Expr* ObjectExpr::Find(const FindPredicateConstFunctor& predicate, unsigned int flags) const
{
    if (predicate)
    {
        /* Call predicate for this expression */
        CALL_EXPR_FIND_PREDICATE(predicate);

        /* Search in sub expression */
        if (prefixExpr)
        {
            if (auto e = prefixExpr->Find(predicate, flags))
                return e;
        }
    }
    return nullptr;
}

const ObjectExpr* ObjectExpr::FetchLValueExpr() const
{
    if (symbolRef)
    {
        /* Fetch l-value from symbol reference */
        switch (symbolRef->Type())
        {
            case AST::Types::VarDecl:
            case AST::Types::BufferDecl:
            case AST::Types::SamplerDecl:
                return this;
            case AST::Types::StructDecl:
                /*
                Fetch l-value from prefix if this is a base structure namespace expression,
                e.g. "obj.BaseStruct::member" -> "BaseStruct" is a base structure namespace.
                */
                if (prefixExpr)
                    return prefixExpr->FetchLValueExpr();
                else
                    return nullptr;
            default:
                return nullptr;
        }
    }
    else if (prefixExpr)
    {
        /* Fetch l-value from prefix expression */
        return prefixExpr->FetchLValueExpr();
    }
    return this;
}

IndexedSemantic ObjectExpr::FetchSemantic() const
{
    if (symbolRef)
    {
        /* Fetch semantic from variable declaration */
        if (auto varDecl = symbolRef->As<VarDecl>())
            return varDecl->semantic;
    }
    else if (prefixExpr)
    {
        /* Fetch semantic from prefix expression */
        return prefixExpr->FetchSemantic();
    }
    return Semantic::Undefined;
}

BaseTypeDenoterPtr ObjectExpr::GetTypeDenoterFromSubscript() const
{
    if (prefixExpr)
    {
        const auto& prefixTypeDen = prefixExpr->GetTypeDenoter()->GetAliased();
        if (auto baseTypeDen = prefixTypeDen.As<BaseTypeDenoter>())
        {
            try
            {
                /* Get vector type from subscript */
                auto vectorType = SubscriptDataType(baseTypeDen->dataType, ident);
                return std::make_shared<BaseTypeDenoter>(vectorType);
            }
            catch (const std::exception& e)
            {
                RuntimeErr(e.what(), this);
            }
        }
    }
    RuntimeErr(R_InvalidSubscriptBaseType, this);
}

std::string ObjectExpr::ToStringAsNamespace() const
{
    std::string s;

    if (prefixExpr)
    {
        if (auto subObjectExpr = prefixExpr->As<ObjectExpr>())
        {
            s += subObjectExpr->ToStringAsNamespace();
            s += "::";
        }
    }

    s += ident;

    return s;
}

void ObjectExpr::ReplaceSymbol(Decl* symbol)
{
    if (symbol)
    {
        symbolRef   = symbol;
        ident       = symbol->ident;
    }
}

VarDecl* ObjectExpr::FetchVarDecl() const
{
    return FetchSymbol<VarDecl>();
}


/* ----- ArrayExpr ----- */

TypeDenoterPtr ArrayExpr::DeriveTypeDenoter(const TypeDenoter* /*expectedTypeDenoter*/)
{
    try
    {
        return prefixExpr->GetTypeDenoter()->GetSub(this);
    }
    catch (const std::exception& e)
    {
        RuntimeErr(e.what(), this);
    }
}

const Expr* ArrayExpr::Find(const FindPredicateConstFunctor& predicate, unsigned int flags) const
{
    if (predicate)
    {
        /* Call predicate for this expression */
        CALL_EXPR_FIND_PREDICATE(predicate);

        /* Search in sub expression */
        if (auto e = prefixExpr->Find(predicate, flags))
            return e;
    }
    return nullptr;
}

const ObjectExpr* ArrayExpr::FetchLValueExpr() const
{
    return prefixExpr->FetchLValueExpr();
}

std::size_t ArrayExpr::NumIndices() const
{
    return arrayIndices.size();
}


/* ----- CastExpr ----- */

TypeDenoterPtr CastExpr::DeriveTypeDenoter(const TypeDenoter* /*expectedTypeDenoter*/)
{
    const auto& castTypeDen = typeSpecifier->GetTypeDenoter();
    const auto& valueTypeDen = expr->GetTypeDenoter();

    if (!valueTypeDen->IsCastableTo(*castTypeDen))
        RuntimeErr(R_IllegalCast(valueTypeDen->ToString(), castTypeDen->ToString(), R_CastExpr), this);

    return castTypeDen;
}

const Expr* CastExpr::Find(const FindPredicateConstFunctor& predicate, unsigned int flags) const
{
    if (predicate)
    {
        /* Call predicate for this expression */
        CALL_EXPR_FIND_PREDICATE(predicate);

        /* Search in sub expression */
        if (auto e = expr->Find(predicate, flags))
            return e;
    }
    return nullptr;
}


/* ----- InitializerExpr ----- */

/*
While this function derives the type of the initializer,
it also modifies its sub expressions to match the expected type.
*/
TypeDenoterPtr InitializerExpr::DeriveTypeDenoter(const TypeDenoter* expectedTypeDenoter)
{
    if (!expectedTypeDenoter)
        RuntimeErr(R_CantDeriveTypeOfInitializer, this);
    if (exprs.empty())
        RuntimeErr(R_CantDeriveTypeOfEmptyInitializer, this);

    /* Get number of 'unrolled' elements */
    const auto numElementsUnrolled = NumElementsUnrolled();

    /* Derive type for either base or structure type */
    const auto& typeDen = expectedTypeDenoter->GetAliased();
    if (auto baseTypeDen = typeDen.As<BaseTypeDenoter>())
    {
        /* Always unroll elements for base types */
        UnrollElements();

        const auto dataType = baseTypeDen->dataType;
        if (IsScalarType(dataType))
        {
            /* Check number of unrolled elements */
            if (numElementsUnrolled != 1)
                RuntimeErr(R_InvalidNumElementsInInitializer(expectedTypeDenoter->ToString(), std::size_t(1u), numElementsUnrolled), this);

            /* Find common type denoter for both sub expressions */
            const auto& expr0TypeDen = exprs[0]->GetTypeDenoter();
            if (!expr0TypeDen->IsCastableTo(*expectedTypeDenoter))
                RuntimeErr(R_IllegalCast(expr0TypeDen->ToString(), expectedTypeDenoter->ToString(), R_InitializerList), this);
        }
        else if (IsVectorType(dataType))
        {
            #if 0
            /* Compare number of elements with size of base type */
            const auto matrixDim        = MatrixTypeDim(baseTypeDen->dataType);
            const auto numTypeElements  = static_cast<std::size_t>(matrixDim.first * matrixDim.second);
            #endif

            //TODO: does not work for { 1, v3 } --> float4(1, v3)

            #if 0
            /* Unroll elements for base types */
            if (numElementsUnrolled == numTypeElements)
                UnrollElements();
            else
                RuntimeErr(R_InvalidNumElementsInInitializer(expectedTypeDenoter->ToString(), numTypeElements, numElementsUnrolled), this);
            #endif
        }
        else if (IsMatrixType(dataType))
        {
            //TODO...
        }
    }
    else if (auto structTypeDen = typeDen.As<StructTypeDenoter>())
    {
        //TODO...
    }
    else if (auto arrayTypeDen = typeDen.As<ArrayTypeDenoter>())
    {
        /* Derive type denoters from sub expressions with the array sub type as expected type denoter */
        for (auto& expr : exprs)
            expr->GetTypeDenoter(arrayTypeDen->subTypeDenoter.get());
    }

    /* Copy expected type denoter */
    return expectedTypeDenoter->Copy();
}

const Expr* InitializerExpr::Find(const FindPredicateConstFunctor& predicate, unsigned int flags) const
{
    if (predicate)
    {
        /* Call predicate for this expression */
        CALL_EXPR_FIND_PREDICATE(predicate);

        /* Search in sub expressions */
        if ((flags & SearchRValue) != 0)
        {
            for (const auto& subExpr : exprs)
            {
                if (auto e = subExpr->Find(predicate, flags))
                    return e;
            }
        }
    }
    return nullptr;
}

std::size_t InitializerExpr::NumElementsUnrolled() const
{
    std::size_t n = 0;

    for (const auto& e : exprs)
    {
        if (auto initSubExpr = e->FindFirstNotOf(AST::Types::BracketExpr)->As<InitializerExpr>())
            n += initSubExpr->NumElementsUnrolled();
        else
            ++n;
    }

    return n;
}

void InitializerExpr::CollectElements(std::vector<ExprPtr>& elements) const
{
    for (const auto& e : exprs)
    {
        if (auto initSubExpr = e->FindFirstNotOf(AST::Types::BracketExpr)->As<InitializerExpr>())
            initSubExpr->CollectElements(elements);
        else
            elements.push_back(e);
    }
}

void InitializerExpr::UnrollElements()
{
    /* Check if unrolling is necessary */
    for (const auto& e : exprs)
    {
        if (e->Type() == AST::Types::InitializerExpr)
        {
            /* Collect all elements and then replace the sub expression by the new list */
            std::vector<ExprPtr> elements;
            CollectElements(elements);
            exprs = elements;
            break;
        }
    }
}

static ExprPtr FetchSubExprFromInitializerExpr(const InitializerExpr* ast, const std::vector<int>& arrayIndices, std::size_t layer)
{
    if (layer < arrayIndices.size())
    {
        /* Get sub expression by array index */
        auto idx = arrayIndices[layer];
        if (idx >= 0 && static_cast<std::size_t>(idx) < ast->exprs.size())
        {
            auto expr = ast->exprs[idx];

            if (layer + 1 == arrayIndices.size())
            {
                /* Return final sub expression */
                return expr;
            }

            /* Continue search in next initializer expression */
            if (auto subInitExpr = expr->As<const InitializerExpr>())
                return FetchSubExprFromInitializerExpr(subInitExpr, arrayIndices, layer + 1);

            RuntimeErr(R_ExpectedInitializerForArrayAccess, expr.get());
        }
        RuntimeErr(R_NotEnoughElementsInInitializer, ast);
    }
    RuntimeErr(R_NotEnoughIndicesForInitializer, ast);
}

ExprPtr InitializerExpr::FetchSubExpr(const std::vector<int>& arrayIndices) const
{
    return FetchSubExprFromInitializerExpr(this, arrayIndices, 0);
}

static bool NextArrayIndicesFromInitializerExpr(const InitializerExpr* ast, std::vector<int>& arrayIndices, std::size_t layer)
{
    if (layer < arrayIndices.size())
    {
        auto& idx = arrayIndices[layer];
        if (idx >= 0 && static_cast<std::size_t>(idx) < ast->exprs.size())
        {
            auto expr = ast->exprs[idx];

            if (auto subInitExpr = expr->As<const InitializerExpr>())
            {
                if (NextArrayIndicesFromInitializerExpr(subInitExpr, arrayIndices, layer + 1))
                    return true;
            }

            /* Increment index */
            ++idx;

            if (static_cast<std::size_t>(idx) == ast->exprs.size())
            {
                idx = 0;
                return false;
            }

            return true;
        }
        else
        {
            /* Reset index */
            idx = 0;
        }
    }
    return false;
}

bool InitializerExpr::NextArrayIndices(std::vector<int>& arrayIndices) const
{
    return NextArrayIndicesFromInitializerExpr(this, arrayIndices, 0);
}


} // /namespace Xsc



// ================================================================================
