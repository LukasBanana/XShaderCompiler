/*
 * GLSLGenerator.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLSLGenerator.h"
#include "GLSLExtensionAgent.h"
#include "GLSLConverter.h"
#include "GLSLKeywords.h"
#include "GLSLIntrinsics.h"
#include "ReferenceAnalyzer.h"
#include "StructParameterAnalyzer.h"
#include "TypeDenoter.h"
#include "Exception.h"
#include "TypeConverter.h"
#include "ExprConverter.h"
#include "FuncNameConverter.h"
#include "Helper.h"
#include "ReportIdents.h"
#include <initializer_list>
#include <algorithm>
#include <cctype>
#include <set>
#include <sstream>


namespace Xsc
{


/*
 * Internal structures
 */

struct IfStmntArgs
{
    bool inHasElseParentNode;
};

struct StructDeclArgs
{
    bool inEndWithSemicolon;
};


/*
 * GLSLGenerator class
 */

GLSLGenerator::GLSLGenerator(Log* log) :
    Generator { log }
{
}

void GLSLGenerator::GenerateCodePrimary(
    Program& program, const ShaderInput& inputDesc, const ShaderOutput& outputDesc)
{
    /* Store parameters */
    versionOut_         = outputDesc.shaderVersion;
    nameMangling_       = outputDesc.nameMangling;
    allowExtensions_    = outputDesc.options.allowExtensions;
    explicitBinding_    = outputDesc.options.explicitBinding;
    preserveComments_   = outputDesc.options.preserveComments;
    separateShaders_    = outputDesc.options.separateShaders;
    separateSamplers_   = outputDesc.options.separateSamplers;
    autoBinding_        = outputDesc.options.autoBinding;
    writeHeaderComment_ = outputDesc.options.writeGeneratorHeader;
    allowLineMarks_     = outputDesc.formatting.lineMarks;
    compactWrappers_    = outputDesc.formatting.compactWrappers;
    alwaysBracedScopes_ = outputDesc.formatting.alwaysBracedScopes;
    entryPointName_     = inputDesc.entryPoint;

    #ifdef XSC_ENABLE_LANGUAGE_EXT
    extensions_         = inputDesc.extensions;
    #endif

    for (const auto& s : outputDesc.vertexSemantics)
    {
        const auto semanticCi = ToCiString(s.semantic);
        vertexSemanticsMap_[semanticCi] = { s.location, 0 };

        if (s.location >= 0)
            usedInLocationsSet_.insert(s.location);
    }

    if (program.entryPointRef)
    {
        try
        {
            /* Pre-process AST before generation begins */
            PreProcessAST(inputDesc, outputDesc);

            /* Visit program AST */
            Visit(&program);

            /* Check for optional warning feedback */
            ReportOptionalFeedback();
        }
        catch (const Report&)
        {
            throw;
        }
        catch (const ASTRuntimeError& e)
        {
            Error(e.what(), e.GetAST());
        }
        catch (const std::exception& e)
        {
            Error(e.what());
        }
    }
    else
        Error(R_EntryPointNotFound(inputDesc.entryPoint));
}


/*
 * ======= Private: =======
 */

std::unique_ptr<std::string> GLSLGenerator::SystemValueToKeyword(const IndexedSemantic& semantic) const
{
    if (semantic == Semantic::Target && versionOut_ > OutputShaderVersion::GLSL120)
        return MakeUnique<std::string>(semantic.ToString());
    else
        return SemanticToGLSLKeyword(semantic, IsVKSL());
}

bool GLSLGenerator::IsWrappedIntrinsic(const Intrinsic intrinsic) const
{
    static const std::set<Intrinsic> wrappedIntrinsics
    {
        Intrinsic::Clip,
        Intrinsic::Lit,
        Intrinsic::SinCos,
        Intrinsic::GroupMemoryBarrierWithGroupSync,
        Intrinsic::DeviceMemoryBarrier,
        Intrinsic::DeviceMemoryBarrierWithGroupSync,
        Intrinsic::AllMemoryBarrierWithGroupSync
    };
    return (wrappedIntrinsics.find(intrinsic) != wrappedIntrinsics.end());
}

bool GLSLGenerator::IsGLSL() const
{
    return IsLanguageGLSL(versionOut_);
}

bool GLSLGenerator::IsESSL() const
{
    return IsLanguageESSL(versionOut_);
}

bool GLSLGenerator::IsVKSL() const
{
    return IsLanguageVKSL(versionOut_);
}

bool GLSLGenerator::HasShadingLanguage420Pack() const
{
    return ( IsVKSL() || ( versionOut_ >= OutputShaderVersion::GLSL420 && versionOut_ <= OutputShaderVersion::GLSL450 ) );
}

bool GLSLGenerator::UseSeparateSamplers() const
{
    return ( IsVKSL() && separateSamplers_ );
}

const std::string* GLSLGenerator::BufferTypeToKeyword(const BufferType bufferType, const AST* ast)
{
    if (auto keyword = BufferTypeToGLSLKeyword(bufferType, IsVKSL(), UseSeparateSamplers()))
        return keyword;
    else
        Error(R_FailedToMapToGLSLKeyword(R_BufferType), ast);
    return nullptr;
}

const std::string* GLSLGenerator::SamplerTypeToKeyword(const SamplerType samplerType, const AST* ast)
{
    if (auto keyword = SamplerTypeToGLSLKeyword(samplerType))
        return keyword;
    else
        Error(R_FailedToMapToGLSLKeyword(R_SamplerType), ast);
    return nullptr;
}

bool GLSLGenerator::IsTypeCompatibleWithSemantic(const Semantic semantic, const TypeDenoter& typeDenoter)
{
    if (auto baseTypeDen = typeDenoter.As<BaseTypeDenoter>())
    {
        auto dataType = baseTypeDen->dataType;

        switch (semantic)
        {
            case Semantic::DispatchThreadID:
            case Semantic::GroupID:
            case Semantic::GroupThreadID:
                return (dataType == DataType::UInt3);

            case Semantic::GroupIndex:
                return (dataType == DataType::UInt);

            case Semantic::GSInstanceID:
            case Semantic::InstanceID:
            case Semantic::OutputControlPointID:
            case Semantic::PrimitiveID:
            case Semantic::SampleIndex:
            case Semantic::VertexID:
                return (dataType == DataType::Int);

            default:
                break;
        }
        return true;
    }
    return false;
}

void GLSLGenerator::ReportOptionalFeedback()
{
    /* Report warnings for unused and overwritten vertex semantic bindings */
    if (WarnEnabled(Warnings::UnlocatedObjects) && explicitBinding_ && IsVertexShader())
    {
        /* Check for vertex semantics that have not been found */
        std::map<int, int> locationUseCount;

        for (const auto& vertSemantic : vertexSemanticsMap_)
        {
            const auto& sem = vertSemantic.second;
            if (sem.found)
                ++locationUseCount[sem.location];
            else
                Warning(R_VertexSemanticNotFound(ToString(vertSemantic.first)));
        }

        /* Check for multiple usages of vertex semantic locations */
        for (const auto& loc : locationUseCount)
        {
            if (loc.second > 1)
                Warning(R_MultiUseOfVertexSemanticLocation(loc.first, loc.second));
        }
    }
}

void GLSLGenerator::ErrorIntrinsic(const std::string& intrinsicName, const AST* ast)
{
    Error(R_FailedToMapToGLSLKeyword(R_Intrinsic(intrinsicName)), ast);
}

int GLSLGenerator::GetNumBindingLocations(const TypeDenoter* typeDenoter)
{
    if (!typeDenoter)
        return -1;

    /* Accumulate array elements */
    int numArrayElements = 1;

    while (auto arrayTypeDen = typeDenoter->As<ArrayTypeDenoter>())
    {
        /* Accumulate array elements of current array type, and move on to next sub type */
        numArrayElements *= arrayTypeDen->NumArrayElements();
        typeDenoter = arrayTypeDen->subTypeDenoter.get();
    }

    if (numArrayElements == 0)
        return -1;

    if (auto baseTypeDen = typeDenoter->GetAliased().As<BaseTypeDenoter>())
    {
        const auto dataType = baseTypeDen->dataType;

        /* Determine number of locations required by type */
        int elementSize = 0;

        if (IsScalarType(dataType))
        {
            /* Single scalar type */
            elementSize = 1;
        }
        else if (IsVectorType(dataType))
        {
            int dims = VectorTypeDim(dataType);

            /* 3- and 4-component double vectors require two locations */
            if (IsDoubleRealType(dataType) && dims > 2)
                elementSize = 2;
            else
                elementSize = 1;
        }
        else if (IsMatrixType(dataType))
        {
            auto dims = MatrixTypeDim(dataType);

            int rowDim = dims.second;
            int rowSize = 0;

            /* 3- and 4-component double vectors require two locations */
            if (IsDoubleRealType(dataType) && rowDim > 2)
                rowSize = 2;
            else
                rowSize = 1;

            elementSize = dims.first * rowSize;
        }

        if (elementSize != 0)
            return elementSize * numArrayElements;
    }

    return -1;
}

int GLSLGenerator::GetBindingLocation(const TypeDenoter* typeDenoter, bool input)
{
    int numLocations = GetNumBindingLocations(typeDenoter);
    if (numLocations == -1)
        return -1;

    /* Find enough consecutive empty locations to hold the type */
    int startLocation   = 0;
    int endLocation     = startLocation + numLocations - 1;

    auto& usedLocationsSet = (input ? usedInLocationsSet_ : usedOutLocationsSet_);
    for (auto entry : usedLocationsSet)
    {
        if (entry >= startLocation && entry <= endLocation)
        {
            startLocation   = entry + 1;
            endLocation     = startLocation + numLocations - 1;
        }
        else if (entry > endLocation)
            break;
    }

    for (auto i = startLocation; i <= endLocation; ++i)
        usedLocationsSet.insert(i);

    return startLocation;
}

/* ------- Visit functions ------- */

#define IMPLEMENT_VISIT_PROC(AST_NAME) \
    void GLSLGenerator::Visit##AST_NAME(AST_NAME* ast, void* args)

IMPLEMENT_VISIT_PROC(Program)
{
    /* Write version and required extensions first */
    WriteProgramHeader();

    /* Write global input/output layouts */
    WriteGlobalLayouts();

    /* Write redeclarations for built-in input/output blocks */
    if (separateShaders_ && versionOut_ > OutputShaderVersion::GLSL140)
        WriteBuiltinBlockRedeclarations();

    /* Write wrapper functions for special intrinsics */
    WriteWrapperIntrinsics();

    /* Write global uniform declarations */
    WriteGlobalUniforms();

    /* Write global input/output semantics */
    BeginSep();
    {
        WriteGlobalInputSemantics(GetProgram()->entryPointRef);
    }
    EndSep();

    BeginSep();
    {
        WriteGlobalOutputSemantics(GetProgram()->entryPointRef);
    }
    EndSep();

    /* Write global program statements */
    WriteStmntList(ast->globalStmnts, true);
}

IMPLEMENT_VISIT_PROC(CodeBlock)
{
    WriteScopeOpen();
    {
        WriteStmntList(ast->stmnts);
    }
    WriteScopeClose();
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
    IncIndent();
    {
        Visit(ast->stmnts);
    }
    DecIndent();
}

IMPLEMENT_VISIT_PROC(ArrayDimension)
{
    Write(ast->ToString());
}

IMPLEMENT_VISIT_PROC(TypeSpecifier)
{
    if (ast->structDecl)
        Visit(ast->structDecl);
    else
        WriteTypeDenoter(*ast->typeDenoter, IsESSL(), ast);
}

/* --- Declarations --- */

IMPLEMENT_VISIT_PROC(VarDecl)
{
    if (auto staticMemberVar = ast->FetchStaticVarDeclRef())
        Write(staticMemberVar->ident);
    else
        Write(InsideStructDecl() ? ast->ident.Original() : ast->ident.Final());

    Visit(ast->arrayDims);

    if (ast->initializer)
    {
        const auto& typeDen = ast->initializer->GetTypeDenoter()->GetAliased();
        if (!typeDen.IsNull())
        {
            Write(" = ");
            Visit(ast->initializer);
        }
    }
}

IMPLEMENT_VISIT_PROC(StructDecl)
{
    if (ast->flags(StructDecl::isNonEntryPointParam) || !ast->flags(StructDecl::isShaderInput | StructDecl::isShaderOutput))
    {
        PushStructDecl(ast);
        {
            if (auto structDeclArgs = reinterpret_cast<StructDeclArgs*>(args))
                WriteStructDecl(ast, structDeclArgs->inEndWithSemicolon);
            else
                WriteStructDecl(ast, false);
        }
        PopStructDecl();
    }
}

