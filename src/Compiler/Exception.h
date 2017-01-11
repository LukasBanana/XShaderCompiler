/*
 * Exception.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_EXCEPTION_H
#define XSC_EXCEPTION_H


#include <stdexcept>


namespace Xsc
{


struct AST;

// Runtime error exception class with a reference to the AST node where the error occured.
class ASTRuntimeError : public std::runtime_error
{

    public:

        explicit ASTRuntimeError(const char* msg, const AST* ast);
        explicit ASTRuntimeError(const std::string& msg, const AST* ast);

        inline const AST* GetAST() const
        {
            return ast_;
        }

    private:

        const AST* ast_;

};


// Throws an std::runtime_error exception.
[[noreturn]]
void RuntimeErr(const char* msg);

// Throws an std::runtime_error exception.
[[noreturn]]
void RuntimeErr(const std::string& msg);

// Throws an ASTRuntimeError exception.
[[noreturn]]
void RuntimeErr(const char* msg, const AST* ast);

// Throws an ASTRuntimeError exception.
[[noreturn]]
void RuntimeErr(const std::string& msg, const AST* ast);

// Throws an std::invalid_argument exception.
[[noreturn]]
void InvalidArg(const char* msg);

// Throws an std::invalid_argument exception.
[[noreturn]]
void InvalidArg(const std::string& msg);


} // /namespace Xsc


#endif



// ================================================================================