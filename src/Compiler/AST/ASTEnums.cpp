/*
 * ASTEnums.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "ASTEnums.h"
#include "Exception.h"
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

bool IsBitwiseOp(const AssignOp o)
{
    return (o >= AssignOp::LShift && o <= AssignOp::Xor);
}


/* ----- BinaryOp Enum ----- */

static const std::map<BinaryOp, std::string> g_mapBinaryOp
{
    { BinaryOp::LogicalAnd,     "&&" },
    { BinaryOp::LogicalOr,      "||" },
    { BinaryOp::Or,             "|"  },
    { BinaryOp::Xor,            "^"  },
    { BinaryOp::And,            "&"  },
    { BinaryOp::LShift,         "<<" },
    { BinaryOp::RShift,         ">>" },
    { BinaryOp::Add,            "+"  },
    { BinaryOp::Sub,            "-"  },
    { BinaryOp::Mul,            "*"  },
    { BinaryOp::Div,            "/"  },
    { BinaryOp::Mod,            "%"  },
    { BinaryOp::Equal,          "==" },
    { BinaryOp::NotEqual,       "!=" },
    { BinaryOp::Less,           "<"  },
    { BinaryOp::Greater,        ">"  },
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

bool IsBitwiseOp(const BinaryOp o)
{
    return (o >= BinaryOp::Or && o <= BinaryOp::RShift);
}


/* ----- UnaryOp Enum ----- */

static const std::map<UnaryOp, std::string> g_mapUnaryOp
{
    { UnaryOp::LogicalNot,  "!"  },
    { UnaryOp::Not,         "~"  },
    { UnaryOp::Nop,         "+"  },
    { UnaryOp::Negate,      "-"  },
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

bool IsBitwiseOp(const UnaryOp o)
{
    return (o == UnaryOp::Not);
}


/* ----- CtrlTransfer Enum ----- */

static const std::map<CtrlTransfer, std::string> g_mapCtrlTransfer
{
    { CtrlTransfer::Break,      "break"    },
    { CtrlTransfer::Continue,   "continue" },
    { CtrlTransfer::Discard,    "discard"  },
};

std::string CtrlTransformToString(const CtrlTransfer ct)
{
    return TypeToString(g_mapCtrlTransfer, ct, "CtrlTransfer");
}

CtrlTransfer StringToCtrlTransfer(const std::string& s)
{
    return StringToType(g_mapCtrlTransfer, s, "CtrlTransfer");
}


/* ----- DataType Enum ----- */

std::string DataTypeToString(const DataType t, bool useTemplateSyntax)
{
    if (t == DataType::String)
        return "string";
    else if (IsScalarType(t))
    {
        switch (t)
        {
            case DataType::Bool:    return "bool";
            case DataType::Int:     return "int";
            case DataType::UInt:    return "uint";
            case DataType::Half:    return "half";
            case DataType::Float:   return "float";
            case DataType::Double:  return "double";
        }
    }
    else if (IsVectorType(t))
    {
        auto dim = VectorTypeDim(t);
        if (useTemplateSyntax)
            return "vector<" + DataTypeToString(BaseDataType(t)) + ", " + std::to_string(dim) + ">";
        else
            return DataTypeToString(BaseDataType(t)) + std::to_string(dim);
    }
    else if (IsMatrixType(t))
    {
        auto dim = MatrixTypeDim(t);
        if (useTemplateSyntax)
            return "matrix<" + DataTypeToString(BaseDataType(t)) + ", " + std::to_string(dim.first) + ", " + std::to_string(dim.second) + ">";
        else
            return DataTypeToString(BaseDataType(t)) + std::to_string(dim.first) + "x" + std::to_string(dim.second);
    }
    return "<undefined>";
}

bool IsScalarType(const DataType t)
{
    return (t >= DataType::Bool && t <= DataType::Double);
}

bool IsVectorType(const DataType t)
{
    return (t >= DataType::Bool2 && t <= DataType::Double4);
}

bool IsMatrixType(const DataType t)
{
    return (t >= DataType::Bool2x2 && t <= DataType::Double4x4);
}

int VectorTypeDim(const DataType t)
{
    switch (t)
    {
        case DataType::Bool:
        case DataType::Int:
        case DataType::UInt:
        case DataType::Half:
        case DataType::Float:
        case DataType::Double:
            return 1;
    
        case DataType::Bool2:
        case DataType::Int2:
        case DataType::UInt2:
        case DataType::Half2:
        case DataType::Float2:
        case DataType::Double2:
            return 2;

        case DataType::Bool3:
        case DataType::Int3:
        case DataType::UInt3:
        case DataType::Half3:
        case DataType::Float3:
        case DataType::Double3:
            return 3;

        case DataType::Bool4:
        case DataType::Int4:
        case DataType::UInt4:
        case DataType::Half4:
        case DataType::Float4:
        case DataType::Double4:
            return 4;

        default:
            break;
    }
    return 0;
}

std::pair<int, int> MatrixTypeDim(const DataType t)
{
    switch (t)
    {
        case DataType::Bool:
        case DataType::Int:
        case DataType::UInt:
        case DataType::Half:
        case DataType::Float:
        case DataType::Double:
            return { 1, 1 };
    
        case DataType::Bool2:
        case DataType::Int2:
        case DataType::UInt2:
        case DataType::Half2:
        case DataType::Float2:
        case DataType::Double2:
            return { 2, 1 };

        case DataType::Bool3:
        case DataType::Int3:
        case DataType::UInt3:
        case DataType::Half3:
        case DataType::Float3:
        case DataType::Double3:
            return { 3, 1 };

        case DataType::Bool4:
        case DataType::Int4:
        case DataType::UInt4:
        case DataType::Half4:
        case DataType::Float4:
        case DataType::Double4:
            return { 4, 1 };

        case DataType::Bool2x2:
        case DataType::Int2x2:
        case DataType::UInt2x2:
        case DataType::Half2x2:
        case DataType::Float2x2:
        case DataType::Double2x2:
            return { 2, 2 };

        case DataType::Bool2x3:
        case DataType::Int2x3:
        case DataType::UInt2x3:
        case DataType::Half2x3:
        case DataType::Float2x3:
        case DataType::Double2x3:
            return { 2, 3 };

        case DataType::Bool2x4:
        case DataType::Int2x4:
        case DataType::UInt2x4:
        case DataType::Half2x4:
        case DataType::Float2x4:
        case DataType::Double2x4:
            return { 2, 4 };

        case DataType::Bool3x2:
        case DataType::Int3x2:
        case DataType::UInt3x2:
        case DataType::Half3x2:
        case DataType::Float3x2:
        case DataType::Double3x2:
            return { 3, 2 };

        case DataType::Bool3x3:
        case DataType::Int3x3:
        case DataType::UInt3x3:
        case DataType::Half3x3:
        case DataType::Float3x3:
        case DataType::Double3x3:
            return { 3, 3 };

        case DataType::Bool3x4:
        case DataType::Int3x4:
        case DataType::UInt3x4:
        case DataType::Half3x4:
        case DataType::Float3x4:
        case DataType::Double3x4:
            return { 3, 4 };

        case DataType::Bool4x2:
        case DataType::Int4x2:
        case DataType::UInt4x2:
        case DataType::Half4x2:
        case DataType::Float4x2:
        case DataType::Double4x2:
            return { 4, 2 };

        case DataType::Bool4x3:
        case DataType::Int4x3:
        case DataType::UInt4x3:
        case DataType::Half4x3:
        case DataType::Float4x3:
        case DataType::Double4x3:
            return { 4, 3 };

        case DataType::Bool4x4:
        case DataType::Int4x4:
        case DataType::UInt4x4:
        case DataType::Half4x4:
        case DataType::Float4x4:
        case DataType::Double4x4:
            return { 4, 4 };

        default:
            break;
    }
    return { 0, 0 };
}

DataType BaseDataType(const DataType t)
{
    #define FIND_BASETYPE(TYPENAME)                                                 \
        if ( ( t >= DataType::TYPENAME##2   && t <= DataType::TYPENAME##4   ) ||    \
             ( t >= DataType::TYPENAME##2x2 && t <= DataType::TYPENAME##4x4 ) )     \
        {                                                                           \
            return DataType::TYPENAME;                                              \
        }

    FIND_BASETYPE( Bool   );
    FIND_BASETYPE( Int    );
    FIND_BASETYPE( UInt   );
    FIND_BASETYPE( Half   );
    FIND_BASETYPE( Float  );
    FIND_BASETYPE( Double );

    return t;

    #undef FIND_BASETYPE
}

DataType VectorDataType(const DataType baseDataType, int vectorSize)
{
    if (IsScalarType(baseDataType))
    {
        if (vectorSize >= 2 && vectorSize <= 4)
        {
            auto Idx = [](const DataType t)
            {
                return static_cast<int>(t);
            };

            auto offset = (Idx(baseDataType) - Idx(DataType::Bool));
            auto idx    = Idx(DataType::Bool2) + offset*3 + (vectorSize - 2);

            return static_cast<DataType>(idx);
        }
        else if (vectorSize == 1)
            return baseDataType;
    }
    return DataType::Undefined;
}

DataType VectorSubscriptDataType(const DataType dataType, const std::string& subscript)
{
    auto IsValidSubscript = [&subscript](std::string compareSubscript, int vectorSize) -> bool
    {
        compareSubscript.resize(vectorSize);

        for (auto chr : subscript)
        {
            if (compareSubscript.find(chr) == std::string::npos)
                return false;
        }

        return true;
    };

    /* Validate swizzle operator size */
    if (subscript.size() < 0 || subscript.size() > 4)
        InvalidArg("vector subscript can not have " + std::to_string(subscript.size()) + " components");

    /* Validate vector subscript */
    int vectorSize = VectorTypeDim(dataType);

    if (vectorSize < 1 || vectorSize > 4)
        InvalidArg("vector subscript is only allowed for scalar and vector types");

    bool validSubscript =
    (
        IsValidSubscript("xyzw", vectorSize) ||
        IsValidSubscript("rgba", vectorSize)
    );

    if (!validSubscript)
        InvalidArg("invalid vector subscript: '" + subscript + "'");

    return VectorDataType(BaseDataType(dataType), static_cast<int>(subscript.size()));
}


} // /namespace Xsc



// ================================================================================