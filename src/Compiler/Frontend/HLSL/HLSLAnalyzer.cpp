/*
 * HLSLAnalyzer.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "HLSLAnalyzer.h"

#include <algorithm>


namespace Xsc
{


/*
 * Internal GL ARB extension descriptions
 */

using ARBExt = Program::ARBExtension;

static const ARBExt ARBEXT_GL_EXT_gpu_shader4               { "GL_EXT_gpu_shader4",                 130 };
static const ARBExt ARBEXT_GL_ARB_derivative_control        { "GL_ARB_derivative_control",          400 };
static const ARBExt ARBEXT_GL_ARB_shading_language_420pack  { "GL_ARB_shading_language_420pack",    420 };
static const ARBExt ARBEXT_GL_ARB_shader_image_load_store   { "GL_ARB_shader_image_load_store",     420 };


/*
 * HLSLAnalyzer class
 */

HLSLAnalyzer::HLSLAnalyzer(Log* log) :
    reportHandler_  { "context", log },
    refAnalyzer_    { symTable_      }
{
    EstablishMaps();
}

bool HLSLAnalyzer::DecorateAST(
    Program& program, const ShaderInput& inputDesc, const ShaderOutput& outputDesc)
{
    /* Store parameters */
    entryPoint_     = inputDesc.entryPoint;
    shaderTarget_   = inputDesc.shaderTarget;
    versionIn_      = inputDesc.shaderVersion;
    versionOut_     = outputDesc.shaderVersion;
    localVarPrefix_ = outputDesc.options.prefix;

    /* Decorate program AST */
    program_ = &program;

    Visit(&program);

    return (!reportHandler_.HasErros());
}


/*
 * ======= Private: =======
 */

void HLSLAnalyzer::EstablishMaps()
{
    intrinsicMap_ = std::map<std::string, IntrinsicClasses>
    {
        { "InterlockedAdd",             IntrinsicClasses::Interlocked },
        { "InterlockedAnd",             IntrinsicClasses::Interlocked },
        { "InterlockedOr",              IntrinsicClasses::Interlocked },
        { "InterlockedXor",             IntrinsicClasses::Interlocked },
        { "InterlockedMin",             IntrinsicClasses::Interlocked },
        { "InterlockedMax",             IntrinsicClasses::Interlocked },
        { "InterlockedCompareExchange", IntrinsicClasses::Interlocked },
        { "InterlockedExchange",        IntrinsicClasses::Interlocked },
    };

    extensionMap_ = std::map<std::string, Program::ARBExtension>
    {
        { "ddx_coarse", ARBEXT_GL_ARB_derivative_control },
        { "ddy_coarse", ARBEXT_GL_ARB_derivative_control },
        { "ddx_fine",   ARBEXT_GL_ARB_derivative_control },
        { "ddy_fine",   ARBEXT_GL_ARB_derivative_control },
    };
}

void HLSLAnalyzer::SubmitReport(bool isError, const std::string& msg, const AST* ast)
{
    auto reportType = (isError ? Report::Types::Error : Report::Types::Warning);
    reportHandler_.SubmitReport(false, reportType, "context error", msg, program_->sourceCode.get(), (ast ? ast->area : SourceArea::ignore));
}

void HLSLAnalyzer::Error(const std::string& msg, const AST* ast)
{
    SubmitReport(true, msg, ast);
}

void HLSLAnalyzer::Warning(const std::string& msg, const AST* ast)
{
    SubmitReport(false, msg, ast);
}

void HLSLAnalyzer::NotifyUndeclaredIdent(const std::string& ident, const AST* ast)
{
    Warning("undeclared identifier \"" + ident + "\"", ast);
}

void HLSLAnalyzer::OpenScope()
{
    symTable_.OpenScope();
}

void HLSLAnalyzer::CloseScope()
{
    symTable_.CloseScope();
}

void HLSLAnalyzer::Register(const std::string& ident, AST* ast, const OnOverrideProc& overrideProc)
{
    try
    {
        symTable_.Register(ident, ast, overrideProc);
    }
    catch (const std::exception& err)
    {
        Error(err.what(), ast);
    }
}

AST* HLSLAnalyzer::Fetch(const std::string& ident) const
{
    return symTable_.Fetch(ident);
}

