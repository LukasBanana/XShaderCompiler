/*
 * MetalGenerator.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "MetalGenerator.h"
#include "MetalKeywords.h"
#include "MetalIntrinsics.h"
//#include "StructParameterAnalyzer.h"
#include "TypeDenoter.h"
#include "Exception.h"
//#include "TypeConverter.h"
//#include "ExprConverter.h"
//#include "FuncNameConverter.h"
//#include "UniformPacker.h"
#include "ReferenceAnalyzer.h"
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

struct IfStmtArgs
{
    bool inHasElseParentNode;
};

struct StructDeclArgs
{
    bool inEndWithSemicolon;
};


/*
 * MetalGenerator class
 */

MetalGenerator::MetalGenerator(Log* log) :
    Generator { log }
{
}

void MetalGenerator::GenerateCodePrimary(
    Program& program, const ShaderInput& inputDesc, const ShaderOutput& outputDesc)
{
    /* Store parameters */
    versionOut_         = outputDesc.shaderVersion;
    preserveComments_   = outputDesc.options.preserveComments;
    writeHeaderComment_ = outputDesc.options.writeGeneratorHeader;
    alwaysBracedScopes_ = outputDesc.formatting.alwaysBracedScopes;
    newLineOpenScope_   = outputDesc.formatting.newLineOpenScope;

    try
    {
        /* Pre-process AST before generation begins */
        PreProcessAST(inputDesc, outputDesc);

        /* Visit program AST */
        Visit(&program);
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


/*
 * ======= Private: =======
 */

const std::string* MetalGenerator::BufferTypeToKeyword(const BufferType bufferType, const AST* ast)
{
    if (auto keyword = BufferTypeToMetalKeyword(bufferType))
        return keyword;
    else
        Error(R_FailedToMapToMetalKeyword(R_BufferType), ast);
    return nullptr;
}

const std::string* MetalGenerator::SamplerTypeToKeyword(const SamplerType samplerType, const AST* ast)
{
    if (auto keyword = SamplerTypeToMetalKeyword(samplerType))
        return keyword;
    else
        Error(R_FailedToMapToMetalKeyword(R_SamplerType), ast);
    return nullptr;
}

std::unique_ptr<std::string> MetalGenerator::SemanticToKeyword(const IndexedSemantic& semantic, const AST* ast)
{
    if (auto keyword = SemanticToMetalKeyword(semantic))
        return keyword;
    else
        Error(R_FailedToMapToMetalKeyword(R_SystemValueSemantic), ast);
    return nullptr;
}

void MetalGenerator::ErrorIntrinsic(const std::string& intrinsicName, const AST* ast)
{
    Error(R_FailedToMapToMetalKeyword(R_Intrinsic(intrinsicName)), ast);
}

/* ------- Visit functions ------- */

#define IMPLEMENT_VISIT_PROC(AST_NAME) \
    void MetalGenerator::Visit##AST_NAME(AST_NAME* ast, void* args)

IMPLEMENT_VISIT_PROC(Program)
{
    /* Write version and required extensions first */
    WriteProgramHeader();

    /* Write global program statements */
    WriteStmtList(ast->globalStmts, true);
}

IMPLEMENT_VISIT_PROC(CodeBlock)
{
    WriteScopeOpen();
    {
        WriteStmtList(ast->stmts);
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
        Visit(ast->stmts);
    }
    DecIndent();
}

IMPLEMENT_VISIT_PROC(ArrayDimension)
{
    Write(ast->ToString());
}

IMPLEMENT_VISIT_PROC(TypeSpecifier)
{
    /* Write type denoter */
    if (ast->structDecl)
        Visit(ast->structDecl);
    else
        WriteTypeDenoter(*ast->typeDenoter, ast);

    /* Write reference specifier for outut parameters */
    if (ast->IsOutput())
        Write("&");
}

/* --- Declarations --- */

IMPLEMENT_VISIT_PROC(VarDecl)
{
    if (auto staticMemberVar = ast->FetchStaticVarDeclRef())
        Write(staticMemberVar->ident);
    else
        Write(InsideStructDecl() ? ast->ident.Original() : ast->ident.Final());

    Visit(ast->arrayDims);

    WriteAttribBegin();
    {
        if (ast->semantic.IsSystemValue())
            WriteSemantic(ast->semantic, ast);
        WriteInterpModifiers(ast->declStmtRef->typeSpecifier->interpModifiers, ast);
    }
    WriteAttribEnd();

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
    PushStructDecl(ast);
    {
        if (auto structDeclArgs = reinterpret_cast<StructDeclArgs*>(args))
            WriteStructDecl(ast, structDeclArgs->inEndWithSemicolon);
        else
            WriteStructDecl(ast, false);
    }
    PopStructDecl();
}

IMPLEMENT_VISIT_PROC(SamplerDecl)
{
    //TODO
}

/* --- Declaration statements --- */

IMPLEMENT_VISIT_PROC(FunctionDecl)
{
    /* Check for valid control paths */
    if (ast->flags(FunctionDecl::hasNonReturnControlPath))
        Error(R_InvalidControlPathInFunc(ast->ToString()), ast);

    /* Write function declaration */
    PushFunctionDecl(ast);
    {
        WriteFunction(ast);
    }
    PopFunctionDecl();

    Blank();
}

IMPLEMENT_VISIT_PROC(UniformBufferDecl)
{
    /* Write uniform buffer declaration */
    ast->DeriveCommonStorageLayout();

    BeginLn();

    Write("struct " + ast->ident);

    /* Write uniform buffer members */
    WriteScopeOpen(false, true);
    BeginSep();
    {
        PushUniformBufferDecl(ast);
        {
            WriteStmtList(ast->varMembers);
        }
        PopUniformBufferDecl();
    }
    EndSep();
    WriteScopeClose();

    Blank();
}

IMPLEMENT_VISIT_PROC(BufferDeclStmt)
{
    if (!InsideGlobalScope())
    {
        //TODO
    }
}

IMPLEMENT_VISIT_PROC(SamplerDeclStmt)
{
    if (!InsideGlobalScope())
    {
        //TODO
    }
}

IMPLEMENT_VISIT_PROC(VarDeclStmt)
{
    PushVarDeclStmt(ast);
    {
        BeginLn();

        /* Write storage classes */
        WriteStorageClasses(ast->typeSpecifier->storageClasses, ast);
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
        for (std::size_t i = 0, n = ast->varDecls.size(); i < n; ++i)
        {
            Visit(ast->varDecls[i]);
            if (i + 1 < n)
                Write(", ");
        }

        Write(";");
        EndLn();
    }
    PopVarDeclStmt();

    if (InsideGlobalScope())
        Blank();
}

IMPLEMENT_VISIT_PROC(AliasDeclStmt)
{
    /* Ignore builtin typedefs (e.g. "WORD" or "FLOAT") that are not referenced */
    if (ast->flags(AST::isBuiltin) && !ast->flags(AST::isReachable))
        return;

    BeginLn();
    Write("typedef ");

    if (ast->structDecl)
    {
        PushAliasDeclStmt(ast);
        {
            Visit(ast->structDecl);
        }
        PopAliasDeclStmt();

        if (newLineOpenScope_)
        {
            EndLn();
            BeginLn();
        }
        else
            Write(" ");
    }

    /* Write alias declarations */
    for (std::size_t i = 0, n = ast->aliasDecls.size(); i < n; ++i)
    {
        auto aliasDecl = ast->aliasDecls[i].get();
        const auto& typeDen = aliasDecl->GetTypeDenoter();

        if (ast->structDecl == nullptr)
        {
            WriteTypeDenoterExt(*typeDen, false, aliasDecl);
            Write(" ");
        }

        /* Write alias name */
        Write(aliasDecl->ident);

        /* Write array dimensions */
        if (auto arrayTypeDen = typeDen->As<ArrayTypeDenoter>())
            Visit(arrayTypeDen->arrayDims);

        if (i + 1 < n)
            Write(", ");
    }

    Write(";");
    EndLn();

    if (InsideGlobalScope())
        Blank();
}

IMPLEMENT_VISIT_PROC(BasicDeclStmt)
{
    if (auto structDecl = ast->declObject->As<StructDecl>())
    {
        /* Visit structure declaration */
        StructDeclArgs structDeclArgs;
        structDeclArgs.inEndWithSemicolon = true;

        Visit(structDecl, &structDeclArgs);
    }
    else
    {
        /* Visit declaration object only */
        Visit(ast->declObject);
    }
}

/* --- Statements --- */

IMPLEMENT_VISIT_PROC(NullStmt)
{
    WriteLn(";");
}

IMPLEMENT_VISIT_PROC(ScopeStmt)
{
    Visit(ast->codeBlock);
}

IMPLEMENT_VISIT_PROC(ForStmt)
{
    /* Write loop header */
    BeginLn();

    Write("for (");

    PushOptions({ false, false });
    {
        Visit(ast->initStmt);
        Write(" "); // initStmt already has the ';'!
        Visit(ast->condition);
        Write("; ");
        Visit(ast->iteration);
    }
    PopOptions();

    Write(")");

    WriteScopedStmt(ast->bodyStmt.get());
}

IMPLEMENT_VISIT_PROC(WhileStmt)
{
    /* Write loop condExpr */
    BeginLn();

    Write("while (");
    Visit(ast->condition);
    Write(")");

    WriteScopedStmt(ast->bodyStmt.get());
}

IMPLEMENT_VISIT_PROC(DoWhileStmt)
{
    BeginLn();

    Write("do");
    WriteScopedStmt(ast->bodyStmt.get());

    /* Write loop condExpr */
    WriteScopeContinue();

    Write("while (");
    Visit(ast->condition);
    Write(");");

    EndLn();
}

IMPLEMENT_VISIT_PROC(IfStmt)
{
    bool hasElseParentNode = (args != nullptr ? reinterpret_cast<IfStmtArgs*>(args)->inHasElseParentNode : false);

    /* Write if condExpr */
    if (hasElseParentNode)
        Write(" ");
    else
        BeginLn();

    Write("if (");
    Visit(ast->condition);
    Write(")");

    /* Write if body */
    WriteScopedStmt(ast->bodyStmt.get());

    if (auto elseBody = ast->elseStmt.get())
    {
        /* Write else if statement */
        WriteScopeContinue();
        Write("else");

        if (elseBody->Type() == AST::Types::IfStmt)
        {
            IfStmtArgs ifStmtArgs;
            ifStmtArgs.inHasElseParentNode = true;
            Visit(elseBody, &ifStmtArgs);
        }
        else
            Visit(elseBody);
    }
}

IMPLEMENT_VISIT_PROC(SwitchStmt)
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

IMPLEMENT_VISIT_PROC(ExprStmt)
{
    BeginLn();
    {
        Visit(ast->expr);
        Write(";");
    }
    EndLn();
}

IMPLEMENT_VISIT_PROC(ReturnStmt)
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
    else if (!ast->flags(ReturnStmt::isEndOfFunction))
        WriteLn("return;");
}

IMPLEMENT_VISIT_PROC(JumpStmt)
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
    WriteTypeDenoter(*ast->typeSpecifier->typeDenoter, ast);
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
    else
        WriteCallExprStandard(ast);
}

