/*
 * Analyzer.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "Analyzer.h"
#include "Exception.h"
#include "ConstExprEvaluator.h"


namespace Xsc
{


Analyzer::Analyzer(Log* log) :
    reportHandler_{ "context", log }
{
}

bool Analyzer::DecorateAST(
    Program& program, const ShaderInput& inputDesc, const ShaderOutput& outputDesc)
{
    /* Decorate program AST */
    sourceCode_ = program.sourceCode.get();

    try
    {
        DecorateASTPrimary(program, inputDesc, outputDesc);
    }
    catch (const ASTRuntimeError& e)
    {
        Error(e.what(), e.GetAST());
    }
    catch (const std::exception& e)
    {
        Error(e.what());
    }

    return (!reportHandler_.HasErros());
}


/*
 * ======= Private: =======
 */

/* ----- Report and error handling ----- */

void Analyzer::SubmitReport(bool isError, const std::string& msg, const AST* ast, const HLSLErr errorCode)
{
    auto reportType = (isError ? Report::Types::Error : Report::Types::Warning);
    reportHandler_.SubmitReport(false, reportType, "context error", msg, sourceCode_, (ast ? ast->area : SourceArea::ignore), errorCode);
}

void Analyzer::Error(const std::string& msg, const AST* ast, const HLSLErr errorCode)
{
    SubmitReport(true, msg, ast, errorCode);
}

void Analyzer::ErrorUndeclaredIdent(const std::string& ident, const AST* ast)
{
    Error("undeclared identifier \"" + ident + "\"", ast);
}

void Analyzer::ErrorUndeclaredIdent(const std::string& ident, const std::string& contextName, const AST* ast)
{
    Error("undeclared identifier \"" + ident + "\" in '" + contextName + "'", ast);
}

void Analyzer::ErrorInternal(const std::string& msg, const AST* ast)
{
    reportHandler_.SubmitReport(false, Report::Types::Error, "internal error", msg, sourceCode_, (ast ? ast->area : SourceArea::ignore));
}

void Analyzer::Warning(const std::string& msg, const AST* ast)
{
    SubmitReport(false, msg, ast);
}

void Analyzer::WarningOnNullStmnt(const StmntPtr& ast, const std::string& stmntTypeName)
{
    if (ast && ast->Type() == AST::Types::NullStmnt)
        Warning("<" + stmntTypeName + "> statement with empty body", ast.get());
}

/* ----- Symbol table functions ----- */

void Analyzer::OpenScope()
{
    symTable_.OpenScope();
}

void Analyzer::CloseScope()
{
    symTable_.CloseScope();
}

void Analyzer::Register(const std::string& ident, AST* ast)
{
    try
    {
        symTable_.Register(
            ident,
            std::make_shared<ASTSymbolOverload>(ident, ast),
            [&](ASTSymbolOverloadPtr& prevSymbol) -> bool
            {
                return prevSymbol->AddSymbolRef(ast);
            }
        );
    }
    catch (const std::exception& err)
    {
        Error(err.what(), ast);
    }
}

AST* Analyzer::Fetch(const std::string& ident, const AST* ast)
{
    try
    {
        if (auto symbol = symTable_.Fetch(ident))
            return symbol->Fetch();
        else
            ErrorUndeclaredIdent(ident, ast);
    }
    catch (const std::exception& e)
    {
        Error(e.what(), ast);
    }
    return nullptr;
}

AST* Analyzer::Fetch(const VarIdentPtr& ident)
{
    return Fetch(ident->ToString(), ident.get());
}

AST* Analyzer::FetchType(const std::string& ident, const AST* ast)
{
    try
    {
        if (auto symbol = symTable_.Fetch(ident))
            return symbol->FetchType();
        else
            ErrorUndeclaredIdent(ident, ast);
    }
    catch (const std::exception& e)
    {
        Error(e.what(), ast);
    }
    return nullptr;
}

FunctionDecl* Analyzer::FetchFunctionDecl(const std::string& ident, const std::vector<ExprPtr>& args, const AST* ast)
{
    try
    {
        /* Fetch symbol with identifier */
        if (auto symbol = symTable_.Fetch(ident))
        {
            /* Derive type denoters from arguments */
            std::vector<TypeDenoterPtr> argTypeDens;

            for (const auto& arg : args)
            {
                try
                {
                    argTypeDens.push_back(arg->GetTypeDenoter());
                }
                catch (const ASTRuntimeError& e)
                {
                    Error(e.what(), e.GetAST());
                    return nullptr;
                }
                catch (const std::exception& e)
                {
                    Error(e.what(), arg.get());
                    return nullptr;
                }
            }

            /* Fetch function call with argument type denoters */
            return symbol->FetchFunctionDecl(argTypeDens);
        }
        else
            ErrorUndeclaredIdent(ident, ast);
    }
    catch (const ASTRuntimeError& e)
    {
        Error(e.what(), e.GetAST());
        return nullptr;
    }
    catch (const std::exception& e)
    {
        Error(e.what(), ast);
    }
    return nullptr;
}

