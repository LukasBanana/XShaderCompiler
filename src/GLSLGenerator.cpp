/*
 * GLSLGenerator.cpp
 * 
 * This file is part of the "HLSL Translator" (Copyright (c) 2014 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLSLGenerator.h"
#include "HLSLAnalyzer.h"
#include "HLSLTree.h"
#include "HLSLKeywords.h"

#include <ctime>
#include <chrono>
#include <initializer_list>
#include <algorithm>
#include <cctype>


namespace HTLib
{


/*
 * Internal members
 */

static const std::string interfaceBlockPrefix = "_I";


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
        case ShaderTargets::CommonShader:
            return "Shader";
        case ShaderTargets::GLSLVertexShader:
            return "Vertex Shader";
        case ShaderTargets::GLSLFragmentShader:
            return "Fragment Shader";
        case ShaderTargets::GLSLGeometryShader:
            return "Geometry Shader";
        case ShaderTargets::GLSLTessControlShader:
            return "Tessellation Control Shader";
        case ShaderTargets::GLSLTessEvaluationShader:
            return "Tessellation Evaluation Shader";
        case ShaderTargets::GLSLComputeShader:
            return "Compute Shader";
    }
    return "";
}

static inline std::vector<std::string> StringList(const std::initializer_list<std::string>& list)
{
    return list;
}


/*
 * GLSLGenerator class
 */

GLSLGenerator::GLSLGenerator(Logger* log, IncludeHandler* includeHandler, const Options& options) :
    writer_         { options.indent    },
    includeHandler_ { includeHandler    },
    log_            { log               },
    localVarPrefix_ { options.prefix    },
    allowBlanks_    { options.blanks    },
    allowLineMarks_ { options.lineMarks }
{
    EstablishMaps();
}

bool GLSLGenerator::GenerateCode(
    Program* program,
    std::ostream& output,
    const std::string& entryPoint,
    const ShaderTargets shaderTarget,
    const InputShaderVersions versionIn,
    const OutputShaderVersions versionOut)
{
    if (!program)
        return false;

    /* Store parameters */
    entryPoint_     = entryPoint;
    shaderTarget_   = shaderTarget;
    versionIn_      = versionIn;
    versionOut_     = versionOut;

    try
    {
        writer_.OutputStream(output);

        /* Write header */
        Comment("GLSL " + TargetToString(shaderTarget));
        
        if (entryPoint.empty())
            Comment("Generated from HLSL Shader");
        else
            Comment("Generated from HLSL Shader \"" + entryPoint + "\"");

        Comment(TimePoint());
        Blank();

        if (shaderTarget_ != ShaderTargets::CommonShader)
        {
            Version(static_cast<int>(versionOut_));
            Blank();
        }

        /* Visit program AST */
        Visit(program);
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
        { "bool",      "bool"   },
        { "bool1",     "bool"   },
        { "bool1x1",   "bool"   },
        { "int",       "int"    },
        { "int1",      "int"    },
        { "int1x1",    "int"    },
        { "uint",      "uint"   },
        { "uint1",     "uint"   },
        { "uint1x1",   "uint"   },
        { "half",      "float"  },
        { "half1",     "float"  },
        { "half1x1",   "float"  },
        { "float",     "float"  },
        { "float1",    "float"  },
        { "float1x1",  "float"  },
        { "double",    "double" },
        { "double1",   "double" },
        { "double1x1", "double" },

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
        { "float2x3",  "mat2x3" },
        { "float2x4",  "mat2x4" },
        { "float3x2",  "mat3x2" },
        { "float3x3",  "mat3"   },
        { "float3x4",  "mat3x4" },
        { "float4x2",  "mat4x2" },
        { "float4x3",  "mat4x3" },
        { "float4x4",  "mat4"   },
        { "double2x2", "mat2"   },
        { "double2x3", "mat2x3" },
        { "double2x4", "mat2x4" },
        { "double3x2", "mat3x2" },
        { "double3x3", "mat3"   },
        { "double3x4", "mat3x4" },
        { "double4x2", "mat4x2" },
        { "double4x3", "mat4x3" },
        { "double4x4", "mat4"   },

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
        /*{ "RWTexture1D",      "" },
        { "RWTexture1DArray", "" },
        { "RWTexture2D",      "" },
        { "RWTexture2DArray", "" },
        { "RWTexture3D",      "" },*/

        /* Storage class types */
        { "groupshared", "shared" },
    };

    intrinsicMap_ = std::map<std::string, std::string>
    {
        { "frac",                            "fract"              },
        { "rsqrt",                           "inversesqrt"        },
        { "lerp",                            "mix"                },
        { "saturate",                        "clamp"              },
        { "ddx",                             "dFdx"               },
        { "ddy",                             "dFdy"               },
        { "ddx_coarse",                      "dFdxCoarse"         },
        { "ddy_coarse",                      "dFdyCoarse"         },
        { "ddx_fine",                        "dFdxFine"           },
        { "ddy_fine",                        "dFdyFine"           },
        { "atan2",                           "atan"               },
        { "GroupMemoryBarrier",              "groupMemoryBarrier" },
        { "GroupMemoryBarrierWithGroupSync", "barrier"            },
        { "AllMemoryBarrier",                "memoryBarrier"      },
        { "AllMemoryBarrierWithGroupSync",   "barrier"            },
    };

    atomicIntrinsicMap_ = std::map<std::string, std::string>
    {
        { "InterlockedAdd",             "atomicAdd"      },
        { "InterlockedAnd",             "atomicAnd"      },
        { "InterlockedOr",              "atomicOr"       },
        { "InterlockedXor",             "atomicXor"      },
        { "InterlockedMin",             "atomicMin"      },
        { "InterlockedMax",             "atomicMax"      },
        { "InterlockedCompareExchange", "atomicCompSwap" },
        { "InterlockedExchange",        "atomicExchange" },
    };

    modifierMap_ = std::map<std::string, std::string>
    {
        { "linear",          "smooth"        },
        { "centroid",        "centroid"      },
        { "nointerpolation", "flat"          },
        { "noperspective",   "noperspective" },
        { "sample",          "sample"        },
    };

    texFuncMap_ = std::map<std::string, std::string>
    {
        { "GetDimensions ",     "textureSize"   },
        { "Load",               "texelFetch"    },
        { "Sample",             "texture"       },
        { "SampleBias",         "textureOffset" },
        //{ "SampleCmp", "" },
        //{ "SampleCmpLevelZero", "" },
        { "SampleGrad",         "textureGrad"   },
        { "SampleLevel",        "textureLod"    },
    };

    semanticMap_ = std::map<std::string, SemanticStage>
    {
        { "SV_CLIPDISTANCE",            { "gl_ClipDistance"                             } },
        { "SV_CULLDISTANCE",            { "gl_CullDistance"                             } },
      //{ "SV_COVERAGE",                { "???"                                         } },
        { "SV_DEPTH",                   { "gl_FragDepth"                                } },
        { "SV_DISPATCHTHREADID",        { "gl_GlobalInvocationID"                       } },
        { "SV_DOMAINLOCATION",          { "gl_TessCoord"                                } },
        { "SV_GROUPID",                 { "gl_WorkGroupID"                              } },
        { "SV_GROUPINDEX",              { "gl_LocalInvocationIndex"                     } },
        { "SV_GROUPTHREADID",           { "gl_LocalInvocationID"                        } },
        { "SV_GSINSTANCEID",            { "gl_InvocationID"                             } },
        { "SV_INSIDETESSFACTOR",        { "gl_Position"                                 } },
        { "SV_ISFRONTFACE",             { "gl_FrontFacing"                              } },
        { "SV_OUTPUTCONTROLPOINTID",    { "gl_PrimitiveID"                              } },
        { "SV_POSITION",                { "gl_Position", "", "", "", "gl_FragCoord", "" } },
      //{ "SV_RENDERTARGETARRAYINDEX",  { "???"                                         } },
        { "SV_SAMPLEINDEX",             { "gl_SampleID"                                 } },
        { "SV_TARGET",                  { "gl_FragColor"                                } },
        { "SV_TESSFACTOR",              { "gl_Position"                                 } },
        { "SV_VIEWPORTARRAYINDEX",      { "gl_ViewportIndex"                            } },
        { "SV_INSTANCEID",              { "gl_InstanceID"                               } },
        { "SV_PRIMITIVEID",             { "gl_PrimitiveID"                              } },
        { "SV_VERTEXID",                { "gl_VertexID"                                 } },
    };
}

