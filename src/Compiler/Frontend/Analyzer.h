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
#include "Visitor.h"
#include "Token.h"
#include "SymbolTable.h"
#include "AST.h"
#include <string>
#include <stack>


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

        /* ----- Report and error handling ----- */

        void SubmitReport(bool isError, const std::string& msg, const AST* ast = nullptr);
        
        void Error(const std::string& msg, const AST* ast = nullptr);
        void ErrorUndeclaredIdent(const std::string& ident, const AST* ast = nullptr);
        void ErrorInternal(const std::string& msg, const AST* ast = nullptr);

        void Warning(const std::string& msg, const AST* ast = nullptr);
        void WarningOnNullStmnt(const StmntPtr& ast, const std::string& stmntTypeName);

        // Returns the report handler.
        inline ReportHandler& GetReportHandler()
        {
            return reportHandler_;
        }

        /* ----- Symbol table functions ----- */

        void OpenScope();
        void CloseScope();

        void Register(const std::string& ident, AST* ast);
        
        AST* Fetch(const std::string& ident);
        AST* Fetch(const VarIdentPtr& ident);
        AST* FetchType(const std::string& ident, const AST* ast = nullptr);
        FunctionDecl* FetchFunctionDecl(const std::string& ident, const std::vector<ExprPtr>& args, const AST* ast = nullptr);

        StructDecl* FetchStructDeclFromIdent(const std::string& ident, const AST* ast = nullptr);
        StructDecl* FetchStructDeclFromTypeDenoter(const TypeDenoter& typeDenoter);

        /* ----- State tracker functions ----- */

        void PushFunctionDeclLevel(bool isEntryPoint);
        void PopFunctionDeclLevel();

        // Returns true if the analyzer is currently inside a function declaration.
        bool InsideFunctionDecl() const;

        // Returns true if the analyzer is currently inside the main entry point.
        bool InsideEntryPoint() const;

        void PushStructDeclLevel();
        void PopStructDeclLevel();

        // Returns true if the analyzer is currently inside a structure declaration.
        bool InsideStructDecl() const;

        void PushFunctionCall(FunctionCall* ast);
        void PopFunctionCall();

        // Returns the active (inner most) function call or null if the analyzer is currently not inside a function call.
        FunctionCall* ActiveFunctionCall() const;

        /* ----- Analyzer functions ----- */

        void AnalyzeTypeDenoter(TypeDenoterPtr& typeDenoter, AST* ast);
        void AnalyzeStructTypeDenoter(StructTypeDenoter& structTypeDen, AST* ast);
        void AnalyzeAliasTypeDenoter(TypeDenoterPtr& typeDenoter, AST* ast);

        TypeDenoterPtr GetTypeDenoterFrom(TypedAST* ast);

        void ValidateTypeCast(const TypeDenoter& sourceTypeDen, const TypeDenoter& destTypeDen, const AST* ast = nullptr);

    private:

        /* === Members === */

        ReportHandler               reportHandler_;
        SourceCode*                 sourceCode_                 = nullptr;

        ASTSymbolOverloadTable      symTable_;

        // Current level of function declarations. Actually only 0 or 1 (but can be more if inner functions are supported).
        unsigned int                funcDeclLevel_              = 0;
        unsigned int                funcDeclLevelOfEntryPoint_  = ~0;

        unsigned int                structDeclLevel_            = 0;

        // Function call stack to join arguments with its function call.
        std::stack<FunctionCall*>   funcCallStack_;

};


} // /namespace Xsc


#endif



// ================================================================================