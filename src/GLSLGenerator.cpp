/*
 * GLSLGenerator.cpp
 * 
 * This file is part of the "HLSL Translator" (Copyright (c) 2014 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLSLGenerator.h"

#include <ctime>
#include <chrono>


namespace HTLib
{


/*
 * Internal functions
 */

static std::string TimePoint()
{
    /* Determine current time point */
    const auto currentTime = std::chrono::high_resolution_clock::now();
    const auto duration = std::chrono::duration_cast<std::chrono::seconds>(currentTime.time_since_epoch());
    const auto date = duration.count();
    
    /* Get time point as string */
    auto timePoint = std::string(std::ctime(&date));

    /* Remove new-line character at the end */
    if (!timePoint.empty() && timePoint.back() == '\n')
        timePoint.resize(timePoint.size() - 1);

    return timePoint;
}

static std::string TargetToString(const ShaderTargets shaderTarget)
{
    switch (shaderTarget)
    {
        case ShaderTargets::GLSLVertexShader:
            return "Vertex";
        case ShaderTargets::GLSLFragmentShader:
            return "Fragment";
        case ShaderTargets::GLSLGeometryShader:
            return "Geometry";
        case ShaderTargets::GLSLTessControlShader:
            return "Tessellation Control";
        case ShaderTargets::GLSLTessEvaluationShader:
            return "Tessellation Evaluation";
        case ShaderTargets::GLSLComputeShader:
            return "Compute";
    }
    return "";
}


/*
 * GLSLGenerator class
 */

GLSLGenerator::GLSLGenerator(Logger* log, IncludeHandler* includeHandler, const Options& options) :
    writer_         { options.indent },
    includeHandler_ { includeHandler },
    log_            { log            }
{
    EstablishMaps();
}

bool GLSLGenerator::GenerateCode(
    std::ostream& output,
    const std::string& entryPoint,
    const ShaderTargets shaderTarget,
    const ShaderVersions shaderVersion)
{
    try
    {
        writer_.OutputStream(output);

        /* Write header */
        Comment("GLSL " + TargetToString(shaderTarget) + " Shader");
        Comment("Generated from HLSL Shader \"" + entryPoint + "\"");
        Comment(TimePoint());
        Version(static_cast<int>(shaderVersion));

        //...
    }
    catch (const std::exception& err)
    {
        if (log_)
            log_->Error(err.what());
        return false;
    }
    return true;
}


/*
 * ======= Private: =======
 */

