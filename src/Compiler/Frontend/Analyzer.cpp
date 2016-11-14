/*
 * Analyzer.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "Analyzer.h"


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

    DecorateASTPrimary(program, inputDesc, outputDesc);

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

void Analyzer::Register(const std::string& ident, AST* ast, const OnOverrideProc& overrideProc)
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

AST* Analyzer::Fetch(const std::string& ident) const
{
    return symTable_.Fetch(ident);
}

AST* Analyzer::Fetch(const VarIdentPtr& ident) const
{
    auto fullIdent = ident->ToString();
    return Fetch(fullIdent);
}

AST* Analyzer::FetchType(const std::string& ident, const AST* ast)
{
    auto symbol = Fetch(ident);
    if ( symbol && ( symbol->Type() == AST::Types::StructDecl || symbol->Type() == AST::Types::AliasDecl ) )
        return symbol;
    else
        Error("identifier '" + ident + "' does not name a type", ast);
    return nullptr;
}

StructDecl* Analyzer::FetchStructDeclFromIdent(const std::string& ident)
{
    auto symbol = FetchType(ident);
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
        AnalyzeStructTypeDenoter(dynamic_cast<StructTypeDenoter&>(*typeDenoter), ast);
    else if (typeDenoter->IsAlias())
        AnalyzeAliasTypeDenoter(typeDenoter, ast);
}

void Analyzer::AnalyzeStructTypeDenoter(StructTypeDenoter& structTypeDen, AST* ast)
{
    structTypeDen.structDeclRef = FetchStructDeclFromIdent(structTypeDen.ident);
}

void Analyzer::AnalyzeAliasTypeDenoter(TypeDenoterPtr& typeDenoter, AST* ast)
{
    auto& aliasTypeDen = static_cast<AliasTypeDenoter&>(*typeDenoter);

    auto symbol = FetchType(aliasTypeDen.ident);
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


} // /namespace Xsc



// ================================================================================