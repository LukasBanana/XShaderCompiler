/*
 * GLSLKeywords.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLSLKeywords.h"
#include <set>
#include <map>


namespace Xsc
{


static std::set<std::string> GenerateKeywordSet()
{
    return
    {
        "main",
        //TODO: add all GLSL keywords ...
    };
}

static const std::set<std::string> g_keywordSetGLSL = GenerateKeywordSet();

bool IsGLSLKeyword(const std::string& ident)
{
    return (g_keywordSetGLSL.find(ident) != g_keywordSetGLSL.end());
}

using DataTypeMap = std::map<DataType, std::string>;

static DataTypeMap GenerateDataTypeMap()
{
    return
    {
        /* Scalar types */
        { DataType::Bool,   "bool"   },
        { DataType::Int,    "int"    },
        { DataType::UInt,   "uint"   },
        { DataType::Half,   "float"  },
        { DataType::Float,  "float"  },
        { DataType::Double, "double" },

        /* Vector types */
        { DataType::Bool2,   "bvec2" },
        { DataType::Bool3,   "bvec3" },
        { DataType::Bool4,   "bvec4" },
        { DataType::Int2,    "ivec2" },
        { DataType::Int3,    "ivec3" },
        { DataType::Int4,    "ivec4" },
        { DataType::UInt2,   "uvec2" },
        { DataType::UInt3,   "uvec3" },
        { DataType::UInt4,   "uvec4" },
        { DataType::Half2,   "vec2"  },
        { DataType::Half3,   "vec3"  },
        { DataType::Half4,   "vec4"  },
        { DataType::Float2,  "vec2"  },
        { DataType::Float3,  "vec3"  },
        { DataType::Float4,  "vec4"  },
        { DataType::Double2, "dvec2" },
        { DataType::Double3, "dvec3" },
        { DataType::Double4, "dvec4" },

        /* Matrix types */
        { DataType::Float2x2,  "mat2"   },
        { DataType::Float2x3,  "mat2x3" },
        { DataType::Float2x4,  "mat2x4" },
        { DataType::Float3x2,  "mat3x2" },
        { DataType::Float3x3,  "mat3"   },
        { DataType::Float3x4,  "mat3x4" },
        { DataType::Float4x2,  "mat4x2" },
        { DataType::Float4x3,  "mat4x3" },
        { DataType::Float4x4,  "mat4"   },
        { DataType::Double2x2, "mat2"   },
        { DataType::Double2x3, "mat2x3" },
        { DataType::Double2x4, "mat2x4" },
        { DataType::Double3x2, "mat3x2" },
        { DataType::Double3x3, "mat3"   },
        { DataType::Double3x4, "mat3x4" },
        { DataType::Double4x2, "mat4x2" },
        { DataType::Double4x3, "mat4x3" },
        { DataType::Double4x4, "mat4"   },
    };
}

static const DataTypeMap g_dataTypeMapGLSL = GenerateDataTypeMap();

const std::string* DataTypeToGLSLKeyword(const DataType dataType)
{
    auto it = g_dataTypeMapGLSL.find(dataType);
    return (it != g_dataTypeMapGLSL.end() ? &(it->second) : nullptr);
}


} // /namespace Xsc



// ================================================================================