void GLSLGenerator::EstablishMaps()
{
    typeMap_ = std::map<std::string, std::string>
    {
        /* Scalar types */
        { "bool",   "bool"  },
        { "int",    "int"   },
        { "uint",   "uint"  },
        { "half",   "float" },
        { "float",  "float" },
        { "double", "dvec"  },

        /* Vector types */
        { "bool2",   "bvec2" },
        { "bool3",   "bvec3" },
        { "bool4",   "bvec4" },
        { "int2",    "ivec2" },
        { "int3",    "ivec3" },
        { "int4",    "ivec4" },
        { "uint2",   "uvec2" },
        { "uint3",   "uvec3" },
        { "uint4",   "uvec4" },
        { "half2",   "vec2"  },
        { "half3",   "vec3"  },
        { "half4",   "vec4"  },
        { "float2",  "vec2"  },
        { "float3",  "vec3"  },
        { "float4",  "vec4"  },
        { "double2", "dvec2" },
        { "double3", "dvec3" },
        { "double4", "dvec4" },

        /* Matrix types */
        { "float2x2",  "mat2"   },
        { "float3x3",  "mat3"   },
        { "float4x4",  "mat4"   },
        { "float2x3",  "mat2x3" },
        { "float2x4",  "mat2x4" },
        { "float3x2",  "mat3x2" },
        { "float3x4",  "mat3x4" },
        { "float4x2",  "mat4x2" },
        { "float4x3",  "mat4x3" },
        { "double2x2", "mat2"   },
        { "double3x3", "mat3"   },
        { "double4x4", "mat4"   },
        { "double2x3", "mat2x3" },
        { "double2x4", "mat2x4" },
        { "double3x2", "mat3x2" },
        { "double3x4", "mat3x4" },
        { "double4x2", "mat4x2" },
        { "double4x3", "mat4x3" },

        /* Texture types */
        { "Texture1D",        "sampler1D"        },
        { "Texture1DArray",   "sampler1DArray"   },
        { "Texture2D",        "sampler2D"        },
        { "Texture2DArray",   "sampler2DArray"   },
        { "Texture3D",        "sampler3D"        },
        { "TextureCube",      "samplerCube"      },
        { "TextureCubeArray", "samplerCubeArray" },
        { "Texture2DMS",      "sampler2DMS"      },
        { "Texture2DMSArray", "sampler2DMSArray" },
    };

    intrinsicMap_ = std::map<std::string, std::string>
    {
        { "frac",  "fract" },
        { "lerp",  "mix"   },
        { "ddx",   "dFdx"  },
        { "ddy",   "dFdy"  },
        { "atan2", "atan"  },
    };

    semanticMap_ = std::map<std::string, SemanticStage>
    {
        { "SV_ClipDistance",            { "gl_ClipDistance"                             } },
        { "SV_CullDistance",            { "gl_CullDistance"                             } },
      //{ "SV_Coverage",                { "???"                                         } },
        { "SV_Depth",                   { "gl_FragDepth"                                } },
        { "SV_DispatchThreadID",        { "gl_GlobalInvocationID"                       } },
        { "SV_DomainLocation",          { "gl_TessCoord"                                } },
        { "SV_GroupID",                 { "gl_WorkGroupID"                              } },
        { "SV_GroupIndex",              { "gl_LocalInvocationIndex"                     } },
        { "SV_GroupThreadID",           { "gl_LocalInvocationID"                        } },
        { "SV_GSInstanceID",            { "gl_InvocationID"                             } },
        { "SV_InsideTessFactor",        { "gl_Position"                                 } },
        { "SV_IsFrontFace",             { "gl_FrontFacing"                              } },
        { "SV_OutputControlPointID",    { "gl_PrimitiveID"                              } },
        { "SV_Position",                { "gl_Position", "", "", "", "gl_FragCoord", "" } },
      //{ "SV_RenderTargetArrayIndex",  { "???"                                         } },
        { "SV_SampleIndex",             { "gl_SampleID"                                 } },
        { "SV_Target",                  { "gl_FragColor"                                } },
        { "SV_TessFactor",              { "gl_Position"                                 } },
        { "SV_ViewportArrayIndex",      { "gl_ViewportIndex"                            } },
        { "SV_InstanceID",              { "gl_InstanceID"                               } },
        { "SV_PrimitiveID",             { "gl_PrimitiveID"                              } },
        { "SV_VertexID",                { "gl_VertexID"                                 } },
    };
}

void GLSLGenerator::BeginLn()
{
    writer_.BeginLine();
}

void GLSLGenerator::EndLn()
{
    writer_.EndLine();
}

void GLSLGenerator::Write(const std::string& text)
{
    writer_.Write(text);
}

void GLSLGenerator::WriteLn(const std::string& text)
{
    writer_.WriteLine(text);
}

void GLSLGenerator::IncTab()
{
    writer_.PushIndent();
}

void GLSLGenerator::DecTab()
{
    writer_.PopIndent();
}

void GLSLGenerator::Comment(const std::string& text)
{
    WriteLn("// " + text);
}

void GLSLGenerator::Version(int versionNumber)
{
    WriteLn("#version " + std::to_string(versionNumber));
}

void GLSLGenerator::Line(int lineNumber)
{
    WriteLn("#line " + std::to_string(lineNumber));
}

void GLSLGenerator::Line(const TokenPtr& tkn)
{
    Line(tkn->Pos().Row());
}

void GLSLGenerator::OpenScope()
{
    WriteLn("{");
    IncTab();
}

void GLSLGenerator::CloseScope()
{
    DecTab();
    WriteLn("}");
}


/*
 * SemanticStage structure
 */

GLSLGenerator::SemanticStage::SemanticStage(const std::string& semantic) :
    vertex          { semantic },
    geometry        { semantic },
    tessControl     { semantic },
    tessEvaluation  { semantic },
    fragment        { semantic },
    compute         { semantic }
{
}

GLSLGenerator::SemanticStage::SemanticStage(
    const std::string& vertex,
    const std::string& geometry,
    const std::string& tessControl,
    const std::string& tessEvaluation,
    const std::string& fragment,
    const std::string& compute) :
        vertex          { vertex         },
        geometry        { geometry       },
        tessControl     { tessControl    },
        tessEvaluation  { tessEvaluation },
        fragment        { fragment       },
        compute         { compute        }
{
}


} // /namespace HTLib



// ================================================================================