AST* HLSLAnalyzer::Fetch(const VarIdentPtr& ident) const
{
    auto fullIdent = ident->ToString();
    return Fetch(fullIdent);
}

void HLSLAnalyzer::ReportNullStmnt(const StmntPtr& ast, const std::string& stmntTypeName)
{
    if (ast && ast->Type() == AST::Types::NullStmnt)
        Warning("<" + stmntTypeName + "> statement with empty body", ast.get());
}

void HLSLAnalyzer::AcquireExtension(const Program::ARBExtension& extension)
{
    if (!IsVersionOut(extension.requiredVersion))
        program_->requiredExtensions.insert(extension.extensionName);
}

bool HLSLAnalyzer::IsVersionOut(int version) const
{
    return static_cast<int>(versionOut_) >= version;
}

FunctionCall* HLSLAnalyzer::CurrentFunction() const
{
    return callStack_.empty() ? nullptr : callStack_.top();
}

/* ------- Visit functions ------- */

#define IMPLEMENT_VISIT_PROC(AST_NAME) \
    void HLSLAnalyzer::Visit##AST_NAME(AST_NAME* ast, void* args)

IMPLEMENT_VISIT_PROC(Program)
{
    /* Analyze context of the entire program */
    for (auto& globDecl : ast->globalDecls)
        Visit(globDecl);

    if (shaderTarget_ != ShaderTarget::CommonShader)
    {
        /* Mark all functions used for the target shader */
        if (mainFunction_)
            refAnalyzer_.MarkReferencesFromEntryPoint(mainFunction_, program_);
        else
            Error("entry point \"" + entryPoint_ + "\" not found");
    }
}

IMPLEMENT_VISIT_PROC(CodeBlock)
{
    OpenScope();

    for (auto& stmnt : ast->stmnts)
        Visit(stmnt);

    CloseScope();
}

IMPLEMENT_VISIT_PROC(FunctionCall)
{
    auto name = ast->name->ToString();

    /* Check if a specific intrinsic is used */
    if (name == "mul")
    {
        ast->flags << FunctionCall::isMulFunc;

        /* Validate number of arguments */
        for (std::size_t i = 2; i < ast->arguments.size(); ++i)
            Error("too many arguments in \"mul\" intrinsic", ast->arguments[i].get());
    }
    else if (name == "rcp")
        ast->flags << FunctionCall::isRcpFunc;
    else
    {
        auto it = intrinsicMap_.find(name);
        if (it != intrinsicMap_.end())
        {
            switch (it->second)
            {
                case IntrinsicClasses::Interlocked:
                    ast->flags << FunctionCall::isAtomicFunc;
                    if (ast->arguments.size() < 2)
                        Error("interlocked intrinsics must have at least 2 arguments", ast);
                    //program_->flags << Program::interlockedIntrinsicsUsed;
                    break;
            }
        }
    }

    /* Decorate function identifier (if it's a member function) */
    if (ast->name->next)
    {
        auto symbol = Fetch(ast->name->ident);
        if (symbol)
        {
            if (symbol->Type() == AST::Types::TextureDecl)
                ast->flags << FunctionCall::isTexFunc;
        }
        else
            NotifyUndeclaredIdent(ast->name->ident, ast);
    }

    /* Check if this function requires a specific extension (or GLSL target version) */
    auto it = extensionMap_.find(name);
    if (it != extensionMap_.end())
        AcquireExtension(it->second);

    /* Analyze function arguments */
    callStack_.push(ast);
    {
        for (auto& arg : ast->arguments)
            Visit(arg);
    }
    callStack_.pop();
}

IMPLEMENT_VISIT_PROC(Structure)
{
    #if 0//!!! there is no forward declaration for structs in HLSL!
    if (!ast->name.empty())
    {
        Register(
            ast->name, ast,
            [](AST* symbol) -> bool
            {
                return symbol->Type() == AST::Types::StructDecl;
            }
        );
    }
    #else
    Register(ast->name, ast);
    #endif

    structStack_.push_back(ast);

    OpenScope();
    {
        for (auto& varDecl : ast->members)
            Visit(varDecl);
    }
    CloseScope();

    structStack_.pop_back();
}

IMPLEMENT_VISIT_PROC(SwitchCase)
{
    Visit(ast->expr);
    for (auto& stmnt : ast->stmnts)
        Visit(stmnt);
}