IMPLEMENT_VISIT_PROC(BracketExpr)
{
    Write("(");
    Visit(ast->expr);
    Write(")");
}

IMPLEMENT_VISIT_PROC(IdentExpr)
{
    WriteObjectExpr(*ast);
}

IMPLEMENT_VISIT_PROC(AssignExpr)
{
    Visit(ast->lvalueExpr);
    Write(" " + AssignOpToString(ast->op) + " ");
    Visit(ast->rvalueExpr);
}

IMPLEMENT_VISIT_PROC(SubscriptExpr)
{
    WriteArrayExpr(*ast);
}

IMPLEMENT_VISIT_PROC(CastExpr)
{
    Write("(");
    WriteTypeDenoter(*ast->typeSpecifier->typeDenoter, ast);
    Write(")");
    Visit(ast->expr);
}

IMPLEMENT_VISIT_PROC(InitializerExpr)
{
    WriteInitializerList(ast, 10);
}

#undef IMPLEMENT_VISIT_PROC

/* --- Helper functions for code generation --- */

/* ----- Pre processing AST ----- */

void MetalGenerator::PreProcessAST(const ShaderInput& inputDesc, const ShaderOutput& outputDesc)
{
    PreProcessReferenceAnalyzer(inputDesc);
}

void MetalGenerator::PreProcessReferenceAnalyzer(const ShaderInput& inputDesc)
{
    /* Mark all reachable AST nodes */
    ReferenceAnalyzer refAnalyzer;
    refAnalyzer.MarkReferences(*GetProgram());
}

