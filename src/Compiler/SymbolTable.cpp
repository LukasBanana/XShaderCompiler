/*
 * SymbolTable.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "SymbolTable.h"


namespace Xsc
{


bool ASTSymbolOverload::AddSymbolRef(AST* ast)
{
    if (!ast)
        return false;

    /* Is this the first symbol reference? */
    if (!refs.empty())
    {
        /* Is this a redefinition of another AST type? */
        if (refs.front()->Type() != ast->Type())
            return false;

        /* Can this type of symbol be overloaded? */
        if (ast->Type() != AST::Types::FunctionDecl)
            return false;
    }

    /* Add AST reference to list */
    refs.push_back(ast);

    return true;
}


} // /namespace Xsc



// ================================================================================