IMPLEMENT_VISIT_PROC(SamplerDecl)
{
    WriteSamplerDecl(*ast);
}

/* --- Declaration statements --- */

IMPLEMENT_VISIT_PROC(FunctionDecl)
{
    /* Is this function reachable from the entry point? */
    if (!ast->flags(AST::isReachable))
    {
        /* Check for valid control paths */
        if (WarnEnabled(Warnings::Basic) && ast->flags(FunctionDecl::hasNonReturnControlPath))
            Warning(R_InvalidControlPathInUnrefFunc(ast->ToString()), ast);
        return;
    }

    /* Check for valid control paths */
    if (ast->flags(FunctionDecl::hasNonReturnControlPath))
        Error(R_InvalidControlPathInFunc(ast->ToString()), ast);

    /* Write line */
    WriteLineMark(ast);

    /* Write function declaration */
    PushFunctionDecl(ast);
    {
        if (ast->flags(FunctionDecl::isEntryPoint))
            WriteFunctionEntryPoint(ast);
        else if (ast->flags(FunctionDecl::isSecondaryEntryPoint))
            WriteFunctionSecondaryEntryPoint(ast);
        else
            WriteFunction(ast);
    }
    PopFunctionDecl();

    Blank();
}

IMPLEMENT_VISIT_PROC(UniformBufferDecl)
{
    if (!ast->flags(AST::isReachable))
        return;

    if (versionOut_ < OutputShaderVersion::GLSL140)
    {
        /* Write individual uniforms */
        for (auto& varDeclStmnt : ast->varMembers)
        {
            varDeclStmnt->typeSpecifier->isUniform = true;
            Visit(varDeclStmnt);
        }
    }
    else
    {
        /* Write uniform buffer header */
        WriteLineMark(ast);

        /* Write uniform buffer declaration */
        ast->DeriveCommonStorageLayout();

        BeginLn();

        WriteLayout(
            {
                [&]() { Write("std140"); },
                [&]()
                {
                    if (ast->commonStorageLayout == TypeModifier::RowMajor)
                        Write("row_major");
                },
                [&]() { WriteLayoutBinding(ast->slotRegisters); },
            }
        );

        Write("uniform " + ast->ident);

        /* Write uniform buffer members */
        WriteScopeOpen(false, true);
        BeginSep();
        {
            PushUniformBufferDecl(ast);
            {
                WriteStmntList(ast->varMembers);
            }
            PopUniformBufferDecl();
        }
        EndSep();
        WriteScopeClose();
    }

    Blank();
}

IMPLEMENT_VISIT_PROC(BufferDeclStmnt)
{
    if (ast->flags(AST::isReachable))
    {
        /* Write buffer declarations */
        for (auto& bufferDecl : ast->bufferDecls)
            WriteBufferDecl(bufferDecl.get());
    }
}

IMPLEMENT_VISIT_PROC(SamplerDeclStmnt)
{
    if (ast->flags(AST::isReachable))
    {
        /* Write sampler declarations */
        if (UseSeparateSamplers() || !IsSamplerStateType(ast->typeDenoter->samplerType))
            Visit(ast->samplerDecls);
    }
}