/* ----- Basics ----- */

void MetalGenerator::WriteComment(const std::string& text)
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

void MetalGenerator::WriteSemantic(const IndexedSemantic& semantic, const AST* ast)
{
    if (auto keyword = SemanticToKeyword(semantic, ast))
        WriteAttrib(*keyword);
}

/* ----- Program ----- */

void MetalGenerator::WriteProgramHeader()
{
    /* Write Metal version */
    WriteProgramHeaderComment();
    WriteProgramHeaderInclude();
}

void MetalGenerator::WriteProgramHeaderComment()
{
    if (writeHeaderComment_)
    {
        /* Write header */
        WriteComment("Metal Shader");
        WriteComment("Generated by XShaderCompiler");
        WriteComment(TimePoint());
        Blank();
    }
}

void MetalGenerator::WriteProgramHeaderInclude()
{
    /* Convert output shader version into Metal version number (with bitwise AND operator) */
    WriteLn("#include <metal_stdlib>");
    WriteLn("#include <simd/simd.h>");
    Blank();
    WriteLn("using namespace metal;");
    Blank();
}

/* ----- Object expression ----- */

void MetalGenerator::WriteObjectExpr(const IdentExpr& identExpr)
{
    WriteObjectExprIdent(identExpr);
}

void MetalGenerator::WriteObjectExprIdent(const IdentExpr& identExpr, bool writePrefix)
{
    /* Write prefix expression */
    if (identExpr.prefixExpr && !identExpr.isStatic && writePrefix)
    {
        Visit(identExpr.prefixExpr);

        if (auto literalExpr = identExpr.prefixExpr->As<LiteralExpr>())
        {
            /* Append space between integer literal and '.' swizzle operator */
            if (literalExpr->IsSpaceRequiredForSubscript())
                Write(" ");
        }

        Write(".");
    }

    /* Write object identifier either from object expression or from symbol reference */
    if (auto symbol = identExpr.symbolRef)
    {
        /* Write original identifier, if the identifier was marked as immutable */
        if (identExpr.flags(IdentExpr::isImmutable))
            Write(symbol->ident.Original());
        else
            Write(symbol->ident);
    }
    else
        Write(identExpr.ident);
}

