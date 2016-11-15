/*
 * Analyzer.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_ANALYZER_H
#define XSC_ANALYZER_H


#include <Xsc/Xsc.h>
#include "ReportHandler.h"
#include "ReferenceAnalyzer.h"
#include "Visitor.h"
#include "Token.h"
#include "SymbolTable.h"
#include "AST.h"
#include <string>


namespace Xsc
{


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

        void SubmitReport(bool isError, const std::string& msg, const AST* ast = nullptr);
        
        void Error(const std::string& msg, const AST* ast = nullptr);
        void ErrorUndeclaredIdent(const std::string& ident, const AST* ast = nullptr);
        void ErrorInternal(const std::string& msg, const AST* ast = nullptr);

        void Warning(const std::string& msg, const AST* ast = nullptr);
        void WarningOnNullStmnt(const StmntPtr& ast, const std::string& stmntTypeName);

        void OpenScope();
        void CloseScope();

        void Register(const std::string& ident, AST* ast);
        
        AST* Fetch(const std::string& ident);
        AST* Fetch(const VarIdentPtr& ident);
        AST* FetchType(const std::string& ident, const AST* ast = nullptr);
        FunctionDecl* FetchFunctionDecl(const std::string& ident, const std::vector<ExprPtr>& args, const AST* ast = nullptr);

        StructDecl* FetchStructDeclFromIdent(const std::string& ident, const AST* ast = nullptr);
        StructDecl* FetchStructDeclFromTypeDenoter(const TypeDenoter& typeDenoter);

        void AnalyzeTypeDenoter(TypeDenoterPtr& typeDenoter, AST* ast);
        void AnalyzeStructTypeDenoter(StructTypeDenoter& structTypeDen, AST* ast);
        void AnalyzeAliasTypeDenoter(TypeDenoterPtr& typeDenoter, AST* ast);

        TypeDenoterPtr GetExprTypeDenoter(Expr* ast);

        void ValidateTypeCast(const TypeDenoter& sourceTypeDen, const TypeDenoter& destTypeDen, const AST* ast = nullptr);

        // Returns the report handler.
        inline ReportHandler& GetReportHandler()
        {
            return reportHandler_;
        }

        // Returns the reference analyzer.
        inline ReferenceAnalyzer& GetRefAnalyzer()
        {
            return refAnalyzer_;
        }

    private:

        /* === Members === */

        ReportHandler           reportHandler_;
        SourceCode*             sourceCode_     = nullptr;

        ASTSymbolOverloadTable  symTable_;
        ReferenceAnalyzer       refAnalyzer_;

};


} // /namespace Xsc


#endif



// ================================================================================