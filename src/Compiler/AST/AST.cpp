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
#include <algorithm>

#ifdef XSC_ENABLE_MEMORY_POOL
#include "MemoryPool.h"
#endif


namespace Xsc
{


/* ----- AST ----- */

AST::~AST()
{
    // dummy
}

#ifdef XSC_ENABLE_MEMORY_POOL

void* AST::operator new (std::size_t count)
{
    return MemoryPool::Instance().Alloc(count);
}

void AST::operator delete (void* ptr)
{
    MemoryPool::Instance().Free(ptr);
}

#endif


/* ----- Stmnt ----- */

void Stmnt::CollectDeclIdents(std::map<const AST*, std::string>& declASTIdents) const
{
    // dummy
}


/* ----- TypedAST ----- */

const TypeDenoterPtr& TypedAST::GetTypeDenoter()
{
    if (!bufferedTypeDenoter_)
        bufferedTypeDenoter_ = DeriveTypeDenoter();
    return bufferedTypeDenoter_;
}

void TypedAST::ResetTypeDenoter()
{
    bufferedTypeDenoter_.reset();
}


/* ----- Expr ----- */

VarDecl* Expr::FetchVarDecl() const
{
    if (auto varIdent = FetchVarIdent())
        return varIdent->FetchVarDecl();
    else
        return nullptr;
}

VarIdent* Expr::FetchVarIdent() const
{
    return nullptr;
}


/* ----- Decl ----- */

std::string Decl::ToString() const
{
    return ident.Original();
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
            auto typeDen = arg->GetTypeDenoter()->Get();
            if (auto baseTypeDen = typeDen->As<BaseTypeDenoter>())
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


/* ----- VarIdent ----- */

std::string VarIdent::ToString() const
{
    std::string name;
    auto ast = this;
    while (true)
    {
        name += ast->ident;
        if (ast->next)
        {
            ast = ast->next.get();
            name += ".";
        }
        else
            break;
    }
    return name;
}

VarIdent* VarIdent::Last()
{
    return (next ? next->Last() : this);
}

TypeDenoterPtr VarIdent::DeriveTypeDenoter()
{
    return GetExplicitTypeDenoter(true);
}

TypeDenoterPtr VarIdent::GetExplicitTypeDenoter(bool recursive)
{
    if (symbolRef)
    {
        /* Derive type denoter from symbol reference */
        switch (symbolRef->Type())
        {
            case AST::Types::FunctionDecl:
            {
                RuntimeErr("illegal type denoter of function object '" + ident + "'", this);
            }
            break;

            case AST::Types::VarDecl:
            {
                auto varDecl = static_cast<VarDecl*>(symbolRef);
                try
                {
                    return varDecl->GetTypeDenoter()->GetFromArray(arrayIndices.size(), (recursive ? next.get() : nullptr));
                }
                catch (const std::exception& e)
                {
                    RuntimeErr(e.what(), this);
                }
            }
            break;

            case AST::Types::BufferDecl:
            {
                auto bufferDecl = static_cast<BufferDecl*>(symbolRef);
                try
                {
                    return bufferDecl->GetTypeDenoter()->GetFromArray(arrayIndices.size(), (recursive ? next.get() : nullptr));
                }
                catch (const std::exception& e)
                {
                    RuntimeErr(e.what(), this);
                }
            }
            break;

            case AST::Types::SamplerDecl:
            {
                auto samplerDecl = static_cast<SamplerDecl*>(symbolRef);
                try
                {
                    return samplerDecl->GetTypeDenoter()->GetFromArray(arrayIndices.size(), (recursive ? next.get() : nullptr));
                }
                catch (const std::exception& e)
                {
                    RuntimeErr(e.what(), this);
                }
            }
            break;

            case AST::Types::StructDecl:
            {
                auto structDecl = static_cast<StructDecl*>(symbolRef);
                if (next)
                    RuntimeErr("can not directly access members of '" + structDecl->ToString() + "'", next.get());
                if (!arrayIndices.empty())
                    RuntimeErr("can not directly acces array of '" + structDecl->ToString() + "'", this);
                return structDecl->GetTypeDenoter()->Get();
            }
            break;

            case AST::Types::AliasDecl:
            {
                auto aliasDecl = static_cast<AliasDecl*>(symbolRef);
                if (next)
                    RuntimeErr("can not directly access members of '" + aliasDecl->ToString() + "'", next.get());
                if (!arrayIndices.empty())
                    RuntimeErr("can not directly access array of '" + aliasDecl->ToString() + "'", this);
                return aliasDecl->GetTypeDenoter()->Get();
            }
            break;
            
            default:
            {
                RuntimeErr("unknown type of symbol reference to derive type denoter of variable identifier '" + ident + "'", this);
            }
            break;
        }
    }
    RuntimeErr("missing symbol reference to derive type denoter of variable identifier '" + ident + "'", this);
}

BaseTypeDenoterPtr VarIdent::GetTypeDenoterFromSubscript(TypeDenoter& baseTypeDenoter) const
{
    if (auto baseTypeDen = baseTypeDenoter.As<BaseTypeDenoter>())
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
    RuntimeErr("invalid base type denoter for vector subscript", this);
}

void VarIdent::PopFront(bool accumulateArrayIndices)
{
    if (next)
    {
        auto nextVarIdent = next;
        
        ident       = nextVarIdent->ident;
        next        = nextVarIdent->next;
        symbolRef   = nextVarIdent->symbolRef;

        if (accumulateArrayIndices)
            arrayIndices.insert(arrayIndices.end(), nextVarIdent->arrayIndices.begin(), nextVarIdent->arrayIndices.end());
        else
            arrayIndices = nextVarIdent->arrayIndices;
    }
}

IndexedSemantic VarIdent::FetchSemantic() const
{
    if (auto varDecl = FetchVarDecl())
        return varDecl->semantic;
    else
        return Semantic::Undefined;
}

Decl* VarIdent::FetchDecl() const
{
    if (symbolRef)
    {
        const auto t = symbolRef->Type();
        if (t == AST::Types::VarDecl || t == AST::Types::StructDecl || t == AST::Types::BufferDecl || t == AST::Types::SamplerDecl)
            return static_cast<Decl*>(symbolRef);
    }
    return nullptr;
}

VarDecl* VarIdent::FetchVarDecl() const
{
    return FetchSymbol<VarDecl>();
}

FunctionDecl* VarIdent::FetchFunctionDecl() const
{
    return FetchSymbol<FunctionDecl>();
}


/* ----- FunctionCall ----- */

TypeDenoterPtr FunctionCall::DeriveTypeDenoter()
{
    if (funcDeclRef)
    {
        /* Return type denoter of associated function declaration */
        return funcDeclRef->returnType->typeDenoter;
    }
    else if (typeDenoter)
    {
        /* Return type denoter fo type constructor */
        return typeDenoter;
    }
    else if (intrinsic != Intrinsic::Undefined)
    {
        /* Return type denoter of associated intrinsic */
        try
        {
            return IntrinsicAdept::Get().GetIntrinsicReturnType(intrinsic, arguments);
        }
        catch (const std::exception& e)
        {
            RuntimeErr(e.what(), this);
        }
    }
    else
        RuntimeErr("missing function reference to derive expression type", this);
}

std::vector<Expr*> FunctionCall::GetArguments() const
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

FunctionDecl* FunctionCall::GetFunctionImpl() const
{
    if (auto funcDecl = funcDeclRef)
    {
        if (funcDecl->funcImplRef)
            return funcDecl->funcImplRef;
        else
            return funcDecl;
    }
    return nullptr;
}

void FunctionCall::ForEachOutputArgument(const ExprIteratorFunctor& iterator)
{
    if (iterator)
    {
        if (funcDeclRef)
        {
            /* Get output parameters from associated function declaration */
            const auto& parameters = funcDeclRef->parameters;
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

void FunctionCall::ForEachArgumentWithParameterType(const ArgumentParameterTypeFunctor& iterator)
{
    if (iterator)
    {
        if (funcDeclRef)
        {
            /* Get parameter type denoters from associated function declaration */
            const auto& parameters = funcDeclRef->parameters;
            for (std::size_t i = 0, n = std::min(arguments.size(), parameters.size()); i < n; ++i)
            {
                auto param = parameters[i]->varDecls.front();
                auto paramTypeDen = param->GetTypeDenoter()->Get();
                iterator(arguments[i], *paramTypeDen);
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
        s += "<undefined>";
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

TypeDenoterPtr ArrayDimension::DeriveTypeDenoter()
{
    return expr->GetTypeDenoter();
}

bool ArrayDimension::HasDynamicSize() const
{
    return (size == 0);
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
    return typeDenoter->ToString();
}

TypeDenoterPtr TypeSpecifier::DeriveTypeDenoter()
{
    return typeDenoter;
}

StructDecl* TypeSpecifier::GetStructDeclRef()
{
    auto typeDen = typeDenoter->Get();
    if (auto structTypeDen = typeDen->As<StructTypeDenoter>())
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

bool TypeSpecifier::HasAnyTypeModifierOf(const std::vector<TypeModifier>& modifiers) const
{
    for (auto mod : modifiers)
    {
        if (typeModifiers.find(mod) != typeModifiers.end())
            return true;
    }
    return false;
}


/* ----- VarDecl ----- */

std::string VarDecl::ToString() const
{
    std::string s;

    s += ident.Original();

    for (std::size_t i = 0; i < arrayDims.size(); ++i)
        s += "[]";

    if (semantic != Semantic::Undefined)
    {
        s += " : ";
        s += semantic.ToString();
    }

    if (initializer)
    {
        s += " = ";
        //TODO: see above
        s += "???";
        //s += initializer->ToString();
    }

    return s;
}

TypeDenoterPtr VarDecl::DeriveTypeDenoter()
{
    if (declStmntRef)
    {
        /* Get base type denoter from declaration statement */
        return declStmntRef->typeSpecifier->typeDenoter->AsArray(arrayDims);
    }
    RuntimeErr("missing reference to declaration statement to derive type denoter of variable identifier '" + ident + "'", this);
}


/* ----- BufferDecl ----- */

TypeDenoterPtr BufferDecl::DeriveTypeDenoter()
{
    return std::make_shared<BufferTypeDenoter>(this)->AsArray(arrayDims);
}

BufferType BufferDecl::GetBufferType() const
{
    return (declStmntRef ? declStmntRef->typeDenoter->bufferType : BufferType::Undefined);
}


/* ----- SamplerDecl ----- */

TypeDenoterPtr SamplerDecl::DeriveTypeDenoter()
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
    return ("struct " + (IsAnonymous() ? "<anonymous>" : ident.Original()));
}

bool StructDecl::IsAnonymous() const
{
    return ident.Empty();
}

VarDecl* StructDecl::Fetch(const std::string& ident, const StructDecl** owner) const
{
    /* Fetch symbol from base struct first */
    if (baseStructRef)
    {
        if (auto symbol = baseStructRef->Fetch(ident, owner))
            return symbol;
    }

    /* Now fetch symbol from members */
    for (const auto& varDeclStmnt : varMembers)
    {
        if (auto symbol = varDeclStmnt->Fetch(ident))
        {
            if (owner)
                *owner = this;
            return symbol;
        }
    }

    return nullptr;
}

FunctionDecl* StructDecl::FetchFunctionDecl(const std::string& ident, const std::vector<TypeDenoterPtr>& argTypeDenoters, const StructDecl** owner) const
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

    return FunctionDecl::FetchFunctionDeclFromList(funcDeclList, ident, argTypeDenoters, false);
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

TypeDenoterPtr StructDecl::DeriveTypeDenoter()
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

std::size_t StructDecl::NumVarMembers() const
{
    std::size_t n = 0;

    if (baseStructRef)
        n += baseStructRef->NumVarMembers();

    for (const auto& member : varMembers)
        n += member->varDecls.size();

    return n;
}

void StructDecl::CollectMemberTypeDenoters(std::vector<TypeDenoterPtr>& memberTypeDens) const
{
    /* First collect type denoters from base structure */
    if (baseStructRef)
        baseStructRef->CollectMemberTypeDenoters(memberTypeDens);

    /* Collect type denoters from this structure */
    for (const auto& member : varMembers)
    {
        /* Add type denoter N times (where N is the number variable declaration within the member statement) */
        for (std::size_t i = 0; i < member->varDecls.size(); ++i)
            memberTypeDens.push_back(member->typeSpecifier->typeDenoter);
    }
}

void StructDecl::ForEachVarDecl(const VarDeclIteratorFunctor& iterator)
{
    /* Iterate over all base struct members */
    if (baseStructRef)
        baseStructRef->ForEachVarDecl(iterator);

    for (auto& member : varMembers)
    {
        /* Iterate over all sub-struct members */
        auto typeDen = member->typeSpecifier->GetTypeDenoter()->Get();
        if (auto structTypeDen = typeDen->As<StructTypeDenoter>())
        {
            if (structTypeDen->structDeclRef)
                structTypeDen->structDeclRef->ForEachVarDecl(iterator);
        }

        /* Iterate over all variables for current member */
        member->ForEachVarDecl(iterator);
    }
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

bool StructDecl::IsBaseOf(const StructDecl& subStructDecl) const
{
    if (subStructDecl.baseStructRef)
    {
        if (subStructDecl.baseStructRef == this)
            return true;
        else
            return IsBaseOf(*subStructDecl.baseStructRef);
    }
    return false;
}


/* ----- AliasDecl ----- */

TypeDenoterPtr AliasDecl::DeriveTypeDenoter()
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
    s += '(';

    /* Append parameter types */
    for (std::size_t i = 0; i < parameters.size(); ++i)
    {
        s += parameters[i]->ToString(useParamNames, true);
        if (i + 1 < parameters.size())
            s += ", ";
    }

    s += ')';

    return s;
}

bool FunctionDecl::EqualsSignature(const FunctionDecl& rhs) const
{
    /* Compare parameter count */
    if (parameters.size() != rhs.parameters.size())
        return false;

    /* Compare parameter type denoters */
    for (std::size_t i = 0; i < parameters.size(); ++i)
    {
        auto lhsTypeDen = parameters[i]->typeSpecifier->typeDenoter.get();
        auto rhsTypeDen = rhs.parameters[i]->typeSpecifier->typeDenoter.get();

        if (!lhsTypeDen->Equals(*rhsTypeDen))
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
    if (paramIndex >= parameters.size())
        return false;

    /* Get type denoters to compare */
    auto paramTypeDen = parameters[paramIndex]->typeSpecifier->typeDenoter.get();

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

static bool ValidateNumArgsForFunctionDecl(const std::vector<FunctionDecl*>& funcDeclList, std::size_t numArgs)
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
    ReportHandler::HintForNextReport("candidates are:");
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
            RuntimeErr("undefined symbol '" + ident + "'");
        else
            return nullptr;
    }

    /* Validate number of arguments for function call */
    const auto numArgs = argTypeDenoters.size();

    if (!ValidateNumArgsForFunctionDecl(funcDeclList, numArgs))
    {
        if (throwErrorIfNoMatch)
        {
            /* Add candidate signatures to report hints */
            ListAllFuncCandidates(funcDeclList);

            /* Throw runtime error */
            RuntimeErr(
                "function '" + ident + "' does not take " + std::to_string(numArgs) + " " +
                std::string(numArgs == 1 ? "parameter" : "parameters")
            );
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
        RuntimeErr("ambiguous function call '" + ident + "(" + argTypeNames + ")'");
    }

    return funcDeclCandidates.front();
}


/* ----- UniformBufferDecl ----- */

std::string UniformBufferDecl::ToString() const
{
    std::string s;

    switch (bufferType)
    {
        case UniformBufferType::Undefined:
            s = "<undefined buffer> ";
            break;
        case UniformBufferType::ConstantBuffer:
            s = "cbuffer ";
            break;
        case UniformBufferType::TextureBuffer:
            s = "tbuffer ";
            break;
    }

    s += ident;

    return s;
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

std::string VarDeclStmnt::ToString(bool useVarNames, bool isParam) const
{
    auto s = typeSpecifier->ToString();
    
    if (useVarNames)
    {
        for (std::size_t i = 0; i < varDecls.size(); ++i)
        {
            s += ' ';
            s += varDecls[i]->ToString();
            if (i + 1 < varDecls.size())
                s += ',';
        }
    }

    if (isParam && !varDecls.empty() && varDecls.front()->initializer)
        return '[' + s + ']';

    return s;
}

VarDecl* VarDeclStmnt::Fetch(const std::string& ident) const
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

bool VarDeclStmnt::HasAnyTypeModifierOf(const std::vector<TypeModifier>& modifiers) const
{
    return typeSpecifier->HasAnyTypeModifierOf(modifiers);
}

void VarDeclStmnt::ForEachVarDecl(const VarDeclIteratorFunctor& iterator)
{
    for (auto& varDecl : varDecls)
        iterator(varDecl);
}

void VarDeclStmnt::MakeImplicitConst()
{
    if (!IsConstOrUniform() && typeSpecifier->storageClasses.find(StorageClass::Static) == typeSpecifier->storageClasses.end())
    {
        flags << VarDeclStmnt::isImplicitConst;
        typeSpecifier->isUniform = true;
    }
}


/* ----- NullExpr ----- */

TypeDenoterPtr NullExpr::DeriveTypeDenoter()
{
    /*
    Return 'int' as type, because null expressions are only
    used as dynamic array dimensions (which must be integral types)
    */
    return std::make_shared<BaseTypeDenoter>(DataType::Int);
}


/* ----- ListExpr ----- */

TypeDenoterPtr ListExpr::DeriveTypeDenoter()
{
    /* Only return type denoter of first sub expression */
    return firstExpr->GetTypeDenoter();
}


/* ----- LiteralExpr ----- */

TypeDenoterPtr LiteralExpr::DeriveTypeDenoter()
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
            case DataType::Float:
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


/* ----- TypeSpecifierExpr ----- */

TypeDenoterPtr TypeSpecifierExpr::DeriveTypeDenoter()
{
    return typeSpecifier->GetTypeDenoter();
}


/* ----- TernaryExpr ----- */

TypeDenoterPtr TernaryExpr::DeriveTypeDenoter()
{
    /* Check if conditional expression is compatible to a boolean */
    const auto& condTypeDen = condExpr->GetTypeDenoter();
    const BaseTypeDenoter boolTypeDen(DataType::Bool);

    if (!condTypeDen->IsCastableTo(boolTypeDen))
    {
        RuntimeErr(
            "can not cast '" + condTypeDen->ToString() + "' to '" +
            boolTypeDen.ToString() + "' in condition of ternary expression", condExpr.get()
        );
    }

    /* Find common type denoter for both sub expressions */
    const auto& thenTypeDen = thenExpr->GetTypeDenoter();
    const auto& elseTypeDen = elseExpr->GetTypeDenoter();

    if (!elseTypeDen->IsCastableTo(*thenTypeDen))
    {
        RuntimeErr(
            "can not cast '" + elseTypeDen->ToString() + "' to '" +
            thenTypeDen->ToString() + "' in ternary expression", this
        );
    }

    auto commonTypeDen = TypeDenoter::FindCommonTypeDenoter(thenTypeDen, elseTypeDen);

    /* Get common boolean type denoter from condition expression */
    auto condTypeDenAliased = condExpr->GetTypeDenoter()->Get();

    if (auto baseTypeDen = condTypeDenAliased->As<BaseTypeDenoter>())
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

bool TernaryExpr::IsVectorCondition() const
{
    auto condTypeDen = condExpr->GetTypeDenoter()->Get();

    if (auto baseTypeDen = condTypeDen->As<BaseTypeDenoter>())
    {
        /* Is the condition a boolean vector type? */
        const auto condVecSize = VectorTypeDim(baseTypeDen->dataType);
        return (condVecSize > 1);
    }

    return false;
}


/* ----- BinaryExpr ----- */

TypeDenoterPtr BinaryExpr::DeriveTypeDenoter()
{
    /* Return type of left-hand-side sub expresion if the types are compatible */
    const auto& lhsTypeDen = lhsExpr->GetTypeDenoter();
    const auto& rhsTypeDen = rhsExpr->GetTypeDenoter();

    if (!rhsTypeDen->IsCastableTo(*lhsTypeDen) || !lhsTypeDen->IsCastableTo(*rhsTypeDen))
    {
        RuntimeErr(
            "can not cast '" + rhsTypeDen->ToString() + "' to '" + lhsTypeDen->ToString() +
            "' in binary expression '" + BinaryOpToString(op) + "'", this
        );
    }

    /* Find common type denoter of left and right sub expressions */
    auto commonTypeDen = TypeDenoter::FindCommonTypeDenoter(lhsTypeDen, rhsTypeDen);

    if (IsBooleanOp(op))
    {
        if (auto baseTypeDen = commonTypeDen->As<BaseTypeDenoter>())
        {
            auto vecBoolType = VectorDataType(DataType::Bool, VectorTypeDim(baseTypeDen->dataType));
            return std::make_shared<BaseTypeDenoter>(vecBoolType);
        }
        else
            return std::make_shared<BaseTypeDenoter>(DataType::Bool);
    }
    else
        return commonTypeDen;
}


/* ----- UnaryExpr ----- */

TypeDenoterPtr UnaryExpr::DeriveTypeDenoter()
{
    const auto& typeDen = expr->GetTypeDenoter();

    if (IsLogicalOp(op))
        return std::make_shared<BaseTypeDenoter>(DataType::Bool);
    else
        return typeDen;
}


/* ----- PostUnaryExpr ----- */

TypeDenoterPtr PostUnaryExpr::DeriveTypeDenoter()
{
    return expr->GetTypeDenoter();
}


/* ----- FunctionCallExpr ----- */

TypeDenoterPtr FunctionCallExpr::DeriveTypeDenoter()
{
    return call->GetTypeDenoter();
}


/* ----- BracketExpr ----- */

TypeDenoterPtr BracketExpr::DeriveTypeDenoter()
{
    return expr->GetTypeDenoter();
}

VarIdent* BracketExpr::FetchVarIdent() const
{
    return expr->FetchVarIdent();
}


/* ----- SuffixExpr ----- */

TypeDenoterPtr SuffixExpr::DeriveTypeDenoter()
{
    return expr->GetTypeDenoter()->Get(varIdent.get());
}


/* ----- ArrayAccessExpr ----- */

TypeDenoterPtr ArrayAccessExpr::DeriveTypeDenoter()
{
    try
    {
        return expr->GetTypeDenoter()->GetFromArray(arrayIndices.size());
    }
    catch (const std::exception& e)
    {
        RuntimeErr(e.what(), this);
    }
}


/* ----- CastExpr ----- */

TypeDenoterPtr CastExpr::DeriveTypeDenoter()
{
    const auto& castTypeDen = typeSpecifier->GetTypeDenoter();
    const auto& valueTypeDen = expr->GetTypeDenoter();

    if (!valueTypeDen->IsCastableTo(*castTypeDen))
    {
        RuntimeErr(
            "can not cast '" + valueTypeDen->ToString() + "' to '" +
            castTypeDen->ToString() + "' in cast expression", this
        );
    }

    return castTypeDen;
}


/* ----- VarAccessExpr ----- */

TypeDenoterPtr VarAccessExpr::DeriveTypeDenoter()
{
    return varIdent->GetTypeDenoter();
}

VarIdent* VarAccessExpr::FetchVarIdent() const
{
    return varIdent.get();
}


/* ----- InitializerExpr ----- */

/*
This function derives the type denoter of the initializer by getting the type denoter of each sub-expression.
If a sub-expression is again an array type denoter, its array dimensions are inserted into the
final return type denoter (see 'ArrayTypeDenoter::InsertSubArray' function)
*/
TypeDenoterPtr InitializerExpr::DeriveTypeDenoter()
{
    if (exprs.empty())
        RuntimeErr("can not derive type of initializer list with no elements", this);

    /* Start with a 1-dimension array type */
    auto finalTypeDen = std::make_shared<ArrayTypeDenoter>();
    finalTypeDen->arrayDims.push_back(ASTFactory::MakeArrayDimension(static_cast<int>(exprs.size())));

    TypeDenoterPtr elementsTypeDen;

    for (const auto& expr : exprs)
    {
        TypeDenoterPtr subTypeDen;

        /* Get elements type denoter from sub expression */
        subTypeDen = expr->GetTypeDenoter();

        if (elementsTypeDen)
        {
            /* Check compatability with current output type dentoer */
            if (auto elementsArrayTypeDen = elementsTypeDen->As<ArrayTypeDenoter>())
            {
                /* Array types must have the same size */
                if (auto arraySubTypeDen = subTypeDen->As<ArrayTypeDenoter>())
                {
                    const auto& lhsDims = elementsArrayTypeDen->arrayDims;
                    const auto& rhsDims = arraySubTypeDen->arrayDims;

                    const auto lhsNumElements = lhsDims.size();
                    const auto rhsNumElements = rhsDims.size();

                    if (lhsNumElements != rhsNumElements)
                    {
                        RuntimeErr(
                            "array dimensions mismatch in initializer expression (expected " +
                            std::to_string(lhsNumElements) + " dimension(s), but got " + std::to_string(rhsNumElements) + ")", expr.get()
                        );
                    }

                    /* Array dimensions must have the same size */
                    for (std::size_t i = 0; i < lhsNumElements; ++i)
                    {
                        const auto lhsSize = lhsDims[i]->size;
                        const auto rhsSize = rhsDims[i]->size;

                        if (lhsSize != rhsSize)
                        {
                            RuntimeErr(
                                "array dimension size mismatch in initializer expression (expected " +
                                std::to_string(lhsSize) + " element(s), but got " + std::to_string(rhsSize) + ")", expr.get()
                            );
                        }
                    }
                }
                else
                {
                    RuntimeErr(
                        "type mismatch in initializer expression (expected array '" + elementsTypeDen->ToString() +
                        "', but got '" + subTypeDen->ToString() + "')", expr.get()
                    );
                }
            }
            else if (!subTypeDen->IsCastableTo(*elementsTypeDen))
            {
                RuntimeErr(
                    "can not cast '" + subTypeDen->ToString() + "' to '" +
                    elementsTypeDen->ToString() + "' in initializer expression", expr.get()
                );
            }
        }
        else
        {
            /* Insert sub array type with array dimensions and base type */
            if (auto arraySubTypeDen = subTypeDen->As<const ArrayTypeDenoter>())
                finalTypeDen->InsertSubArray(*arraySubTypeDen);
            else
                finalTypeDen->baseTypeDenoter = subTypeDen;

            /* Set first output type denoter */
            elementsTypeDen = subTypeDen;
        }
    }

    return finalTypeDen;
}

unsigned int InitializerExpr::NumElements() const
{
    unsigned int n = 0;
    
    for (const auto& e : exprs)
    {
        if (e->Type() == AST::Types::InitializerExpr)
            n += static_cast<const InitializerExpr&>(*e).NumElements();
        else
            ++n;
    }

    return n;
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

            RuntimeErr("initializer expression expected for array access", expr.get());
        }
        RuntimeErr("not enough elements in initializer expression", ast);
    }
    RuntimeErr("not enough array indices specified for initializer expression", ast);
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