/* ----- Array expression ----- */

void MetalGenerator::WriteArrayExpr(const SubscriptExpr& subscriptExpr)
{
    Visit(subscriptExpr.prefixExpr);
    WriteArrayIndices(subscriptExpr.arrayIndices);
}

void MetalGenerator::WriteArrayIndices(const std::vector<ExprPtr>& arrayIndices)
{
    for (auto& arrayIndex : arrayIndices)
    {
        Write("[");
        Visit(arrayIndex);
        Write("]");
    }
}

/* ----- Type denoter ----- */

void MetalGenerator::WriteStorageClasses(const std::set<StorageClass>& storageClasses, const AST* ast)
{
    for (auto storage : storageClasses)
    {
        /* Ignore static storage class (reserved word in Metal) */
        if (storage != StorageClass::Static)
        {
            if (auto keyword = StorageClassToMetalKeyword(storage))
                Write(*keyword + " ");
            //else if (WarnEnabled(Warnings::Basic))
            //    Warning(R_NotAllStorageClassesMappedToMetal, ast);
        }
    }
}

void MetalGenerator::WriteInterpModifiers(const std::set<InterpModifier>& interpModifiers, const AST* ast)
{
    for (auto modifier : interpModifiers)
    {
        if (auto keyword = InterpModifierToMetalKeyword(modifier))
            WriteAttrib(*keyword);
        //else if (WarnEnabled(Warnings::Basic))
        //    Warning(R_NotAllInterpModMappedToMetal, ast);
    }
}

void MetalGenerator::WriteTypeModifiers(const std::set<TypeModifier>& typeModifiers, const TypeDenoterPtr& typeDenoter)
{
    if (typeModifiers.find(TypeModifier::Const) != typeModifiers.end())
        Write("const ");
}

void MetalGenerator::WriteTypeModifiersFrom(const TypeSpecifierPtr& typeSpecifier)
{
    WriteTypeModifiers(typeSpecifier->typeModifiers, typeSpecifier->GetTypeDenoter()->GetSub());
}

void MetalGenerator::WriteDataType(DataType dataType, const AST* ast)
{
    /* Map Metal data type */
    if (auto keyword = DataTypeToMetalKeyword(dataType))
        Write(*keyword);
    else
        Error(R_FailedToMapToMetalKeyword(R_DataType + " <" + DataTypeToString(dataType) + ">"), ast);
}