/* --- Global declarations --- */

IMPLEMENT_VISIT_PROC(FunctionDecl)
{
    reportHandler_.PushContextDesc(ast->SignatureToString(false));

    const auto isEntryPoint = (ast->name == entryPoint_);

    /* Find previous function forward declarations */
    auto symbol = Fetch(ast->name);
    if (symbol && symbol->Type() == AST::Types::FunctionDecl)
    {
        auto funcDecl = dynamic_cast<FunctionDecl*>(symbol);
        if (funcDecl)
        {
            /* Append previous forward declarations (required for the 'reference analyzer') */
            ast->forwardDeclsRef = funcDecl->forwardDeclsRef;
            ast->forwardDeclsRef.push_back(funcDecl);
        }
    }

    /* Register symbol name  */
    Register(
        ast->name, ast,
        [](AST* symbol) -> bool
        {
            return symbol->Type() == AST::Types::FunctionDecl;
        }
    );

    /* Visit attributes */
    for (auto& attrib : ast->attribs)
    {
        Visit(attrib);

        /* Check for special attributes */
        auto name = attrib->name->ToString();

        if (name == "earlydepthstencil")
            AcquireExtension(ARBEXT_GL_ARB_shader_image_load_store);
    }

    /* Visit function header */
    Visit(ast->returnType);

    OpenScope();
    {
        for (auto& param : ast->parameters)
            Visit(param);

        /* Special case for the main entry point */
        if (isEntryPoint)
        {
            mainFunction_ = ast;

            /* Decorate program's input and output semantics */
            for (auto& param : ast->parameters)
                program_->inputSemantics.parameters.push_back(param.get());

            program_->outputSemantics.returnType = ast->returnType.get();
            program_->outputSemantics.functionSemantic = ast->semantic;

            /* Add flags */
            ast->flags << FunctionDecl::isEntryPoint;

            /* Add flags to input- and output parameters of the main entry point */
            DecorateEntryInOut(ast->returnType.get(), false);
            for (auto& param : ast->parameters)
                DecorateEntryInOut(param.get(), true);

            /* Check if fragment shader use a slightly different screen space (VPOS vs. SV_Position) */
            if (shaderTarget_ == ShaderTarget::GLSLFragmentShader && versionIn_ <= InputShaderVersion::HLSL3)
                program_->flags << Program::hasSM3ScreenSpace;
        }

        /* Visit function body */
        isInsideFunc_ = true;
        isInsideEntryPoint_ = isEntryPoint;
        {
            Visit(ast->codeBlock);
        }
        isInsideEntryPoint_ = false;
        isInsideFunc_ = false;
    }
    CloseScope();

    reportHandler_.PopContextDesc();
}

IMPLEMENT_VISIT_PROC(UniformBufferDecl)
{
    for (auto& member : ast->members)
    {
        Visit(member);

        /* Decorate all members with a reference to this uniform buffer */
        for (auto& varDecl : member->varDecls)
            varDecl->uniformBufferRef = ast;
    }
}

IMPLEMENT_VISIT_PROC(TextureDecl)
{
    /* Register all texture identifiers */
    for (auto& name : ast->names)
    {
        if (!name->registerName.empty())
            AcquireExtension(ARBEXT_GL_ARB_shading_language_420pack);
        Register(name->ident, ast);
    }
}

IMPLEMENT_VISIT_PROC(SamplerDecl)
{
    /* Register all sampler identifiers */
    for (auto& name : ast->names)
        Register(name->ident, ast);
}

IMPLEMENT_VISIT_PROC(StructDecl)
{
    Visit(ast->structure);
}

/* --- Statements --- */

IMPLEMENT_VISIT_PROC(CodeBlockStmnt)
{
    Visit(ast->codeBlock);
}

IMPLEMENT_VISIT_PROC(ForLoopStmnt)
{
    ReportNullStmnt(ast->bodyStmnt, "for loop");

    for (auto& attrib : ast->attribs)
        Visit(attrib);

    OpenScope();
    {
        Visit(ast->initSmnt);
        Visit(ast->condition);
        Visit(ast->iteration);

        OpenScope();
        {
            Visit(ast->bodyStmnt);
        }
        CloseScope();
    }
    CloseScope();
}

