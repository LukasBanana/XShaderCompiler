/*
 * AST.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "AST.h"
#include "ASTFactory.h"
#include "Exception.h"
#include "HLSLIntrinsics.h"
#include "Variant.h"
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


/* ----- TypedAST ----- */

const TypeDenoterPtr& TypedAST::GetTypeDenoter()
{
    if (!bufferedTypeDenoter_)
        bufferedTypeDenoter_ = DeriveTypeDenoter();
    return bufferedTypeDenoter_;
}

void TypedAST::ResetBufferedTypeDenoter()
{
    bufferedTypeDenoter_.reset();
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

VarIdent* VarIdent::LastVarIdent()
{
    return (next ? next->LastVarIdent() : this);
}

VarIdent* VarIdent::FirstConstVarIdent()
{
    if (symbolRef)
    {
        if (auto varDecl = symbolRef->As<VarDecl>())
        {
            if (varDecl->declStmntRef->IsConst())
                return this;
            if (next)
                return next->FirstConstVarIdent();
        }
    }
    return nullptr;
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
                    RuntimeErr("can not directly access members of 'struct " + structDecl->SignatureToString() + "'", next.get());
                if (!arrayIndices.empty())
                    RuntimeErr("can not directly acces array of 'struct " + structDecl->SignatureToString() + "'", this);
                return structDecl->GetTypeDenoter()->Get();
            }
            break;

            case AST::Types::AliasDecl:
            {
                auto aliasDecl = static_cast<AliasDecl*>(symbolRef);
                if (next)
                    RuntimeErr("can not directly access members of '" + aliasDecl->ident + "'", next.get());
                if (!arrayIndices.empty())
                    RuntimeErr("can not directly access array of '" + aliasDecl->ident + "'", this);
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

void VarIdent::PopFront()
{
    if (next)
    {
        auto nextVarIdent = next;
        ident           = nextVarIdent->ident;
        arrayIndices    = nextVarIdent->arrayIndices;
        next            = nextVarIdent->next;
        symbolRef       = nextVarIdent->symbolRef;
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


/* ----- TypeName ----- */

std::string TypeName::ToString() const
{
    return typeDenoter->ToString();
}

TypeDenoterPtr TypeName::DeriveTypeDenoter()
{
    return typeDenoter;
}


/* ----- VarDecl ----- */

std::string VarDecl::ToString() const
{
    std::string s;

    s += ident;

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

const std::string& VarDecl::FinalIdent() const
{
    return (renamedIdent.empty() ? ident : renamedIdent);
}

TypeDenoterPtr VarDecl::DeriveTypeDenoter()
{
    if (declStmntRef)
    {
        /* Get base type denoter from declaration statement */
        return declStmntRef->varType->typeDenoter->AsArray(arrayDims);
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

std::string StructDecl::SignatureToString() const
{
    return ("struct " + (IsAnonymous() ? "<anonymous>" : ident));
}

bool StructDecl::IsAnonymous() const
{
    return ident.empty();
}

VarDecl* StructDecl::Fetch(const std::string& ident) const
{
    /* Fetch symbol from base struct first */
    if (baseStructRef)
    {
        auto varDecl = baseStructRef->Fetch(ident);
        if (varDecl)
            return varDecl;
    }

    /* Now fetch symbol from members */
    for (const auto& varDeclStmnt : members)
    {
        auto symbol = varDeclStmnt->Fetch(ident);
        if (symbol)
            return symbol;
    }

    return nullptr;
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
    for (const auto& member : members)
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

std::size_t StructDecl::NumMembers() const
{
    std::size_t n = 0;

    if (baseStructRef)
        n += baseStructRef->NumMembers();

    for (const auto& member : members)
        n += member->varDecls.size();

    return n;
}

void StructDecl::CollectMemberTypeDenoters(std::vector<TypeDenoterPtr>& memberTypeDens) const
{
    /* First collect type denoters from base structure */
    if (baseStructRef)
        baseStructRef->CollectMemberTypeDenoters(memberTypeDens);

    /* Collect type denoters from this structure */
    for (const auto& member : members)
    {
        /* Add type denoter N times (where N is the number variable declaration within the member statement) */
        for (std::size_t i = 0; i < member->varDecls.size(); ++i)
            memberTypeDens.push_back(member->varType->typeDenoter);
    }
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

bool FunctionDecl::IsForwardDecl() const
{
    return (codeBlock == nullptr);
}

bool FunctionDecl::HasVoidReturnType() const
{
    return returnType->typeDenoter->IsVoid();
}

std::string FunctionDecl::SignatureToString(bool useParamNames) const
{
    std::string s;

    s += returnType->ToString();
    s += ' ';
    s += ident;
    s += '(';

    for (std::size_t i = 0; i < parameters.size(); ++i)
    {
        s += parameters[i]->ToString(useParamNames);
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
        auto lhsTypeDen = parameters[i]->varType->typeDenoter.get();
        auto rhsTypeDen = rhs.parameters[i]->varType->typeDenoter.get();

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

bool FunctionDecl::MatchParameterWithTypeDenoter(std::size_t paramIndex, const TypeDenoter& argType, bool implicitConversion) const
{
    if (paramIndex >= parameters.size())
        return false;

    /* Get type denoters to compare */
    auto paramTypeDen = parameters[paramIndex]->varType->typeDenoter.get();

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

const std::string& FunctionDecl::FinalIdent() const
{
    return (renamedIdent.empty() ? ident : renamedIdent);
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


/* ----- VarDelcStmnt ----- */

std::string VarDeclStmnt::ToString(bool useVarNames) const
{
    auto s = varType->ToString();
    
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

    return s;
}

VarDecl* VarDeclStmnt::Fetch(const std::string& ident) const
{
    for (const auto& var : varDecls)
    {
        if (var->ident == ident)
            return var.get();
    }
    return nullptr;
}

bool VarDeclStmnt::IsInput() const
{
    return (isInput || !isOutput);
}

bool VarDeclStmnt::IsOutput() const
{
    return isOutput;
}

bool VarDeclStmnt::IsConst() const
{
    return (isUniform || (typeModifiers.find(TypeModifier::Const) != typeModifiers.end()));
}

bool VarDeclStmnt::HasAnyTypeModifierOf(const std::vector<TypeModifier>& modifiers) const
{
    for (auto mod : modifiers)
    {
        if (typeModifiers.find(mod) != typeModifiers.end())
            return true;
    }
    return false;
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
        ResetBufferedTypeDenoter();
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


/* ----- TypeNameExpr ----- */

TypeDenoterPtr TypeNameExpr::DeriveTypeDenoter()
{
    return typeName->GetTypeDenoter();
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

    /* Return type of 'then'-branch sub expresion if the types are compatible */
    const auto& thenTypeDen = thenExpr->GetTypeDenoter();
    const auto& elseTypeDen = elseExpr->GetTypeDenoter();

    if (!elseTypeDen->IsCastableTo(*thenTypeDen))
    {
        RuntimeErr(
            "can not cast '" + elseTypeDen->ToString() + "' to '" +
            thenTypeDen->ToString() + "' in ternary expression", this
        );
    }

    return thenTypeDen;
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

    if (IsBooleanOp(op))
        return std::make_shared<BaseTypeDenoter>(DataType::Bool);
    else
        return lhsTypeDen;
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
    if (call->funcDeclRef)
        return call->funcDeclRef->returnType->typeDenoter;
    else if (call->typeDenoter)
        return call->typeDenoter;
    else if (call->intrinsic != Intrinsic::Undefined)
    {
        try
        {
            return GetTypeDenoterForHLSLIntrinsicWithArgs(call->intrinsic, call->arguments);
        }
        catch (const std::exception& e)
        {
            RuntimeErr(e.what(), this);
        }
    }
    else
        RuntimeErr("missing function reference to derive expression type", this);
}


/* ----- BracketExpr ----- */

TypeDenoterPtr BracketExpr::DeriveTypeDenoter()
{
    return expr->GetTypeDenoter();
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
    const auto& castTypeDen = typeExpr->GetTypeDenoter();
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