void MetalGenerator::WriteTypeDenoter(const TypeDenoter& typeDenoter, const AST* ast)
{
    WriteTypeDenoterExt(typeDenoter, true, ast);
}

void MetalGenerator::WriteTypeDenoterExt(const TypeDenoter& typeDenoter, bool writeArrayDims, const AST* ast)
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
            /* Map Metal base type */
            WriteDataType(baseTypeDen->dataType, ast);
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

            /* Convert buffer type to Metal buffer (or sampler type) */
            if (auto keyword = BufferTypeToKeyword(bufferType, ast))
                Write(*keyword);

            /* Write template arguments */
            if (IsTextureBufferType(bufferType))
            {
                Write("<");

                /* Write base type */
                auto genericDataType = DataType::Undefined;
                if (auto genericTypeDen = bufferTypeDen->genericTypeDenoter.get())
                    genericDataType = genericTypeDen->FetchDataType();

                if (genericDataType != DataType::Undefined)
                    WriteDataType(BaseDataType(genericDataType));
                else
                    Write("float");

                /* Write access flags */
                if (IsRWBufferType(bufferType))
                    Write(", access::read_write");

                Write(">");
            }
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

            /* Convert sampler type to Metal sampler type */
            if (auto keyword = SamplerTypeToKeyword(samplerType, ast))
                Write(*keyword);
        }
        else if (auto structTypeDen = typeDenoter.As<StructTypeDenoter>())
        {
            /* Write struct identifier (either from structure declaration or stored identifier) */
            if (auto structDecl = structTypeDen->structDeclRef)
                Write(structDecl->ident);
            else
                Write(typeDenoter.Ident());
        }
        else if (auto aliasTypeDen = typeDenoter.As<AliasTypeDenoter>())
        {
            /* Write alias name */
            Write(aliasTypeDen->ident);
        }
        else if (auto arrayTypeDen = typeDenoter.As<ArrayTypeDenoter>())
        {
            /* Write sub type of array type denoter and array dimensions */
            WriteTypeDenoter(*arrayTypeDen->subTypeDenoter, ast);

            /* Write array dimensions only if enabled (must be disabled for 'typedef' statements) */
            if (writeArrayDims)
                Visit(arrayTypeDen->arrayDims);
        }
        else
            Error(R_FailedToDetermineMetalDataType, ast);
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

/* ----- Attributes ----- */

void MetalGenerator::WriteAttribBegin()
{
    if (!attribList_.scheduled)
    {
        attribList_.scheduled   = true;
        attribList_.started     = false;
    }
}

void MetalGenerator::WriteAttribEnd()
{
    if (attribList_.started)
        Write("]]");

    attribList_.scheduled   = false;
    attribList_.started     = false;
}

void MetalGenerator::WriteAttribNext()
{
    if (attribList_.scheduled)
    {
        /* Write beginning of attribute list (if not done yet) */
        if (!attribList_.started)
        {
            attribList_.started = true;
            Write(" ");
            Separator();
            Write("[[");
        }
        else
            Write(", ");
    }
}

void MetalGenerator::WriteAttrib(const std::string& value)
{
    if (attribList_.scheduled)
    {
        /* Write attribute value */
        WriteAttribNext();
        Write(value);
    }
}

/* ----- Function declaration ----- */

