/*
 * Analyzer.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "Analyzer.h"
#include "Exception.h"
#include "ConstExprEvaluator.h"
#include "EndOfScopeAnalyzer.h"
#include "ControlPathAnalyzer.h"
#include "ReportIdents.h"


namespace Xsc
{


Analyzer::Analyzer(Log* log) :
    reportHandler_{ R_Context(), log }
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
    catch (const std::underflow_error& e)
    {
        ErrorInternal(e.what());
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

void Analyzer::SubmitReport(bool isError, const std::string& msg, const AST* ast)
{
    auto reportType = (isError ? Report::Types::Error : Report::Types::Warning);
    reportHandler_.SubmitReport(
        false, reportType, (isError ? R_ContextError() : R_Warning()),
        msg, sourceCode_, (ast ? ast->area : SourceArea::ignore)
    );
}

void Analyzer::Error(const std::string& msg, const AST* ast)
{
    SubmitReport(true, msg, ast);
}

void Analyzer::ErrorUndeclaredIdent(const std::string& ident, const AST* ast)
{
    ErrorUndeclaredIdent(ident, "", "", ast);
}

void Analyzer::ErrorUndeclaredIdent(const std::string& ident, const std::string& contextName, const AST* ast)
{
    ErrorUndeclaredIdent(ident, contextName, "", ast);
}

void Analyzer::ErrorUndeclaredIdent(const std::string& ident, const std::string& contextName, const std::string& similarIdent, const AST* ast)
{
    Error(R_UndeclaredIdent(ident, contextName, similarIdent), ast);
}

void Analyzer::ErrorInternal(const std::string& msg, const AST* ast)
{
    reportHandler_.SubmitReport(false, Report::Types::Error, R_InternalError(), msg, sourceCode_, (ast ? ast->area : SourceArea::ignore));
}

void Analyzer::Warning(const std::string& msg, const AST* ast)
{
    SubmitReport(false, msg, ast);
}

void Analyzer::WarningOnNullStmnt(const StmntPtr& ast, const std::string& stmntTypeName)
{
    if (ast && ast->Type() == AST::Types::NullStmnt)
        Warning(R_StatementWithEmptyBody(stmntTypeName), ast.get());
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
        /* Register symbol in global symbol table */
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
        /* If we are inside the local scope of a member function -> try to fetch symbol from parent structure */
        if (auto structDecl = ActiveFunctionStructDecl())
        {
            if (auto symbol = structDecl->Fetch(ident))
                return symbol;
        }

        /* Fetch symbol from global symbol table */
        if (auto symbol = symTable_.Fetch(ident))
            return symbol->Fetch();
        
        /* Report undefined identifier error */
        ErrorUndeclaredIdent(ident, "", symTable_.FetchSimilar(ident), ast);
    }
    catch (const std::exception& e)
    {
        Error(e.what(), ast);
    }
    return nullptr;
}

AST* Analyzer::FetchFromCurrentScopeOrNull(const std::string& ident) const
{
    /* Fetch symbol from current scope of global symbol table */
    if (auto symbol = symTable_.FetchFromCurrentScope(ident))
        return symbol->Fetch(false);
    else
        return nullptr;
}

AST* Analyzer::FetchType(const std::string& ident, const AST* ast)
{
    try
    {
        if (auto symbol = symTable_.Fetch(ident))
            return symbol->FetchType();
        else
            ErrorUndeclaredIdent(ident, "", symTable_.FetchSimilar(ident), ast);
    }
    catch (const std::exception& e)
    {
        Error(e.what(), ast);
    }
    return nullptr;
}