IMPLEMENT_VISIT_PROC(WhileLoopStmnt)
{
    ReportNullStmnt(ast->bodyStmnt, "while loop");

    for (auto& attrib : ast->attribs)
        Visit(attrib);

    OpenScope();
    {
        Visit(ast->condition);
        Visit(ast->bodyStmnt);
    }
    CloseScope();
}

IMPLEMENT_VISIT_PROC(DoWhileLoopStmnt)
{
    ReportNullStmnt(ast->bodyStmnt, "do-while loop");

    for (auto& attrib : ast->attribs)
        Visit(attrib);

    OpenScope();
    {
        Visit(ast->bodyStmnt);
        Visit(ast->condition);
    }
    CloseScope();
}

IMPLEMENT_VISIT_PROC(IfStmnt)
{
    ReportNullStmnt(ast->bodyStmnt, "if");

    for (auto& attrib : ast->attribs)
        Visit(attrib);

    OpenScope();
    {
        Visit(ast->condition);
        Visit(ast->bodyStmnt);
    }
    CloseScope();

    Visit(ast->elseStmnt);
}

IMPLEMENT_VISIT_PROC(ElseStmnt)
{
    ReportNullStmnt(ast->bodyStmnt, "else");

    OpenScope();
    {
        Visit(ast->bodyStmnt);
    }
    CloseScope();
}

IMPLEMENT_VISIT_PROC(SwitchStmnt)
{
    for (auto& attrib : ast->attribs)
        Visit(attrib);

    OpenScope();
    {
        Visit(ast->selector);

        for (auto& switchCase : ast->cases)
            Visit(switchCase);
    }
    CloseScope();
}

IMPLEMENT_VISIT_PROC(VarDeclStmnt)
{
    Visit(ast->varType);

    for (auto& varDecl : ast->varDecls)
        Visit(varDecl);

    /* Decorate variable type */
    if (isInsideEntryPoint_ && ast->varDecls.empty())
    {
        auto symbolRef = ast->varType->symbolRef;
        if (symbolRef && symbolRef->Type() == AST::Types::Structure)
        {
            auto structType = dynamic_cast<Structure*>(symbolRef);
            if (structType && structType->flags(Structure::isShaderOutput) && structType->aliasName.empty())
            {
                /* Store alias name for shader output interface block */
                structType->aliasName = ast->varDecls.front()->name;
            }
        }
    }
}

IMPLEMENT_VISIT_PROC(AssignStmnt)
{
    DecorateVarObjectSymbol(ast);
    Visit(ast->expr);
}

IMPLEMENT_VISIT_PROC(ExprStmnt)
{
    Visit(ast->expr);
}

IMPLEMENT_VISIT_PROC(FunctionCallStmnt)
{
    Visit(ast->call);
}

IMPLEMENT_VISIT_PROC(ReturnStmnt)
{
    Visit(ast->expr);

    /* Analyze entry point return statement */
    if (isInsideEntryPoint_ && ast->expr->Type() == AST::Types::VarAccessExpr)
    {
        auto varAccessExpr = dynamic_cast<VarAccessExpr*>(ast->expr.get());
        if (varAccessExpr && varAccessExpr->varIdent->symbolRef)
        {
            auto varObject = varAccessExpr->varIdent->symbolRef;
            if (varObject->Type() == AST::Types::VarDecl)
            {
                auto varDecl = dynamic_cast<VarDecl*>(varObject);
                if (varDecl && varDecl->declStmntRef && varDecl->declStmntRef->varType)
                {
                    /*
                    Variable declaration statement has been found,
                    now find the structure object to add the alias name for the interface block.
                    */
                    auto varType = varDecl->declStmntRef->varType.get();
                    if (varType->symbolRef && varType->symbolRef->Type() == AST::Types::Structure)
                    {
                        auto structType = dynamic_cast<Structure*>(varType->symbolRef);
                        if (structType)
                        {
                            /* Store alias name for the interface block */
                            structType->aliasName = varAccessExpr->varIdent->ident;

                            /*
                            Don't generate code for this variable declaration,
                            because this variable is now already used as interface block.
                            */
                            varDecl->flags << VarDecl::disableCodeGen;
                        }
                    }
                }
            }
        }
    }
}

IMPLEMENT_VISIT_PROC(StructDeclStmnt)
{
    Visit(ast->structure);
}

