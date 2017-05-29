/*
 * SLParser.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
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
        
        /* === Functions === */

        // Accepts the semicolon token (Accept(Tokens::Semicolon)).
        void Semi();

        /* ----- Parsing ----- */

        virtual CodeBlockPtr            ParseCodeBlock() = 0;
        virtual VarDeclStmntPtr         ParseParameter() = 0;
        virtual StmntPtr                ParseLocalStmnt() = 0;
        virtual StmntPtr                ParseForLoopInitializer() = 0;
        virtual SwitchCasePtr           ParseSwitchCase() = 0;
        virtual VarDeclPtr              ParseVarDecl(VarDeclStmnt* declStmntRef, const TokenPtr& identTkn = nullptr) = 0;

        ArrayDimensionPtr               ParseArrayDimension(bool allowDynamicDimension = false);

        NullStmntPtr                    ParseNullStmnt();
        CodeBlockStmntPtr               ParseCodeBlockStmnt();
        ForLoopStmntPtr                 ParseForLoopStmnt();
        WhileLoopStmntPtr               ParseWhileLoopStmnt();
        DoWhileLoopStmntPtr             ParseDoWhileLoopStmnt();
        IfStmntPtr                      ParseIfStmnt();
        ElseStmntPtr                    ParseElseStmnt();
        SwitchStmntPtr                  ParseSwitchStmnt();
        CtrlTransferStmntPtr            ParseCtrlTransferStmnt();
        ReturnStmntPtr                  ParseReturnStmnt();
        ExprStmntPtr                    ParseExprStmnt(const ExprPtr& expr = nullptr);

        ExprPtr                         ParseExpr();                // expr
        ExprPtr                         ParseExprWithSequenceOpt(); // expr (, expr)*
        ExprPtr                         ParseArrayIndex();          // [ expr ]
        ExprPtr                         ParseInitializer();         // = expr

        SequenceExprPtr                 ParseSequenceExpr(const ExprPtr& firstExpr);
        ArrayExprPtr                    ParseArrayExpr(const ExprPtr& expr);
        InitializerExprPtr              ParseInitializerExpr();

        std::vector<VarDeclPtr>         ParseVarDeclList(VarDeclStmnt* declStmntRef, TokenPtr firstIdentTkn = nullptr);
        std::vector<VarDeclStmntPtr>    ParseParameterList();
        std::vector<StmntPtr>           ParseLocalStmntList();
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

        void                            ParseStmntWithCommentOpt(std::vector<StmntPtr>& stmnts, const std::function<StmntPtr()>& parseFunction);

};


} // /namespace Xsc


#endif



// ================================================================================
