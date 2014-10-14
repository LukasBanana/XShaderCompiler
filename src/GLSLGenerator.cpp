/*
 * GLSLGenerator.cpp
 * 
 * This file is part of the "HLSL Translator" (Copyright (c) 2014 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLSLGenerator.h"
#include "HLSLTree.h"

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
    writer_         { options.indent    },
    includeHandler_ { includeHandler    },
    log_            { log               },
    allowBlanks_    { !options.noblanks }
{
    EstablishMaps();
}

bool GLSLGenerator::GenerateCode(
    Program* program,
    std::ostream& output,
    const std::string& entryPoint,
    const ShaderTargets shaderTarget,
    const ShaderVersions shaderVersion)
{
    if (!program)
        return false;

    /* Store parameters */
    entryPoint_     = entryPoint;
    shaderTarget_   = shaderTarget;
    shaderVersion_  = shaderVersion;

    try
    {
        writer_.OutputStream(output);

        /* Write header */
        Comment("GLSL " + TargetToString(shaderTarget) + " Shader");
        Comment("Generated from HLSL Shader \"" + entryPoint + "\"");
        Comment(TimePoint());
        Blank();

        Version(static_cast<int>(shaderVersion));
        Blank();

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

        /* Storage class types */
        { "groupshared", "shared" },
    };

    intrinsicMap_ = std::map<std::string, std::string>
    {
        { "frac",                            "fract"              },
        { "lerp",                            "mix"                },
        { "ddx",                             "dFdx"               },
        { "ddy",                             "dFdy"               },
        { "atan2",                           "atan"               },
        { "GroupMemoryBarrier",              "groupMemoryBarrier" },
        { "GroupMemoryBarrierWithGroupSync", "barrier"            },
        { "AllMemoryBarrier",                "memoryBarrier"      },
        { "AllMemoryBarrierWithGroupSync",   "barrier"            },
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

void GLSLGenerator::Line(const AST* ast)
{
    Line(ast->pos.Row());
}

void GLSLGenerator::Blank()
{
    if (allowBlanks_)
        WriteLn("");
}

void GLSLGenerator::AppendCommonMacros()
{
    WriteLn("#define saturate(x) clamp(x, 0.0, 1.0)");
    WriteLn("#define clip(x) if (lessThan(x, 0.0)) { discard; }");
    Blank();
}

void GLSLGenerator::AppendInterlockedMacros()
{
    WriteLn("#define InterlockedAdd(dest, value, prev) prev = atomicAdd(dest, value)");
    WriteLn("#define InterlockedAnd(dest, value, prev) prev = atomicAnd(dest, value)");
    WriteLn("#define InterlockedOr(dest, value, prev) prev = atomicOr(dest, value)");
    WriteLn("#define InterlockedXor(dest, value, prev) prev = atomicXor(dest, value)");
    WriteLn("#define InterlockedMin(dest, value, prev) prev = atomicMin(dest, value)");
    WriteLn("#define InterlockedMax(dest, value, prev) prev = atomicMax(dest, value)");
    WriteLn("#define InterlockedCompareExchange(dest, value, prev) prev = atomicCompSwap(dest, value)");
    WriteLn("#define InterlockedExchange(dest, value, prev) prev = atomicExchange(dest, value)");
    Blank();
}

void GLSLGenerator::AppendMulFunctions()
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

void GLSLGenerator::AppendRcpFunctions()
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

bool GLSLGenerator::IsVersion(int version) const
{
    return static_cast<int>(shaderVersion_) >= version;
}

/* ------- Visit functions ------- */

#define IMPLEMENT_VISIT_PROC(className) \
    void GLSLGenerator::Visit##className(className* ast, void* args)

IMPLEMENT_VISIT_PROC(Program)
{
    /* Append default helper macros and functions */
    AppendCommonMacros();

    if (ast->flags(Program::interlockedIntrinsicsUsed))
        AppendInterlockedMacros();
    if (ast->flags(Program::mulIntrinsicUsed))
        AppendMulFunctions();
    if (ast->flags(Program::rcpIntrinsicUsed))
        AppendRcpFunctions();

    for (auto& globDecl : ast->globalDecls)
    {
        Visit(globDecl);
        Blank();
    }
}

IMPLEMENT_VISIT_PROC(CodeBlock)
{
    OpenScope();
    {
        for (auto& stmnt : ast->stmnts)
            Visit(stmnt);
    }
    CloseScope();
}

IMPLEMENT_VISIT_PROC(FunctionCall)
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

    /* Write arguments */
    Write("(");

    for (size_t i = 0; i < ast->arguments.size(); ++i)
    {
        Visit(ast->arguments[i]);
        if (i + 1 < ast->arguments.size())
            Write(", ");
    }

    Write(")");
}

IMPLEMENT_VISIT_PROC(Structure)
{
    bool semicolon = (args != nullptr ? *reinterpret_cast<bool*>(&args) : false);

    WriteLn("struct " + ast->name);
    
    OpenScope();
    {
        for (auto& varDecl : ast->members)
            Visit(varDecl);
    }
    CloseScope(semicolon);
}

/* --- Global declarations --- */

IMPLEMENT_VISIT_PROC(FunctionDecl)
{
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

            for (size_t i = 0; i < ast->parameters.size(); ++i)
            {
                VisitParameter(ast->parameters[i].get());
                if (i + 1 < ast->parameters.size())
                    Write(", ");
            }

            Write(")");
        }
    }
    EndLn();

    /* Write function body */
    Visit(ast->codeBlock);
}

IMPLEMENT_VISIT_PROC(BufferDecl)
{
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
}

IMPLEMENT_VISIT_PROC(StructDecl)
{
    Line(ast);
    bool semicolon = true;
    Visit(ast->structure, &semicolon);
}

/* --- Statements --- */

IMPLEMENT_VISIT_PROC(VarDeclStmnt)
{
    BeginLn();

    /* Write type modifier and variable types */
    if (ast->typeModifier == "const")
        Write(ast->typeModifier + " ");

    Visit(ast->varType);
    Write(" ");

    /* Write variable declarations */
    for (size_t i = 0; i < ast->varDecls.size(); ++i)
    {
        Visit(ast->varDecls[i]);
        if (i + 1 < ast->varDecls.size())
            Write(", ");
    }

    Write(";");
    EndLn();
}

IMPLEMENT_VISIT_PROC(ReturnStmnt)
{
    BeginLn();

    Write("return");

    if (ast->expr)
    {
        Write(" ");
        Visit(ast->expr);
    }

    Write(";");
    EndLn();
}

IMPLEMENT_VISIT_PROC(CtrlTransferStmnt)
{
    WriteLn(ast->instruction + ";");
}

/* --- Expressions --- */

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
    Visit(ast->varIdent);
    if (ast->assignExpr)
    {
        Write(" " + ast->assignOp + " ");
        Visit(ast->assignExpr);
    }
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
    Write(ast->name);

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
    //else if (name == ...)
    //    ...
}

void GLSLGenerator::WriteAttributeNumThreads(FunctionCall* ast)
{
    if (ast->arguments.size() == 3)
    {
        BeginLn();
        {
            Write("layout(");

            Write("local_size_x = ");
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

void GLSLGenerator::VisitParameter(VarDeclStmnt* ast)
{
    if (ast->typeModifier == "const")
        Write(ast->typeModifier + " ");

    Visit(ast->varType);
    Write(" ");

    if (ast->varDecls.size() == 1)
        Visit(ast->varDecls[0]);
    else
        Error("invalid number of variables in function parameter", ast);
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