FunctionDecl* Analyzer::FetchFunctionDecl(const std::string& ident, const AST* ast)
{
    try
    {
        if (auto symbol = symTable_.Fetch(ident))
            return symbol->FetchFunctionDecl();
        else
            ErrorUndeclaredIdent(ident, ast);
    }
    catch (const std::exception& e)
    {
        Error(e.what(), ast);
    }
    return nullptr;
}

VarDecl* Analyzer::FetchFromStructDecl(const StructTypeDenoter& structTypeDenoter, const std::string& ident, const AST* ast)
{
    if (structTypeDenoter.structDeclRef)
    {
        if (auto varDecl = structTypeDenoter.structDeclRef->Fetch(ident))
            return varDecl;
        else
            ErrorUndeclaredIdent(ident, structTypeDenoter.ToString(), ast);
    }
    else
        Error("missing reference to structure declaration in type denoter '" + structTypeDenoter.ToString() + "'", ast);
    return nullptr;
}

StructDecl* Analyzer::FetchStructDeclFromIdent(const std::string& ident, const AST* ast)
{
    auto symbol = FetchType(ident, ast);
    if (symbol)
    {
        if (symbol->Type() == AST::Types::StructDecl)
            return static_cast<StructDecl*>(symbol);
        else if (symbol->Type() == AST::Types::AliasDecl)
            return FetchStructDeclFromTypeDenoter(*(static_cast<AliasDecl*>(symbol)->typeDenoter));
    }
    return nullptr;
}

StructDecl* Analyzer::FetchStructDeclFromTypeDenoter(const TypeDenoter& typeDenoter)
{
    if (typeDenoter.IsStruct())
        return static_cast<const StructTypeDenoter&>(typeDenoter).structDeclRef;
    else if (typeDenoter.IsAlias())
    {
        auto aliasDecl = static_cast<const AliasTypeDenoter&>(typeDenoter).aliasDeclRef;
        if (aliasDecl)
            return FetchStructDeclFromTypeDenoter(*(aliasDecl->typeDenoter));
    }
    return nullptr;
}

/* ----- Function declaration tracker ----- */

void Analyzer::PushFunctionDeclLevel(bool isEntryPoint)
{
    ++funcDeclLevel_;
    if (isEntryPoint)
        funcDeclLevelOfEntryPoint_ = funcDeclLevel_;
}

void Analyzer::PopFunctionDeclLevel()
{
    if (funcDeclLevel_ > 0)
    {
        if (funcDeclLevelOfEntryPoint_ == funcDeclLevel_)
            funcDeclLevelOfEntryPoint_ = ~0;
        --funcDeclLevel_;
    }
    else
        ErrorInternal("function declaration level underflow");
}

bool Analyzer::InsideFunctionDecl() const
{
    return (funcDeclLevel_ > 0);
}

bool Analyzer::InsideEntryPoint() const
{
    return (funcDeclLevel_ >= funcDeclLevelOfEntryPoint_);
}

/* ----- Structure declaration tracker ----- */

void Analyzer::PushStructDecl(StructDecl* ast)
{
    if (!structDeclStack_.empty())
    {
        /* Mark structure as nested structure */
        ast->flags << StructDecl::isNestedStruct;

        /* Add reference of the new structure to all parent structures */
        for (auto parentStruct : structDeclStack_)
            parentStruct->nestedStructDeclRefs.push_back(ast);
    }

    /* Push new structure onto stack */
    structDeclStack_.push_back(ast);
}

void Analyzer::PopStructDecl()
{
    if (!structDeclStack_.empty())
        structDeclStack_.pop_back();
    else
        ErrorInternal("structure declaration level underflow");
}

bool Analyzer::InsideStructDecl() const
{
    return !structDeclStack_.empty();
}

/* ----- Function call tracker ----- */

void Analyzer::PushFunctionCall(FunctionCall* ast)
{
    funcCallStack_.push(ast);
}

void Analyzer::PopFunctionCall()
{
    if (funcCallStack_.empty())
        ErrorInternal("function call stack underflow");
    else
        funcCallStack_.pop();
}

FunctionCall* Analyzer::ActiveFunctionCall() const
{
    return (funcCallStack_.empty() ? nullptr : funcCallStack_.top());
}

/* ----- Analyzer functions ----- */