VarDecl* Analyzer::FetchVarDecl(const std::string& ident, const AST* ast)
{
    try
    {
        if (auto symbol = symTable_.Fetch(ident))
            return symbol->FetchVarDecl();
        else
            ErrorUndeclaredIdent(ident, "", symTable_.FetchSimilar(ident), ast);
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
        /* Get type denoters from arguments */
        std::vector<TypeDenoterPtr> argTypeDens;
        if (!CollectArgumentTypeDenoters(args, argTypeDens))
            return nullptr;

        if (auto structDecl = ActiveFunctionStructDecl())
        {
            /* Fetch function with argument type denoters form structure */
            if (auto funcDecl = structDecl->FetchFunctionDecl(ident, argTypeDens))
                return funcDecl;
        }

        /* Fetch function with argument type denoters from global symbol table */
        if (auto symbol = symTable_.Fetch(ident))
            return symbol->FetchFunctionDecl(argTypeDens);
        
        /* Check if identifier exists but does not name a function */
        if (Fetch(ident, ast) != nullptr)
            Error(R_IdentIsNotFunc(ident), ast);
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
            ErrorUndeclaredIdent(ident, "", symTable_.FetchSimilar(ident), ast);
    }
    catch (const std::exception& e)
    {
        Error(e.what(), ast);
    }
    return nullptr;
}

VarDecl* Analyzer::FetchFromStruct(const StructTypeDenoter& structTypeDenoter, const std::string& ident, const AST* ast)
{
    if (auto structDecl = structTypeDenoter.structDeclRef)
    {
        if (auto symbol = structDecl->Fetch(ident))
            return symbol;
        else
            ErrorUndeclaredIdent(ident, structDecl->ToString(), structDecl->FetchSimilar(ident), ast);
    }
    else
        Error(R_MissingReferenceToStructInType(structTypeDenoter.ToString()), ast);
    return nullptr;
}

