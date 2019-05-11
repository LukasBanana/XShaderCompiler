/*
 * SLParser.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_SL_PARSER_H
#define XSC_SL_PARSER_H


#include "Parser.h"
#include "Visitor.h"
#include "Token.h"
#include "Variant.h"
#include <Xsc/Log.h>
#include <vector>
#include <string>


namespace Xsc
{


// Syntax parser base class for HLSL and GLSL.
class SLParser : public Parser
{

    public:

        SLParser(Log* log = nullptr);

    protected:

        // Accepts the semicolon token (Accept(Tokens::Semicolon)).
        void Semi();

        /* ----- Parsing ----- */

        virtual CodeBlockPtr            ParseCodeBlock() = 0;
        virtual VarDeclStmtPtr          ParseParameter() = 0;
        virtual StmtPtr                 ParseLocalStmt() = 0;
        virtual StmtPtr                 ParseForStmtInitializer() = 0;
        virtual SwitchCasePtr           ParseSwitchCase() = 0;
        virtual VarDeclPtr              ParseVarDecl(VarDeclStmt* declStmtRef, const TokenPtr& identTkn = nullptr) = 0;

        ArrayDimensionPtr               ParseArrayDimension(bool allowDynamicDimension = false);

        NullStmtPtr                     ParseNullStmt();
        ScopeStmtPtr                    ParseScopeStmt();
        ForStmtPtr                      ParseForStmt();
        WhileStmtPtr                    ParseWhileStmt();
        DoWhileStmtPtr                  ParseDoWhileStmt();
        IfStmtPtr                       ParseIfStmt();
        StmtPtr                         ParseElseStmt();
        SwitchStmtPtr                   ParseSwitchStmt();
        JumpStmtPtr                     ParseJumpStmt();
        ReturnStmtPtr                   ParseReturnStmt();
        ExprStmtPtr                     ParseExprStmt(const ExprPtr& expr = nullptr);

        ExprPtr                         ParseExpr();                // expr
        ExprPtr                         ParseExprWithSequenceOpt(); // expr (, expr)*
        ExprPtr                         ParseArrayIndex();          // [ expr ]
        ExprPtr                         ParseInitializer();         // = expr

        SequenceExprPtr                 ParseSequenceExpr(const ExprPtr& firstExpr);
        SubscriptExprPtr                ParseSubscriptExpr(const ExprPtr& expr);
        InitializerExprPtr              ParseInitializerExpr();

        std::vector<VarDeclPtr>         ParseVarDeclList(VarDeclStmt* declStmtRef, TokenPtr firstIdentTkn = nullptr);
        std::vector<VarDeclStmtPtr>     ParseParameterList();
        std::vector<StmtPtr>            ParseLocalStmtList();
        std::vector<ExprPtr>            ParseExprList(const Tokens listTerminatorToken, bool allowLastComma = false);
        std::vector<ArrayDimensionPtr>  ParseArrayDimensionList(bool allowDynamicDimension = false);
        std::vector<ExprPtr>            ParseArrayIndexList();
        std::vector<ExprPtr>            ParseArgumentList();
        std::vector<ExprPtr>            ParseInitializerList();
        std::vector<SwitchCasePtr>      ParseSwitchCaseList();

        std::string                     ParseIdent(TokenPtr identTkn = nullptr, SourceArea* area = nullptr);

        TypeDenoterPtr                  ParseTypeDenoterWithArrayOpt(const TypeDenoterPtr& baseTypeDenoter);
        VoidTypeDenoterPtr              ParseVoidTypeDenoter();

        Variant                         ParseAndEvaluateConstExpr();
        int                             ParseAndEvaluateConstExprInt();
        int                             ParseAndEvaluateVectorDimension();

        void                            ParseStmtWithCommentOpt(std::vector<StmtPtr>& stmts, const std::function<StmtPtr()>& parseFunction);

};


} // /namespace Xsc


#endif



// ================================================================================
