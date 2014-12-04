/*
 * HLSLTree.cpp
 * 
 * This file is part of the "HLSL Translator" (Copyright (c) 2014 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "HLSLTree.h"


namespace HTLib
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


} // /namespace HTLib



// ================================================================================