FunctionDecl* Analyzer::FetchFunctionDeclFromStruct(
    const StructTypeDenoter& structTypeDenoter, const std::string& ident,
    const std::vector<ExprPtr>& args, const AST* ast)
{
    if (auto structDecl = structTypeDenoter.structDeclRef)
    {
        /* Get type denoters from arguments */
        std::vector<TypeDenoterPtr> argTypeDens;
        if (CollectArgumentTypeDenoters(args, argTypeDens))
        {
            if (auto symbol = structDecl->FetchFunctionDecl(ident, argTypeDens))
                return symbol;
            else
                ErrorUndeclaredIdent(ident, structDecl->ToString(), structDecl->FetchSimilar(ident), ast);
        }
    }
    else
        Error(R_MissingReferenceToStructInType(structTypeDenoter.ToString()), ast);
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

bool Analyzer::InsideGlobalScope() const
{
    return symTable_.InsideGlobalScope();
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
        {
            Visit(arrayTypeDen->arrayDims);
            AnalyzeTypeDenoter(arrayTypeDen->baseTypeDenoter, ast);
        }
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

void Analyzer::AnalyzeTypeSpecifier(TypeSpecifier* typeSpecifier)
{
    Visit(typeSpecifier->structDecl);

    if (typeSpecifier->typeDenoter)
        AnalyzeTypeDenoter(typeSpecifier->typeDenoter, typeSpecifier);
    else
        Error(R_MissingVariableType(), typeSpecifier);
}

void Analyzer::AnalyzeTypeSpecifierForParameter(TypeSpecifier* typeSpecifier)
{
    if (typeSpecifier->isOutput && typeSpecifier->isUniform)
        Error(R_ParameterCantBeUniformAndOut(), typeSpecifier);
}

void Analyzer::AnalyzeFunctionEndOfScopes(FunctionDecl& funcDecl)
{
    /* Analyze end of scopes from function body */
    EndOfScopeAnalyzer scopeAnalyzer;
    scopeAnalyzer.MarkEndOfScopesFromFunction(funcDecl);
}

void Analyzer::AnalyzeFunctionControlPath(FunctionDecl& funcDecl)
{
    /* Mark control paths from function body */
    ControlPathAnalyzer pathAnalyzer;
    pathAnalyzer.MarkControlPathsFromFunction(funcDecl);
}

TypeDenoterPtr Analyzer::GetTypeDenoterFrom(TypedAST* ast)
{
    if (ast)
    {
        try
        {
            /* Validate and return type denoter of typed AST */
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
    }
    else
        ErrorInternal(R_NullPointerArgument(__FUNCTION__));
    return nullptr;
}

void Analyzer::ValidateTypeCast(const TypeDenoter& sourceTypeDen, const TypeDenoter& destTypeDen, const std::string& contextDesc, const AST* ast)
{
    if (!sourceTypeDen.IsCastableTo(destTypeDen))
        Error(R_IllegalCast(sourceTypeDen.ToString(), destTypeDen.ToString(), contextDesc), ast);
}

void Analyzer::ValidateTypeCastFrom(TypedAST* sourceAST, TypedAST* destAST, const std::string& contextDesc)
{
    if (auto sourceTypeDen = GetTypeDenoterFrom(sourceAST))
    {
        if (auto destTypeDen = GetTypeDenoterFrom(destAST))
            ValidateTypeCast(*sourceTypeDen, *destTypeDen, contextDesc, sourceAST);
    }
}

void Analyzer::AnalyzeConditionalExpression(Expr* expr)
{
    /* Visit expression tree */
    Visit(expr);

    /* Verify boolean type denoter in conditional expression */
    auto condTypeDen = expr->GetTypeDenoter()->Get();
    if (!condTypeDen->IsScalar())
        Error(R_ConditionalExprNotScalar(condTypeDen->ToString()), expr);
}

/* ----- Const-expression evaluation ----- */

Variant Analyzer::EvaluateConstExpr(Expr& expr)
{
    try
    {
        /* Evaluate expression and throw error on var-access */
        ConstExprEvaluator exprEvaluator;
        return exprEvaluator.EvaluateExpr(
            expr,
            [this](VarAccessExpr* ast) -> Variant
            {
                return EvaluateConstVarAccessdExpr(*ast);
            }
        );
    }
    catch (const ASTRuntimeError& e)
    {
        Error(e.what(), (e.GetAST() ? e.GetAST() : &expr));
    }
    catch (const std::exception& e)
    {
        Error(e.what(), &expr);
    }
    catch (const VarAccessExpr* varAccessExpr)
    {
        Error(R_ExpectedConstExpr(), varAccessExpr);
    }
    return Variant();
}

Variant Analyzer::EvaluateConstVarAccessdExpr(VarAccessExpr& expr)
{
    /* Find variable */
    if (auto symbol = expr.varIdent->symbolRef)
    {
        if (auto varDecl = symbol->As<VarDecl>())
        {
            if (auto varDeclStmnt = varDecl->declStmntRef)
            {
                if (varDeclStmnt->IsConstOrUniform() && varDecl->initializer)
                {
                    /* Evaluate initializer of constant variable */
                    return EvaluateConstExpr(*varDecl->initializer);
                }
            }
        }
    }

    /* Throw expression due to non-constness */
    throw (&expr);
}

int Analyzer::EvaluateConstExprInt(Expr& expr)
{
    auto variant = EvaluateConstExpr(expr);
    if (variant.Type() != Variant::Types::Int)
        Warning(R_ExpectedConstIntExpr(), &expr);
    return static_cast<int>(variant.ToInt());
}

float Analyzer::EvaluateConstExprFloat(Expr& expr)
{
    auto variant = EvaluateConstExpr(expr);
    if (variant.Type() != Variant::Types::Real)
        Warning(R_ExpectedConstFloatExpr(), &expr);
    return static_cast<float>(variant.ToReal());
}


/*
 * ======= Private: =======
 */

bool Analyzer::CollectArgumentTypeDenoters(const std::vector<ExprPtr>& args, std::vector<TypeDenoterPtr>& argTypeDens)
{
    for (const auto& arg : args)
    {
        try
        {
            argTypeDens.push_back(arg->GetTypeDenoter());
        }
        catch (const ASTRuntimeError& e)
        {
            Error(e.what(), e.GetAST());
            return false;
        }
        catch (const std::exception& e)
        {
            Error(e.what(), arg.get());
            return false;
        }
    }
    return true;
}


} // /namespace Xsc



// ================================================================================
