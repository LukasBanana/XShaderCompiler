/*
 * AST.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "AST.h"
#include "Exception.h"
#include "HLSLIntrinsics.h"

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

//TODO: incomplete for arrays of TextureDecl, and SamplerDecl!
TypeDenoterPtr VarIdent::DeriveTypeDenoter()
{
    if (symbolRef)
    {
        /* Derive type denoter from symbol reference */
        switch (symbolRef->Type())
        {
            case AST::Types::VarDecl:
            {
                auto varDecl = static_cast<VarDecl*>(symbolRef);
                return varDecl->GetTypeDenoter()->GetFromArray(arrayIndices.size(), next.get());
            }
            break;

            case AST::Types::TextureDecl:
            {
                auto textureDecl = static_cast<TextureDecl*>(symbolRef);
                return std::make_shared<TextureTypeDenoter>(textureDecl);
            }
            break;

            case AST::Types::SamplerDecl:
            {
                auto samplerDecl = static_cast<SamplerDecl*>(symbolRef);
                return std::make_shared<SamplerTypeDenoter>(samplerDecl);
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
        systemSemantic  = nextVarIdent->systemSemantic;
    }
}


/* ----- StructDecl ----- */

std::string StructDecl::SignatureToString() const
{
    return (IsAnonymous() ? "<anonymous>" : ident);
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


/* ----- AliasDecl ----- */

TypeDenoterPtr AliasDecl::DeriveTypeDenoter()
{
    return typeDenoter;
}


/* ----- PackOffset ----- */

std::string PackOffset::ToString() const
{
    std::string s;

    s += "packoffset(";
    s += registerName;

    if (!vectorComponent.empty())
    {
        s += '.';
        s += vectorComponent;
    }

    s += ')';

    return s;
}


/* ----- VarSemantic ----- */

std::string VarSemantic::ToString() const
{
    if (!registerName.empty())
    {
        std::string s;

        s += "register(";
        s += registerName;
        s += ')';

        return s;
    }

    if (packOffset)
        return packOffset->ToString();

    //return semantic;
    return "<Semantic>";
}


/* ----- VarType ----- */

std::string VarType::ToString() const
{
    return typeDenoter->ToString();
}


/* ----- VarDecl ----- */

std::string VarDecl::ToString() const
{
    std::string s;

    s += ident;

    for (const auto& expr : arrayDims)
    {
        s += '[';
        //TODO: add "std::string Expr::ToString()" function!
        //s += expr->ToString();
        s += "???";
        s += ']';
    }

    for (const auto& sem : semantics)
    {
        s += " : ";
        s += sem->ToString();
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
        return declStmntRef->varType->typeDenoter->AsArray(arrayDims);
    }
    RuntimeErr("missing reference to declaration statement to derive type denoter of variable identifier '" + ident + "'", this);
}

VarSemantic* VarDecl::FirstSemantic() const
{
    return (!semantics.empty() ? semantics.front().get() : nullptr);
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
    return (inputModifier == "in" || inputModifier == "inout" || inputModifier.empty());
}

bool VarDeclStmnt::IsOutput() const
{
    return (inputModifier == "out" || inputModifier == "inout");
}


/* ----- FunctionDecl ----- */

bool FunctionDecl::IsForwardDecl() const
{
    return (codeBlock == nullptr);
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

TypeDenoterPtr FunctionDecl::GetTypeDenoterForArgs(const std::vector<ExprPtr>& /*args*/)
{
    return returnType->typeDenoter;
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
    return std::make_shared<BaseTypeDenoter>(dataType);
}


/* ----- TypeNameExpr ----- */

TypeDenoterPtr TypeNameExpr::DeriveTypeDenoter()
{
    return typeDenoter;
}


/* ----- TernaryExpr ----- */

TypeDenoterPtr TernaryExpr::DeriveTypeDenoter()
{
    /* Only return type denoter of the 'then'-branch (both types must be compatible) */
    return thenExpr->GetTypeDenoter();
}


/* ----- BinaryExpr ----- */

TypeDenoterPtr BinaryExpr::DeriveTypeDenoter()
{
    /* Return type of left-hand-side sub expresion if the types are compatible */
    const auto& lhsTypeDen = lhsExpr->GetTypeDenoter();
    const auto& rhsTypeDen = rhsExpr->GetTypeDenoter();

    if (!rhsTypeDen->IsCastableTo(*lhsTypeDen))
    {
        RuntimeErr(
            "can not cast '" + rhsTypeDen->ToString() + "' to '" + lhsTypeDen->ToString() +
            "' in binary expression '" + BinaryOpToString(op) + "'", this
        );
    }

    return lhsTypeDen;
}


/* ----- UnaryExpr ----- */

TypeDenoterPtr UnaryExpr::DeriveTypeDenoter()
{
    return expr->GetTypeDenoter();
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
        return call->funcDeclRef->GetTypeDenoterForArgs(call->arguments);
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
    return expr->GetTypeDenoter()->GetFromArray(arrayIndices.size());
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

TypeDenoterPtr InitializerExpr::DeriveTypeDenoter()
{
    if (exprs.empty())
        RuntimeErr("can not derive type of initializer list with no elements", this);
    return std::make_shared<ArrayTypeDenoter>(exprs.front()->GetTypeDenoter(), std::vector<ExprPtr>{ nullptr });
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


} // /namespace Xsc



// ================================================================================
