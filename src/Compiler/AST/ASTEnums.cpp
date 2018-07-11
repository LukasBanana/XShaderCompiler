/*
 * ASTEnums.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "ASTEnums.h"
#include "Exception.h"
#include "Token.h"
#include "ReportIdents.h"
#include <map>
#include <algorithm>


namespace Xsc
{


/* ----- Helper functions ----- */

[[noreturn]]
static void MapFailed(const std::string& from, const std::string& to)
{
    throw std::invalid_argument(R_FailedToMap(from, to));
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

template <typename T>
std::string TypeToStringSecondary(const std::map<std::string, T>& typeMap, const T& type, const char* typeName)
{
    for (const auto& entry : typeMap)
    {
        if (entry.second == type)
            return entry.first;
    }
    MapFailed(typeName, "string");
}

template <typename T>
T StringToTypeSecondary(const std::map<std::string, T>& typeMap, const std::string& str, const char* typeName)
{
    auto it = typeMap.find(str);
    if (it != typeMap.end())
        return it->second;
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

BinaryOp AssignOpToBinaryOp(const AssignOp op)
{
    switch (op)
    {
        case AssignOp::Add:     return BinaryOp::Add;
        case AssignOp::Sub:     return BinaryOp::Sub;
        case AssignOp::Mul:     return BinaryOp::Mul;
        case AssignOp::Div:     return BinaryOp::Div;
        case AssignOp::Mod:     return BinaryOp::Mod;
        case AssignOp::LShift:  return BinaryOp::LShift;
        case AssignOp::RShift:  return BinaryOp::RShift;
        case AssignOp::Or:      return BinaryOp::Or;
        case AssignOp::And:     return BinaryOp::And;
        case AssignOp::Xor:     return BinaryOp::Xor;
        default:                return BinaryOp::Undefined;
    }
}

bool IsLogicalOp(const BinaryOp o)
{
    return (o >= BinaryOp::LogicalAnd && o <= BinaryOp::LogicalOr);
}

bool IsBitwiseOp(const BinaryOp o)
{
    return (o >= BinaryOp::Or && o <= BinaryOp::RShift);
}

bool IsCompareOp(const BinaryOp o)
{
    return (o >= BinaryOp::Equal && o <= BinaryOp::GreaterEqual);
}

bool IsBooleanOp(const BinaryOp o)
{
    return (IsLogicalOp(o) || IsCompareOp(o));
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

bool IsLogicalOp(const UnaryOp o)
{
    return (o == UnaryOp::LogicalNot);
}

bool IsBitwiseOp(const UnaryOp o)
{
    return (o == UnaryOp::Not);
}

bool IsLValueOp(const UnaryOp o)
{
    return (o == UnaryOp::Inc || o == UnaryOp::Dec);
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

MatrixSubscriptUsage::MatrixSubscriptUsage(const DataType dataTypeIn, const std::string& subscript) :
    dataTypeIn  { dataTypeIn                                         },
    dataTypeOut { SubscriptDataType(dataTypeIn, subscript, &indices) }
{
}

bool MatrixSubscriptUsage::operator < (const MatrixSubscriptUsage& rhs) const
{
    /* Only sort by input data type and indices */
    if (dataTypeIn < rhs.dataTypeIn) { return true;  }
    if (dataTypeIn > rhs.dataTypeIn) { return false; }
    return indices < rhs.indices;
}

std::string MatrixSubscriptUsage::IndicesToString() const
{
    std::string s;

    for (const auto& i : indices)
    {
        s += '_';
        s += std::to_string(i.first);
        s += std::to_string(i.second);
    }

    return s;
}

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
            default:                break;
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
    return R_Undefined;
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

bool IsBooleanType(const DataType t)
{
    return
    (
        (t == DataType::Bool) ||
        (t >= DataType::Bool2 && t <= DataType::Bool4) ||
        (t >= DataType::Bool2x2 && t <= DataType::Bool4x4)
    );
}

bool IsRealType(const DataType t)
{
    return
    (
        (t >= DataType::Half && t <= DataType::Double) ||
        (t >= DataType::Half2 && t <= DataType::Double4) ||
        (t >= DataType::Half2x2 && t <= DataType::Double4x4)
    );
}

bool IsHalfRealType(const DataType t)
{
    return
    (
        (t == DataType::Half) ||
        (t >= DataType::Half2 && t <= DataType::Half4) ||
        (t >= DataType::Half2x2 && t <= DataType::Half4x4)
    );
}

bool IsDoubleRealType(const DataType t)
{
    return
    (
        (t == DataType::Double) ||
        (t >= DataType::Double2 && t <= DataType::Double4) ||
        (t >= DataType::Double2x2 && t <= DataType::Double4x4)
    );
}

bool IsIntegralType(const DataType t)
{
    return
    (
        (t >= DataType::Int && t <= DataType::UInt) ||
        (t >= DataType::Int2 && t <= DataType::UInt4) ||
        (t >= DataType::Int2x2 && t <= DataType::UInt4x4)
    );
}

bool IsIntType(const DataType t)
{
    return
    (
        (t == DataType::Int) ||
        (t >= DataType::Int2 && t <= DataType::Int4) ||
        (t >= DataType::Int2x2 && t <= DataType::Int4x4)
    );
}

bool IsUIntType(const DataType t)
{
    return
    (
        (t == DataType::UInt) ||
        (t >= DataType::UInt2 && t <= DataType::UInt4) ||
        (t >= DataType::UInt2x2 && t <= DataType::UInt4x4)
    );
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
            return 0;
    }
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
            return { 0, 0 };
    }
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

static int Idx(const DataType t)
{
    return static_cast<int>(t);
};

DataType VectorDataType(const DataType baseDataType, int vectorSize)
{
    if (IsScalarType(baseDataType))
    {
        if (vectorSize >= 2 && vectorSize <= 4)
        {
            auto offset = (Idx(baseDataType) - Idx(DataType::Bool));
            auto idx    = Idx(DataType::Bool2) + offset*3 + (vectorSize - 2);

            return static_cast<DataType>(idx);
        }
        else if (vectorSize == 1)
            return baseDataType;
    }
    return DataType::Undefined;
}

DataType MatrixDataType(const DataType baseDataType, int rows, int columns)
{
    if (IsScalarType(baseDataType))
    {
        if (rows == 1 && columns == 1)
            return baseDataType;
        if (rows == 1)
            return VectorDataType(baseDataType, columns);
        if (columns == 1)
            return VectorDataType(baseDataType, rows);

        if (rows >= 2 && rows <= 4 && columns >= 2 && columns <= 4)
        {
            auto offset = (Idx(baseDataType) - Idx(DataType::Bool));
            auto idx    = Idx(DataType::Bool2x2) + offset*9 + (rows - 2)*3 + (columns - 2);

            return static_cast<DataType>(idx);
        }
    }
    return DataType::Undefined;
}

static DataType SubscriptDataTypeVector(
    const DataType dataType, const std::string& subscript, int vectorSize, std::vector<std::pair<int, int>>* indices)
{
    auto IsValidSubscript = [&](std::string compareSubscript, int vectorSize) -> bool
    {
        compareSubscript.resize(vectorSize);

        std::vector<std::pair<int, int>> subscriptIndices;
        subscriptIndices.reserve(vectorSize);

        for (auto chr : subscript)
        {
            auto pos = compareSubscript.find(chr);
            if (pos != std::string::npos)
                subscriptIndices.push_back({ static_cast<int>(pos), 0 });
            else
                return false;
        }

        if (indices)
            *indices = std::move(subscriptIndices);

        return true;
    };

    /* Validate swizzle operator size */
    auto subscriptSize = subscript.size();

    if (subscriptSize < 1 || subscriptSize > 4)
        InvalidArg(R_VectorSubscriptCantHaveNComps(subscriptSize));

    /* Validate vector subscript */
    if (vectorSize < 1 || vectorSize > 4)
        InvalidArg(R_InvalidVectorDimension(vectorSize));

    bool validSubscript =
    (
        IsValidSubscript("xyzw", vectorSize) ||
        IsValidSubscript("rgba", vectorSize)
    );

    if (!validSubscript)
        InvalidArg(R_InvalidVectorSubscript(subscript, DataTypeToString(dataType)));

    return VectorDataType(BaseDataType(dataType), static_cast<int>(subscriptSize));
}

static void ParseNextMatrixSubscript(
    const std::string& s, std::size_t& i, char& zeroBase, int rows, int cols, std::vector<std::pair<int, int>>* indices)
{
    if (i + 3 > s.size())
        InvalidArg(R_IncompleteMatrixSubscript(s));
    if (s[i] != '_')
        InvalidArg(R_InvalidCharInMatrixSubscript(std::string(1, s[i]), s));
    ++i;

    if (s[i] == 'm')
    {
        /* Check mixture of zero-based and one-based matrix subscripts */
        if (zeroBase == 1)
            InvalidArg(R_InvalidMatrixSubscriptMixture(s));
        else
            zeroBase = 0;

        /* Take characterr */
        ++i;
        if (i + 2 > s.size())
            InvalidArg(R_IncompleteMatrixSubscript(s));
    }
    else
    {
        /* Check mixture of zero-based and one-based matrix subscripts */
        if (zeroBase == 0)
            InvalidArg(R_InvalidMatrixSubscriptMixture(s));
        else
            zeroBase = 1;
    }

    /* Parse matrix indices */
    int subscriptIndices[2] = { 0 };

    const int maxIdx[2] = { rows, cols };
    const JoinableString matrixDimStr(" {0}x{1}");

    for (int j = 0; j < 2; ++j)
    {
        if (s[i] >= '0' + zeroBase && s[i] < '0' + maxIdx[j] + zeroBase)
        {
            /* Store subscript index */
            subscriptIndices[j] = s[i] - ('0' + zeroBase);
        }
        else
        {
            InvalidArg(
                R_InvalidCharInMatrixSubscript(
                    std::string(1, s[i]), s,
                    (zeroBase == 0 ? R_ZeroBased : R_OneBased) + matrixDimStr(rows, cols)
                )
            );
        }
        ++i;
    }

    if (indices)
        indices->push_back({ subscriptIndices[0], subscriptIndices[1] });
}

/*
Matrix subscription rules for HLSL:
see https://msdn.microsoft.com/en-us/library/windows/desktop/bb509634(v=vs.85).aspx#Matrix
*/
static DataType SubscriptDataTypeMatrix(
    const DataType dataType, const std::string& subscript, int rows, int cols, std::vector<std::pair<int, int>>* indices)
{
    /* Validate matrix subscript */
    if (rows < 1 || rows > 4 || cols < 1 || cols > 4)
        InvalidArg(R_InvalidMatrixDimension(rows, cols));

    /* Parse all matrix row-column subscriptions (e.g. zero-based "_m00", or one-based "_11") */
    int vectorSize = 0;
    char zeroBase = -1;

    for (std::size_t i = 0; i < subscript.size(); ++vectorSize)
        ParseNextMatrixSubscript(subscript, i, zeroBase, rows, cols, indices);

    return VectorDataType(BaseDataType(dataType), vectorSize);
}

DataType SubscriptDataType(const DataType dataType, const std::string& subscript, std::vector<std::pair<int, int>>* indices)
{
    auto matrixDim = MatrixTypeDim(dataType);
    if (matrixDim.second == 1)
        return SubscriptDataTypeVector(dataType, subscript, matrixDim.first, indices);
    else
        return SubscriptDataTypeMatrix(dataType, subscript, matrixDim.first, matrixDim.second, indices);
}

static DataType IntLiteralTokenToDataType(const Token& tkn)
{
    const auto& s = tkn.Spell();

    if (!s.empty())
    {
        /* Has literal the 'u' or 'U' suffix for unsigned integers? */
        if (s.back() == 'u' || s.back() == 'U')
            return DataType::UInt;
    }

    return DataType::Int;
}

static DataType FloatLiteralTokenToDataType(const Token& tkn)
{
    const auto& s = tkn.Spell();

    if (!s.empty())
    {
        /* Has literal the 'f' or 'F' suffix for single precision floats? */
        if (s.back() == 'f' || s.back() == 'F')
            return DataType::Float;

        /* Has literal the 'h' or 'H' suffix for half precision floats? */
        if (s.back() == 'h' || s.back() == 'H')
            return DataType::Half;
    }

    return DataType::Double;
}

DataType TokenToDataType(const Token& tkn)
{
    switch (tkn.Type())
    {
        case Token::Types::BoolLiteral:
            return DataType::Bool;
        case Token::Types::IntLiteral:
            return IntLiteralTokenToDataType(tkn);
        case Token::Types::FloatLiteral:
            return FloatLiteralTokenToDataType(tkn);
        case Token::Types::StringLiteral:
            return DataType::String;
        default:
            break;
    }
    return DataType::Undefined;
}

DataType DoubleToFloatDataType(const DataType dataType)
{
    if (dataType == DataType::Double)
        return DataType::Float;
    if (dataType >= DataType::Double2 && dataType <= DataType::Double4)
        return static_cast<DataType>(Idx(dataType) - Idx(DataType::Double2) + Idx(DataType::Float2));
    if (dataType >= DataType::Double2x2 && dataType <= DataType::Double4x4)
        return static_cast<DataType>(Idx(dataType) - Idx(DataType::Double2x2) + Idx(DataType::Float2x2));
    return dataType;
}


/* ----- BufferType Enum ----- */

static const std::map<BufferType, std::string> g_mapBufferType
{
    { BufferType::Buffer,                  "Buffer"                  },
    { BufferType::StructuredBuffer,        "StructuredBuffer"        },
    { BufferType::ByteAddressBuffer,       "ByteAddressBuffer"       },

    { BufferType::RWBuffer,                "RWBuffer"                },
    { BufferType::RWStructuredBuffer,      "RWStructuredBuffer"      },
    { BufferType::RWByteAddressBuffer,     "RWByteAddressBuffer"     },
    { BufferType::AppendStructuredBuffer,  "AppendStructuredBuffer"  },
    { BufferType::ConsumeStructuredBuffer, "ConsumeStructuredBuffer" },

    { BufferType::RWTexture1D,             "RWTexture1D"             },
    { BufferType::RWTexture1DArray,        "RWTexture1DArray"        },
    { BufferType::RWTexture2D,             "RWTexture2D"             },
    { BufferType::RWTexture2DArray,        "RWTexture2DArray"        },
    { BufferType::RWTexture3D,             "RWTexture3D"             },

    { BufferType::Texture1D,               "Texture1D"               },
    { BufferType::Texture1DArray,          "Texture1DArray"          },
    { BufferType::Texture2D,               "Texture2D"               },
    { BufferType::Texture2DArray,          "Texture2DArray"          },
    { BufferType::Texture3D,               "Texture3D"               },
    { BufferType::TextureCube,             "TextureCube"             },
    { BufferType::TextureCubeArray,        "TextureCubeArray"        },
    { BufferType::Texture2DMS,             "Texture2DMS"             },
    { BufferType::Texture2DMSArray,        "Texture2DMSArray"        },

    { BufferType::GenericTexture,          "Texture"                 },

    { BufferType::InputPatch,              "InputPatch"              },
    { BufferType::OutputPatch,             "OutputPatch"             },

    { BufferType::PointStream,             "PointStream"             },
    { BufferType::LineStream,              "LineStream"              },
    { BufferType::TriangleStream,          "TriangleStream"          },

    { BufferType::GenericBuffer,           "buffer"                  },
};

std::string BufferTypeToString(const BufferType t)
{
    return TypeToString(g_mapBufferType, t, "BufferType");
}

bool IsStorageBufferType(const BufferType t)
{
    return
    (
        ( t >= BufferType::StructuredBuffer   && t <= BufferType::ByteAddressBuffer       ) ||
        ( t >= BufferType::RWStructuredBuffer && t <= BufferType::ConsumeStructuredBuffer ) ||
        ( t == BufferType::GenericBuffer                                                  )
    );
}

bool IsRWBufferType(const BufferType t)
{
    return (t >= BufferType::RWBuffer && t <= BufferType::RWTexture3D);
}

bool IsTextureBufferType(const BufferType t)
{
    return (t >= BufferType::RWTexture1D && t <= BufferType::GenericTexture);
}

bool IsTextureMSBufferType(const BufferType t)
{
    return (t >= BufferType::Texture2DMS && t <= BufferType::Texture2DMSArray);
}

bool IsImageBufferType(const BufferType t)
{
    return ( ( t >= BufferType::RWTexture1D && t <= BufferType::GenericTexture ) || t == BufferType::Buffer );
}

bool IsRWImageBufferType(const BufferType t)
{
    return ( ( t >= BufferType::RWTexture1D && t <= BufferType::RWTexture3D ) || t == BufferType::RWBuffer );
}

bool IsPatchBufferType(const BufferType t)
{
    return (t >= BufferType::InputPatch && t <= BufferType::OutputPatch);
}

bool IsStreamBufferType(const BufferType t)
{
    return (t >= BufferType::PointStream && t <= BufferType::TriangleStream);
}

int GetBufferTypeTextureDim(const BufferType t)
{
    switch (t)
    {
        case BufferType::Buffer:
        case BufferType::RWBuffer:
        case BufferType::Texture1D:
        case BufferType::RWTexture1D:
            return 1;

        case BufferType::Texture1DArray:
        case BufferType::RWTexture1DArray:
        case BufferType::Texture2D:
        case BufferType::RWTexture2D:
        case BufferType::Texture2DMS:
            return 2;

        case BufferType::Texture2DArray:
        case BufferType::RWTexture2DArray:
        case BufferType::Texture2DMSArray:
        case BufferType::Texture3D:
        case BufferType::RWTexture3D:
        case BufferType::TextureCube:
            return 3;

        case BufferType::TextureCubeArray:
            return 4;

        default:
            return 0;
    }
}


/* ----- SamplerType Enum ----- */

bool IsSamplerStateType(const SamplerType t)
{
    return (t >= SamplerType::SamplerState && t <= SamplerType::SamplerComparisonState);
}

bool IsSamplerTypeShadow(const SamplerType t)
{
    return (t >= SamplerType::Sampler1DShadow && t <= SamplerType::SamplerCubeArrayShadow);
}

bool IsSamplerTypeArray(const SamplerType t)
{
    return ((t >= SamplerType::Sampler1DArray && t <= SamplerType::SamplerCubeArray) || t == SamplerType::Sampler2DMSArray);
}

int GetSamplerTypeTextureDim(const SamplerType t)
{
    switch (t)
    {
        case SamplerType::Sampler1D:
        case SamplerType::SamplerBuffer:
        case SamplerType::Sampler1DShadow:
            return 1;

        case SamplerType::Sampler2D:
        case SamplerType::Sampler2DRect:
        case SamplerType::Sampler1DArray:
        case SamplerType::Sampler2DMS:
        case SamplerType::Sampler2DShadow:
        case SamplerType::Sampler2DRectShadow:
        case SamplerType::Sampler1DArrayShadow:
            return 2;

        case SamplerType::Sampler3D:
        case SamplerType::SamplerCube:
        case SamplerType::Sampler2DArray:
        case SamplerType::Sampler2DMSArray:
        case SamplerType::SamplerCubeShadow:
        case SamplerType::Sampler2DArrayShadow:
            return 3;

        case SamplerType::SamplerCubeArray:
        case SamplerType::SamplerCubeArrayShadow:
            return 4;

        default:
            return 0;
    }
}

SamplerType TextureTypeToSamplerType(const BufferType t)
{
    switch (t)
    {
        case BufferType::Texture1D:         return SamplerType::Sampler1D;
        case BufferType::Texture1DArray:    return SamplerType::Sampler1DArray;
        case BufferType::Texture2D:         return SamplerType::Sampler2D;
        case BufferType::Texture2DArray:    return SamplerType::Sampler2DArray;
        case BufferType::Texture3D:         return SamplerType::Sampler3D;
        case BufferType::TextureCube:       return SamplerType::SamplerCube;
        case BufferType::TextureCubeArray:  return SamplerType::SamplerCubeArray;
        default:                            return SamplerType::Undefined;
    }
}

SamplerType SamplerTypeToShadowSamplerType(const SamplerType t)
{
    switch (t)
    {
        case SamplerType::Sampler1D:        return SamplerType::Sampler1DShadow;
        case SamplerType::Sampler1DArray:   return SamplerType::Sampler1DArrayShadow;
        case SamplerType::Sampler2D:        return SamplerType::Sampler2DShadow;
        case SamplerType::Sampler2DArray:   return SamplerType::Sampler2DArrayShadow;
        case SamplerType::Sampler2DRect:    return SamplerType::Sampler2DRectShadow;
        case SamplerType::SamplerCube:      return SamplerType::SamplerCubeShadow;
        case SamplerType::SamplerCubeArray: return SamplerType::SamplerCubeArrayShadow;
        default:                            return SamplerType::Undefined;
    }
}


/* ----- ImageLayoutFormat Enum ----- */

DataType GetImageLayoutFormatBaseType(const ImageLayoutFormat format)
{
    if (format >= ImageLayoutFormat::F32X4 && format <= ImageLayoutFormat::SN8X1)
        return DataType::Float;
    if (format >= ImageLayoutFormat::I32X4 && format <= ImageLayoutFormat::I8X1)
        return DataType::Int;
    if (format >= ImageLayoutFormat::UI32X4 && format <= ImageLayoutFormat::UI8X1)
        return DataType::UInt;
    return DataType::Undefined;
}

ImageLayoutFormat DataTypeToImageLayoutFormat(const DataType t)
{
    switch (t)
    {
        case DataType::Int:    return ImageLayoutFormat::I32X1;
        case DataType::Int2:   return ImageLayoutFormat::I32X2;
        case DataType::Int4:   return ImageLayoutFormat::I32X4;
        case DataType::UInt:   return ImageLayoutFormat::UI32X1;
        case DataType::UInt2:  return ImageLayoutFormat::UI32X2;
        case DataType::UInt4:  return ImageLayoutFormat::UI32X4;
        case DataType::Float:  return ImageLayoutFormat::F32X1;
        case DataType::Float2: return ImageLayoutFormat::F32X2;
        case DataType::Float4: return ImageLayoutFormat::F32X4;
        default:               return ImageLayoutFormat::Undefined;
    }
}


/* ----- RegisterType Enum ----- */

RegisterType CharToRegisterType(char c)
{
    switch (c)
    {
        case 'b': return RegisterType::ConstantBuffer;
        case 't': return RegisterType::TextureBuffer;
        case 'c': return RegisterType::BufferOffset;
        case 's': return RegisterType::Sampler;
        case 'u': return RegisterType::UnorderedAccessView;
    }
    return RegisterType::Undefined;
}

char RegisterTypeToChar(const RegisterType t)
{
    switch (t)
    {
        case RegisterType::Undefined:           break;
        case RegisterType::ConstantBuffer:      return 'b';
        case RegisterType::TextureBuffer:       return 't';
        case RegisterType::BufferOffset:        return 'c';
        case RegisterType::Sampler:             return 's';
        case RegisterType::UnorderedAccessView: return 'u';
    }
    return '\0';
}

std::string RegisterTypeToString(const RegisterType t)
{
    switch (t)
    {
        case RegisterType::Undefined:           break;
        case RegisterType::ConstantBuffer:      return "ConstantBuffer";
        case RegisterType::TextureBuffer:       return "TextureBuffer";
        case RegisterType::BufferOffset:        return "BufferOffset";
        case RegisterType::Sampler:             return "Sampler";
        case RegisterType::UnorderedAccessView: return "UnorderedAccessView";
    }
    return "";
}


/* ----- AttributeType Enum ----- */

bool IsShaderModel3AttributeType(const AttributeType t)
{
    return (t >= AttributeType::Branch && t <= AttributeType::Xps);
}

bool IsShaderModel5AttributeType(const AttributeType t)
{
    return (t >= AttributeType::Domain && t <= AttributeType::PatchConstantFunc);
}

bool IsHLSLAttributeType(const AttributeType t)
{
    return (t >= AttributeType::Branch && t <= AttributeType::PatchConstantFunc);
}

bool IsGLSLAttributeType(const AttributeType t)
{
    return (t >= AttributeType::Align && t <= AttributeType::XfbStride);
}

#ifdef XSC_ENABLE_LANGUAGE_EXT

bool IsExtAttributeType(const AttributeType t)
{
    return (t >= AttributeType::Space && t <= AttributeType::Layout);
}

#endif


/* ----- AttributeValue Enum ----- */

bool IsAttributeValueDomain(const AttributeValue t)
{
    return (t >= AttributeValue::DomainTri && t <= AttributeValue::DomainIsoline);
}

bool IsAttributeValueOutputTopology(const AttributeValue t)
{
    return (t >= AttributeValue::OutputTopologyPoint && t <= AttributeValue::OutputTopologyTriangleCCW);
}

bool IsAttributeValuePartitioning(const AttributeValue t)
{
    return (t >= AttributeValue::PartitioningInteger && t <= AttributeValue::PartitioningFractionalOdd);
}

bool IsAttributeValueTrianglePartitioning(const AttributeValue t)
{
    return (t >= AttributeValue::OutputTopologyTriangleCW && t <= AttributeValue::OutputTopologyTriangleCCW);
}


/* ----- Intrinsic Enum ----- */

bool IsGlobalIntrinsic(const Intrinsic t)
{
    return (t >= Intrinsic::Abort && t <= Intrinsic::TexCubeProj);
}

bool IsTextureIntrinsic(const Intrinsic t)
{
    return (t >= Intrinsic::Texture_GetDimensions && t <= Intrinsic::Texture_GatherCmpAlpha_8);
}

bool IsStreamOutputIntrinsic(const Intrinsic t)
{
    return (t >= Intrinsic::StreamOutput_Append && t <= Intrinsic::StreamOutput_RestartStrip);
}

bool IsImageIntrinsic(const Intrinsic t)
{
    return (t >= Intrinsic::Image_Load && t <= Intrinsic::Image_Store);
}

bool IsInterlockedIntristic(const Intrinsic t)
{
    return (t >= Intrinsic::InterlockedAdd && t <= Intrinsic::InterlockedXor);
}

bool IsTextureGatherIntrisic(const Intrinsic t)
{
    return (t >= Intrinsic::Texture_Gather_2 && t <= Intrinsic::Texture_GatherCmpAlpha_8);
}

bool IsTextureSampleIntrinsic(const Intrinsic t)
{
    return (t >= Intrinsic::Texture_Sample_2 && t <= Intrinsic::Texture_SampleLevel_5);
}

bool IsTextureCompareIntrinsic(const Intrinsic t)
{
    return
    (
        ( t >= Intrinsic::Texture_SampleCmp_3 && t <= Intrinsic::Texture_SampleCmpLevelZero_5 ) ||
        ( t >= Intrinsic::Texture_GatherCmp_3 && t <= Intrinsic::Texture_GatherCmpAlpha_8     )
    );
}

bool IsTextureCompareLevelZeroIntrinsic(const Intrinsic t)
{
    return (t >= Intrinsic::Texture_SampleCmpLevelZero_3 && t <= Intrinsic::Texture_SampleCmpLevelZero_5);
}

bool IsTextureLoadIntrinsic(const Intrinsic t)
{
    return (t >= Intrinsic::Texture_Load_1 && t <= Intrinsic::Texture_Load_3);
}

Intrinsic CompareOpToIntrinsic(const BinaryOp op)
{
    switch (op)
    {
        case BinaryOp::Equal:           return Intrinsic::Equal;
        case BinaryOp::NotEqual:        return Intrinsic::NotEqual;
        case BinaryOp::Less:            return Intrinsic::LessThan;
        case BinaryOp::Greater:         return Intrinsic::GreaterThan;
        case BinaryOp::LessEqual:       return Intrinsic::LessThanEqual;
        case BinaryOp::GreaterEqual:    return Intrinsic::GreaterThanEqual;
        default:                        return Intrinsic::Undefined;
    }
}

Intrinsic InterlockedToImageAtomicIntrinsic(const Intrinsic t)
{
    switch (t)
    {
        case Intrinsic::InterlockedAdd:             return Intrinsic::Image_AtomicAdd;
        case Intrinsic::InterlockedAnd:             return Intrinsic::Image_AtomicAnd;
        case Intrinsic::InterlockedOr:              return Intrinsic::Image_AtomicOr;
        case Intrinsic::InterlockedXor:             return Intrinsic::Image_AtomicXor;
        case Intrinsic::InterlockedMin:             return Intrinsic::Image_AtomicMin;
        case Intrinsic::InterlockedMax:             return Intrinsic::Image_AtomicMax;
        case Intrinsic::InterlockedCompareExchange: return Intrinsic::Image_AtomicCompSwap;
        case Intrinsic::InterlockedExchange:        return Intrinsic::Image_AtomicExchange;
        default:                                    return t;
    }
}


/* ----- Gather intrinsics ----- */

struct GatherIntrinsicInfo
{
    GatherIntrinsicInfo(int componentIdx, int offsetCount, bool isCompare = false) :
        componentIdx { componentIdx },
        offsetCount  { offsetCount  },
        isCompare    { isCompare    }
    {
    }

    int     componentIdx    = 0;
    int     offsetCount     = 0;
    bool    isCompare       = false;
};

static std::map<Intrinsic, GatherIntrinsicInfo> GenerateGatherIntrinsicInfoMap()
{
    using T = Intrinsic;

    return
    {
        { T::Texture_Gather_2,          { 0, 0       } },
        { T::Texture_GatherRed_2,       { 0, 0       } },
        { T::Texture_GatherGreen_2,     { 1, 0       } },
        { T::Texture_GatherBlue_2,      { 2, 0       } },
        { T::Texture_GatherAlpha_2,     { 3, 0       } },

        { T::Texture_Gather_3,          { 0, 1       } },
        { T::Texture_Gather_4,          { 0, 1       } },
        { T::Texture_GatherRed_3,       { 0, 1       } },
        { T::Texture_GatherRed_4,       { 0, 1       } },
        { T::Texture_GatherGreen_3,     { 1, 1       } },
        { T::Texture_GatherGreen_4,     { 1, 1       } },
        { T::Texture_GatherBlue_3,      { 2, 1       } },
        { T::Texture_GatherBlue_4,      { 2, 1       } },
        { T::Texture_GatherAlpha_3,     { 3, 1       } },
        { T::Texture_GatherAlpha_4,     { 3, 1       } },

        { T::Texture_GatherRed_6,       { 0, 4       } },
        { T::Texture_GatherRed_7,       { 0, 4       } },
        { T::Texture_GatherGreen_6,     { 1, 4       } },
        { T::Texture_GatherGreen_7,     { 1, 4       } },
        { T::Texture_GatherBlue_6,      { 2, 4       } },
        { T::Texture_GatherBlue_7,      { 2, 4       } },
        { T::Texture_GatherAlpha_6,     { 3, 4       } },
        { T::Texture_GatherAlpha_7,     { 3, 4       } },

        { T::Texture_GatherCmp_3,       { 0, 0, true } },
        { T::Texture_GatherCmpRed_3,    { 0, 0, true } },
        { T::Texture_GatherCmpGreen_3,  { 1, 0, true } },
        { T::Texture_GatherCmpBlue_3,   { 2, 0, true } },
        { T::Texture_GatherCmpAlpha_3,  { 3, 0, true } },

        { T::Texture_GatherCmp_4,       { 0, 1, true } },
        { T::Texture_GatherCmp_5,       { 0, 1, true } },
        { T::Texture_GatherCmpRed_4,    { 0, 1, true } },
        { T::Texture_GatherCmpRed_5,    { 0, 1, true } },
        { T::Texture_GatherCmpGreen_4,  { 1, 1, true } },
        { T::Texture_GatherCmpGreen_5,  { 1, 1, true } },
        { T::Texture_GatherCmpBlue_4,   { 2, 1, true } },
        { T::Texture_GatherCmpBlue_5,   { 2, 1, true } },
        { T::Texture_GatherCmpAlpha_4,  { 3, 1, true } },
        { T::Texture_GatherCmpAlpha_5,  { 3, 1, true } },

        { T::Texture_GatherCmpRed_7,    { 0, 4, true } },
        { T::Texture_GatherCmpRed_8,    { 0, 4, true } },
        { T::Texture_GatherCmpGreen_7,  { 1, 4, true } },
        { T::Texture_GatherCmpGreen_8,  { 1, 4, true } },
        { T::Texture_GatherCmpBlue_7,   { 2, 4, true } },
        { T::Texture_GatherCmpBlue_8,   { 2, 4, true } },
        { T::Texture_GatherCmpAlpha_7,  { 3, 4, true } },
        { T::Texture_GatherCmpAlpha_8,  { 3, 4, true } },
    };
}

static const auto g_gatherIntrinsicInfoMap = GenerateGatherIntrinsicInfoMap();

int GetGatherIntrinsicOffsetParamCount(const Intrinsic t)
{
    auto it = g_gatherIntrinsicInfoMap.find(t);
    if (it != g_gatherIntrinsicInfoMap.end())
        return it->second.offsetCount;
    else
        return 0;
}

int GetGatherIntrinsicComponentIndex(const Intrinsic t)
{
    auto it = g_gatherIntrinsicInfoMap.find(t);
    if (it != g_gatherIntrinsicInfoMap.end())
        return it->second.componentIdx;
    else
        return 0;
}


/* ----- IndexedSemantic Class ----- */

IndexedSemantic::IndexedSemantic(Semantic semantic, int index) :
    semantic_ { semantic },
    index_    { index    }
{
}

IndexedSemantic::IndexedSemantic(const std::string& userDefined) :
    semantic_ { Semantic::UserDefined }
{
    /* Extract index from user defined semantic (all right-most numeric characters) */
    auto pos = userDefined.find_last_not_of("0123456789");
    if (pos != std::string::npos && pos + 1 < userDefined.size())
    {
        ++pos;
        userDefined_ = userDefined.substr(0, pos);
        auto indexStr = userDefined.substr(pos);
        index_ = std::stoi(indexStr);
    }
    else
        userDefined_ = userDefined;
}

IndexedSemantic::IndexedSemantic(const IndexedSemantic& rhs, int index) :
    semantic_    { rhs.semantic_    },
    index_       { index            },
    userDefined_ { rhs.userDefined_ }
{
}

bool IndexedSemantic::operator < (const IndexedSemantic& rhs) const
{
    if (semantic_ > rhs.semantic_) return false;
    if (semantic_ < rhs.semantic_) return true;

    if (index_ > rhs.index_) return false;
    if (index_ < rhs.index_) return true;

    return userDefined_ < rhs.userDefined_;
}

bool IndexedSemantic::IsValid() const
{
    return (semantic_ != Semantic::Undefined);
}

bool IndexedSemantic::IsSystemValue() const
{
    return IsSystemSemantic(semantic_);
}

bool IndexedSemantic::IsUserDefined() const
{
    return IsUserSemantic(semantic_);
}

std::string IndexedSemantic::ToString() const
{
    std::string s;

    if (semantic_ == Semantic::UserDefined)
    {
        /* Return user defined semantics always in upper case */
        s = userDefined_;
        std::transform(s.begin(), s.end(), s.begin(), ::toupper);
    }
    else
        s = SemanticToString(semantic_);

    s += std::to_string(index_);

    return s;
}

void IndexedSemantic::Reset()
{
    semantic_   = Semantic::Undefined;
    index_      = 0;
    userDefined_.clear();
}

void IndexedSemantic::ResetIndex(int index)
{
    index_ = index;
}

void IndexedSemantic::MakeUserDefined(const std::string& semanticName)
{
    if (IsSystemSemantic(semantic_))
    {
        userDefined_ = (semanticName.empty() ? SemanticToString(semantic_) : semanticName);
        semantic_ = Semantic::UserDefined;
    }
}


/* ----- Semantic Enum ----- */

bool IsSystemSemantic(const Semantic t)
{
    return (t >= Semantic::ClipDistance && t <= Semantic::ViewportArrayIndex);
}

bool IsUserSemantic(const Semantic t)
{
    return (t == Semantic::UserDefined);
}

std::string SemanticToString(const Semantic t)
{
    #define CASE_TO_STRING(IDENT) case Semantic::IDENT: return ("SV_" + std::string(#IDENT))

    switch (t)
    {
        case Semantic::Undefined:
            return R_Undefined;
        case Semantic::UserDefined:
            return R_UserDefined;

        CASE_TO_STRING( ClipDistance           );
        CASE_TO_STRING( CullDistance           );
        CASE_TO_STRING( Coverage               );
        CASE_TO_STRING( Depth                  );
        CASE_TO_STRING( DepthGreaterEqual      );
        CASE_TO_STRING( DepthLessEqual         );
        CASE_TO_STRING( DispatchThreadID       );
        CASE_TO_STRING( DomainLocation         );
        CASE_TO_STRING( FragCoord              );
        CASE_TO_STRING( GroupID                );
        CASE_TO_STRING( GroupIndex             );
        CASE_TO_STRING( GroupThreadID          );
        CASE_TO_STRING( GSInstanceID           );
        CASE_TO_STRING( InnerCoverage          );
        CASE_TO_STRING( InsideTessFactor       );
        CASE_TO_STRING( InstanceID             );
        CASE_TO_STRING( IsFrontFace            );
        CASE_TO_STRING( OutputControlPointID   );
        CASE_TO_STRING( PointSize              );
        CASE_TO_STRING( PrimitiveID            );
        CASE_TO_STRING( RenderTargetArrayIndex );
        CASE_TO_STRING( SampleIndex            );
        CASE_TO_STRING( StencilRef             );
        CASE_TO_STRING( Target                 );
        CASE_TO_STRING( TessFactor             );
        CASE_TO_STRING( VertexID               );
        CASE_TO_STRING( VertexPosition         );
        CASE_TO_STRING( ViewportArrayIndex     );
    }
    return "";

    #undef CASE_TO_STRING
}


/* ----- Reflection::Filter Enum ----- */

namespace DetailsMap0
{

using T = Reflection::Filter;

static const std::map<std::string, T> g_mapFilter
{
    { "MIN_MAG_MIP_POINT",                          T::MinMagMipPoint                       },
    { "MIN_MAG_POINT_MIP_LINEAR",                   T::MinMagPointMipLinear                 },
    { "MIN_POINT_MAG_LINEAR_MIP_POINT",             T::MinPointMagLinearMipPoint            },
    { "MIN_POINT_MAG_MIP_LINEAR",                   T::MinPointMagMipLinear                 },
    { "MIN_LINEAR_MAG_MIP_POINT",                   T::MinLinearMagMipPoint                 },
    { "MIN_LINEAR_MAG_POINT_MIP_LINEAR",            T::MinLinearMagPointMipLinear           },
    { "MIN_MAG_LINEAR_MIP_POINT",                   T::MinMagLinearMipPoint                 },
    { "MIN_MAG_MIP_LINEAR",                         T::MinMagMipLinear                      },
    { "ANISOTROPIC",                                T::Anisotropic                          },
    { "COMPARISON_MIN_MAG_MIP_POINT",               T::ComparisonMinMagMipPoint             },
    { "COMPARISON_MIN_MAG_POINT_MIP_LINEAR",        T::ComparisonMinMagPointMipLinear       },
    { "COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT",  T::ComparisonMinPointMagLinearMipPoint  },
    { "COMPARISON_MIN_POINT_MAG_MIP_LINEAR",        T::ComparisonMinPointMagMipLinear       },
    { "COMPARISON_MIN_LINEAR_MAG_MIP_POINT",        T::ComparisonMinLinearMagMipPoint       },
    { "COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR", T::ComparisonMinLinearMagPointMipLinear },
    { "COMPARISON_MIN_MAG_LINEAR_MIP_POINT",        T::ComparisonMinMagLinearMipPoint       },
    { "COMPARISON_MIN_MAG_MIP_LINEAR",              T::ComparisonMinMagMipLinear            },
    { "COMPARISON_ANISOTROPIC",                     T::ComparisonAnisotropic                },
    { "MINIMUM_MIN_MAG_MIP_POINT",                  T::MinimumMinMagMipPoint                },
    { "MINIMUM_MIN_MAG_POINT_MIP_LINEAR",           T::MinimumMinMagPointMipLinear          },
    { "MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT",     T::MinimumMinPointMagLinearMipPoint     },
    { "MINIMUM_MIN_POINT_MAG_MIP_LINEAR",           T::MinimumMinPointMagMipLinear          },
    { "MINIMUM_MIN_LINEAR_MAG_MIP_POINT",           T::MinimumMinLinearMagMipPoint          },
    { "MINIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR",    T::MinimumMinLinearMagPointMipLinear    },
    { "MINIMUM_MIN_MAG_LINEAR_MIP_POINT",           T::MinimumMinMagLinearMipPoint          },
    { "MINIMUM_MIN_MAG_MIP_LINEAR",                 T::MinimumMinMagMipLinear               },
    { "MINIMUM_ANISOTROPIC",                        T::MinimumAnisotropic                   },
    { "MAXIMUM_MIN_MAG_MIP_POINT",                  T::MaximumMinMagMipPoint                },
    { "MAXIMUM_MIN_MAG_POINT_MIP_LINEAR",           T::MaximumMinMagPointMipLinear          },
    { "MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT",     T::MaximumMinPointMagLinearMipPoint     },
    { "MAXIMUM_MIN_POINT_MAG_MIP_LINEAR",           T::MaximumMinPointMagMipLinear          },
    { "MAXIMUM_MIN_LINEAR_MAG_MIP_POINT",           T::MaximumMinLinearMagMipPoint          },
    { "MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR",    T::MaximumMinLinearMagPointMipLinear    },
    { "MAXIMUM_MIN_MAG_LINEAR_MIP_POINT",           T::MaximumMinMagLinearMipPoint          },
    { "MAXIMUM_MIN_MAG_MIP_LINEAR",                 T::MaximumMinMagMipLinear               },
    { "MAXIMUM_ANISOTROPIC",                        T::MaximumAnisotropic                   },
};

} // /namespace DetailsMap0

std::string FilterToString(const Reflection::Filter t)
{
    return TypeToStringSecondary(DetailsMap0::g_mapFilter, t, "SamplerState::Filter");
}

Reflection::Filter StringToFilter(const std::string& s)
{
    return StringToTypeSecondary(DetailsMap0::g_mapFilter, s, "SamplerState::Filter");
}


/* ----- SamplerState::TextureAddressMode Enum ----- */

namespace DetailsMap1
{

using T = Reflection::TextureAddressMode;

static const std::map<std::string, T> g_mapTexAddressMode
{
    { "WRAP",        T::Wrap       },
    { "MIRROR",      T::Mirror     },
    { "CLAMP",       T::Clamp      },
    { "BORDER",      T::Border     },
    { "MIRROR_ONCE", T::MirrorOnce },
};

} // /namespace DetailsMap1

std::string TexAddressModeToString(const Reflection::TextureAddressMode t)
{
    return TypeToStringSecondary(DetailsMap1::g_mapTexAddressMode, t, "SamplerState::TextureAddressMode");
}

Reflection::TextureAddressMode StringToTexAddressMode(const std::string& s)
{
    return StringToTypeSecondary(DetailsMap1::g_mapTexAddressMode, s, "SamplerState::TextureAddressMode");
}


/* ----- SamplerState::ComparisonFunc Enum ----- */

namespace DetailsMap2
{

using T = Reflection::ComparisonFunc;

static const std::map<std::string, T> g_mapCompareFunc
{
    { "NEVER",         T::Never        },
    { "LESS",          T::Less         },
    { "EQUAL",         T::Equal        },
    { "LESS_EQUAL",    T::LessEqual    },
    { "GREATER",       T::Greater      },
    { "NOT_EQUAL",     T::NotEqual     },
    { "GREATER_EQUAL", T::GreaterEqual },
    { "ALWAYS",        T::Always       },
};

} // /namespace DetailsMap2

std::string CompareFuncToString(const Reflection::ComparisonFunc t)
{
    return TypeToStringSecondary(DetailsMap2::g_mapCompareFunc, t, "SamplerState::ComparisonFunc");
}

Reflection::ComparisonFunc StringToCompareFunc(const std::string& s)
{
    return StringToTypeSecondary(DetailsMap2::g_mapCompareFunc, s, "SamplerState::ComparisonFunc");
}


} // /namespace Xsc



// ================================================================================
