/*
 * Analyzer.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_ANALYZER_H
#define XSC_ANALYZER_H


#include <Xsc/Xsc.h>
#include "ReportHandler.h"
#include "Visitor.h"
#include "Variant.h"
#include "Token.h"
#include "SymbolTable.h"
#include "AST.h"
#include <string>
#include <stack>


namespace Xsc
{


struct StructTypeDenoter;

// Context analyzer base class.
class Analyzer : protected Visitor
{
    
    public:
        
        Analyzer(Log* log = nullptr);

        bool DecorateAST(
            Program& program,
            const ShaderInput& inputDesc,
            const ShaderOutput& outputDesc
        );

    protected:
        
        using OnOverrideProc = ASTSymbolTable::OnOverrideProc;

        virtual void DecorateASTPrimary(
            Program& program,
            const ShaderInput& inputDesc,
            const ShaderOutput& outputDesc
        ) = 0;

        /* ----- Report and error handling ----- */

        void SubmitReport(bool isError, const std::string& msg, const AST* ast = nullptr);
        
        void Error(const std::string& msg, const AST* ast = nullptr);
        void ErrorUndeclaredIdent(const std::string& ident, const AST* ast = nullptr);
        void ErrorUndeclaredIdent(const std::string& ident, const std::string& contextName, const AST* ast = nullptr);
        void ErrorUndeclaredIdent(const std::string& ident, const std::string& contextName, const std::string& similarIdent, const AST* ast = nullptr);
        void ErrorInternal(const std::string& msg, const AST* ast = nullptr);

        void Warning(const std::string& msg, const AST* ast = nullptr);
        void WarningOnNullStmnt(const StmntPtr& ast, const std::string& stmntTypeName);

        // Returns the report handler.
        inline ReportHandler& GetReportHandler()
        {
            return reportHandler_;
        }

        // Returns true if the specified warnings flags are enabled.
        bool WarnEnabled(unsigned int flags) const;

        /* ----- Symbol table functions ----- */

        // Opens a new scope in the smybol table.
        void OpenScope();

        // Closes the current scope in the symbol table.
        void CloseScope();

        // Registers the AST node in the current scope with the specified identifier.
        void Register(const std::string& ident, AST* ast);
        
        // Tries to fetch an AST node with the specified identifier from the symbol table and reports an error on failure.
        AST* Fetch(const std::string& ident, const AST* ast = nullptr);

        // Tries to fetch an AST node with the specified identifier from the current scope of the symbol table and returns null on failure.
        AST* FetchFromCurrentScopeOrNull(const std::string& ident) const;

        // Tries to fetch a declaration node with the specified identifier from the symbol table and reports an error on failure.
        Decl* FetchDecl(const std::string& ident, const AST* ast = nullptr);

        // Tries to fetch a 'StructDecl' or 'AliasDecl' with the specified identifier from the symbol table and reports an error on failure.
        AST* FetchType(const std::string& ident, const AST* ast = nullptr);

        // Tries to fetch a 'VarDecl' with the specified identifier from the symbol table and reports an error on failure.
        VarDecl* FetchVarDecl(const std::string& ident, const AST* ast = nullptr);

        // Tries to fetch a 'FunctionDecl' with the specified identifier and arguments from the symbol table and reports an error on failure.
        FunctionDecl* FetchFunctionDecl(const std::string& ident, const std::vector<ExprPtr>& args, const AST* ast = nullptr);

        // Tries to fetch a 'FunctionDecl' with the specified identifier from the symbol table and reports an error on failure (used for patch-constant-function).
        FunctionDecl* FetchFunctionDecl(const std::string& ident, const AST* ast = nullptr);

        // Tries to fetch a 'VarDecl' with the specified identifier from the structure type denoter and reports an error on failure.
        VarDecl* FetchFromStruct(const StructTypeDenoter& structTypeDenoter, const std::string& ident, const AST* ast = nullptr);

        FunctionDecl* FetchFunctionDeclFromStruct(
            const StructTypeDenoter& structTypeDenoter, const std::string& ident,
            const std::vector<ExprPtr>& args, const AST* ast = nullptr
        );

        StructDecl* FetchStructDeclFromIdent(const std::string& ident, const AST* ast = nullptr);
        StructDecl* FetchStructDeclFromTypeDenoter(const TypeDenoter& typeDenoter);

        // Returns true if the visitor is currently inside the global scope (i.e. out of any function declaration).
        bool InsideGlobalScope() const;

        /* ----- Analyzer functions ----- */

        void AnalyzeTypeDenoter(TypeDenoterPtr& typeDenoter, const AST* ast);
        void AnalyzeBufferTypeDenoter(BufferTypeDenoter& bufferTypeDen, const AST* ast);
        void AnalyzeStructTypeDenoter(StructTypeDenoter& structTypeDen, const AST* ast);
        void AnalyzeAliasTypeDenoter(TypeDenoterPtr& typeDenoter, const AST* ast);

        void AnalyzeTypeSpecifier(TypeSpecifier* typeSpecifier);
        void AnalyzeTypeSpecifierForParameter(TypeSpecifier* typeSpecifier);

        void AnalyzeFunctionEndOfScopes(FunctionDecl& funcDecl);
        void AnalyzeFunctionControlPath(FunctionDecl& funcDecl);

        TypeDenoterPtr GetTypeDenoterFrom(TypedAST* ast, const TypeDenoter* expectedTypeDenoter = nullptr);

        void ValidateTypeCast(const TypeDenoter& sourceTypeDen, const TypeDenoter& destTypeDen, const std::string& contextDesc, const AST* ast = nullptr);
        void ValidateTypeCastFrom(TypedAST* sourceAST, TypedAST* destAST, const std::string& contextDesc);

        void AnalyzeConditionalExpression(Expr* expr);

        /* ----- Const-expression evaluation ----- */

        // Evaluates the specified constant expression.
        Variant EvaluateConstExpr(Expr& expr);

        // Evaluates the specified constant object expression or throws the expression if it's not constant.
        Variant EvaluateConstExprObject(const ObjectExpr& expr);

        // Evaluates the specified constant integer expression.
        int EvaluateConstExprInt(Expr& expr);

        // Evaluates the specified constant floating-point expression.
        float EvaluateConstExprFloat(Expr& expr);

    private:

        /* === Functions === */

        bool CollectArgumentTypeDenoters(const std::vector<ExprPtr>& args, std::vector<TypeDenoterPtr>& argTypeDens);

        // Tries to find a similar identifier in the following order: symbol table, structure (if enabled).
        std::string FetchSimilarIdent(const std::string& ident, StructDecl* structDecl = nullptr) const;

        // Callback for the symbol table when a symbol is realsed from its scope.
        void OnReleaseSymbol(const ASTSymbolOverloadPtr& symbol);

        /* === Members === */

        ReportHandler           reportHandler_;
        SourceCode*             sourceCode_     = nullptr;

        ASTSymbolOverloadTable  symTable_;

        Flags                   warnings_;

};


} // /namespace Xsc


#endif



// ================================================================================