IMPLEMENT_VISIT_PROC(CtrlTransferStmnt)
{
    // do nothing
}

/* --- Expressions --- */

IMPLEMENT_VISIT_PROC(ListExpr)
{
    Visit(ast->firstExpr);
    Visit(ast->nextExpr);
}

IMPLEMENT_VISIT_PROC(LiteralExpr)
{
    // do nothing
}

IMPLEMENT_VISIT_PROC(TernaryExpr)
{
    Visit(ast->condExpr);
    Visit(ast->thenExpr);
    Visit(ast->elseExpr);
}

IMPLEMENT_VISIT_PROC(BinaryExpr)
{
    /* Visit sub expressions */
    Visit(ast->lhsExpr);
    Visit(ast->rhsExpr);

    /* Check if bitwise operators are used -> requires "GL_EXT_gpu_shader4" extensions */
    if (IsBitwiseOp(ast->op))
        AcquireExtension(ARBEXT_GL_EXT_gpu_shader4);
}

IMPLEMENT_VISIT_PROC(UnaryExpr)
{
    Visit(ast->expr);
}

IMPLEMENT_VISIT_PROC(PostUnaryExpr)
{
    Visit(ast->expr);
}

IMPLEMENT_VISIT_PROC(FunctionCallExpr)
{
    Visit(ast->call);
}

IMPLEMENT_VISIT_PROC(BracketExpr)
{
    Visit(ast->expr);
}

IMPLEMENT_VISIT_PROC(CastExpr)
{
    Visit(ast->expr);
}

IMPLEMENT_VISIT_PROC(VarAccessExpr)
{
    /* Decorate AST */
    DecorateVarObjectSymbol(ast);

    /* Check if bitwise operators are used -> requires "GL_EXT_gpu_shader4" extensions */
    const auto& op = ast->assignOp;
    if (op == "|=" || op == "&=" || op == "^=" || op == "%=")
        AcquireExtension(ARBEXT_GL_EXT_gpu_shader4);

    /* Visit optional assign expression */
    Visit(ast->assignExpr);
}

IMPLEMENT_VISIT_PROC(InitializerExpr)
{
    AcquireExtension(ARBEXT_GL_ARB_shading_language_420pack);

    for (auto& expr : ast->exprs)
        Visit(expr);
}

/* --- Variables --- */

IMPLEMENT_VISIT_PROC(PackOffset)
{
    // do nothing
}

IMPLEMENT_VISIT_PROC(VarSemantic)
{
    // do nothing
}

IMPLEMENT_VISIT_PROC(VarType)
{
    if (!ast->baseType.empty())
    {
        /* Decorate variable type */
        auto symbol = Fetch(ast->baseType);
        if (symbol)
            ast->symbolRef = symbol;
    }
    else if (ast->structType)
        Visit(ast->structType);
    else
        Error("missing variable type", ast);
}

IMPLEMENT_VISIT_PROC(VarIdent)
{
    for (auto& index : ast->arrayIndices)
        Visit(index);
    Visit(ast->next);
}

IMPLEMENT_VISIT_PROC(VarDecl)
{
    if (isInsideFunc_)
        ast->flags << VarDecl::isInsideFunc;

    for (auto& dim : ast->arrayDims)
        Visit(dim);

    for (auto& semantic : ast->semantics)
    {
        Visit(semantic);

        /* Store references to members with system value semantic (SV_...) in all parent structures */
        if (IsSystemValueSemnatic(semantic->semantic))
        {
            for (auto& structure : structStack_)
                structure->systemValuesRef[ast->name] = ast;
        }
    }

    Visit(ast->initializer);

    Register(ast->name, ast);
}

#undef IMPLEMENT_VISIT_PROC

/* --- Helper functions for context analysis --- */