IMPLEMENT_VISIT_PROC(VarDeclStmnt)
{
    if (!ast->flags(AST::isReachable) && !InsideFunctionDecl() && !InsideStructDecl())
        return;

    auto varDecls = ast->varDecls;

    //TODO: refactor this!
    #if 1
    auto varTypeStructDecl = ast->typeSpecifier->GetStructDeclRef();

    for (auto it = varDecls.begin(); it != varDecls.end();)
    {
        auto varDecl = it->get();

        /*
        First check if code generation is disabled for variable declaration,
        then check if this is a system value semantic inside an interface block.
        */
        if ( varDecl->flags(VarDecl::isEntryPointLocal) &&
             ( !varTypeStructDecl || !varTypeStructDecl->flags(StructDecl::isNonEntryPointParam) ) )
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
    #endif

    /* Ignore declaration statement of static member variables */
    if (ast->typeSpecifier->HasAnyStorageClassOf({ StorageClass::Static }) && ast->FetchStructDeclRef() != nullptr)
        return;

    PushVarDeclStmnt(ast);
    {
        BeginLn();

        /* Write storage classes and interpolation modifiers (must be before in/out keywords) */
        if (!InsideStructDecl())
        {
            WriteInterpModifiers(ast->typeSpecifier->interpModifiers, ast);
            WriteStorageClasses(ast->typeSpecifier->storageClasses, ast);
        }

        Separator();

        /* Write input modifiers */
        if (ast->flags(VarDeclStmnt::isShaderInput))
            Write("in ");
        else if (ast->flags(VarDeclStmnt::isShaderOutput))
            Write("out ");
        else if (ast->IsUniform())
            Write("uniform ");

        Separator();

        /* Write type modifiers */
        WriteTypeModifiersFrom(ast->typeSpecifier);
        Separator();

        /* Write variable type */
        if (ast->typeSpecifier->structDecl)
        {
            /* Do not end line here with "EndLn" */
            Visit(ast->typeSpecifier);
            BeginLn();
        }
        else
        {
            Visit(ast->typeSpecifier);
            Write(" ");
        }

        Separator();

        /* Write variable declarations */
        for (std::size_t i = 0; i < varDecls.size(); ++i)
        {
            Visit(varDecls[i]);
            if (i + 1 < varDecls.size())
                Write(", ");
        }

        Write(";");
        EndLn();
    }
    PopVarDeclStmnt();

    if (InsideGlobalScope())
        Blank();
}

IMPLEMENT_VISIT_PROC(AliasDeclStmnt)
{
    if (ast->structDecl && !ast->structDecl->IsAnonymous())
    {
        WriteLineMark(ast);

        /* Write structure declaration and end it with a semicolon */
        StructDeclArgs structDeclArgs;
        structDeclArgs.inEndWithSemicolon = true;

        Visit(ast->structDecl, &structDeclArgs);
    }
}

IMPLEMENT_VISIT_PROC(BasicDeclStmnt)
{
    if (ast->flags(AST::isReachable))
    {
        if (auto structDecl = ast->declObject->As<StructDecl>())
        {
            if ( structDecl->flags(StructDecl::isNonEntryPointParam) ||
                 !structDecl->flags(StructDecl::isShaderInput | StructDecl::isShaderOutput) )
            {
                WriteLineMark(ast);

                /* Visit structure declaration */
                StructDeclArgs structDeclArgs;
                structDeclArgs.inEndWithSemicolon = true;

                Visit(structDecl, &structDeclArgs);
            }
        }
        else
        {
            /* Visit declaration object only */
            Visit(ast->declObject);
        }
    }
}

/* --- Statements --- */

IMPLEMENT_VISIT_PROC(NullStmnt)
{
    WriteLn(";");
}

IMPLEMENT_VISIT_PROC(CodeBlockStmnt)
{
    Visit(ast->codeBlock);
}

IMPLEMENT_VISIT_PROC(ForLoopStmnt)
{
    /* Write loop header */
    BeginLn();

    Write("for (");

    PushOptions({ false, false });
    {
        if (ast->initStmnt->Type() == AST::Types::SamplerDeclStmnt && !UseSeparateSamplers())
            Write(";");
        else
            Visit(ast->initStmnt);
        Write(" "); // initStmnt already has the ';'!
        Visit(ast->condition);
        Write("; ");
        Visit(ast->iteration);
    }
    PopOptions();

    Write(")");

    WriteScopedStmnt(ast->bodyStmnt.get());
}

IMPLEMENT_VISIT_PROC(WhileLoopStmnt)
{
    /* Write loop condExpr */
    BeginLn();

    Write("while (");
    Visit(ast->condition);
    Write(")");

    WriteScopedStmnt(ast->bodyStmnt.get());
}

IMPLEMENT_VISIT_PROC(DoWhileLoopStmnt)
{
    BeginLn();

    Write("do");
    WriteScopedStmnt(ast->bodyStmnt.get());

    /* Write loop condExpr */
    WriteScopeContinue();

    Write("while (");
    Visit(ast->condition);
    Write(");");

    EndLn();
}

IMPLEMENT_VISIT_PROC(IfStmnt)
{
    bool hasElseParentNode = (args != nullptr ? reinterpret_cast<IfStmntArgs*>(args)->inHasElseParentNode : false);

    /* Write if condExpr */
    if (!hasElseParentNode)
        BeginLn();

    Write("if (");
    Visit(ast->condition);
    Write(")");

    /* Write if body */
    WriteScopedStmnt(ast->bodyStmnt.get());

    Visit(ast->elseStmnt);
}

IMPLEMENT_VISIT_PROC(ElseStmnt)
{
    if (ast->bodyStmnt->Type() == AST::Types::IfStmnt)
    {
        /* Write else if statement */
        WriteScopeContinue();
        Write("else ");

        if (ast->bodyStmnt->Type() == AST::Types::IfStmnt)
        {
            IfStmntArgs ifStmntArgs;
            ifStmntArgs.inHasElseParentNode = true;
            Visit(ast->bodyStmnt, &ifStmntArgs);
        }
        else
            Visit(ast->bodyStmnt);
    }
    else
    {
        /* Write else statement */
        WriteScopeContinue();
        Write("else");
        WriteScopedStmnt(ast->bodyStmnt.get());
    }
}

IMPLEMENT_VISIT_PROC(SwitchStmnt)
{
    /* Write selector */
    BeginLn();

    Write("switch (");
    Visit(ast->selector);
    Write(")");

    /* Write switch cases */
    WriteScopeOpen();
    {
        Visit(ast->cases);
    }
    WriteScopeClose();
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

IMPLEMENT_VISIT_PROC(ReturnStmnt)
{
    if (InsideEntryPoint() || InsideSecondaryEntryPoint())
    {
        /* Write all output semantics assignment with the expression of the return statement */
        WriteOutputSemanticsAssignment(ast->expr.get());

        /* Is this return statement at the end of the function scope? */
        if (!ast->flags(ReturnStmnt::isEndOfFunction))
            WriteLn("return;");
    }
    else
    {
        if (ast->expr)
        {
            BeginLn();
            {
                Write("return ");
                Visit(ast->expr);
                Write(";");
            }
            EndLn();
        }
        else if (!ast->flags(ReturnStmnt::isEndOfFunction))
            WriteLn("return;");
    }
}

IMPLEMENT_VISIT_PROC(CtrlTransferStmnt)
{
    WriteLn(CtrlTransformToString(ast->transfer) + ";");
}

/* --- Expressions --- */

IMPLEMENT_VISIT_PROC(SequenceExpr)
{
    for (std::size_t i = 0, n = ast->exprs.size(); i < n; ++i)
    {
        Visit(ast->exprs[i]);
        if (i + 1 < n)
            Write(", ");
    }
}

IMPLEMENT_VISIT_PROC(LiteralExpr)
{
    Write(ast->value);
}

IMPLEMENT_VISIT_PROC(TypeSpecifierExpr)
{
    WriteTypeDenoter(*ast->typeSpecifier->typeDenoter, false, ast);
}

IMPLEMENT_VISIT_PROC(TernaryExpr)
{
    Visit(ast->condExpr);
    Write(" ? ");
    Visit(ast->thenExpr);
    Write(" : ");
    Visit(ast->elseExpr);
}

IMPLEMENT_VISIT_PROC(BinaryExpr)
{
    Visit(ast->lhsExpr);
    Write(" " + BinaryOpToString(ast->op) + " ");
    Visit(ast->rhsExpr);
}

IMPLEMENT_VISIT_PROC(UnaryExpr)
{
    Write(UnaryOpToString(ast->op));
    Visit(ast->expr);
}

IMPLEMENT_VISIT_PROC(PostUnaryExpr)
{
    Visit(ast->expr);
    Write(UnaryOpToString(ast->op));
}

IMPLEMENT_VISIT_PROC(CallExpr)
{
    /* Check for special cases of intrinsic function calls */
    if (ast->intrinsic == Intrinsic::Mul)
        WriteCallExprIntrinsicMul(ast);
    else if (ast->intrinsic == Intrinsic::Rcp)
        WriteCallExprIntrinsicRcp(ast);
    else if (ast->intrinsic == Intrinsic::Clip && ast->flags(CallExpr::canInlineIntrinsicWrapper))
        WriteCallExprIntrinsicClip(ast);
    else if (ast->intrinsic == Intrinsic::InterlockedCompareExchange)
        WriteCallExprIntrinsicAtomicCompSwap(ast);
    else if (ast->intrinsic >= Intrinsic::InterlockedAdd && ast->intrinsic <= Intrinsic::InterlockedXor)
        WriteCallExprIntrinsicAtomic(ast);
    else if (ast->intrinsic == Intrinsic::Image_AtomicCompSwap)
        WriteCallExprIntrinsicImageAtomicCompSwap(ast);
    else if (ast->intrinsic >= Intrinsic::Image_AtomicAdd && ast->intrinsic <= Intrinsic::Image_AtomicExchange)
        WriteCallExprIntrinsicImageAtomic(ast);
    else if (ast->intrinsic == Intrinsic::StreamOutput_Append)
        WriteCallExprIntrinsicStreamOutputAppend(ast);
    else if (ast->intrinsic == Intrinsic::Texture_QueryLod)
        WriteCallExprIntrinsicTextureQueryLod(ast, true);
    else if (ast->intrinsic == Intrinsic::Texture_QueryLodUnclamped)
        WriteCallExprIntrinsicTextureQueryLod(ast, false);
    else
        WriteCallExprStandard(ast);
}

IMPLEMENT_VISIT_PROC(BracketExpr)
{
    Write("(");
    Visit(ast->expr);
    Write(")");
}

IMPLEMENT_VISIT_PROC(ObjectExpr)
{
    WriteObjectExpr(*ast);
}

IMPLEMENT_VISIT_PROC(AssignExpr)
{
    Visit(ast->lvalueExpr);
    Write(" " + AssignOpToString(ast->op) + " ");
    Visit(ast->rvalueExpr);
}

IMPLEMENT_VISIT_PROC(ArrayExpr)
{
    WriteArrayExpr(*ast);
}

IMPLEMENT_VISIT_PROC(CastExpr)
{
    WriteTypeDenoter(*ast->typeSpecifier->typeDenoter, false, ast);
    Write("(");
    Visit(ast->expr);
    Write(")");
}

IMPLEMENT_VISIT_PROC(InitializerExpr)
{
    if (ast->GetTypeDenoter()->GetAliased().IsArray())
    {
        WriteScopeOpen();

        for (std::size_t i = 0; i < ast->exprs.size(); ++i)
        {
            BeginLn();
            Visit(ast->exprs[i]);
            if (i + 1 < ast->exprs.size())
                Write(",");
            EndLn();
        }

        WriteScopeClose();
        BeginLn();
    }
    else
    {
        Write("{ ");

        for (std::size_t i = 0; i < ast->exprs.size(); ++i)
        {
            Visit(ast->exprs[i]);
            if (i + 1 < ast->exprs.size())
                Write(", ");
        }

        Write(" }");
    }
}

#undef IMPLEMENT_VISIT_PROC

/* --- Helper functions for code generation --- */

/* ----- Pre processing AST ----- */

void GLSLGenerator::PreProcessAST(const ShaderInput& inputDesc, const ShaderOutput& outputDesc)
{
    PreProcessStructParameterAnalyzer(inputDesc);
    PreProcessTypeConverter();
    PreProcessExprConverterPrimary();
    PreProcessGLSLConverter(inputDesc, outputDesc);
    PreProcessFuncNameConverter();
    PreProcessReferenceAnalyzer(inputDesc);
    PreProcessExprConverterSecondary();
}

void GLSLGenerator::PreProcessStructParameterAnalyzer(const ShaderInput& inputDesc)
{
    /* Mark all structures that are used for another reason than entry-point parameter */
    StructParameterAnalyzer structAnalyzer;
    structAnalyzer.MarkStructsFromEntryPoint(*GetProgram(), inputDesc.shaderTarget);
}

void GLSLGenerator::PreProcessTypeConverter()
{
    /* Convert type of specific semantics */
    TypeConverter typeConverter;
    typeConverter.Convert(*GetProgram(), GLSLConverter::ConvertVarDeclType);
}

void GLSLGenerator::PreProcessExprConverterPrimary()
{
    /* Convert expressions (Before reference analysis) */
    ExprConverter converter;
    Flags converterFlags = ExprConverter::All;

    converterFlags.Remove(ExprConverter::ConvertMatrixSubscripts);

    if (HasShadingLanguage420Pack())
    {
        /*
        Remove specific conversions when the GLSL output version is explicitly set to 4.20 or higher,
        i.e. "GL_ARB_shading_language_420pack" extension is available.
        */
        converterFlags.Remove(ExprConverter::ConvertVectorSubscripts);
        converterFlags.Remove(ExprConverter::ConvertInitializerToCtor);
    }

    converter.Convert(*GetProgram(), converterFlags, nameMangling_);
}

void GLSLGenerator::PreProcessGLSLConverter(const ShaderInput& inputDesc, const ShaderOutput& outputDesc)
{
    /* Convert AST for GLSL code generation (Before reference analysis) */
    GLSLConverter converter;
    converter.ConvertAST(*GetProgram(), inputDesc, outputDesc);
}

void GLSLGenerator::PreProcessFuncNameConverter()
{
    /* Convert function names after main conversion, since functon owner structs may have been renamed as well */
    FuncNameConverter funcNameConverter;
    funcNameConverter.Convert(
        *GetProgram(),
        nameMangling_,
        [](const FunctionDecl& lhs, const FunctionDecl& rhs)
        {
            /* Compare function signatures and ignore generic sub types (GLSL has no distinction for these types) */
            return lhs.EqualsSignature(rhs, TypeDenoter::IgnoreGenericSubType);
        },
        FuncNameConverter::All
    );
}

void GLSLGenerator::PreProcessReferenceAnalyzer(const ShaderInput& inputDesc)
{
    /* Mark all reachable AST nodes */
    ReferenceAnalyzer refAnalyzer;
    refAnalyzer.MarkReferencesFromEntryPoint(*GetProgram(), inputDesc.shaderTarget);
}

void GLSLGenerator::PreProcessExprConverterSecondary()
{
    /* Convert AST for GLSL code generation (After reference analysis) */
    ExprConverter converter;
    converter.Convert(*GetProgram(), ExprConverter::ConvertMatrixSubscripts, nameMangling_);
}

/* ----- Basics ----- */

void GLSLGenerator::WriteComment(const std::string& text)
{
    std::size_t start = 0, end = 0;

    while (end < text.size())
    {
        /* Get next comment line */
        end = text.find('\n', start);

        auto line = (end < text.size() ? text.substr(start, end - start) : text.substr(start));

        /* Write comment line */
        BeginLn();
        {
            Write("// ");
            Write(line);
        }
        EndLn();

        start = end + 1;
    }
}

void GLSLGenerator::WriteLineMark(int lineNumber)
{
    if (allowLineMarks_)
        WriteLn("#line " + std::to_string(lineNumber));
}

void GLSLGenerator::WriteLineMark(const TokenPtr& tkn)
{
    WriteLineMark(tkn->Pos().Row());
}

void GLSLGenerator::WriteLineMark(const AST* ast)
{
    WriteLineMark(ast->area.Pos().Row());
}

/* ----- Program ----- */

void GLSLGenerator::WriteProgramHeader()
{
    /* Determine all required GLSL extensions with the GLSL extension agent */
    GLSLExtensionAgent extensionAgent;
    auto requiredExtensions = extensionAgent.DetermineRequiredExtensions(
        *GetProgram(), versionOut_, GetShaderTarget(), allowExtensions_, explicitBinding_, separateShaders_,
        [this](const std::string& msg, const AST* ast)
        {
            /* Report either error or warning whether extensions are allowed or not */
            if (!allowExtensions_)
                Error(msg, ast, false);
            else if (WarnEnabled(Warnings::RequiredExtensions))
                Warning(msg, ast);
        }
    );

    /* Write GLSL version */
    if (IsESSL())
    {
        /* In ESSL, the '#version'-directive must compellingly be in the first line */
        WriteProgramHeaderVersion();
        WriteProgramHeaderComment();
    }
    else
    {
        /* In GLSL/VKSL, write the commentary first */
        WriteProgramHeaderComment();
        WriteProgramHeaderVersion();
    }

    /* Write all required extensions */
    if (!requiredExtensions.empty())
    {
        for (const auto& ext : requiredExtensions)
            WriteProgramHeaderExtension(ext);
        Blank();
    }
}

void GLSLGenerator::WriteProgramHeaderVersion()
{
    /* Convert output shader version into GLSL version number (with bitwise AND operator) */
    int versionNumber = (static_cast<int>(versionOut_)) & static_cast<int>(OutputShaderVersion::GLSL);

    BeginLn();
    {
        Write("#version " + std::to_string(versionNumber));

        if (IsLanguageESSL(versionOut_))
            Write(" es");
    }
    EndLn();
    Blank();
}

void GLSLGenerator::WriteProgramHeaderComment()
{
    if (writeHeaderComment_)
    {
        /* Write header */
        if (entryPointName_.empty())
            WriteComment("GLSL " + ToString(GetShaderTarget()));
        else
            WriteComment("GLSL " + ToString(GetShaderTarget()) + " \"" + entryPointName_ + "\"");

        WriteComment("Generated by XShaderCompiler");
        WriteComment(TimePoint());

        Blank();
    }
}

void GLSLGenerator::WriteProgramHeaderExtension(const std::string& extensionName)
{
    WriteLn("#extension " + extensionName + " : enable");// "require" or "enable"
}

/* ----- Global layouts ----- */

void GLSLGenerator::WriteGlobalLayouts()
{
    auto program = GetProgram();

    bool layoutsWritten = false;

    switch (GetShaderTarget())
    {
        case ShaderTarget::TessellationControlShader:
            layoutsWritten = WriteGlobalLayoutsTessControl(program->layoutTessControl);
            break;
        case ShaderTarget::TessellationEvaluationShader:
            layoutsWritten = WriteGlobalLayoutsTessEvaluation(program->layoutTessEvaluation);
            break;
        case ShaderTarget::GeometryShader:
            layoutsWritten = WriteGlobalLayoutsGeometry(program->layoutGeometry);
            break;
        case ShaderTarget::FragmentShader:
            layoutsWritten = WriteGlobalLayoutsFragment(program->layoutFragment);
            break;
        case ShaderTarget::ComputeShader:
            layoutsWritten = WriteGlobalLayoutsCompute(program->layoutCompute);
            break;
        default:
            break;
    }

    if (layoutsWritten)
        Blank();
}

bool GLSLGenerator::WriteGlobalLayoutsTessControl(const Program::LayoutTessControlShader& layout)
{
    WriteLayoutGlobalIn(
        {
            [&]() { Write("vertices = " + std::to_string(layout.outputControlPoints)); },
        }
    );
    return true;
}

bool GLSLGenerator::WriteGlobalLayoutsTessEvaluation(const Program::LayoutTessEvaluationShader& layout)
{
    WriteLayoutGlobalIn(
        {
            [&]()
            {
                /* Map GLSL domain type (abstract patch type) */
                if (auto keyword = AttributeValueToGLSLKeyword(layout.domainType))
                    Write(*keyword);
                else
                    Error(R_FailedToMapToGLSLKeyword(R_DomainType, R_TessAbstractPatchType));
            },

            [&]()
            {
                if (IsAttributeValuePartitioning(layout.partitioning))
                {
                    /* Map GLSL partitioning (spacing) */
                    if (auto keyword = AttributeValueToGLSLKeyword(layout.partitioning))
                        Write(*keyword);
                    else
                        Error(R_FailedToMapToGLSLKeyword(R_Partitioning, R_TessSpacing));
                }
            },

            [&]()
            {
                if (IsAttributeValueTrianglePartitioning(layout.outputTopology))
                {
                    /* Map GLSL output topology (primitive ordering) */
                    if (auto keyword = AttributeValueToGLSLKeyword(layout.outputTopology))
                        Write(*keyword);
                    else
                        Error(R_FailedToMapToGLSLKeyword(R_OutputToplogy, R_TessPrimitiveOrdering));
                }
            },
        }
    );
    return true;
}

bool GLSLGenerator::WriteGlobalLayoutsGeometry(const Program::LayoutGeometryShader& layout)
{
    /* Write input layout */
    WriteLayoutGlobalIn(
        {
            [&]()
            {
                /* Map GLSL input primitive */
                if (layout.inputPrimitive == PrimitiveType::Undefined)
                    Error(R_MissingInputPrimitiveType(R_GeometryShader));
                else if (auto keyword = PrimitiveTypeToGLSLKeyword(layout.inputPrimitive))
                    Write(*keyword);
                else
                    Error(R_FailedToMapToGLSLKeyword(R_InputGeometryPrimitive));
            },
        }
    );

    /* Write output layout */
    WriteLayoutGlobalOut(
        {
            [&]()
            {
                /* Map GLSL output primitive */
                if (layout.outputPrimitive == BufferType::Undefined)
                    Error(R_MissingOutputPrimitiveType(R_GeometryShader));
                else if (auto keyword = BufferTypeToGLSLKeyword(layout.outputPrimitive))
                    Write(*keyword);
                else
                    Error(R_FailedToMapToGLSLKeyword(R_OutputGeometryPrimitive));
            },

            [&]()
            {
                Write("max_vertices = " + std::to_string(layout.maxVertices));
            },
        }
    );

    return true;
}

bool GLSLGenerator::WriteGlobalLayoutsFragment(const Program::LayoutFragmentShader& layout)
{
    bool layoutsWritten = false;

    /* Define 'gl_FragCoord' origin to upper-left (not required for Vulkan) */
    if (!IsVKSL() && !IsESSL() && GetProgram()->layoutFragment.fragCoordUsed)
    {
        WriteLayoutGlobalIn(
            {
                [&]()
                {
                    Write("origin_upper_left");
                },

                [&]()
                {
                    if (layout.pixelCenterInteger)
                        Write("pixel_center_integer");
                },
            },
            [&]()
            {
                Write("vec4 gl_FragCoord");
            }
        );
        layoutsWritten = true;
    }

    if (layout.earlyDepthStencil)
    {
        WriteLayoutGlobalIn(
            {
                [&]() { Write("early_fragment_tests"); }
            }
        );
        layoutsWritten = true;
    }

    return layoutsWritten;
}

bool GLSLGenerator::WriteGlobalLayoutsCompute(const Program::LayoutComputeShader& layout)
{
    WriteLayoutGlobalIn(
        {
            [&]() { Write("local_size_x = " + std::to_string(layout.numThreads[0])); },
            [&]() { Write("local_size_y = " + std::to_string(layout.numThreads[1])); },
            [&]() { Write("local_size_z = " + std::to_string(layout.numThreads[2])); },
        }
    );
    return true;
}

/* ----- Built-in block redeclarations ----- */

void GLSLGenerator::WriteBuiltinBlockRedeclarations()
{
    switch (GetShaderTarget())
    {
        case ShaderTarget::TessellationControlShader:
            WriteBuiltinBlockRedeclarationsPerVertex(true, "gl_in[gl_MaxPatchVertices]");
            WriteBuiltinBlockRedeclarationsPerVertex(false, "gl_out[]");
            break;
        case ShaderTarget::TessellationEvaluationShader:
            WriteBuiltinBlockRedeclarationsPerVertex(true, "gl_in[gl_MaxPatchVertices]");
            WriteBuiltinBlockRedeclarationsPerVertex(false);
            break;
        case ShaderTarget::GeometryShader:
            WriteBuiltinBlockRedeclarationsPerVertex(true, "gl_in[]");
            WriteBuiltinBlockRedeclarationsPerVertex(false);
            break;
        case ShaderTarget::VertexShader:
            WriteBuiltinBlockRedeclarationsPerVertex(false);
            break;
        default:
            break;
    }
}

void GLSLGenerator::WriteBuiltinBlockRedeclarationsPerVertex(bool input, const std::string& name)
{
    auto entryPoint = GetProgram()->entryPointRef;

    /* Gather all semantics that are contained in the redeclared vertex block */
    std::vector<Semantic> semantics;

    if (input)
    {
        for (const auto& param : entryPoint->inputSemantics.varDeclRefsSV)
            semantics.push_back(param->semantic);
    }
    else
    {
        for (const auto& param : entryPoint->outputSemantics.varDeclRefsSV)
            semantics.push_back(param->semantic);

        if (IsSystemSemantic(entryPoint->semantic))
            semantics.push_back(entryPoint->semantic);
    }

    if (semantics.empty())
        return;

    /* Write input/output per-vertex block */
    BeginLn();
    {
        Write(input ? "in" : "out");
        Write(" gl_PerVertex");

        WriteScopeOpen(false, name.empty());
        {
            for (const auto& semantic : semantics)
            {
                switch (semantic)
                {
                    case Semantic::VertexPosition:
                        WriteLn("vec4 gl_Position;");
                        break;
                    case Semantic::PointSize:
                        WriteLn("float gl_PointSize;");
                        break;
                    case Semantic::CullDistance:
                        if (IsVKSL() || ( IsGLSL() && versionOut_ >= OutputShaderVersion::GLSL450))
                            WriteLn("float gl_CullDistance[];");
                        break;
                    case Semantic::ClipDistance:
                        WriteLn("float gl_ClipDistance[];");
                        break;
                    default:
                        break;
                }
            }
        }
        WriteScopeClose();

        if (!name.empty())
            WriteLn(name + ";");
    }
    EndLn();

    Blank();
}

/* ----- Layout ----- */

void GLSLGenerator::WriteLayout(const std::initializer_list<LayoutEntryFunctor>& entryFunctors)
{
    PushWritePrefix("layout(");
    {
        for (const auto& entryFunc : entryFunctors)
        {
            /* Write comma separator, if this is not the first entry */
            if (TopWritePrefix())
            {
                /* Push comman separator as prefix for the next layout entry */
                PushWritePrefix(", ");
                {
                    entryFunc();
                }
                PopWritePrefix();
            }
            else
            {
                /* Call function for the first layout entry */
                entryFunc();
            }
        }
    }
    PopWritePrefix(") ");
}

void GLSLGenerator::WriteLayout(const std::string& value)
{
    WriteLayout({ [&]() { Write(value); } });
}

void GLSLGenerator::WriteLayoutGlobal(const std::initializer_list<LayoutEntryFunctor>& entryFunctors, const LayoutEntryFunctor& varFunctor, const std::string& modifier)
{
    BeginLn();
    {
        WriteLayout(entryFunctors);
        if (varFunctor)
        {
            Write(modifier + ' ');
            varFunctor();
            Write(";");
        }
        else
            Write(modifier + ';');
    }
    EndLn();
}

void GLSLGenerator::WriteLayoutGlobalIn(const std::initializer_list<LayoutEntryFunctor>& entryFunctors, const LayoutEntryFunctor& varFunctor)
{
    WriteLayoutGlobal(entryFunctors, varFunctor, "in");
}

void GLSLGenerator::WriteLayoutGlobalOut(const std::initializer_list<LayoutEntryFunctor>& entryFunctors, const LayoutEntryFunctor& varFunctor)
{
    WriteLayoutGlobal(entryFunctors, varFunctor, "out");
}

void GLSLGenerator::WriteLayoutBinding(const std::vector<RegisterPtr>& slotRegisters)
{
    /* For ESSL: "binding" qualifier is only available since ESSL 310 */
    if ( explicitBinding_ && ( !IsESSL() || versionOut_ >= OutputShaderVersion::ESSL310 ) )
    {
        if (auto slotRegister = Register::GetForTarget(slotRegisters, GetShaderTarget()))
            Write("binding = " + std::to_string(slotRegister->slot));
    }
}

/* ----- Input semantics ----- */

void GLSLGenerator::WriteLocalInputSemantics(FunctionDecl* entryPoint)
{
    entryPoint->inputSemantics.ForEach(
        [this](VarDecl* varDecl)
        {
            if (varDecl->flags(Decl::isWrittenTo))
                WriteLocalInputSemanticsVarDecl(varDecl);
        }
    );

    for (auto& param : entryPoint->parameters)
    {
        const auto& typeDen = param->typeSpecifier->GetTypeDenoter()->GetAliased();
        if (auto structTypeDen = typeDen.As<StructTypeDenoter>())
            WriteLocalInputSemanticsStructDeclParam(param.get(), structTypeDen->structDeclRef);
    }
}

void GLSLGenerator::WriteLocalInputSemanticsVarDecl(VarDecl* varDecl)
{
    /* Is semantic of the variable declaration a system value semantic? */
    auto semanticKeyword = SystemValueToKeyword(varDecl->semantic);

    if (!semanticKeyword)
    {
        semanticKeyword = MakeUnique<std::string>(varDecl->ident);
        varDecl->ident.AppendPrefix(nameMangling_.temporaryPrefix);
    }

    /* Write local variable definition statement */
    BeginLn();
    {
        /* Write desired variable type and identifier */
        auto typeSpecifier = varDecl->declStmntRef->typeSpecifier.get();

        Visit(typeSpecifier);
        Write(" " + varDecl->ident + " = ");

        /* Is a type conversion required? */
        if (!IsTypeCompatibleWithSemantic(varDecl->semantic, typeSpecifier->typeDenoter->GetAliased()))
        {
            /* Write type cast with semantic keyword */
            Visit(typeSpecifier);
            Write("(" + *semanticKeyword + ");");
        }
        else
        {
            /* Write semantic keyword */
            Write(*semanticKeyword + ";");
        }
    }
    EndLn();
}

void GLSLGenerator::WriteLocalInputSemanticsStructDeclParam(VarDeclStmnt* param, StructDecl* structDecl)
{
    if (structDecl && structDecl->flags(StructDecl::isNonEntryPointParam) && structDecl->flags(StructDecl::isShaderInput))
    {
        /* Write parameter as variable declaration */
        Visit(param);

        BeginSep();
        {
            /* Write global shader input to local variable assignments */
            auto paramVar = param->varDecls.front().get();

            if (paramVar->arrayDims.empty())
            {
                structDecl->ForEachVarDecl(
                    [&](VarDeclPtr& varDecl)
                    {
                        BeginLn();
                        {
                            Separator();
                            Write(paramVar->ident + "." + varDecl->ident.Original());
                            Separator();
                            Write(" = ");
                            WriteVarDeclIdentOrSystemValue(varDecl.get());
                            Write(";");
                        }
                        EndLn();
                    }
                );
            }
            else if (paramVar->arrayDims.size() == 1)
            {
                /* Get array dimension sizes from parameter */
                auto arraySize = paramVar->arrayDims.front()->size;

                for (int i = 0; i < arraySize; ++i)
                {
                    /* Construct array indices output string */
                    structDecl->ForEachVarDecl(
                        [&](VarDeclPtr& varDecl)
                        {
                            BeginLn();
                            {
                                Separator();
                                Write(paramVar->ident + "[" + std::to_string(i) + "]." + varDecl->ident.Original());
                                Separator();
                                Write(" = ");
                                WriteVarDeclIdentOrSystemValue(varDecl.get(), i);
                                Write(";");
                            }
                            EndLn();
                        }
                    );
                }
            }
            else
                Error(R_TooManyIndicesForShaderInputParam, paramVar);
        }
        EndSep();
    }
}

void GLSLGenerator::WriteGlobalInputSemantics(FunctionDecl* entryPoint)
{
    auto& varDeclRefs = entryPoint->inputSemantics.varDeclRefs;

    for (auto varDecl : varDeclRefs)
        WriteGlobalInputSemanticsVarDecl(varDecl);

    if (!varDeclRefs.empty())
        Blank();
}

void GLSLGenerator::WriteGlobalInputSemanticsVarDecl(VarDecl* varDecl)
{
    /* Write global variable definition statement */
    BeginLn();
    {
        const auto& interpModifiers = varDecl->declStmntRef->typeSpecifier->interpModifiers;

        if (versionOut_ <= OutputShaderVersion::GLSL120)
        {
            if (WarnEnabled(Warnings::Basic) && !interpModifiers.empty())
                Warning(R_InterpModNotSupportedForGLSL120, varDecl);

            if (IsVertexShader())
                Write("attribute ");
            else
                Write("varying ");
            Separator();
        }
        else
        {
            WriteInterpModifiers(interpModifiers, varDecl->declStmntRef);
            Separator();

            if ( ( !IsESSL() && explicitBinding_ ) || ( IsESSL() && IsVertexShader() ) )
            {
                /* Get slot index */
                int location = -1;

                if (IsVertexShader() && varDecl->semantic.IsValid())
                {
                    /* Fetch location from globally specified vertex semantic map (e.g. '-S<IDENT>=VALUE' shell command) */
                    auto it = vertexSemanticsMap_.find(ToCiString(varDecl->semantic.ToString()));
                    if (it != vertexSemanticsMap_.end())
                    {
                        location = it->second.location;
                        it->second.found = true;
                    }
                }

                if (location == -1 && autoBinding_)
                    location = GetBindingLocation(varDecl->GetTypeDenoter().get(), true);

                if (location != -1)
                {
                    /* Write layout location */
                    WriteLayout(
                        {
                            [&]() { Write("location = " + std::to_string(location)); }
                        }
                    );

                    /* Reset the semantic index for code reflection output */
                    varDecl->semantic.ResetIndex(location);
                }
            }

            Separator();
            Write("in ");
            Separator();
        }

        Visit(varDecl->declStmntRef->typeSpecifier);
        Separator();

        Write(" " + varDecl->ident);

        if (varDecl->flags(VarDecl::isDynamicArray))
            Write("[]");

        Write(";");
    }
    EndLn();
}

/* ----- Output semantics ----- */

void GLSLGenerator::WriteLocalOutputSemantics(FunctionDecl* entryPoint)
{
    for (const auto& param : entryPoint->parameters)
    {
        const auto& typeDen = param->typeSpecifier->GetTypeDenoter()->GetAliased();
        if (auto structTypeDen = typeDen.As<StructTypeDenoter>())
            WriteLocalOutputSemanticsStructDeclParam(param.get(), structTypeDen->structDeclRef);
    }
}

void GLSLGenerator::WriteLocalOutputSemanticsStructDeclParam(VarDeclStmnt* param, StructDecl* structDecl)
{
    if (structDecl && structDecl->flags(StructDecl::isNonEntryPointParam) && structDecl->flags(StructDecl::isShaderOutput))
    {
        /* Write parameter as variable declaration */
        Visit(param);
    }
}

void GLSLGenerator::WriteGlobalOutputSemantics(FunctionDecl* entryPoint)
{
    /* Write non-system-value output semantics */
    auto& varDeclRefs = entryPoint->outputSemantics.varDeclRefs;

    bool paramsWritten = (!varDeclRefs.empty());

    for (auto varDecl : varDeclRefs)
        WriteGlobalOutputSemanticsVarDecl(varDecl);

    /* Write 'SV_Target' system-value output semantics */
    if (IsFragmentShader() && versionOut_ > OutputShaderVersion::GLSL120)
    {
        /* Write 'SV_Target' system-value output semantics from variables */
        auto& varDeclRefs = entryPoint->outputSemantics.varDeclRefsSV;

        for (auto varDecl : varDeclRefs)
        {
            if (varDecl->semantic == Semantic::Target)
            {
                WriteGlobalOutputSemanticsVarDecl(varDecl, true);
                paramsWritten = true;
            }
        }

        if (entryPoint->semantic == Semantic::Target)
        {
            /* Write 'SV_Target' system-value output semantic from entry point return semantic */
            WriteGlobalOutputSemanticsSlot(
                entryPoint->returnType.get(),
                entryPoint->semantic,
                entryPoint->semantic.ToString()
            );
            paramsWritten = true;
        }
    }

    if (entryPoint->semantic.IsUserDefined())
    {
        /* Write user-defined output semantic from entry point return semantic */
        WriteGlobalOutputSemanticsSlot(
            entryPoint->returnType.get(),
            entryPoint->semantic,
            nameMangling_.outputPrefix + entryPoint->semantic.ToString()
        );
        paramsWritten = true;
    }

    if (paramsWritten)
        Blank();
}

void GLSLGenerator::WriteGlobalOutputSemanticsVarDecl(VarDecl* varDecl, bool useSemanticName)
{
    /* Write global variable definition statement */
    WriteGlobalOutputSemanticsSlot(
        varDecl->declStmntRef->typeSpecifier.get(),
        varDecl->semantic,
        (useSemanticName ? varDecl->semantic.ToString() : varDecl->ident.Final()),
        varDecl
    );
}

void GLSLGenerator::WriteGlobalOutputSemanticsSlot(TypeSpecifier* typeSpecifier, IndexedSemantic& semantic, const std::string& ident, VarDecl* varDecl)
{
    /* Write global output semantic slot */
    BeginLn();
    {
        VarDeclStmnt* varDeclStmnt = (varDecl != nullptr ? varDecl->declStmntRef : nullptr);

        if (versionOut_ <= OutputShaderVersion::GLSL120)
        {
            if (WarnEnabled(Warnings::Basic) && varDeclStmnt && !varDeclStmnt->typeSpecifier->interpModifiers.empty())
                Warning(R_InterpModNotSupportedForGLSL120, varDecl);

            Write("varying ");
            Separator();
        }
        else
        {
            if (varDeclStmnt)
                WriteInterpModifiers(varDeclStmnt->typeSpecifier->interpModifiers, varDecl);
            Separator();

            if ( ( !IsESSL() && explicitBinding_ ) || ( IsESSL() && IsFragmentShader() ) )
            {
                /* Get slot index: directly for fragment output, and automatically otherwise */
                int location = -1;

                if (IsFragmentShader())
                    location = semantic.Index();
                else if (autoBinding_)
                    location = GetBindingLocation(typeSpecifier->typeDenoter.get(), false);

                if (location != -1)
                {
                    /* Write layout location */
                    WriteLayout(
                        {
                            [&]() { Write("location = " + std::to_string(location)); }
                        }
                    );

                    /* Reset the semantic index for code reflection output */
                    semantic.ResetIndex(location);
                }
            }

            Write("out ");
            Separator();
        }

        Visit(typeSpecifier);
        Separator();

        Write(" " + ident);

        if (varDecl && varDecl->flags(VarDecl::isDynamicArray))
            Write("[]");

        Write(";");
    }
    EndLn();
}

void GLSLGenerator::WriteOutputSemanticsAssignment(Expr* expr, bool writeAsListedExpr)
{
    auto entryPoint = GetProgram()->entryPointRef;

    /* Fetch variable identifier if expression is set */
    const ObjectExpr* lvalueExpr = nullptr;
    if (expr)
        lvalueExpr = expr->FetchLValueExpr();

    /* Write wrapped structures */
    for (const auto& paramStruct : entryPoint->paramStructs)
    {
        if (paramStruct.expr == nullptr || paramStruct.expr == expr)
            WriteOutputSemanticsAssignmentStructDeclParam(paramStruct, writeAsListedExpr);
    }

    /* Write assignment to single function return semantic */
    auto semantic = entryPoint->semantic;

    if (expr)
    {
        if (semantic.IsValid())
        {
            if (semantic.IsSystemValue())
            {
                if (auto semanticKeyword = SystemValueToKeyword(semantic))
                {
                    BeginLn();
                    {
                        Write(*semanticKeyword);
                        Write(" = ");
                        Visit(expr);
                        Write(";");
                    }
                    EndLn();
                }
                else
                    Error(R_FailedToMapToGLSLKeyword(R_OutputSemantic), entryPoint);
            }
            else if (semantic.IsUserDefined())
            {
                BeginLn();
                {
                    Write(nameMangling_.outputPrefix + semantic.ToString());
                    Write(" = ");
                    Visit(expr);
                    Write(";");
                }
                EndLn();
            }
        }
    }
}

void GLSLGenerator::WriteOutputSemanticsAssignmentStructDeclParam(
    const FunctionDecl::ParameterStructure& paramStruct, bool writeAsListedExpr, const std::string& tempIdent)
{
    auto paramExpr  = paramStruct.expr;
    auto paramVar   = paramStruct.varDecl;
    auto structDecl = paramStruct.structDecl;

    if (structDecl && structDecl->flags(StructDecl::isNonEntryPointParam) && structDecl->flags(StructDecl::isShaderOutput))
    {
        /* Write global shader input to local variable assignments */
        structDecl->ForEachVarDecl(
            [&](VarDeclPtr& varDecl)
            {
                auto openLine = IsOpenLine();
                if (!writeAsListedExpr && !openLine)
                    BeginLn();

                if (auto semanticKeyword = SystemValueToKeyword(varDecl->semantic))
                    Write(*semanticKeyword);
                else
                    Write(varDecl->ident);

                Write(" = ");

                if (paramExpr)
                    Visit(paramExpr);
                else if (paramVar)
                    Write(paramVar->ident);
                else
                    Write(tempIdent);

                Write("." + varDecl->ident.Original() + (writeAsListedExpr ? ", " : ";"));

                if (!writeAsListedExpr)
                {
                    EndLn();
                    if (openLine)
                        BeginLn();
                }
            }
        );
    }
}

/* ----- Uniforms ----- */

void GLSLGenerator::WriteGlobalUniforms()
{
    bool uniformsWritten = false;

    for (auto& param : GetProgram()->entryPointRef->parameters)
    {
        if (param->IsUniform())
        {
            WriteGlobalUniformsParameter(param.get());
            uniformsWritten = true;
        }
    }

    if (uniformsWritten)
        Blank();
}

void GLSLGenerator::WriteGlobalUniformsParameter(VarDeclStmnt* param)
{
    /* Write uniform type */
    BeginLn();
    {
        Write("uniform ");
        Visit(param->typeSpecifier);
        Write(" ");

        /* Write parameter identifier */
        if (param->varDecls.size() == 1)
            Visit(param->varDecls.front());
        else
            Error(R_InvalidParamVarCount, param);

        Write(";");
    }
    EndLn();
}

void GLSLGenerator::WriteVarDeclIdentOrSystemValue(VarDecl* varDecl, int arrayIndex)
{
    /* Find system value semantic in variable identifier */
    if (auto semanticKeyword = SystemValueToKeyword(varDecl->semantic))
    {
        if (arrayIndex >= 0)
        {
            if (varDecl->flags(VarDecl::isShaderInput))
                Write("gl_in");
            else
                Write("gl_out");
            Write("[" + std::to_string(arrayIndex) + "].");
        }
        Write(*semanticKeyword);
    }
    else
    {
        Write(varDecl->ident);
        if (arrayIndex >= 0)
            Write("[" + std::to_string(arrayIndex) + "]");
    }
}

/* ----- Object expression ----- */

void GLSLGenerator::WriteObjectExpr(const ObjectExpr& objectExpr)
{
    if (objectExpr.flags(ObjectExpr::isImmutable))
        WriteObjectExprIdent(objectExpr);
    else if (auto symbol = objectExpr.symbolRef)
        WriteObjectExprIdentOrSystemValue(objectExpr, symbol);
    else
        WriteObjectExprIdent(objectExpr);
}

void GLSLGenerator::WriteObjectExprIdent(const ObjectExpr& objectExpr, bool writePrefix)
{
    /* Write prefix expression */
    if (objectExpr.prefixExpr && !objectExpr.isStatic && writePrefix)
    {
        Visit(objectExpr.prefixExpr);

        if (auto literalExpr = objectExpr.prefixExpr->As<LiteralExpr>())
        {
            /* Append space between integer literal and '.' swizzle operator */
            if (literalExpr->IsSpaceRequiredForSubscript())
                Write(" ");
        }

        Write(".");
    }

    /* Write object identifier either from object expression or from symbol reference */
    if (auto symbol = objectExpr.symbolRef)
    {
        /* Write original identifier, if the identifier was marked as immutable */
        if (objectExpr.flags(ObjectExpr::isImmutable))
            Write(symbol->ident.Original());
        else
            Write(symbol->ident);
    }
    else
        Write(objectExpr.ident);
}

/*
Writes either the object identifier as it is (e.g. "vertexOutput.position.xyz"),
or a system value if the identifier has a system value semantic (e.g. "gl_Position.xyz").
*/
void GLSLGenerator::WriteObjectExprIdentOrSystemValue(const ObjectExpr& objectExpr, Decl* symbol)
{
    /* Find system value semantic in object identifier */
    std::unique_ptr<std::string> semanticKeyword;
    Flags varFlags;

    if (auto varDecl = symbol->As<VarDecl>())
    {
        /* Copy flags from variable */
        varFlags = varDecl->flags;

        /* Is this variable an entry-point output semantic, or an r-value? */
        if (GetProgram()->entryPointRef->outputSemantics.Contains(varDecl) || !varDecl->flags(Decl::isWrittenTo))
        {
            /* Get GLSL keyword for system value semantic (or null if semantic is no system value) */
            semanticKeyword = SystemValueToKeyword(varDecl->semantic);
        }
    }

    if (varFlags(VarDecl::isShaderInput | VarDecl::isShaderOutput) && objectExpr.prefixExpr)
    {
        /* Write special "gl_in/out" array prefix, or write array indices as postfix for input/output semantics */
        if (auto arrayExpr = objectExpr.prefixExpr->FindFirstNotOf(AST::Types::BracketExpr)->As<ArrayExpr>())
        {
            if (semanticKeyword)
            {
                /* Example: gl_in[0].gl_Position */
                if (varFlags(VarDecl::isShaderInput))
                    Write("gl_in");
                else
                    Write("gl_out");
                WriteArrayIndices(arrayExpr->arrayIndices);
                Write("." + *semanticKeyword);
            }
            else
            {
                /* Example: xsv_NORMAL0[0] */
                WriteObjectExprIdent(objectExpr, false);
                WriteArrayIndices(arrayExpr->arrayIndices);
            }
        }
        else
            Error(R_MissingArrayPrefixForIOSemantic(objectExpr.ident), &objectExpr);
    }
    else if (semanticKeyword)
    {
        /* Ignore prefix expression if the object refers to a system value semantic */
        Write(*semanticKeyword);
    }
    else
    {
        /* Write object expression with standard identifier */
        WriteObjectExprIdent(objectExpr);
    }
}

/* ----- Array expression ----- */

void GLSLGenerator::WriteArrayExpr(const ArrayExpr& arrayExpr)
{
    Visit(arrayExpr.prefixExpr);
    WriteArrayIndices(arrayExpr.arrayIndices);
}

void GLSLGenerator::WriteArrayIndices(const std::vector<ExprPtr>& arrayIndices)
{
    for (auto& arrayIndex : arrayIndices)
    {
        Write("[");
        Visit(arrayIndex);
        Write("]");
    }
}

/* ----- Type denoter ----- */

void GLSLGenerator::WriteStorageClasses(const std::set<StorageClass>& storageClasses, const AST* ast)
{
    for (auto storage : storageClasses)
    {
        /* Ignore static storage class (reserved word in GLSL) */
        if (storage != StorageClass::Static)
        {
            if (auto keyword = StorageClassToGLSLKeyword(storage))
                Write(*keyword + " ");
            else if (WarnEnabled(Warnings::Basic))
                Warning(R_NotAllStorageClassesMappedToGLSL, ast);
        }
    }
}

void GLSLGenerator::WriteInterpModifiers(const std::set<InterpModifier>& interpModifiers, const AST* ast)
{
    for (auto modifier : interpModifiers)
    {
        if (auto keyword = InterpModifierToGLSLKeyword(modifier))
            Write(*keyword + " ");
        else if (WarnEnabled(Warnings::Basic))
            Warning(R_NotAllInterpModMappedToGLSL, ast);
    }
}

void GLSLGenerator::WriteTypeModifiers(const std::set<TypeModifier>& typeModifiers, const TypeDenoterPtr& typeDenoter)
{
    /* Matrix packing alignment can only be written for uniform buffers */
    if (InsideUniformBufferDecl() && typeDenoter && typeDenoter->IsMatrix())
    {
        const auto commonStorageLayout = GetUniformBufferDeclStack().back()->commonStorageLayout;

        if (commonStorageLayout == TypeModifier::ColumnMajor)
        {
            /* Only write 'row_major' type modifier, because 'column_major' is the default in the current uniform buffer */
            if (typeModifiers.find(TypeModifier::RowMajor) != typeModifiers.end())
                WriteLayout("row_major");
        }
        else
        {
            /* Only write 'column_major' type modifier, because 'row_major' is the default in the current uniform buffer */
            if (typeModifiers.find(TypeModifier::ColumnMajor) != typeModifiers.end())
                WriteLayout("column_major");
        }
    }

    if (typeModifiers.find(TypeModifier::Const) != typeModifiers.end())
    {
        /*
        Write const type modifier, but only if GLSL version is at leat 420,
        because GLSL does only support const expression initializers for constant objects.
        see https://www.khronos.org/opengl/wiki/Type_Qualifier_(GLSL)#Constant_qualifier
        */
        if ( ( IsGLSL() && versionOut_ >= OutputShaderVersion::GLSL420 ) || IsVKSL() )
            Write("const ");
    }
}

void GLSLGenerator::WriteTypeModifiersFrom(const TypeSpecifierPtr& typeSpecifier)
{
    WriteTypeModifiers(typeSpecifier->typeModifiers, typeSpecifier->GetTypeDenoter()->GetSub());
}

void GLSLGenerator::WriteDataType(DataType dataType, bool writePrecisionSpecifier, const AST* ast)
{
    /* Replace doubles with floats, if doubles are not supported */
    if (versionOut_ < OutputShaderVersion::GLSL400)
        dataType = DoubleToFloatDataType(dataType);

    /* Write optional precision specifier */
    if (writePrecisionSpecifier)
    {
        if (IsHalfRealType(dataType))
            Write("mediump ");
        else
            Write("highp ");
    }

    /* Map GLSL data type */
    if (auto keyword = DataTypeToGLSLKeyword(dataType))
        Write(*keyword);
    else
        Error(R_FailedToMapToGLSLKeyword(R_DataType), ast);
}

void GLSLGenerator::WriteTypeDenoter(const TypeDenoter& typeDenoter, bool writePrecisionSpecifier, const AST* ast)
{
    try
    {
        if (typeDenoter.IsVoid())
        {
            /* Just write void type */
            Write("void");
        }
        else if (auto baseTypeDen = typeDenoter.As<BaseTypeDenoter>())
        {
            /* Map GLSL base type */
            WriteDataType(baseTypeDen->dataType, writePrecisionSpecifier, ast);
        }
        else if (auto bufferTypeDen = typeDenoter.As<BufferTypeDenoter>())
        {
            /* Get buffer type */
            auto bufferType = bufferTypeDen->bufferType;
            if (bufferType == BufferType::Undefined)
            {
                if (auto bufferDecl = bufferTypeDen->bufferDeclRef)
                    bufferType = bufferDecl->GetBufferType();
                else
                    Error(R_MissingRefInTypeDen(R_BufferTypeDen), ast);
            }

            /* Convert buffer type to GLSL buffer (or sampler type) */
            if (auto keyword = BufferTypeToKeyword(bufferType, ast))
                Write(*keyword);
        }
        else if (auto samplerTypeDen = typeDenoter.As<SamplerTypeDenoter>())
        {
            /* Get sampler type */
            auto samplerType = samplerTypeDen->samplerType;
            if (samplerType == SamplerType::Undefined)
            {
                if (auto samplerDecl = samplerTypeDen->samplerDeclRef)
                    samplerType = samplerDecl->GetSamplerType();
                else
                    Error(R_MissingRefInTypeDen(R_SamplerTypeDen), ast);
            }

            if (!IsSamplerStateType(samplerType) || UseSeparateSamplers())
            {
                /* Convert sampler type to GLSL sampler type */
                if (auto keyword = SamplerTypeToKeyword(samplerType, ast))
                    Write(*keyword);
            }
            else
                Error(R_CantTranslateSamplerToGLSL, ast);
        }
        else if (auto structTypeDen = typeDenoter.As<StructTypeDenoter>())
        {
            /* Write struct identifier (either from structure declaration or stored identifier) */
            if (auto structDecl = structTypeDen->structDeclRef)
                Write(structDecl->ident);
            else
                Write(typeDenoter.Ident());
        }
        else if (typeDenoter.IsAlias())
        {
            /* Write aliased type denoter */
            WriteTypeDenoter(typeDenoter.GetAliased(), writePrecisionSpecifier, ast);
        }
        else if (auto arrayTypeDen = typeDenoter.As<ArrayTypeDenoter>())
        {
            /* Write sub type of array type denoter and array dimensions */
            WriteTypeDenoter(*arrayTypeDen->subTypeDenoter, writePrecisionSpecifier, ast);
            Visit(arrayTypeDen->arrayDims);
        }
        else
            Error(R_FailedToDetermineGLSLDataType, ast);
    }
    catch (const Report&)
    {
        throw;
    }
    catch (const std::exception& e)
    {
        Error(e.what(), ast);
    }
}

/* ----- Function declaration ----- */

void GLSLGenerator::WriteFunction(FunctionDecl* ast)
{
    /* Write function header */
    if (auto structDecl = ast->returnType->structDecl.get())
    {
        /* Write structure declaration of function return type as a separated declaration */
        StructDeclArgs structDeclArgs;
        structDeclArgs.inEndWithSemicolon = true;

        Visit(structDecl, &structDeclArgs);

        BeginLn();
        Write(structDecl->ident + " " + ast->ident + "(");
    }
    else
    {
        BeginLn();
        Visit(ast->returnType);
        Write(" " + ast->ident + "(");
    }

    /* Write parameters */
    for (std::size_t i = 0; i < ast->parameters.size(); ++i)
    {
        WriteParameter(ast->parameters[i].get());
        if (i + 1 < ast->parameters.size())
            Write(", ");
    }

    Write(")");

    if (ast->codeBlock)
    {
        /* Write function body */
        Visit(ast->codeBlock);
    }
    else
    {
        /* This is only a function forward declaration, so finish with statement terminator */
        Write(";");
        EndLn();
    }
}

void GLSLGenerator::WriteFunctionEntryPoint(FunctionDecl* ast)
{
    if (ast->IsForwardDecl())
        return;

    /* Write function header */
    BeginLn();
    Write("void main()");

    /* Write function body */
    WriteScopeOpen();
    {
        if (IsTessControlShader())
        {
            //TODO:
            // THIS IS INCOMPLETE!!!
            // more work is to do, to translate the patch constant function to GLSL!)
            if (auto patchConstFunc = GetProgram()->layoutTessControl.patchConstFunctionRef)
            {
                /* Call patch constant function inside main entry point only for the first invocation */
                WriteLn("if (gl_InvocationID == 0)");
                IncIndent();
                {
                    WriteLn(patchConstFunc->ident + "();");
                }
                DecIndent();
                Blank();
            }
        }

        WriteFunctionEntryPointBody(ast);
    }
    WriteScopeClose();
}

void GLSLGenerator::WriteFunctionEntryPointBody(FunctionDecl* ast)
{
    /* Write input/output parameters of system values as local variables */
    WriteLocalInputSemantics(ast);
    WriteLocalOutputSemantics(ast);

    /* Write code block (without additional scope) */
    WriteStmntList(ast->codeBlock->stmnts);

    /* Is the last statement a return statement? (ignore if the function has a non-void return type) */
    if ( ast->HasVoidReturnType() && ( ast->codeBlock->stmnts.empty() || ast->codeBlock->stmnts.back()->Type() != AST::Types::ReturnStmnt ) )
    {
        /* Write output semantic at the end of the code block, if no return statement was written before */
        WriteOutputSemanticsAssignment(nullptr);
    }
}

void GLSLGenerator::WriteFunctionSecondaryEntryPoint(FunctionDecl* ast)
{
    if (ast->IsForwardDecl())
        return;

    /* Write function header */
    BeginLn();
    Write("void " + ast->ident + "()");

    /* Write function body */
    WriteScopeOpen();
    {
        WriteFunctionEntryPointBody(ast);
    }
    WriteScopeClose();
}

/* ----- Function call ----- */

void GLSLGenerator::AssertIntrinsicNumArgs(CallExpr* funcCall, std::size_t numArgsMin, std::size_t numArgsMax)
{
    auto numArgs = funcCall->arguments.size();
    if (numArgs < numArgsMin || numArgs > numArgsMax)
        Error(R_InvalidIntrinsicArgCount(funcCall->ident), funcCall);
}

void GLSLGenerator::WriteCallExprStandard(CallExpr* funcCall)
{
    /* Write function name */
    if (funcCall->intrinsic != Intrinsic::Undefined)
    {
        if (!IsWrappedIntrinsic(funcCall->intrinsic))
        {
            /* Write GLSL intrinsic keyword */
            if (auto keyword = IntrinsicToGLSLKeyword(funcCall->intrinsic))
                Write(*keyword);
            else
                ErrorIntrinsic(funcCall->ident, funcCall);
        }
        else if (!funcCall->ident.empty())
        {
            /* Write wrapper function name */
            Write(funcCall->ident);
        }
        else
            Error(R_MissingFuncName, funcCall);
    }
    else if (auto funcDecl = funcCall->GetFunctionImpl())
    {
        /* Write final identifier of function declaration */
        Write(funcDecl->ident);
    }
    else if (funcCall->flags(CallExpr::isWrapperCall))
    {
        /* Write expression identifier */
        Write(funcCall->ident);
    }
    else if (funcCall->typeDenoter)
    {
        /* Write type denoter */
        WriteTypeDenoter(*funcCall->typeDenoter, false, funcCall);
    }
    else
        Error(R_MissingFuncName, funcCall);

    /* Write arguments */
    Write("(");
    WriteCallExprArguments(funcCall);
    Write(")");
}

void GLSLGenerator::WriteCallExprIntrinsicMul(CallExpr* funcCall)
{
    AssertIntrinsicNumArgs(funcCall, 2, 2);

    auto WriteMulArgument = [&](const ExprPtr& expr)
    {
        /*
        Determine if the expression needs extra brackets when converted from a function call "mul(lhs, rhs)" to a binary expression "lhs * rhs",
        e.g. "mul(wMatrix, pos + float4(0, 1, 0, 0))" -> "wMatrix * (pos + float4(0, 1, 0, 0))" needs extra brackets
        */
        auto type = expr->Type();
        if (type == AST::Types::TernaryExpr || type == AST::Types::BinaryExpr || type == AST::Types::UnaryExpr || type == AST::Types::PostUnaryExpr)
        {
            Write("(");
            Visit(expr);
            Write(")");
        }
        else
            Visit(expr);
    };

    /* Convert this function call into a multiplication */
    Write("(");
    {
        /* Swap order of arguments */
        WriteMulArgument(funcCall->arguments[1]);
        Write(" * ");
        WriteMulArgument(funcCall->arguments[0]);
    }
    Write(")");
}

void GLSLGenerator::WriteCallExprIntrinsicRcp(CallExpr* funcCall)
{
    AssertIntrinsicNumArgs(funcCall, 1, 1);

    /* Get type denoter of argument expression */
    auto& expr = funcCall->arguments.front();
    const auto& typeDen = expr->GetTypeDenoter()->GetAliased();

    if (auto baseTypeDen = typeDen.As<BaseTypeDenoter>())
    {
        /* Convert this function call into a division */
        Write("(");
        {
            WriteTypeDenoter(*baseTypeDen, false, funcCall);
            Write("(");
            WriteLiteral("1", baseTypeDen->dataType, funcCall);
            Write(") / (");
            Visit(expr);
        }
        Write("))");
    }
    else
        Error(R_InvalidIntrinsicArgType("rcp"), expr.get());
}

void GLSLGenerator::WriteCallExprIntrinsicClip(CallExpr* funcCall)
{
    AssertIntrinsicNumArgs(funcCall, 1, 1);

    /* Get type denoter of argument expression */
    auto& expr = funcCall->arguments.front();
    const auto& typeDen = expr->GetTypeDenoter()->GetAliased();

    if (auto baseTypeDen = typeDen.As<BaseTypeDenoter>())
    {
        /* Convert this function call into a condition */
        Write("if (");

        if (baseTypeDen->IsVector())
        {
            /* Convert to: 'any(lessThan(...))' */
            Write("any(lessThan(");

            auto binaryExpr = expr->As<BinaryExpr>();

            if (binaryExpr && binaryExpr->op == BinaryOp::Sub)
            {
                /* Convert to: 'any(lessThan(LHS-EXPR, RHS-EXPR))' */
                Visit(binaryExpr->lhsExpr);
                Write(", ");
                Visit(binaryExpr->rhsExpr);
            }
            else
            {
                /* Convert to: 'any(lessThan(EXPR, TYPE(0)))' */
                Visit(expr);
                Write(", ");
                WriteLiteral("0", baseTypeDen->dataType, expr.get());
            }

            Write("))");
        }
        else if (baseTypeDen->IsScalar())
        {
            /* Convert to: 'EXPR < ...' */
            auto binaryExpr = expr->As<BinaryExpr>();

            if (binaryExpr && binaryExpr->op == BinaryOp::Sub)
            {
                /* Convert to: 'LHS-EXPR < RHS-EXPR' */
                Visit(binaryExpr->lhsExpr);
                Write(" < ");
                Visit(binaryExpr->rhsExpr);
            }
            else
            {
                /* Convert to: 'EXPR < TYPE(0)' */
                Visit(expr);
                Write(" < ");
                WriteLiteral("0", baseTypeDen->dataType, expr.get());
            }
        }
        else
            Error(R_InvalidIntrinsicArgType("clip"), expr.get());
    }
    else
        Error(R_InvalidIntrinsicArgType("clip"), expr.get());

    Write(")");

    /* Write if-body (we are still inside an active line, so first 'EndLn', then 'BeginLn') */
    EndLn();
    IncIndent();
    BeginLn();
    Write("discard");
    DecIndent();
}

void GLSLGenerator::WriteCallExprIntrinsicAtomic(CallExpr* callExpr)
{
    AssertIntrinsicNumArgs(callExpr, 2, 3);

    /* Find atomic intrinsic mapping */
    if (auto keyword = IntrinsicToGLSLKeyword(callExpr->intrinsic))
    {
        /* Write function call */
        if (callExpr->arguments.size() >= 3)
        {
            Visit(callExpr->arguments[2]);
            Write(" = ");
        }
        Write(*keyword + "(");
        WriteCallExprArguments(callExpr, 0, 2);
        Write(")");
    }
    else
        ErrorIntrinsic(callExpr->ident, callExpr);
}

void GLSLGenerator::WriteCallExprIntrinsicAtomicCompSwap(CallExpr* callExpr)
{
    AssertIntrinsicNumArgs(callExpr, 4, 4);

    /* Find atomic intrinsic mapping */
    if (auto keyword = IntrinsicToGLSLKeyword(callExpr->intrinsic))
    {
        /* Write function call */
        Visit(callExpr->arguments[3]);
        Write(" = " + *keyword + "(");
        WriteCallExprArguments(callExpr, 0, 3);
        Write(")");
    }
    else
        ErrorIntrinsic(callExpr->ident, callExpr);
}

void GLSLGenerator::WriteCallExprIntrinsicImageAtomic(CallExpr* callExpr)
{
    AssertIntrinsicNumArgs(callExpr, 3, 4);

    /* Find atomic intrinsic mapping */
    if (auto keyword = IntrinsicToGLSLKeyword(callExpr->intrinsic))
    {
        /* Write function call */
        if (callExpr->arguments.size() >= 4)
        {
            Visit(callExpr->arguments[3]);
            Write(" = ");
        }
        Write(*keyword + "(");
        WriteCallExprArguments(callExpr, 0, 3);
        Write(")");
    }
    else
        ErrorIntrinsic(callExpr->ident, callExpr);
}

void GLSLGenerator::WriteCallExprIntrinsicImageAtomicCompSwap(CallExpr* callExpr)
{
    AssertIntrinsicNumArgs(callExpr, 5, 5);

    /* Find atomic intrinsic mapping */
    if (auto keyword = IntrinsicToGLSLKeyword(callExpr->intrinsic))
    {
        /* Write function call */
        Visit(callExpr->arguments[4]);
        Write(" = " + *keyword + "(");
        WriteCallExprArguments(callExpr, 0, 4);
        Write(")");
    }
    else
        ErrorIntrinsic(callExpr->ident, callExpr);
}

void GLSLGenerator::WriteCallExprIntrinsicStreamOutputAppend(CallExpr* funcCall)
{
    AssertIntrinsicNumArgs(funcCall, 1, 1);

    /* Write output semantic assignments by intrinsic argument */
    auto expr = funcCall->arguments.front().get();
    WriteOutputSemanticsAssignment(expr, true);

    /* Write "EmitVertex" intrinsic */
    Write("EmitVertex()");
}

// "CalculateLevelOfDetail"          -> "textureQueryLod(...).y"
// "CalculateLevelOfDetailUnclamped" -> "textureQueryLod(...).x"
void GLSLGenerator::WriteCallExprIntrinsicTextureQueryLod(CallExpr* funcCall, bool clamped)
{
    AssertIntrinsicNumArgs(funcCall, 2, 2);

    /* Find 'textureQueryLod' intrinsic mapping */
    if (auto keyword = IntrinsicToGLSLKeyword(funcCall->intrinsic))
    {
        /* Write function call */
        Write(*keyword + "(");
        Visit(funcCall->arguments[0]);
        Write(", ");
        Visit(funcCall->arguments[1]);
        Write(").");
        Write(clamped ? "y" : "x");
    }
    else
        ErrorIntrinsic(funcCall->ident, funcCall);
}

void GLSLGenerator::WriteCallExprArguments(CallExpr* callExpr, std::size_t firstArgIndex, std::size_t numWriteArgs)
{
    if (numWriteArgs <= numWriteArgs + firstArgIndex)
        numWriteArgs = numWriteArgs + firstArgIndex;
    else
        numWriteArgs = ~0u;

    const auto n = callExpr->arguments.size();
    const auto m = std::min(numWriteArgs, n + callExpr->defaultArgumentRefs.size());

    for (std::size_t i = firstArgIndex; i < m; ++i)
    {
        if (i < n)
            Visit(callExpr->arguments[i]);
        else
            Visit(callExpr->defaultArgumentRefs[i - n]);

        if (i + 1 < m)
            Write(", ");
    }
}

/* ----- Intrinsics wrapper ----- */

void GLSLGenerator::WriteWrapperIntrinsics()
{
    auto program = GetProgram();

    /* Write wrappers with parameters (usage cases are required) */
    if (auto usage = program->FetchIntrinsicUsage(Intrinsic::Clip))
        WriteWrapperIntrinsicsClip(*usage);
    if (auto usage = program->FetchIntrinsicUsage(Intrinsic::Lit))
        WriteWrapperIntrinsicsLit(*usage);
    if (auto usage = program->FetchIntrinsicUsage(Intrinsic::SinCos))
        WriteWrapperIntrinsicsSinCos(*usage);

    /* Write wrappers with no parameters (usage cases are not required) */
    if (program->FetchIntrinsicUsage(Intrinsic::GroupMemoryBarrierWithGroupSync) != nullptr)
        WriteWrapperIntrinsicsMemoryBarrier(Intrinsic::GroupMemoryBarrier, true);
    if (program->FetchIntrinsicUsage(Intrinsic::DeviceMemoryBarrier) != nullptr)
        WriteWrapperIntrinsicsMemoryBarrier(Intrinsic::DeviceMemoryBarrier, false);
    if (program->FetchIntrinsicUsage(Intrinsic::DeviceMemoryBarrierWithGroupSync) != nullptr)
        WriteWrapperIntrinsicsMemoryBarrier(Intrinsic::DeviceMemoryBarrier, true);
    if (program->FetchIntrinsicUsage(Intrinsic::AllMemoryBarrierWithGroupSync) != nullptr)
        WriteWrapperIntrinsicsMemoryBarrier(Intrinsic::AllMemoryBarrier, true);

    /* Write matrix subscript wrappers */
    for (const auto& usage : program->usedMatrixSubscripts)
        WriteWrapperMatrixSubscript(usage);
}

void GLSLGenerator::WriteWrapperIntrinsicsClip(const IntrinsicUsage& usage)
{
    bool wrappersWritten = false;

    for (const auto& argList : usage.argLists)
    {
        auto arg0Type = (!argList.argTypes.empty() ? argList.argTypes.front() : DataType::Undefined);

        if (IsScalarType(arg0Type) || IsVectorType(arg0Type))
        {
            BeginLn();
            {
                /* Write function signature */
                Write("void clip(");
                WriteDataType(arg0Type, IsESSL());
                Write(" x)");

                /* Write function body */
                WriteScopeOpen(compactWrappers_);
                {
                    Write("if (");

                    if (IsScalarType(arg0Type))
                    {
                        Write("x < ");
                        WriteLiteral("0", arg0Type);
                    }
                    else if (IsVectorType(arg0Type))
                    {
                        Write("any(lessThan(x, ");
                        WriteDataType(arg0Type);
                        Write("(0)))");
                    }

                    Write(")");
                    WriteScopeOpen(compactWrappers_);
                    {
                        Write("discard;");
                    }
                    WriteScopeClose();
                }
                WriteScopeClose();
            }
            EndLn();

            wrappersWritten = true;
        }
    }

    if (wrappersWritten)
        Blank();
}

void GLSLGenerator::WriteWrapperIntrinsicsLit(const IntrinsicUsage& usage)
{
    BeginLn();
    {
        /* Write function signature */
        Write("vec4 lit(");
        WriteDataType(DataType::Float, IsESSL());
        Write(" n_dot_l, ");
        WriteDataType(DataType::Float, IsESSL());
        Write(" n_dot_h, ");
        WriteDataType(DataType::Float, IsESSL());
        Write(" m)");

        /* Write function body */
        WriteScopeOpen(compactWrappers_);
        {
            Write("return vec4(1.0f, max(0.0f, n_dot_l), max(0.0f, n_dot_h * m), 1.0f);");
        }
        WriteScopeClose();
    }
    EndLn();

    Blank();
}

void GLSLGenerator::WriteWrapperIntrinsicsSinCos(const IntrinsicUsage& usage)
{
    bool wrappersWritten = false;

    for (const auto& argList : usage.argLists)
    {
        if (argList.argTypes.size() == 3)
        {
            BeginLn();
            {
                /* Write function signature */
                Write("void sincos(");
                WriteDataType(argList.argTypes[0], IsESSL());
                Write(" x, out ");
                WriteDataType(argList.argTypes[1], IsESSL());
                Write(" s, out ");
                WriteDataType(argList.argTypes[2], IsESSL());
                Write(" c)");

                /* Write function body */
                WriteScopeOpen(compactWrappers_);
                {
                    Write("s = sin(x), c = cos(x);");
                }
                WriteScopeClose();
            }
            EndLn();

            wrappersWritten = true;
        }
    }

    if (wrappersWritten)
        Blank();
}

static std::string GetWrapperNameForMemoryBarrier(const Intrinsic intrinsic, bool groupSync)
{
    std::string s;

    switch (intrinsic)
    {
        case Intrinsic::GroupMemoryBarrier:
            s += "Group";
            break;
        case Intrinsic::DeviceMemoryBarrier:
            s += "Device";
            break;
        case Intrinsic::AllMemoryBarrier:
            s += "All";
            break;
        default:
            return "";
    }

    s += "MemoryBarrier";

    if (groupSync)
        s += "WithGroupSync";

    return s;
}

void GLSLGenerator::WriteWrapperIntrinsicsMemoryBarrier(const Intrinsic intrinsic, bool groupSync)
{
    BeginLn();
    {
        /* Write function signature */
        Write("void ");
        Write(GetWrapperNameForMemoryBarrier(intrinsic, groupSync));
        Write("()");

        /* Write function body */
        WriteScopeOpen(compactWrappers_);
        {
            switch (intrinsic)
            {
                case Intrinsic::GroupMemoryBarrier:
                    WriteLn("groupMemoryBarrier();");
                    break;
                case Intrinsic::DeviceMemoryBarrier:
                    WriteLn("memoryBarrierAtomicCounter();");
                    WriteLn("memoryBarrierImage();");
                    WriteLn("memoryBarrierBuffer();");
                    break;
                case Intrinsic::AllMemoryBarrier:
                    WriteLn("memoryBarrier();");
                    break;
                default:
                    break;
            }

            if (groupSync)
                WriteLn("barrier();");
        }
        WriteScopeClose();
    }
    EndLn();

    Blank();
}

void GLSLGenerator::WriteWrapperMatrixSubscript(const MatrixSubscriptUsage& usage)
{
    /* Only generate wrappers for matrix subscripts with more than one index */
    if (IsScalarType(usage.dataTypeOut))
        return;

    BeginLn();
    {
        /* Write function signature */
        WriteDataType(usage.dataTypeOut, IsESSL());

        Write(" ");
        Write(ExprConverter::GetMatrixSubscriptWrapperIdent(nameMangling_, usage));
        Write("(");
        WriteDataType(usage.dataTypeIn, IsESSL());
        Write(" m)");

        /* Write function body */
        WriteScopeOpen(compactWrappers_);
        {
            BeginLn();
            {
                Write("return ");

                /* Write vector type constructor with dimension of the number of indices */
                WriteDataType(usage.dataTypeOut, IsESSL());
                Write("(");

                /* Write matrix elements as arguments for vector type c'tor */
                for (std::size_t i = 0, n = usage.indices.size(); i < n; ++i)
                {
                    const auto& idx = usage.indices[i];
                    Write("m[" + std::to_string(idx.first) + "][" + std::to_string(idx.second) + "]");
                    if (i + 1 < n)
                        Write(", ");
                }

                Write(");");
            }
            EndLn();
        }
        WriteScopeClose();
    }
    EndLn();

    Blank();
}

/* ----- Structure ----- */

bool GLSLGenerator::WriteStructDecl(StructDecl* structDecl, bool endWithSemicolon)
{
    /* Write structure signature */
    BeginLn();

    Write("struct");
    if (!structDecl->ident.Empty())
        Write(' ' + structDecl->ident);

    /* Write structure members */
    WriteScopeOpen(false, endWithSemicolon);
    BeginSep();
    {
        WriteStmntList(structDecl->varMembers);
    }
    EndSep();
    WriteScopeClose();

    /* Only append blank line if struct is not part of a variable declaration */
    if (!InsideVarDeclStmnt())
        Blank();

    /* Write member functions */
    std::vector<BasicDeclStmnt*> funcMemberStmnts;
    funcMemberStmnts.reserve(structDecl->funcMembers.size());

    for (auto& funcDecl : structDecl->funcMembers)
        funcMemberStmnts.push_back(funcDecl->declStmntRef);

    WriteStmntList(funcMemberStmnts);

    return true;
}

/* ----- BufferDecl ----- */

void GLSLGenerator::WriteBufferDecl(BufferDecl* bufferDecl)
{
    if (bufferDecl->flags(AST::isReachable))
    {
        if (IsStorageBufferType(bufferDecl->GetBufferType()))
            WriteBufferDeclStorageBuffer(bufferDecl);
        else
            WriteBufferDeclTexture(bufferDecl);
        Blank();
    }
}

void GLSLGenerator::WriteBufferDeclTexture(BufferDecl* bufferDecl)
{
    const std::string* bufferTypeKeyword = nullptr;

    if (bufferDecl->flags(BufferDecl::isUsedForCompare) && !UseSeparateSamplers())
    {
        /* Convert type to a shadow sampler type */
        SamplerType samplerType = TextureTypeToSamplerType(bufferDecl->GetBufferType());
        SamplerType shadowSamplerType = SamplerTypeToShadowSamplerType(samplerType);

        bufferTypeKeyword = SamplerTypeToKeyword(shadowSamplerType, bufferDecl->declStmntRef);
    }
    else
    {
        /* Determine GLSL sampler type (or VKSL texture type) */
        bufferTypeKeyword = BufferTypeToKeyword(bufferDecl->GetBufferType(), bufferDecl->declStmntRef);
    }

    if (!bufferTypeKeyword)
        return;

    bool isWriteOnly = (!bufferDecl->flags(BufferDecl::isUsedForImageRead));

    /* Determine image layout format */
    auto imageLayoutFormat  = ImageLayoutFormat::Undefined;
    auto isRWBuffer         = IsRWImageBufferType(bufferDecl->GetBufferType());

    if (!isWriteOnly && isRWBuffer)
    {
        #ifdef XSC_ENABLE_LANGUAGE_EXT

        if (extensions_(Extensions::LayoutAttribute))
        {
            /* Take image layout format from type denoter */
            imageLayoutFormat = bufferDecl->declStmntRef->typeDenoter->layoutFormat;
        }

        #endif

        /* Attempt to derive a default format */
        if (imageLayoutFormat == ImageLayoutFormat::Undefined)
        {
            if (bufferDecl->declStmntRef->typeDenoter->genericTypeDenoter)
            {
                if (auto baseTypeDen = bufferDecl->declStmntRef->typeDenoter->genericTypeDenoter->As<BaseTypeDenoter>())
                    imageLayoutFormat = DataTypeToImageLayoutFormat(baseTypeDen->dataType);
            }
        }
    }

    BeginLn();
    {
        /* Write uniform declaration */
        WriteLayout(
            {
                [&]()
                {
                    if (!isWriteOnly)
                    {
                        if (auto keyword = ImageLayoutFormatToGLSLKeyword(imageLayoutFormat))
                            Write(*keyword);
                    }
                },

                [&]()
                {
                    WriteLayoutBinding(bufferDecl->slotRegisters);
                },
            }
        );

        /* If no format qualifier, reads are not allowed */
        if (isRWBuffer && (isWriteOnly || imageLayoutFormat == ImageLayoutFormat::Undefined))
            Write("writeonly ");

        Write("uniform ");

        /* Write sampler type and identifier */
        if (auto genericTypeDen = bufferDecl->declStmntRef->typeDenoter->genericTypeDenoter)
        {
            if (auto baseTypeDen = genericTypeDen->As<BaseTypeDenoter>())
            {
                if (IsIntType(baseTypeDen->dataType))
                    Write("i");
                else if (IsUIntType(baseTypeDen->dataType))
                    Write("u");
            }
        }

        Write(*bufferTypeKeyword + " " + bufferDecl->ident);

        /* Write array dimensions and statement terminator */
        Visit(bufferDecl->arrayDims);
        Write(";");
    }
    EndLn();
}

void GLSLGenerator::WriteBufferDeclStorageBuffer(BufferDecl* bufferDecl)
{
    /* Determine GLSL buffer type */
    auto bufferTypeKeyword = BufferTypeToKeyword(bufferDecl->GetBufferType(), bufferDecl->declStmntRef);
    if (!bufferTypeKeyword)
        return;

    /* Write buffer declaration */
    BeginLn();
    {
        WriteLayout(
            {
                [&]() { Write("std430"); },
                [&]() { WriteLayoutBinding(bufferDecl->slotRegisters); },
            }
        );
        Write(*bufferTypeKeyword + " ");

        if (nameMangling_.renameBufferFields)
        {
            Write(bufferDecl->ident);
            bufferDecl->ident.AppendPrefix(nameMangling_.temporaryPrefix);
        }
        else
            Write(nameMangling_.temporaryPrefix + bufferDecl->ident);

        /* Write buffer array (of variable size) */
        WriteScopeOpen(false, true);
        {
            /* Write optional memory type qualifier */
            if (!IsRWBufferType(bufferDecl->GetBufferType()))
                Write("readonly ");

            /* Write generic type denoterand identifier */
            auto genericTypeDen = bufferDecl->declStmntRef->typeDenoter->GetGenericTypeDenoter();
            WriteTypeDenoter(*genericTypeDen, IsESSL(), bufferDecl);
            Write(" " + bufferDecl->ident + "[];");
        }
        WriteScopeClose();
    }
    EndLn();
}

/* ----- SamplerDecl ----- */

void GLSLGenerator::WriteSamplerDecl(SamplerDecl& samplerDecl)
{
    if (UseSeparateSamplers() || !IsSamplerStateType(samplerDecl.declStmntRef->typeDenoter->samplerType))
    {
        /* Determine GLSL sampler type */
        auto samplerTypeKeyword = SamplerTypeToKeyword(samplerDecl.GetSamplerType(), samplerDecl.declStmntRef);
        if (!samplerTypeKeyword)
            return;

        BeginLn();
        {
            /* Write layout binding */
            WriteLayout(
                { [&]() { WriteLayoutBinding(samplerDecl.slotRegisters); } }
            );

            /* Write uniform sampler declaration (sampler declarations must only appear in global scope) */
            Write("uniform " + *samplerTypeKeyword + " " + samplerDecl.ident);

            /* Write array dimensions and statement terminator */
            Visit(samplerDecl.arrayDims);
            Write(";");
        }
        EndLn();

        Blank();
    }
}

/* ----- Misc ----- */

void GLSLGenerator::WriteStmntComment(Stmnt* ast, bool insertBlank)
{
    if (ast && !ast->comment.empty())
    {
        if (insertBlank)
            Blank();
        WriteComment(ast->comment);
    }
}

template <typename T>
T* GetRawPtr(T* ptr)
{
    return ptr;
}

template <typename T>
T* GetRawPtr(const std::shared_ptr<T>& ptr)
{
    return ptr.get();
}

template <typename T>
void GLSLGenerator::WriteStmntList(const std::vector<T>& stmnts, bool isGlobalScope)
{
    if (preserveComments_)
    {
        /* Write statements with optional commentaries */
        for (std::size_t i = 0; i < stmnts.size(); ++i)
        {
            auto ast = GetRawPtr(stmnts[i]);

            if (!isGlobalScope || ast->flags(AST::isReachable))
                WriteStmntComment(ast, (!isGlobalScope && (i > 0)));

            Visit(ast);
        }
    }
    else
    {
        /* Write statements only */
        Visit(stmnts);
    }
}

void GLSLGenerator::WriteParameter(VarDeclStmnt* ast)
{
    /* Write input modifier */
    if (ast->IsOutput())
    {
        if (ast->IsInput())
            Write("inout ");
        else
            Write("out ");
    }

    /* Write type modifiers */
    WriteTypeModifiersFrom(ast->typeSpecifier);

    /* Write parameter type */
    Visit(ast->typeSpecifier);
    Write(" ");

    /* Write parameter identifier (without default initializer) */
    if (ast->varDecls.size() == 1)
    {
        auto paramVar = ast->varDecls.front().get();
        Write(paramVar->ident);
        Visit(paramVar->arrayDims);
    }
    else
        Error(R_InvalidParamVarCount, ast);
}

void GLSLGenerator::WriteScopedStmnt(Stmnt* ast)
{
    if (ast)
    {
        if (ast->Type() != AST::Types::CodeBlockStmnt)
        {
            WriteScopeOpen(false, false, alwaysBracedScopes_);
            {
                Visit(ast);
            }
            WriteScopeClose();
        }
        else
            Visit(ast);
    }
}

void GLSLGenerator::WriteLiteral(const std::string& value, const DataType& dataType, const AST* ast)
{
    if (IsScalarType(dataType))
    {
        Write(value);

        switch (dataType)
        {
            case DataType::UInt:
                if (!value.empty() && value.back() != 'u' && value.back() != 'U')
                    Write("u");
                break;
            case DataType::Float:
                if (value.find_first_of(".eE") == std::string::npos)
                    Write(".0");
                Write("f");
                break;
            default:
                break;
        }
    }
    else if (IsVectorType(dataType))
    {
        WriteDataType(dataType, false, ast);
        Write("(");
        Write(value);
        Write(")");
    }
    else
        Error(R_FailedToWriteLiteralType(value), ast);
}


} // /namespace Xsc



// ================================================================================
