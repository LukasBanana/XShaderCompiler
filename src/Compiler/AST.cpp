/*
 * AST.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "AST.h"


namespace Xsc
{


/* --- Helper functions --- */

//TODO: move this into "VarIdent" struct.
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

//TODO: move this into "VarIdent" struct.
VarIdent* LastVarIdent(VarIdent* varIdent)
{
    return ((varIdent && varIdent->next) ? LastVarIdent(varIdent->next.get()) : varIdent);
}


} // /namespace Xsc



// ================================================================================