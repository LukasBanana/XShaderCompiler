/*
 * ASTEnums.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "ASTEnums.h"
#include <map>


namespace Xsc
{


/* ----- Helper functions ----- */

[[noreturn]]
static void MapFailed(const std::string& from, const std::string& to)
{
    throw std::invalid_argument("failed to map " + from + " to " + to);
}

template <typename T>
std::string TypeToString(const std::map<T, std::string>& typeMap, const T& type, const char* typeName)
{
    auto it = typeMap.find(type);
    if (it != typeMap.end())
        return it->second;
    MapFailed(typeName, "string");
}

template <typename T>
T StringToType(const std::map<T, std::string>& typeMap, const std::string& str, const char* typeName)
{
     for (const auto& entry : typeMap)
     {
         if (entry.second == str)
             return entry.first;
     }
     MapFailed("string", typeName);
}

/* ----- AssignOp Enum ----- */

static const std::map<AssignOp, std::string> g_mapAssignOp
{
    { AssignOp::Set,    "="   },
    { AssignOp::Add,    "+="  },
    { AssignOp::Sub,    "-="  },
    { AssignOp::Mul,    "*="  },
    { AssignOp::Div,    "/="  },
    { AssignOp::Mod,    "%="  },
    { AssignOp::LShift, "<<=" },
    { AssignOp::RShift, ">>=" },
    { AssignOp::Or,     "|="  },
    { AssignOp::And,    "&="  },
    { AssignOp::Xor,    "^="  },
};

std::string AssignOpToString(const AssignOp o)
{
    return TypeToString(g_mapAssignOp, o, "AssignOp");
}

AssignOp StringToAssignOp(const std::string& s)
{
    return StringToType(g_mapAssignOp, s, "AssignOp");
}

/* ----- BinaryOp Enum ----- */

static const std::map<BinaryOp, std::string> g_mapBinaryOp
{
    { BinaryOp::LogicalAnd,     "&&" },
    { BinaryOp::LogicalOr,      "||" },
    { BinaryOp::Or,             "|" },
    { BinaryOp::Xor,            "^" },
    { BinaryOp::And,            "&" },
    { BinaryOp::LShift,         "<<" },
    { BinaryOp::RShift,         ">>" },
    { BinaryOp::Add,            "+" },
    { BinaryOp::Sub,            "-" },
    { BinaryOp::Mul,            "*" },
    { BinaryOp::Div,            "/" },
    { BinaryOp::Mod,            "%" },
    { BinaryOp::Equal,          "==" },
    { BinaryOp::NotEqual,       "!=" },
    { BinaryOp::Less,           "<" },
    { BinaryOp::Greater,        ">" },
    { BinaryOp::LessEqual,      "<=" },
    { BinaryOp::GreaterEqual,   ">=" },
};

std::string BinaryOpToString(const BinaryOp o)
{
    return TypeToString(g_mapBinaryOp, o, "BinaryOp");
}

BinaryOp StringToBinaryOp(const std::string& s)
{
    return StringToType(g_mapBinaryOp, s, "BinaryOp");
}

/* ----- UnaryOp Enumeration ----- */

static const std::map<UnaryOp, std::string> g_mapUnaryOp
{
    { UnaryOp::LogicalNot,  "!" },
    { UnaryOp::Not,         "~" },
    { UnaryOp::Nop,         "+" },
    { UnaryOp::Negate,      "-" },
    { UnaryOp::Inc,         "++" },
    { UnaryOp::Dec,         "--" },
};

std::string UnaryOpToString(const UnaryOp o)
{
    return TypeToString(g_mapUnaryOp, o, "UnaryOp");
}

UnaryOp StringToUnaryOp(const std::string& s)
{
    return StringToType(g_mapUnaryOp, s, "UnaryOp");
}


} // /namespace Xsc



// ================================================================================