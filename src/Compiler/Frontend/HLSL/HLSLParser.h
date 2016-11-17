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
#include "Variant.h"

#include <vector>
#include <map>
#include <string>


namespace Xsc
{


// Syntax parser class for the shading language HLSL.
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

        // Returns true if the current token is a base data type (i.e. scalar, vector, matrix, or string type denoter).
        bool IsBaseDataType() const;
        
        // Returns true if the current token is a literal.
        bool IsLiteral() const;

        // Returns true if the current token is part of a primary expression.
        bool IsPrimaryExpr() const;

        // Returns true if the current token is part of an arithmetic unary expression, i.e. either '-' or '+'.
        bool IsArithmeticUnaryExpr() const;

        // Returns true if the specified expression is a valid left-hand-side of a cast expression.
        bool IsLhsOfCastExpr(const ExprPtr& expr) const;

        // Makes a new VarType AST node for the specified struct decl.
        VarTypePtr MakeVarType(const StructDeclPtr& structDecl);

        TokenPtr AcceptIt() override;

        void ProcessDirective(const std::string& ident);

        /* ----- Parsing ----- */

        ProgramPtr                      ParseProgram(const SourceCodePtr& source);

        CodeBlockPtr                    ParseCodeBlock();
        FunctionCallPtr                 ParseFunctionCall(VarIdentPtr varIdent = nullptr);
        FunctionCallPtr                 ParseFunctionCall(const TypeDenoterPtr& typeDenoter);
        VarDeclStmntPtr                 ParseParameter();
        SwitchCasePtr                   ParseSwitchCase();
        SamplerValuePtr                 ParseSamplerValue();
        AttributePtr                    ParseAttribute();
        PackOffsetPtr                   ParsePackOffset(bool parseColon = true);
        ExprPtr                         ParseArrayDimension();
        ExprPtr                         ParseInitializer();
        VarSemanticPtr                  ParseVarSemantic();
        VarIdentPtr                     ParseVarIdent();
        VarTypePtr                      ParseVarType(bool parseVoidType = false);

        VarDeclPtr                      ParseVarDecl(VarDeclStmnt* declStmntRef, const TokenPtr& identTkn = nullptr);
        TextureDeclPtr                  ParseTextureDecl(TextureDeclStmnt* declStmntRef);
        SamplerDeclPtr                  ParseSamplerDecl();
        StructDeclPtr                   ParseStructDecl(bool parseStructTkn = true, const TokenPtr& identTkn = nullptr);
        AliasDeclPtr                    ParseAliasDecl(TypeDenoterPtr typeDenoter);

        StmntPtr                        ParseGlobalStmnt();
        StmntPtr                        ParseStructDeclOrVarDeclOrFunctionDeclStmnt();
        FunctionDeclPtr                 ParseFunctionDecl(const VarTypePtr& returnType = nullptr, const TokenPtr& identTkn = nullptr);
        BufferDeclStmntPtr              ParseBufferDeclStmnt();
        TextureDeclStmntPtr             ParseTextureDeclStmnt();
        SamplerDeclStmntPtr             ParseSamplerDeclStmnt();
        //StructDeclStmntPtr              ParseStructDeclStmnt();
        VarDeclStmntPtr                 ParseVarDeclStmnt();
        AliasDeclStmntPtr               ParseAliasDeclStmnt();

        StmntPtr                        ParseStmnt();
        NullStmntPtr                    ParseNullStmnt();
        CodeBlockStmntPtr               ParseCodeBlockStmnt();
        ForLoopStmntPtr                 ParseForLoopStmnt(const std::vector<AttributePtr>& attribs);
        WhileLoopStmntPtr               ParseWhileLoopStmnt(const std::vector<AttributePtr>& attribs);
        DoWhileLoopStmntPtr             ParseDoWhileLoopStmnt(const std::vector<AttributePtr>& attribs);
        IfStmntPtr                      ParseIfStmnt(const std::vector<AttributePtr>& attribs);
        ElseStmntPtr                    ParseElseStmnt();
        SwitchStmntPtr                  ParseSwitchStmnt(const std::vector<AttributePtr>& attribs);
        CtrlTransferStmntPtr            ParseCtrlTransferStmnt();
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
        SuffixExprPtr                   ParseSuffixExpr(const ExprPtr& expr);
        ArrayAccessExprPtr              ParseArrayAccessExpr(const ExprPtr& expr);
        ExprPtr                         ParseVarAccessOrFunctionCallExpr();
        VarAccessExprPtr                ParseVarAccessExpr(const VarIdentPtr& varIdent = nullptr);
        ExprPtr                         ParseFunctionCallExpr(const VarIdentPtr& varIdent = nullptr, const TypeDenoterPtr& typeDenoter = nullptr);
        InitializerExprPtr              ParseInitializerExpr();

        std::vector<VarDeclPtr>         ParseVarDeclList(VarDeclStmnt* declStmntRef, TokenPtr firstIdentTkn = nullptr);
        std::vector<VarDeclStmntPtr>    ParseVarDeclStmntList();
        std::vector<VarDeclStmntPtr>    ParseParameterList();
        std::vector<VarDeclStmntPtr>    ParseAnnotationList();
        std::vector<StmntPtr>           ParseStmntList();
        std::vector<ExprPtr>            ParseExprList(const Tokens listTerminatorToken, bool allowLastComma = false);
        std::vector<ExprPtr>            ParseArrayDimensionList();
        std::vector<ExprPtr>            ParseArgumentList();
        std::vector<ExprPtr>            ParseInitializerList();
        std::vector<VarSemanticPtr>     ParseVarSemanticList();
        std::vector<AttributePtr>       ParseAttributeList();
        std::vector<SwitchCasePtr>      ParseSwitchCaseList();
        std::vector<TextureDeclPtr>     ParseTextureDeclList(TextureDeclStmnt* declStmntRef);
        std::vector<SamplerDeclPtr>     ParseSamplerDeclList();
        std::vector<SamplerValuePtr>    ParseSamplerValueList();
        std::vector<AliasDeclPtr>       ParseAliasDeclList(TypeDenoterPtr typeDenoter);

        std::string                     ParseIdent();
        std::string                     ParseRegister(bool parseColon = true);
        std::string                     ParseSemantic();

        TypeDenoterPtr                  ParseTypeDenoter(bool allowVoidType = true);
        TypeDenoterPtr                  ParseTypeDenoterPrimary();
        TypeDenoterPtr                  ParseTypeDenoterWithStructDeclOpt(StructDeclPtr& structDecl, bool allowVoidType = true);
        VoidTypeDenoterPtr              ParseVoidTypeDenoter();
        BaseTypeDenoterPtr              ParseBaseTypeDenoter();
        BaseTypeDenoterPtr              ParseBaseVectorTypeDenoter();
        BaseTypeDenoterPtr              ParseBaseMatrixTypeDenoter();
        TextureTypeDenoterPtr           ParseTextureTypeDenoter();
        SamplerTypeDenoterPtr           ParseSamplerTypeDenoter();
        StructTypeDenoterPtr            ParseStructTypeDenoter();
        AliasTypeDenoterPtr             ParseAliasTypeDenoter(std::string ident = "");

        Variant                         ParseAndEvaluateConstExpr();
        int                             ParseAndEvaluateConstIntExpr();
        int                             ParseAndEvaluateVectorDimension();

        void                            ParseAndIgnoreTechnique();

        /* === Members === */

        // True, if the parser is currently inside a local scope of a function (to detect illegal semantics inside local scopes).
        bool localScope_ = false;

};


} // /namespace Xsc


#endif



// ================================================================================
