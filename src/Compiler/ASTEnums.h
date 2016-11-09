/*
 * ASTEnums.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_AST_ENUMS_H
#define XSC_AST_ENUMS_H


#include <string>


namespace Xsc
{


/* ----- Enumeration ----- */

// Assignment operator enumeration:
// =, +=, -=, *=, /=, %=, <<=, >>=, |= , &=, ^=
enum class AssignOp
{
    Set,
    Add,
    Sub,
    Mul,
    Div,
    Mod,
    LShift,
    RShift,
    Or,
    And,
    Xor,
};

// Binary operator enumeration:
// &&, ||, |, ^, &, <<, >>, +, -, *, /, %, ==, !=, <, >, <=, >=
enum class BinaryOp
{
    LogicalAnd,
    LogicalOr,
    Or,
    Xor,
    And,
    LShift,
    RShift,
    Add,
    Sub,
    Mul,
    Div,
    Mod,
    Equal,
    NotEqual,
    Less,
    Greater,
    LessEqual,
    GreaterEqual,
};

// Unary operator enumeration:
// !, ~, +, -, ++, --
enum class UnaryOp
{
    LogicalNot, // Logical not (e.g. !x)
    Not,        // Bitwise not (e.g. ~x)
    Nop,        // No-op (e.g. +x is equal to x)
    Negate,     // Negate (e.g. -x)
    Inc,        // Increment (e.g. ++x)
    Dec,        // Decrement (e.g. --x)
};


/* ----- Functions ----- */

std::string AssignOpToString(const AssignOp o);
AssignOp StringToAssignOp(const std::string& s);

std::string BinaryOpToString(const BinaryOp o);
BinaryOp StringToBinaryOp(const std::string& s);


std::string UnaryOpToString(const UnaryOp o);
UnaryOp StringToUnaryOp(const std::string& s);


} // /namespace Xsc


#endif



// ================================================================================