void MetalGenerator::WriteFunction(FunctionDecl* ast)
{
    /* Write function header */
    BeginLn();

    auto entryPointTarget = ast->DetermineEntryPointType();
    if (entryPointTarget != ShaderTarget::Undefined)
    {
        /* Write entry point target type and store in function declaration */
        WriteFunctionEntryPointType(entryPointTarget);
        ast->entryPointType = entryPointTarget;
    }

    Visit(ast->returnType);
    Write(" " + ast->ident + "(");

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

void MetalGenerator::WriteFunctionEntryPointType(const ShaderTarget target)
{
    switch (target)
    {
        case ShaderTarget::VertexShader:
            Write("vertex ");
            break;
        case ShaderTarget::FragmentShader:
            Write("fragment ");
            break;
        case ShaderTarget::ComputeShader:
            Write("kernel ");
            break;
        default:
            break;
    }
}

/* ----- Function call ----- */

void MetalGenerator::AssertIntrinsicNumArgs(CallExpr* funcCall, std::size_t numArgsMin, std::size_t numArgsMax)
{
    auto numArgs = funcCall->arguments.size();
    if (numArgs < numArgsMin || numArgs > numArgsMax)
        Error(R_InvalidIntrinsicArgCount(funcCall->ident), funcCall);
}

void MetalGenerator::WriteCallExprStandard(CallExpr* funcCall)
{
    /* Write prefix expression */
    if (funcCall->prefixExpr)
    {
        Visit(funcCall->prefixExpr);
        Write(".");
    }

    /* Write function name */
    if (funcCall->intrinsic != Intrinsic::Undefined)
    {
        /* Write Metal intrinsic keyword */
        if (auto intr = IntrinsicToMetalKeyword(funcCall->intrinsic))
        {
            Write(intr->ident);
            if (intr->isTemplate)
            {
                Write("<");
                WriteTypeDenoter(funcCall->GetTypeDenoter()->GetAliased(), funcCall);
                Write(">");
            }
        }
        else
            ErrorIntrinsic(funcCall->ident, funcCall);
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
        WriteTypeDenoter(*funcCall->typeDenoter, funcCall);
    }
    else
        Error(R_MissingFuncName, funcCall);

    /* Write arguments */
    Write("(");
    WriteCallExprArguments(funcCall);
    Write(")");
}

void MetalGenerator::WriteCallExprIntrinsicMul(CallExpr* funcCall)
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

void MetalGenerator::WriteCallExprArguments(CallExpr* callExpr)
{
    for (std::size_t i = 0, n = callExpr->arguments.size(); i < n; ++i)
    {
        Visit(callExpr->arguments[i]);
        if (i + 1 < n)
            Write(", ");
    }
}

/* ----- Structure ----- */

bool MetalGenerator::WriteStructDecl(StructDecl* structDecl, bool endWithSemicolon)
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
        WriteStmtList(structDecl->localStmts);
    }
    EndSep();
    WriteScopeClose();

    /* Only append blank line if struct is not part of a variable declaration */
    if (!InsideVarDeclStmt() && !InsideAliasDeclStmt())
        Blank();

    return true;
}

/* ----- Misc ----- */

void MetalGenerator::WriteStmtComment(Stmt* ast, bool insertBlank)
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
void MetalGenerator::WriteStmtList(const std::vector<T>& stmts, bool isGlobalScope)
{
    if (preserveComments_)
    {
        /* Write statements with optional commentaries */
        for (std::size_t i = 0; i < stmts.size(); ++i)
        {
            auto ast = GetRawPtr(stmts[i]);

            WriteStmtComment(ast, (!isGlobalScope && (i > 0)));

            Visit(ast);
        }
    }
    else
    {
        /* Write statements only */
        Visit(stmts);
    }
}

void MetalGenerator::WriteParameter(VarDeclStmt* ast)
{
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

        WriteAttribBegin();
        {
            #if 1 // TODO: move this to MetalConverter
            if (ast->typeSpecifier->GetStructDeclRef() != nullptr)
            {
                /* Write '[[stage_in]]' attribute if parameter has structure type inside an entry point */
                if (auto funcDecl = ActiveFunctionDecl())
                {
                    if (funcDecl->entryPointType != ShaderTarget::Undefined)
                        WriteAttrib("stage_in");
                }
            }
            #endif // /TODO
            else if (paramVar->semantic.IsSystemValue())
            {
                /* Write system value semantic as attribute */
                WriteSemantic(paramVar->semantic);
            }

            /* Write interpolation modifiers as attributes */
            WriteInterpModifiers(ast->typeSpecifier->interpModifiers);
        }
        WriteAttribEnd();

        /* Write default argument for non-entry point functions */
        if (auto funcDecl = ActiveFunctionDecl())
        {
            if (paramVar->initializer != nullptr && funcDecl->entryPointType == ShaderTarget::Undefined)
            {
                Write(" = ");
                Visit(paramVar->initializer);
            }
        }
    }
    else
        Error(R_InvalidParamVarCount, ast);
}

void MetalGenerator::WriteScopedStmt(Stmt* ast)
{
    if (ast)
    {
        if (ast->Type() != AST::Types::ScopeStmt)
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

void MetalGenerator::WriteLiteral(const std::string& value, const DataType& dataType, const AST* ast)
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
        WriteDataType(dataType, ast);
        Write("(");
        Write(value);
        Write(")");
    }
    else
        Error(R_FailedToWriteLiteralType(value), ast);
}


} // /namespace Xsc



// ================================================================================
