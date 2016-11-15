/*
 * Analyzer.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "Analyzer.h"
#include "Exception.h"


namespace Xsc
{


Analyzer::Analyzer(Log* log) :
    reportHandler_  { "context", log },
    refAnalyzer_    { symTable_      }
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

void Analyzer::SubmitReport(bool isError, const std::string& msg, const AST* ast)
{
    auto reportType = (isError ? Report::Types::Error : Report::Types::Warning);
    reportHandler_.SubmitReport(false, reportType, "context error", msg, sourceCode_, (ast ? ast->area : SourceArea::ignore));
}

void Analyzer::Error(const std::string& msg, const AST* ast)
{
    SubmitReport(true, msg, ast);
}

void Analyzer::ErrorUndeclaredIdent(const std::string& ident, const AST* ast)
{
    Error("undeclared identifier \"" + ident + "\"", ast);
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

AST* Analyzer::Fetch(const std::string& ident)
{
    try
    {
        auto symbol = symTable_.Fetch(ident);
        if (symbol)
            return symbol->Fetch();
        else
            ErrorUndeclaredIdent(ident);
    }
    catch (const std::exception& e)
    {
        Error(e.what());
    }
    return nullptr;
}

AST* Analyzer::Fetch(const VarIdentPtr& ident)
{
    auto fullIdent = ident->ToString();
    return Fetch(fullIdent);
}

AST* Analyzer::FetchType(const std::string& ident, const AST* ast)
{
    try
    {
        auto symbol = symTable_.Fetch(ident);
        if (symbol)
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
        auto symbol = symTable_.Fetch(ident);
        if (symbol)
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
    catch (const std::exception& e)
    {
        Error(e.what(), ast);
    }
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

void Analyzer::AnalyzeTypeDenoter(TypeDenoterPtr& typeDenoter, AST* ast)
{
    if (typeDenoter->IsStruct())
        AnalyzeStructTypeDenoter(static_cast<StructTypeDenoter&>(*typeDenoter), ast);
    else if (typeDenoter->IsAlias())
        AnalyzeAliasTypeDenoter(typeDenoter, ast);
    else if (typeDenoter->IsArray())
        AnalyzeTypeDenoter(static_cast<ArrayTypeDenoter&>(*typeDenoter).baseTypeDenoter, ast);
}

void Analyzer::AnalyzeStructTypeDenoter(StructTypeDenoter& structTypeDen, AST* ast)
{
    if (!structTypeDen.structDeclRef)
        structTypeDen.structDeclRef = FetchStructDeclFromIdent(structTypeDen.ident, ast);
}

void Analyzer::AnalyzeAliasTypeDenoter(TypeDenoterPtr& typeDenoter, AST* ast)
{
    auto& aliasTypeDen = static_cast<AliasTypeDenoter&>(*typeDenoter);

    if (!aliasTypeDen.aliasDeclRef)
    {
        auto symbol = FetchType(aliasTypeDen.ident, ast);
        if (symbol)
        {
            if (symbol->Type() == AST::Types::StructDecl)
            {
                /* Replace type denoter by a struct type denoter */
                auto structTypeDen = std::make_shared<StructTypeDenoter>();

                structTypeDen->ident            = aliasTypeDen.ident;
                structTypeDen->structDeclRef    = static_cast<StructDecl*>(symbol);
            
                typeDenoter = structTypeDen;
            }
            else if (symbol->Type() == AST::Types::AliasDecl)
                aliasTypeDen.aliasDeclRef = static_cast<AliasDecl*>(symbol);
        }
    }
}

TypeDenoterPtr Analyzer::GetExprTypeDenoter(Expr* ast)
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

void Analyzer::ValidateTypeCast(const TypeDenoter& sourceTypeDen, const TypeDenoter& destTypeDen, const AST* ast)
{
    if (!sourceTypeDen.IsCastableTo(destTypeDen))
        Error("can not cast '" + sourceTypeDen.ToString() + "' to '" + destTypeDen.ToString() + "'", ast);
}


} // /namespace Xsc



// ================================================================================