void GLSLGenerator::Error(const std::string& msg, const AST* ast)
{
    if (ast)
        throw std::runtime_error("code generation error (" + ast->pos.ToString() + ") : " + msg);
    else
        throw std::runtime_error("code generation error : " + msg);
}

void GLSLGenerator::ErrorInvalidNumArgs(const std::string& functionName, const AST* ast)
{
    Error("invalid number of arguments for " + functionName, ast);
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

void GLSLGenerator::PushOptions(const CodeWriter::Options& options)
{
    writer_.PushOptions(options);
}

void GLSLGenerator::PopOptions()
{
    writer_.PopOptions();
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
    if (allowLineMarks_)
        WriteLn("#line " + std::to_string(lineNumber));
}

void GLSLGenerator::Line(const TokenPtr& tkn)
{
    Line(tkn->Pos().Row());
}

void GLSLGenerator::Line(const AST* ast)
{
    Line(ast->pos.Row());
}

void GLSLGenerator::Blank()
{
    if (allowBlanks_)
        WriteLn("");
}

void GLSLGenerator::Extension(const std::string& extensionName)
{
    WriteLn("#extension " + extensionName + " : enable");// "require" or "enable"
}

void GLSLGenerator::AppendRequiredExtensions(Program* ast)
{
    for (const auto& ext : ast->requiredExtensions)
        Extension(ext);
    Blank();
}

void GLSLGenerator::AppendCommonMacros()
{
    //WriteLn("#define clip(x) if (lessThan(x, 0.0)) { discard; }");
    //Blank();
}

#if 0

/*
Remove this if it is clear, that it will never be used!
*/

void GLSLGenerator::AppendMulIntrinsics()
{
    WriteLn("mat2 mul(mat2 m, vec2 v) { return m * v; }");
    WriteLn("mat2 mul(vec2 v, mat2 m) { return v * m; }");
    WriteLn("mat2 mul(mat2 a, mat2 b) { return a * b; }");

    WriteLn("mat3 mul(mat3 m, vec3 v) { return m * v; }");
    WriteLn("mat3 mul(vec3 v, mat3 m) { return v * m; }");
    WriteLn("mat3 mul(mat3 a, mat3 b) { return a * b; }");

    WriteLn("mat4 mul(mat4 m, vec4 v) { return m * v; }");
    WriteLn("mat4 mul(vec4 v, mat4 m) { return v * m; }");
    WriteLn("mat4 mul(mat4 a, mat4 b) { return a * b; }");

    Blank();
}

#endif

void GLSLGenerator::AppendRcpIntrinsics()
{
    WriteLn("float rcp(float x) { return 1.0 / x; }");
    WriteLn("double rcp(double x) { return 1.0 / x; }");

    WriteLn("vec2 rcp(vec2 v) { return vec2(1.0 / v.x, 1.0 / v.y); }");
    WriteLn("dvec2 rcp(dvec2 v) { return dvec2(1.0 / v.x, 1.0 / v.y); }");

    WriteLn("vec3 rcp(vec3 v) { return vec3(1.0 / v.x, 1.0 / v.y, 1.0 / v.z); }");
    WriteLn("dvec3 rcp(dvec4 v) { return dvec3(1.0 / v.x, 1.0 / v.y, 1.0 / v.z); }");

    WriteLn("vec4 rcp(vec3 v) { return vec4(1.0 / v.x, 1.0 / v.y, 1.0 / v.z, 1.0 / v.w); }");
    WriteLn("dvec4 rcp(dvec4 v) { return dvec4(1.0 / v.x, 1.0 / v.y, 1.0 / v.z, 1.0 / v.w); }");

    WriteLn("mat2 rcp(mat2 m) { mat2 r; r[0] = rcp(m[0]); r[1] = rcp(m[1]); return r; }");
    WriteLn("mat3 rcp(mat3 m) { mat3 r; r[0] = rcp(m[0]); r[1] = rcp(m[1]); r[2] = rcp(m[2]); return r; }");
    WriteLn("mat4 rcp(mat4 m) { mat4 r; r[0] = rcp(m[0]); r[1] = rcp(m[1]); r[2] = rcp(m[2]); r[3] = rcp(m[3]); return r; }");

    Blank();
}

void GLSLGenerator::AppendClipIntrinsics()
{
    WriteLn("void clip(float x) { if (x < 0.0) discard; }");

    for (const auto& typeName : StringList({ "vec2", "vec3", "vec4" }))
        WriteLn("void clip(" + typeName + " x) { if (any(lessThan(x, " + typeName + "(0.0)))) discard; }");

    Blank();
}

void GLSLGenerator::AppendSinCosIntrinsics()
{
    for (const auto& typeName : StringList({ "float", "vec2", "vec3", "vec4" }))
    {
        WriteLn(
            "void sincos(" + typeName + " x, out " + typeName +
            " s, out " + typeName + " c) { s = sin(x); c = cos(x); }"
        );
    }
    Blank();
}

void GLSLGenerator::OpenScope()
{
    WriteLn("{");
    IncTab();
}

void GLSLGenerator::CloseScope(bool semicolon)
{
    DecTab();
    WriteLn(semicolon ? "};" : "}");
}

void GLSLGenerator::ValidateRegisterPrefix(const std::string& registerName, char prefix)
{
    if (registerName.empty() || registerName[0] != prefix)
    {
        Error(
            "invalid register prefix '" + std::string(1, registerName[0]) +
            "' (expected '" + std::string(1, prefix) + "')"
        );
    }
}

int GLSLGenerator::RegisterIndex(const std::string& registerName)
{
    return std::stoi(registerName.substr(1));
}

std::string GLSLGenerator::BRegister(const std::string& registerName)
{
    ValidateRegisterPrefix(registerName, 'b');
    return registerName.substr(1);
}

std::string GLSLGenerator::TRegister(const std::string& registerName)
{
    ValidateRegisterPrefix(registerName, 't');
    return registerName.substr(1);
}

std::string GLSLGenerator::SRegister(const std::string& registerName)
{
    ValidateRegisterPrefix(registerName, 's');
    return registerName.substr(1);
}

std::string GLSLGenerator::URegister(const std::string& registerName)
{
    ValidateRegisterPrefix(registerName, 'u');
    return registerName.substr(1);
}

bool GLSLGenerator::MustResolveStruct(Structure* ast) const
{
    return
        ( shaderTarget_ == ShaderTargets::GLSLVertexShader && ast->flags(Structure::isShaderInput) ) ||
        ( shaderTarget_ == ShaderTargets::GLSLFragmentShader && ast->flags(Structure::isShaderOutput) ) ||
        ( shaderTarget_ == ShaderTargets::GLSLComputeShader && ( ast->flags(Structure::isShaderInput) || ast->flags(Structure::isShaderOutput) ) );
}

bool GLSLGenerator::IsVersionOut(int version) const
{
    return static_cast<int>(versionOut_) >= version;
}

/* ------- Visit functions ------- */

#define IMPLEMENT_VISIT_PROC(className) \
    void GLSLGenerator::Visit##className(className* ast, void* args)

IMPLEMENT_VISIT_PROC(Program)
{
    program_ = ast;

    /* Append required extensions first */
    AppendRequiredExtensions(ast);

    /* Write 'gl_FragCoord' layout */
    if (shaderTarget_ == ShaderTargets::GLSLFragmentShader)
    {
        BeginLn();
        {
            Write("layout(origin_upper_left");
            if (program_->flags(Program::hasSM3ScreenSpace))
                Write(", pixel_center_integer");
            Write(") in vec4 gl_FragCoord;");
        }
        EndLn();
        Blank();
    }

    /* Append default helper macros and functions */
    AppendCommonMacros();

    if (ast->flags(Program::rcpIntrinsicUsed))
        AppendRcpIntrinsics();
    if (ast->flags(Program::clipIntrinsicUsed))
        AppendClipIntrinsics();
    if (ast->flags(Program::sinCosIntrinsicUsed))
        AppendSinCosIntrinsics();

    if (shaderTarget_ == ShaderTargets::GLSLFragmentShader)
        WriteFragmentShaderOutput();

    for (auto& globDecl : ast->globalDecls)
        Visit(globDecl);
}

IMPLEMENT_VISIT_PROC(CodeBlock)
{
    bool writeScope = (args != nullptr ? *reinterpret_cast<bool*>(args) : true);

    if (writeScope)
        OpenScope();
    
    for (auto& stmnt : ast->stmnts)
        Visit(stmnt);
    
    if (writeScope)
        CloseScope();
}

IMPLEMENT_VISIT_PROC(FunctionCall)
{
    if (ast->flags(FunctionCall::isMulFunc) && ast->arguments.size() == 2)
    {
        /* Convert this function call into a multiplication */
        Write("((");
        Visit(ast->arguments[0]);
        Write(") * (");
        Visit(ast->arguments[1]);
        Write("))");
    }
    else if (ast->flags(FunctionCall::isTexFunc) && ast->name->next)
    {
        /* Get function name */
        const auto& inFuncName = ast->name->next->ident;

        auto it = texFuncMap_.find(inFuncName);
        if (it == texFuncMap_.end())
            Error("texture member function \"" + inFuncName + "\" is not supported", ast);

        const auto& funcName = it->second;

        /* Write function call */
        Write(funcName + "(");
        
        for (size_t i = 0; i < ast->arguments.size(); ++i)
        {
            const auto& arg = ast->arguments[i];
            
            Visit(arg);
            if (i + 1 < ast->arguments.size())
                Write(", ");
        }

        Write(")");
    }
    else if (ast->flags(FunctionCall::isAtomicFunc) && ast->arguments.size() >= 2)
    {
        /* Find atomic intrinsic mapping */
        auto it = atomicIntrinsicMap_.find(ast->name->ident);
        if (it != atomicIntrinsicMap_.end())
        {
            /* Write function call */
            if (ast->arguments.size() >= 3)
            {
                Visit(ast->arguments[2]);
                Write(" = ");
            }
            Write(it->second + "(");
            Visit(ast->arguments[0]);
            Write(", ");
            Visit(ast->arguments[1]);
            Write(")");
        }
        else
            Error("unknown interlocked intrinsic \"" + ast->name->ident + "\"", ast);
    }
    else
    {
        /* Write function name */
        auto name = FullVarIdent(ast->name);

        auto it = intrinsicMap_.find(name);
        if (it != intrinsicMap_.end())
            Write(it->second);
        else
        {
            auto it = typeMap_.find(name);
            if (it != typeMap_.end())
                Write(it->second);
            else
                Visit(ast->name);
        }

        /*
        Remove arguments which contain a sampler state object,
        since GLSL does not support sampler states.
        --> Only "Texture2D" will be mapped to "sampler2D",
            but "SamplerState" can not be translated.
        */
        for (auto it = ast->arguments.begin(); it != ast->arguments.end();)
        {
            if (ExprContainsSampler(it->get()))
                it = ast->arguments.erase(it);
            else
                ++it;
        }

        /* Write arguments */
        Write("(");

        for (size_t i = 0; i < ast->arguments.size(); ++i)
        {
            Visit(ast->arguments[i]);
            if (i + 1 < ast->arguments.size())
                Write(", ");
        }

        /* Check for special cases */
        if (name == "saturate")
            Write(", 0.0, 1.0");

        Write(")");
    }
}

IMPLEMENT_VISIT_PROC(Structure)
{
    bool semicolon = (args != nullptr ? *reinterpret_cast<bool*>(&args) : false);

    /*
    Check if struct must be resolved:
    -> vertex shaders can not have input interface blocks and
       fragment shaders can not have output interface blocks
    -> see https://www.opengl.org/wiki/Interface_Block_%28GLSL%29#Input_and_output
    */
    auto resolveStruct = MustResolveStruct(ast);

    if ( resolveStruct || ( !ast->flags(Structure::isShaderInput) && !ast->flags(Structure::isShaderOutput) ) )
    {
        /* Write structure declaration */
        WriteLn("struct " + ast->name);

        OpenScope();
        {
            for (auto& varDecl : ast->members)
                Visit(varDecl);
        }
        CloseScope(semicolon);
    }

    /* Write structure members as global input/output variables (if structure must be resolved) */
    if (resolveStruct)
    {
        for (auto& member : ast->members)
        {
            /* Append struct input/output flag to member */
            if (ast->flags(Structure::isShaderInput))
                member->flags << VarDeclStmnt::isShaderInput;
            else if (ast->flags(Structure::isShaderOutput))
                member->flags << VarDeclStmnt::isShaderOutput;

            Visit(member);
        }
    }
    /* Write this structure as interface block (if structure doesn't need to be resolved) */
    else if (ast->flags(Structure::isShaderInput) || ast->flags(Structure::isShaderOutput))
    {
        BeginLn();
        {
            if (ast->flags(Structure::isShaderInput))
                Write("in");
            else
                Write("out");
            Write(" " + interfaceBlockPrefix + ast->name);
        }
        EndLn();

        OpenScope();
        {
            isInsideInterfaceBlock_ = true;

            for (auto& varDecl : ast->members)
                Visit(varDecl);

            isInsideInterfaceBlock_ = false;
        }
        CloseScope();

        WriteLn(ast->aliasName + ";");
    }
}

IMPLEMENT_VISIT_PROC(SwitchCase)
{
    /* Write case header */
    if (ast->expr)
    {
        BeginLn();
        {
            Write("case ");
            Visit(ast->expr);
            Write(":");
        }
        EndLn();
    }
    else
        WriteLn("default:");

    /* Write statement list */
    IncTab();
    {
        for (auto& stmnt : ast->stmnts)
            Visit(stmnt);
    }
    DecTab();
}

/* --- Global declarations --- */

IMPLEMENT_VISIT_PROC(FunctionDecl)
{
    if (!ast->flags(FunctionDecl::isReferenced) && shaderTarget_ != ShaderTargets::CommonShader)
        return; // function not used

    Line(ast);

    /* Write attributes */
    for (auto& attrib : ast->attribs)
        VisitAttribute(attrib.get());

    /* Write function header */
    BeginLn();
    {
        if (ast->flags(FunctionDecl::isEntryPoint))
            Write("void main()");
        else
        {
            Visit(ast->returnType);
            Write(" " + ast->name + "(");

            /*
            Remove parameters which contain a sampler state object,
            since GLSL does not support sampler states.
            --> Only "Texture2D" will be mapped to "sampler2D",
                but "SamplerState" can not be translated.
            */
            for (auto it = ast->parameters.begin(); it != ast->parameters.end();)
            {
                if (VarTypeIsSampler((*it)->varType.get()))
                    it = ast->parameters.erase(it);
                else
                    ++it;
            }

            /* Write parameters */
            for (size_t i = 0; i < ast->parameters.size(); ++i)
            {
                VisitParameter(ast->parameters[i].get());
                if (i + 1 < ast->parameters.size())
                    Write(", ");
            }

            Write(")");

            if (!ast->codeBlock)
            {
                /*
                This is only a function forward declaration
                -> finish with line terminator
                */
                Write(";");
            }
        }
    }
    EndLn();

    if (ast->codeBlock)
    {
        /* Write function body */
        if (ast->flags(FunctionDecl::isEntryPoint))
        {
            OpenScope();
            {
                /* Write input parameters as local variables */
                WriteEntryPointInputSemantics();

                /* Write code block (without additional scope) */
                isInsideEntryPoint_ = true;
                {
                    bool writeScope = false;
                    Visit(ast->codeBlock, &writeScope);
                }
                isInsideEntryPoint_ = false;
            }
            CloseScope();
        }
        else
        {
            /* Write default code block */
            Visit(ast->codeBlock);
        }
    }

    Blank();
}

IMPLEMENT_VISIT_PROC(UniformBufferDecl)
{
    if (!ast->flags(UniformBufferDecl::isReferenced) && shaderTarget_ != ShaderTargets::CommonShader)
        return; // uniform buffer not used

    /* Write uniform buffer header */
    Line(ast);

    BeginLn();
    {
        Write("layout(std140");

        if (!ast->registerName.empty())
            Write(", binding = " + BRegister(ast->registerName));

        Write(") uniform ");
        Write(ast->name);
    }
    EndLn();

    OpenScope();
    {
        for (auto& member : ast->members)
            Visit(member);
    }
    CloseScope(true);

    Blank();
}

IMPLEMENT_VISIT_PROC(TextureDecl)
{
    if (!ast->flags(TextureDecl::isReferenced) && shaderTarget_ != ShaderTargets::CommonShader)
        return; // texture not used

    /* Determine GLSL sampler type */
    auto it = typeMap_.find(ast->textureType);
    if (it == typeMap_.end())
        Error("texture type \"" + ast->textureType + "\" not supported yet", ast);

    auto samplerType = it->second;

    /* Write texture samplers */
    for (auto& name : ast->names)
    {
        if (name->flags(BufferDeclIdent::isReferenced) || shaderTarget_ == ShaderTargets::CommonShader)
        {
            BeginLn();
            {
                if (!name->registerName.empty())
                    Write("layout(binding = " + TRegister(name->registerName) + ") ");
                Write("uniform " + samplerType + " " + name->ident + ";");
            }
            EndLn();
        }
    }

    Blank();
}

IMPLEMENT_VISIT_PROC(StructDecl)
{
    if (!ast->structure->flags(Structure::isReferenced) && shaderTarget_ != ShaderTargets::CommonShader)
        return; // structure not used

    Line(ast);
    bool semicolon = true;
    Visit(ast->structure, &semicolon);

    Blank();
}

IMPLEMENT_VISIT_PROC(DirectiveDecl)
{
    WriteLn(ast->line);
}

/* --- Statements --- */

IMPLEMENT_VISIT_PROC(NullStmnt)
{
    WriteLn(";");
}

IMPLEMENT_VISIT_PROC(DirectiveStmnt)
{
    WriteLn(ast->line);
}

IMPLEMENT_VISIT_PROC(CodeBlockStmnt)
{
    Visit(ast->codeBlock);
}

IMPLEMENT_VISIT_PROC(ForLoopStmnt)
{
    /* Write loop header */
    BeginLn();
    {
        Write("for (");

        PushOptions({ false, false });
        {
            Visit(ast->initSmnt);
            Write(" "); // initStmnt already has the ';'!
            Visit(ast->condition);
            Write("; ");
            Visit(ast->iteration);
        }
        PopOptions();

        Write(")");
    }
    EndLn();

    VisitScopedStmnt(ast->bodyStmnt.get());
}

IMPLEMENT_VISIT_PROC(WhileLoopStmnt)
{
    /* Write loop condition */
    BeginLn();
    {
        Write("while (");
        Visit(ast->condition);
        Write(")");
    }
    EndLn();

    VisitScopedStmnt(ast->bodyStmnt.get());
}

IMPLEMENT_VISIT_PROC(DoWhileLoopStmnt)
{
    WriteLn("do");
    VisitScopedStmnt(ast->bodyStmnt.get());

    /* Write loop condition */
    BeginLn();
    {
        Write("while (");
        Visit(ast->condition);
        Write(");");
    }
    EndLn();
}

IMPLEMENT_VISIT_PROC(IfStmnt)
{
    bool hasElseParentNode = (args != nullptr ? *reinterpret_cast<bool*>(&args) : false);

    /* Write if condition */
    if (!hasElseParentNode)
        BeginLn();
    
    Write("if (");
    Visit(ast->condition);
    Write(")");
    
    EndLn();

    /* Write if body */
    VisitScopedStmnt(ast->bodyStmnt.get());

    Visit(ast->elseStmnt);
}

IMPLEMENT_VISIT_PROC(ElseStmnt)
{
    if (ast->bodyStmnt->Type() == AST::Types::IfStmnt)
    {
        /* Write else if statement */
        BeginLn();
        Write("else ");

        bool hasElseParentNode = true;
        Visit(ast->bodyStmnt, &hasElseParentNode);
    }
    else
    {
        /* Write else statement */
        WriteLn("else");
        VisitScopedStmnt(ast->bodyStmnt.get());
    }
}

IMPLEMENT_VISIT_PROC(SwitchStmnt)
{
    /* Write selector */
    BeginLn();
    {
        Write("switch (");
        Visit(ast->selector);
        Write(")");
    }
    EndLn();

    /* Write switch cases */
    OpenScope();
    {
        for (auto& switchCase : ast->cases)
            Visit(switchCase);
    }
    CloseScope();
}

IMPLEMENT_VISIT_PROC(VarDeclStmnt)
{
    auto varDecls = ast->varDecls;

    for (auto it = varDecls.begin(); it != varDecls.end();)
    {
        /*
        First check if code generation is disabled for variable declaration,
        then check if this is a system value semantic inside an interface block.
        */
        if ( (*it)->flags(VarDecl::disableCodeGen) ||
             ( isInsideInterfaceBlock_ && HasSystemValueSemantic((*it)->semantics) ) )
        {
            /*
            Code generation is disabled for this variable declaration
            -> Remove this from the list
            */
            it = varDecls.erase(it);
        }
        else
            ++it;
    }

    if (varDecls.empty())
    {
        /*
        All variable declarations within this statement are disabled
        -> Break code generation here
        */
        return;
    }

    BeginLn();

    /* Write modifiers */
    if (ast->flags(VarDeclStmnt::isShaderInput))
        Write("in ");
    else if (ast->flags(VarDeclStmnt::isShaderOutput))
        Write("out ");

    for (const auto& modifier : ast->storageModifiers)
    {
        auto it = modifierMap_.find(modifier);
        if (it != modifierMap_.end())
            Write(it->second + " ");
    }

    for (const auto& modifier : ast->typeModifiers)
    {
        if (modifier == "const")
            Write(modifier + " ");
    }

    /* Write variable type */
    if (ast->varType->structType)
    {
        EndLn();
        Visit(ast->varType);
        BeginLn();
    }
    else
    {
        Visit(ast->varType);
        Write(" ");
    }

    /* Write variable declarations */
    for (size_t i = 0; i < varDecls.size(); ++i)
    {
        Visit(varDecls[i]);
        if (i + 1 < varDecls.size())
            Write(", ");
    }

    Write(";");
    EndLn();
}

IMPLEMENT_VISIT_PROC(AssignStmnt)
{
    BeginLn();
    {
        WriteVarIdent(ast->varIdent.get());
        Write(" " + ast->op + " ");
        Visit(ast->expr);
        Write(";");
    }
    EndLn();
}

IMPLEMENT_VISIT_PROC(ExprStmnt)
{
    BeginLn();
    {
        Visit(ast->expr);
        Write(";");
    }
    EndLn();
}

IMPLEMENT_VISIT_PROC(FunctionCallStmnt)
{
    BeginLn();
    {
        Visit(ast->call);
        Write(";");
    }
    EndLn();
}

IMPLEMENT_VISIT_PROC(ReturnStmnt)
{
    if (isInsideEntryPoint_)
    {
        if (ast->expr)
        {
            OpenScope();
            {
                WriteEntryPointOutputSemantics(ast->expr.get());
                WriteLn("return;");
            }
            CloseScope();
        }
        else
            WriteLn("return;");
    }
    else
    {
        BeginLn();
        {
            Write("return");

            if (ast->expr)
            {
                Write(" ");
                Visit(ast->expr);
            }

            Write(";");
        }
        EndLn();
    }
}

IMPLEMENT_VISIT_PROC(CtrlTransferStmnt)
{
    WriteLn(ast->instruction + ";");
}

/* --- Expressions --- */

IMPLEMENT_VISIT_PROC(ListExpr)
{
    Visit(ast->firstExpr);
    Write(", ");
    Visit(ast->nextExpr);
}

IMPLEMENT_VISIT_PROC(LiteralExpr)
{
    Write(ast->literal);
}

IMPLEMENT_VISIT_PROC(TypeNameExpr)
{
    auto it = typeMap_.find(ast->typeName);
    if (it != typeMap_.end())
        Write(it->second);
    else
        Write(ast->typeName);
}

IMPLEMENT_VISIT_PROC(BinaryExpr)
{
    Visit(ast->lhsExpr);
    Write(" " + ast->op + " ");
    Visit(ast->rhsExpr);
}

IMPLEMENT_VISIT_PROC(UnaryExpr)
{
    Write(ast->op);
    Visit(ast->expr);
}

IMPLEMENT_VISIT_PROC(PostUnaryExpr)
{
    Visit(ast->expr);
    Write(ast->op);
}

IMPLEMENT_VISIT_PROC(FunctionCallExpr)
{
    Visit(ast->call);
}

IMPLEMENT_VISIT_PROC(BracketExpr)
{
    Write("(");
    Visit(ast->expr);
    Write(")");
}

IMPLEMENT_VISIT_PROC(CastExpr)
{
    Visit(ast->typeExpr);
    Write("(");
    Visit(ast->expr);
    Write(")");
}

IMPLEMENT_VISIT_PROC(VarAccessExpr)
{
    WriteVarIdent(ast->varIdent.get());
    if (ast->assignExpr)
    {
        Write(" " + ast->assignOp + " ");
        Visit(ast->assignExpr);
    }
}

IMPLEMENT_VISIT_PROC(InitializerExpr)
{
    Write("{ ");
        
    for (size_t i = 0; i < ast->exprs.size(); ++i)
    {
        Visit(ast->exprs[i]);
        if (i + 1 < ast->exprs.size())
            Write(", ");
    }

    Write(" }");
}

/* --- Variables --- */

IMPLEMENT_VISIT_PROC(PackOffset)
{
}

IMPLEMENT_VISIT_PROC(VarSemantic)
{
}

IMPLEMENT_VISIT_PROC(VarType)
{
    if (!ast->baseType.empty())
    {
        /* Write GLSL base type */
        auto typeName = ast->baseType;

        auto it = typeMap_.find(typeName);
        if (it != typeMap_.end())
            typeName = it->second;

        Write(typeName);
    }
    else if (ast->structType)
        Visit(ast->structType);
}

IMPLEMENT_VISIT_PROC(VarIdent)
{
    /* Write single identifier */
    Write(ast->ident);

    /* Write array index expressions */
    for (auto& index : ast->arrayIndices)
    {
        Write("[");
        Visit(index);
        Write("]");
    }

    if (ast->next)
    {
        Write(".");
        Visit(ast->next);
    }
}

IMPLEMENT_VISIT_PROC(VarDecl)
{
    if (ast->flags(VarDecl::isInsideFunc))
        Write(localVarPrefix_);
    
    Write(ast->name);

    for (auto& dim : ast->arrayDims)
    {
        Write("[");
        Visit(dim);
        Write("]");
    }

    if (ast->initializer)
    {
        Write(" = ");
        Visit(ast->initializer);
    }
}

#undef IMPLEMENT_VISIT_PROC

/* --- Helper functions for code generation --- */

void GLSLGenerator::VisitAttribute(FunctionCall* ast)
{
    auto name = FullVarIdent(ast->name);

    if (name == "numthreads")
        WriteAttributeNumThreads(ast);
    else if (name == "earlydepthstencil")
        WriteLn("layout(early_fragment_tests) in;");
}

void GLSLGenerator::WriteAttributeNumThreads(FunctionCall* ast)
{
    if (ast->arguments.size() == 3)
    {
        BeginLn();
        {
            Write("layout(local_size_x = ");
            Visit(ast->arguments[0]);

            Write(", local_size_y = ");
            Visit(ast->arguments[1]);

            Write(", local_size_z = ");
            Visit(ast->arguments[2]);

            Write(") in;");
        }
        EndLn();
    }
    else
        ErrorInvalidNumArgs("\"numthreads\" attribute", ast);
}

void GLSLGenerator::WriteEntryPointParameter(VarDeclStmnt* ast, size_t& writtenParamCounter)
{
    /* Get variable declaration */
    if (ast->varDecls.size() != 1)
        Error("invalid number of variables inside parameter of entry point", ast);
    auto varDecl = ast->varDecls.front().get();

    /* Check if a structure input is used */
    auto typeRef = ast->varType->symbolRef;
    Structure* structType = nullptr;

    if (typeRef && typeRef->Type() == AST::Types::Structure)
        structType = dynamic_cast<Structure*>(typeRef);

    if (structType)
    {
        if (MustResolveStruct(structType))
        {
            /* Write variable declaration */
            BeginLn();
            {
                Visit(ast->varType);
                Write(" " + varDecl->name + ";");
            }
            EndLn();

            /* Fill structure members */
            for (const auto& member : structType->members)
            {
                for (const auto& memberVar : member->varDecls)
                    WriteLn(varDecl->name + "." + memberVar->name + " = " + memberVar->name + ";");
            }

            ++writtenParamCounter;
        }
    }
    else
    {
        /* Get single semantic */
        if (varDecl->semantics.size() != 1)
            Error("invalid number of semantics inside parameter fo entry point", varDecl);
        auto semantic = varDecl->semantics.front()->semantic;

        /* Map semantic to GL built-in constant */
        SemanticStage semanticStage;
        if (!FetchSemantic(semantic, semanticStage))
            return;

        /* Write local variable definition statement */
        BeginLn();
        {
            Visit(ast->varType);
            Write(" " + varDecl->name + " = " + semanticStage[shaderTarget_] + ";");
        }
        EndLn();

        ++writtenParamCounter;
    }
}

void GLSLGenerator::WriteEntryPointInputSemantics()
{
    auto& parameters = program_->inputSemantics.parameters;

    size_t writtenParamCounter = 0;
    for (auto& param : parameters)
        WriteEntryPointParameter(param, writtenParamCounter);

    if (writtenParamCounter > 0)
        Blank();
}

void GLSLGenerator::WriteEntryPointOutputSemantics(Expr* ast)
{
    auto& outp = program_->outputSemantics;

    if (!outp.singleOutputVariable.empty())
    {
        BeginLn();
        {
            Write(outp.singleOutputVariable + " = ");
            Visit(ast);
            Write(";");
        }
        EndLn();
    }
    else if (outp.returnType->symbolRef)
    {
        
        //!TODO!
        
    }
}

void GLSLGenerator::WriteFragmentShaderOutput()
{
    auto& outp = program_->outputSemantics;

    if (outp.returnType->symbolRef || outp.returnType->structType)
    {
        /* Get structure AST node */
        Structure* structAST = nullptr;

        if (outp.returnType->symbolRef && outp.returnType->symbolRef->Type() == AST::Types::Structure)
            structAST = dynamic_cast<Structure*>(outp.returnType->symbolRef);
        else if (outp.returnType->structType)
            structAST = outp.returnType->structType.get();

        if (structAST)
        {
            for (const auto& member : structAST->members)
            {
                

            }
        }
    }
    else
    {
        /* Write single output semantic declaration */
        SemanticStage semantic;
        if (FetchSemantic(outp.functionSemantic, semantic))
        {
            if (semantic.fragment == "gl_FragColor")
            {
                if (IsVersionOut(130))
                {
                    BeginLn();
                    {
                        Write("layout(location = " + std::to_string(semantic.index) + ") out ");
                        Visit(outp.returnType);
                        Write(" " + outp.functionSemantic + ";");
                    }
                    EndLn();
                    outp.singleOutputVariable = outp.functionSemantic;
                }
                else
                    outp.singleOutputVariable = "gl_FragData[" + std::to_string(semantic.index) + "]";
            }
            else if (semantic.fragment == "gl_FragDepth")
                outp.singleOutputVariable = semantic.fragment;
            else
                Error("invalid output semantic for pixel shader: \"" + outp.functionSemantic + "\"");
        }
        else
            Error("unknown shader output semantic: \"" + outp.functionSemantic + "\"");
    }

    Blank();
}

VarIdent* GLSLGenerator::FirstSystemSemanticVarIdent(VarIdent* ast)
{
    /* Check if current var-ident AST node has a system semantic */
    SemanticStage semantic;
    if (FetchSemantic(ast->systemSemantic, semantic))
        return ast;

    /* Search in next var-ident AST node */
    if (ast->next)
        return FirstSystemSemanticVarIdent(ast->next.get());

    return nullptr;
}

void GLSLGenerator::WriteVarIdent(VarIdent* ast)
{
    /* Find system value semantic in variable identifier */
    SemanticStage semantic;
    auto semanticVarIdent = FirstSystemSemanticVarIdent(ast);

    if (semanticVarIdent && FetchSemantic(semanticVarIdent->systemSemantic, semantic))
    {
        /* Write shader target respective system semantic */
        Write(semantic[shaderTarget_]);

        if (semanticVarIdent->next)
        {
            Write(".");
            Visit(semanticVarIdent->next);
        }
    }
    else
    {
        /* Write default variable identifier */
        Visit(ast);
    }
}

void GLSLGenerator::VisitParameter(VarDeclStmnt* ast)
{
    /* Write modifiers */
    if (!ast->inputModifier.empty())
        Write(ast->inputModifier + " ");

    for (const auto& modifier : ast->typeModifiers)
    {
        if (modifier == "const")
            Write(modifier + " ");
    }

    /* Write parameter type */
    Visit(ast->varType);
    Write(" ");

    /* Write parameter identifier */
    if (ast->varDecls.size() == 1)
        Visit(ast->varDecls[0]);
    else
        Error("invalid number of variables in function parameter", ast);
}

void GLSLGenerator::VisitScopedStmnt(Stmnt* ast)
{
    if (ast)
    {
        if (ast->Type() != AST::Types::CodeBlockStmnt)
        {
            IncTab();
            Visit(ast);
            DecTab();
        }
        else
            Visit(ast);
    }
}

bool GLSLGenerator::ExprContainsSampler(Expr* ast)
{
    if (ast)
    {
        if (ast->Type() == AST::Types::BracketExpr)
        {
            auto bracketExpr = dynamic_cast<BracketExpr*>(ast);
            return ExprContainsSampler(bracketExpr->expr.get());
        }
        if (ast->Type() == AST::Types::BinaryExpr)
        {
            auto binaryExpr = dynamic_cast<BinaryExpr*>(ast);
            return
                ExprContainsSampler(binaryExpr->lhsExpr.get()) ||
                ExprContainsSampler(binaryExpr->rhsExpr.get());
        }
        if (ast->Type() == AST::Types::UnaryExpr)
        {
            auto unaryExpr = dynamic_cast<UnaryExpr*>(ast);
            return ExprContainsSampler(unaryExpr->expr.get());
        }
        if (ast->Type() == AST::Types::VarAccessExpr)
        {
            auto symbolRef = dynamic_cast<VarAccessExpr*>(ast)->varIdent->symbolRef;
            if (symbolRef && symbolRef->Type() == AST::Types::SamplerDecl)
                return true;
        }
    }
    return false;
}

bool GLSLGenerator::VarTypeIsSampler(VarType* ast)
{
    auto it = HLSLKeywords().find(ast->baseType);
    return it != HLSLKeywords().end() && it->second == Token::Types::Sampler;
}

bool GLSLGenerator::FetchSemantic(std::string semanticName, SemanticStage& semantic) const
{
    if (semanticName.empty())
        return false;

    /* Extract optional index */
    int index = 0;

    if (!semanticName.empty() && std::isdigit(semanticName.back()))
    {
        index = std::stoi(std::string(1, semanticName.back()));
        semanticName.pop_back();
    }

    /* Search for semantic */
    std::transform(semanticName.begin(), semanticName.end(), semanticName.begin(), ::toupper);

    auto it = semanticMap_.find(semanticName);
    if (it != semanticMap_.end())
    {
        /* Return semantic */
        semantic = it->second;
        semantic.index = index;
        return true;
    }

    return false;
}

bool GLSLGenerator::IsSystemValueSemantic(const VarSemantic* ast) const
{
    SemanticStage semantic;
    return FetchSemantic(ast->semantic, semantic);
}

bool GLSLGenerator::HasSystemValueSemantic(const std::vector<VarSemanticPtr>& semantics) const
{
    for (const auto& varSemantic : semantics)
    {
        if (IsSystemValueSemantic(varSemantic.get()))
            return true;
    }
    return false;
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

const std::string& GLSLGenerator::SemanticStage::operator [] (const ShaderTargets target) const
{
    switch (target)
    {
        case ShaderTargets::GLSLVertexShader:
            return vertex;
        case ShaderTargets::GLSLGeometryShader:
            return geometry;
        case ShaderTargets::GLSLTessControlShader:
            return tessControl;
        case ShaderTargets::GLSLTessEvaluationShader:
            return tessEvaluation;
        case ShaderTargets::GLSLFragmentShader:
            return fragment;
        case ShaderTargets::GLSLComputeShader:
            return compute;
    }
    throw std::out_of_range("'target' parameter out of range in " __FUNCTION__);
    return vertex;
}


} // /namespace HTLib



// ================================================================================