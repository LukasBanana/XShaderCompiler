/*
 * AST.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "AST.h"


namespace Xsc
{


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
    return (structType ? structType->name : baseType);
}


/* ----- VarDecl ----- */

std::string VarDecl::ToString() const
{
    std::string s;

    s += name;

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


/* ----- FunctionDecl ----- */

std::string FunctionDecl::ToString(bool useParamNames) const
{
    std::string s;

    s += returnType->ToString();
    s += ' ';
    s += name;
    s += '(';

    for (std::size_t i = 0; i < parameters.size(); ++i)
    {

        if (i + 1 < parameters.size())
            s += ", ";
    }

    s += ')';

    return s;
}


} // /namespace Xsc



// ================================================================================