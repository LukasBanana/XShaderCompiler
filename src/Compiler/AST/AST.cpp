/*
 * AST.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "AST.h"
#include "Exception.h"


namespace Xsc
{


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
                return varDecl->GetTypeDenoter()->Get(next.get());
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
                return structDecl->GetTypeDenoter();
            }
            break;
        }
    }
    RuntimeErr("missing symbol reference to derive type denoter of variable identifier '" + ident + "'", this);
}


/* ----- StructDecl ----- */

std::string StructDecl::SignatureToString() const
{
    return (IsAnonymous() ? "<anonymous>" : name);
}

bool StructDecl::IsAnonymous() const
{
    return name.empty();
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

    return semantic;
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
        return declStmntRef->varType->typeDenoter;
    RuntimeErr("missing reference to declaration statement to derive type denoter of variable identifier '" + ident + "'", this);
}


/* ----- VarDelcStmnt ----- */

std::string VarDeclStmnt::ToString(bool useVarNames) const
{
    std::string s;

    if (!inputModifier.empty())
    {
        s += inputModifier;
        s += ' ';
    }

    for (const auto& modifier : storageModifiers)
    {
        s += modifier;
        s += ' ';
    }

    for (const auto& modifier : typeModifiers)
    {
        s += modifier;
        s += ' ';
    }

    s += varType->ToString();
    
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
    s += name;
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


/* ----- ListExpr ----- */

TypeDenoterPtr ListExpr::DeriveTypeDenoter()
{
    /* Only return type denoter of first sub expression */
    return firstExpr->GetTypeDenoter();
}


/* ----- LiteralExpr ----- */

TypeDenoterPtr LiteralExpr::DeriveTypeDenoter()
{
    switch (type)
    {
        case Token::Types::BoolLiteral:
            return std::make_shared<BaseTypeDenoter>(DataType::Bool);
        case Token::Types::IntLiteral:
            return std::make_shared<BaseTypeDenoter>(DataType::Int);
        case Token::Types::FloatLiteral:
            return std::make_shared<BaseTypeDenoter>(DataType::Float);
    }
    return std::make_shared<BaseTypeDenoter>();
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
    /* Only return type denoter of left hand side */
    return lhsExpr->GetTypeDenoter();
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
    {
        auto typeDenoter = call->funcDeclRef->returnType->typeDenoter;
        return typeDenoter->Get(varIdentSuffix.get());
    }
    else
        throw std::runtime_error("missing function reference to derive expression type");
}


/* ----- BracketExpr ----- */

TypeDenoterPtr BracketExpr::DeriveTypeDenoter()
{
    return expr->GetTypeDenoter()->Get(varIdentSuffix.get());
}


/* ----- CastExpr ----- */

TypeDenoterPtr CastExpr::DeriveTypeDenoter()
{
    return typeExpr->GetTypeDenoter();
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
        throw std::runtime_error("can not derive type of initializer list with no elements");
    return std::make_shared<ArrayTypeDenoter>(exprs.front()->GetTypeDenoter());
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