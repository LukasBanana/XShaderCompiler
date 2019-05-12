/*
 * Exception.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "Exception.h"


namespace Xsc
{


ASTRuntimeError::ASTRuntimeError(const char* msg, const AST* ast) :
    std::runtime_error { msg },
    ast_               { ast }
{
}

ASTRuntimeError::ASTRuntimeError(const std::string& msg, const AST* ast) :
    std::runtime_error { msg },
    ast_               { ast }
{
}

ASTRuntimeError::ASTRuntimeError(const std::string& msg, const AST* ast, const std::vector<const AST*>& astAppendices) :
    std::runtime_error { msg           },
    ast_               { ast           },
    astAppendices_     { astAppendices }
{
}

[[noreturn]]
void RuntimeErr(const char* msg)
{
    throw std::runtime_error(msg);
}

[[noreturn]]
void RuntimeErr(const std::string& msg)
{
    throw std::runtime_error(msg);
}

[[noreturn]]
void RuntimeErr(const char* msg, const AST* ast)
{
    if (ast)
        throw ASTRuntimeError(msg, ast);
    else
        throw std::runtime_error(msg);
}

[[noreturn]]
void RuntimeErr(const std::string& msg, const AST* ast)
{
    if (ast)
        throw ASTRuntimeError(msg, ast);
    else
        throw std::runtime_error(msg);
}

[[noreturn]]
void RuntimeErr(const std::string& msg, const AST* ast, const std::vector<const AST*>& astAppendices)
{
    if (ast)
        throw ASTRuntimeError(msg, ast, astAppendices);
    else
        throw std::runtime_error(msg);
}

[[noreturn]]
void InvalidArg(const char* msg)
{
    throw std::invalid_argument(msg);
}

[[noreturn]]
void InvalidArg(const std::string& msg)
{
    throw std::invalid_argument(msg);
}


} // /namespace Xsc



// ================================================================================