void Analyzer::AnalyzeTypeDenoter(TypeDenoterPtr& typeDenoter, const AST* ast)
{
    if (typeDenoter)
    {
        if (auto bufferTypeDen = typeDenoter->As<BufferTypeDenoter>())
            AnalyzeBufferTypeDenoter(*bufferTypeDen, ast);
        else if (auto structTypeDen = typeDenoter->As<StructTypeDenoter>())
            AnalyzeStructTypeDenoter(*structTypeDen, ast);
        else if (typeDenoter->IsAlias())
            AnalyzeAliasTypeDenoter(typeDenoter, ast);
        else if (auto arrayTypeDen = typeDenoter->As<ArrayTypeDenoter>())
            AnalyzeTypeDenoter(arrayTypeDen->baseTypeDenoter, ast);
    }
}

void Analyzer::AnalyzeBufferTypeDenoter(BufferTypeDenoter& bufferTypeDen, const AST* ast)
{
    /* Analyze generic type denoter (sub type denoter) */
    AnalyzeTypeDenoter(bufferTypeDen.genericTypeDenoter, ast);
}

void Analyzer::AnalyzeStructTypeDenoter(StructTypeDenoter& structTypeDen, const AST* ast)
{
    if (!structTypeDen.structDeclRef)
        structTypeDen.structDeclRef = FetchStructDeclFromIdent(structTypeDen.ident, ast);
}

void Analyzer::AnalyzeAliasTypeDenoter(TypeDenoterPtr& typeDenoter, const AST* ast)
{
    auto& aliasTypeDen = static_cast<AliasTypeDenoter&>(*typeDenoter);
    if (!aliasTypeDen.aliasDeclRef)
    {
        /* Fetch type declaration from type name */
        if (auto symbol = FetchType(aliasTypeDen.ident, ast))
        {
            if (auto structDecl = symbol->As<StructDecl>())
            {
                /* Replace type denoter by a struct type denoter */
                typeDenoter = std::make_shared<StructTypeDenoter>(structDecl);
            }
            else if (auto aliasDecl = symbol->As<AliasDecl>())
            {
                /* Decorate alias type denoter with reference to alias declaration */
                aliasTypeDen.aliasDeclRef = aliasDecl;
            }
        }
    }
}

TypeDenoterPtr Analyzer::GetTypeDenoterFrom(TypedAST* ast)
{
    try
    {
        /* Validate and return type denoter of initializer expression */
        return ast->GetTypeDenoter();
    }
    catch (const ASTRuntimeError& e)
    {
        Error(e.what(), e.GetAST());
    }
    catch (const std::exception& e)
    {
        Error(e.what(), ast);
    }
    return nullptr;
}

void Analyzer::ValidateTypeCast(const TypeDenoter& sourceTypeDen, const TypeDenoter& destTypeDen, const std::string& contextDesc, const AST* ast)
{
    if (!sourceTypeDen.IsCastableTo(destTypeDen))
    {
        std::string err = "can not cast '" + sourceTypeDen.ToString() + "' to '" + destTypeDen.ToString() + "'";
        if (!contextDesc.empty())
            err += " in " + contextDesc;
        Error(err, ast);
    }
}

void Analyzer::ValidateTypeCastFrom(TypedAST* sourceAST, TypedAST* destAST, const std::string& contextDesc)
{
    if (auto sourceTypeDen = GetTypeDenoterFrom(sourceAST))
    {
        if (auto destTypeDen = GetTypeDenoterFrom(destAST))
            ValidateTypeCast(*sourceTypeDen, *destTypeDen, contextDesc, sourceAST);
    }
}

/* ----- Const-expression evaluation ----- */

Variant Analyzer::EvaluateConstExpr(Expr& expr)
{
    try
    {
        /* Evaluate expression and throw error on var-access */
        ConstExprEvaluator exprEvaluator;
        return exprEvaluator.EvaluateExpr(expr, [](VarAccessExpr* ast) -> Variant { throw ast; });
    }
    catch (const std::exception& e)
    {
        Error(e.what(), &expr);
    }
    catch (const VarAccessExpr* varAccessExpr)
    {
        Error("expected constant expression", varAccessExpr);
    }
    return Variant();
}

int Analyzer::EvaluateConstExprInt(Expr& expr)
{
    auto variant = EvaluateConstExpr(expr);
    if (variant.Type() != Variant::Types::Int)
        Warning("expected constant integer expression", &expr);
    return static_cast<int>(variant.ToInt());
}

float Analyzer::EvaluateConstExprFloat(Expr& expr)
{
    auto variant = EvaluateConstExpr(expr);
    if (variant.Type() != Variant::Types::Real)
        Warning("expected constant floating-point expression", &expr);
    return static_cast<float>(variant.ToReal());
}


} // /namespace Xsc



// ================================================================================