//INCOMPLETE!
void HLSLAnalyzer::DecorateEntryInOut(VarDeclStmnt* ast, bool isInput)
{
    const auto structFlag = (isInput ? Structure::isShaderInput : Structure::isShaderOutput);

    /* Add flag to variable declaration statement */
    ast->flags << (isInput ? VarDeclStmnt::isShaderInput : VarDeclStmnt::isShaderOutput);

    /* Add flag to structure type */
    auto& varType = ast->varType;
    if (varType->structType)
        varType->structType->flags << structFlag;

    /* Add flag to optional symbol reference */
    auto& symbolRef = varType->symbolRef;
    if (symbolRef && symbolRef->Type() == AST::Types::Structure)
    {
        auto structType = dynamic_cast<Structure*>(symbolRef);
        if (structType)
        {
            structType->flags << structFlag;
            if (!ast->varDecls.empty())
            {
                /*
                Set structure alias name;
                This will be the name of the shader interface block
                */
                structType->aliasName = ast->varDecls.front()->name;
            }
        }
    }
}

//INCOMPLETE!
void HLSLAnalyzer::DecorateEntryInOut(VarType* ast, bool isInput)
{
    const auto structFlag = (isInput ? Structure::isShaderInput : Structure::isShaderOutput);

    /* Add flag to structure type */
    if (ast->structType)
        ast->structType->flags << structFlag;

    /* Add flag to optional symbol reference */
    auto& symbolRef = ast->symbolRef;
    if (symbolRef && symbolRef->Type() == AST::Types::Structure)
    {
        auto structType = dynamic_cast<Structure*>(symbolRef);
        if (structType)
            structType->flags << structFlag;
    }
}

void HLSLAnalyzer::DecorateVarObject(AST* symbol, VarIdent* varIdent)
{
    /* Decorate variable identifier with this symbol */
    varIdent->symbolRef = symbol;

    if (symbol->Type() == AST::Types::VarDecl)
    {
        auto varDecl = dynamic_cast<VarDecl*>(symbol);
        if (varDecl)
        {
            /* Check if this identifier contains a system semantic (SV_...) */
            FetchSystemValueSemantic(varDecl->semantics, varIdent->systemSemantic);

            /* Check if the next identifiers contain a system semantic in their respective structure member */
            if (varDecl->declStmntRef)
            {
                auto varTypeSymbol = varDecl->declStmntRef->varType->symbolRef;
                if (varTypeSymbol && varTypeSymbol->Type() == AST::Types::Structure)
                {
                    auto structSymbol = dynamic_cast<Structure*>(varTypeSymbol);
                    if (structSymbol)
                    {
                        auto ident = varIdent->next.get();
                        while (ident)
                        {
                            /* Search member in structure */
                            auto systemVal = structSymbol->systemValuesRef.find(ident->ident);
                            if (systemVal != structSymbol->systemValuesRef.end())
                                FetchSystemValueSemantic(systemVal->second->semantics, ident->systemSemantic);

                            /* Check next identifier */
                            ident = ident->next.get();
                        }
                    }
                }
            }

            /* Append prefix to local variables */
            if (varDecl->flags(VarDecl::isInsideFunc))
                varIdent->ident = localVarPrefix_ + varIdent->ident;
        }
    }
    else if (symbol->Type() == AST::Types::SamplerDecl)
    {
        /* Exchange sampler object by its respective texture object () */
        auto samplerDecl = dynamic_cast<SamplerDecl*>(symbol);
        auto currentFunc = CurrentFunction();
        if (samplerDecl && currentFunc && currentFunc->flags(FunctionCall::isTexFunc))
            varIdent->ident = currentFunc->name->ident;
    }
}

bool HLSLAnalyzer::FetchSystemValueSemantic(const std::vector<VarSemanticPtr>& varSemantics, std::string& semanticName) const
{
    for (auto& semantic : varSemantics)
    {
        if (IsSystemValueSemnatic(semantic->semantic))
        {
            semanticName = semantic->semantic;
            return true;
        }
    }
    return false;
}

bool HLSLAnalyzer::IsSystemValueSemnatic(std::string semantic) const
{
    if (semantic.size() > 3)
    {
        std::transform(semantic.begin(), semantic.begin() + 2, semantic.begin(), ::toupper);
        return semantic.substr(0, 3) == "SV_";
    }
    return false;
}

/* --- Helper templates for context analysis --- */

template <typename T> void HLSLAnalyzer::DecorateVarObjectSymbol(T ast)
{
    auto symbol = Fetch(ast->varIdent->ident);
    if (symbol)
        DecorateVarObject(symbol, ast->varIdent.get());
    else
        NotifyUndeclaredIdent(ast->varIdent->ident, ast);
}


} // /namespace Xsc



// ================================================================================