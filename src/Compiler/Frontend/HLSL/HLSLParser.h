/*
 * HLSLParser.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_HLSL_PARSER_H
#define XSC_HLSL_PARSER_H


#include <Xsc/Log.h>
#include "HLSLScanner.h"
#include "ReferenceAnalyzer.h"
#include "Parser.h"
#include "Visitor.h"
#include "Token.h"

#include <vector>
#include <map>
#include <string>


namespace Xsc
{


/*
Syntax parser class for the shading language HLSL.
This parser is not fully context free. To parse cast expressions correctly,
the respective type identifiers are stored in a symbol table.
*/
class HLSLParser : public Parser
{
    
    public:
        
        HLSLParser(Log* log = nullptr);

        ProgramPtr ParseSource(const SourceCodePtr& source);

    private:
        
        /* === Enumerations === */

        // Variable declaration modifiers.
        enum class VarModifiers
        {
            // Storage class or interpolation modifier (extern, linear, centroid, nointerpolation, noperspective, sample).
            StorageModifier,

            // Type modifier (const, row_major, column_major).
            TypeModifier,
        };

        /* === Functions === */

        ScannerPtr MakeScanner() override;

        // Accepts the semicolon token (Accept(Tokens::Semicolon)).
        void Semi();

        // Returns true if the current token is a data type.
        bool IsDataType() const;
        
        // Returns true if the current token is a literal.
        bool IsLiteral() const;

        // Returns true if the current token is part of a primary expression.
        bool IsPrimaryExpr() const;

        // Returns true if the current token is part of an arithmetic unary expression, i.e. either '-' or '+'.
        bool IsArithmeticUnaryExpr() const;

        // Returns true if the specified expression is a valid left-hand-side of a cast expression.
        bool IsLhsOfCastExpr(const ExprPtr& expr) const;

        /* === Parse functions === */

        ProgramPtr                      ParseProgram(const SourceCodePtr& source);

        CodeBlockPtr                    ParseCodeBlock();
        FunctionCallPtr                 ParseFunctionCall(VarIdentPtr varIdent = nullptr);
        StructurePtr                    ParseStructure();
        VarDeclStmntPtr                 ParseParameter();
        SwitchCasePtr                   ParseSwitchCase();
        SamplerValuePtr                 ParseSamplerValue();

        FunctionCallPtr                 ParseAttribute();
        PackOffsetPtr                   ParsePackOffset(bool parseColon = true);
        ExprPtr                         ParseArrayDimension();
        ExprPtr                         ParseInitializer();
        VarSemanticPtr                  ParseVarSemantic();
        VarIdentPtr                     ParseVarIdent();
        VarTypePtr                      ParseVarType(bool parseVoidType = false);
        VarDeclPtr                      ParseVarDecl();
        BufferDeclPtr                   ParseBufferDecl(bool registerAllowed = true);
        SamplerDeclPtr                  ParseSamplerDecl(bool registerAllowed = true);

        StmntPtr                        ParseGlobalStmnt();
        FunctionDeclPtr                 ParseFunctionDecl();
        BufferDeclStmntPtr              ParseBufferDeclStmnt();
        TextureDeclStmntPtr             ParseTextureDeclStmnt();
        SamplerDeclStmntPtr             ParseSamplerDeclStmnt();
        StructDeclStmntPtr              ParseStructDeclStmnt();

        StmntPtr                        ParseStmnt();
        NullStmntPtr                    ParseNullStmnt();
        CodeBlockStmntPtr               ParseCodeBlockStmnt();
        ForLoopStmntPtr                 ParseForLoopStmnt(const std::vector<FunctionCallPtr>& attribs);
        WhileLoopStmntPtr               ParseWhileLoopStmnt(const std::vector<FunctionCallPtr>& attribs);
        DoWhileLoopStmntPtr             ParseDoWhileLoopStmnt(const std::vector<FunctionCallPtr>& attribs);
        IfStmntPtr                      ParseIfStmnt(const std::vector<FunctionCallPtr>& attribs);
        ElseStmntPtr                    ParseElseStmnt();
        SwitchStmntPtr                  ParseSwitchStmnt(const std::vector<FunctionCallPtr>& attribs);
        CtrlTransferStmntPtr            ParseCtrlTransferStmnt();
        VarDeclStmntPtr                 ParseVarDeclStmnt();
        ReturnStmntPtr                  ParseReturnStmnt();
        ExprStmntPtr                    ParseExprStmnt(const VarIdentPtr& varIdent = nullptr);
        StmntPtr                        ParseStructDeclOrVarDeclStmnt();
        StmntPtr                        ParseVarDeclOrAssignOrFunctionCallStmnt();

        ExprPtr                         ParseExpr(bool allowComma = false, const ExprPtr& initExpr = nullptr);
        ExprPtr                         ParsePrimaryExpr() override;
        LiteralExprPtr                  ParseLiteralExpr();
        ExprPtr                         ParseTypeNameOrFunctionCallExpr();
        UnaryExprPtr                    ParseUnaryExpr();
        ExprPtr                         ParseBracketOrCastExpr();
        ExprPtr                         ParseVarAccessOrFunctionCallExpr();
        VarAccessExprPtr                ParseVarAccessExpr(const VarIdentPtr& varIdent = nullptr);
        FunctionCallExprPtr             ParseFunctionCallExpr(const VarIdentPtr& varIdent = nullptr);
        InitializerExprPtr              ParseInitializerExpr();

        std::vector<VarDeclPtr>         ParseVarDeclList();
        std::vector<VarDeclStmntPtr>    ParseVarDeclStmntList();
        std::vector<VarDeclStmntPtr>    ParseParameterList();
        std::vector<StmntPtr>           ParseStmntList();
        std::vector<ExprPtr>            ParseExprList(const Tokens listTerminatorToken, bool allowLastComma = false);
        std::vector<ExprPtr>            ParseArrayDimensionList();
        std::vector<ExprPtr>            ParseArgumentList();
        std::vector<ExprPtr>            ParseInitializerList();
        std::vector<VarSemanticPtr>     ParseVarSemanticList();
        std::vector<FunctionCallPtr>    ParseAttributeList();
        std::vector<SwitchCasePtr>      ParseSwitchCaseList();
        std::vector<BufferDeclPtr>      ParseBufferDeclList();
        std::vector<SamplerDeclPtr>     ParseSamplerDeclList();
        std::vector<SamplerValuePtr>    ParseSamplerValueList();

        std::string                     ParseRegister(bool parseColon = true, bool registerAllowed = true);
        std::string                     ParseSemantic();

        /* === Members === */

        // Symbol table for all types which are allowed in a cast expression (currently only structure types).
        ASTSymbolTable typeSymTable_;

};


} // /namespace Xsc


#endif



// ================================================================================
