/*
 * HLSLTree.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "HLSLTree.h"


namespace Xsc
{


/* --- Helper functions --- */

std::string FullVarIdent(const VarIdentPtr& varIdent)
{
    std::string name;
    auto ast = varIdent;
    while (true)
    {
        name += ast->ident;
        if (ast->next)
        {
            ast = ast->next;
            name += ".";
        }
        else
            break;
    }
    return name;
}

VarIdent* LastVarIdent(VarIdent* varIdent)
{
    return (varIdent && varIdent->next) ? LastVarIdent(varIdent->next.get()) : varIdent;
}


} // /namespace Xsc



// ================================================================================