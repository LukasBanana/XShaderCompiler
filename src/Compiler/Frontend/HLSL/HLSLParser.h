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
#include "SymbolTable.h"
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

        // Returns true if the current token is part of an arithmetic unary expression, i.e. either '-' or '+'.
        bool IsArithmeticUnaryExpr() const;

        // Returns true if the current token is a variable declaration modifier.
        bool IsVarDeclModifier() const;

        // Converts the specified expression to a type name expression if it is a left-hand-side of a cast expression.
        TypeNameExprPtr MakeToTypeNameIfLhsOfCastExpr(const ExprPtr& expr);

        // Overrides the token accept function to process all directives before the actual parsing.
        TokenPtr AcceptIt() override;

        // Processes the specified directive (only '#line'-directive are allowed after pre-processing).
        void ProcessDirective(const std::string& ident);

        /* ----- Symbol table ----- */

        // Opens a new scope of the type name symbol table.
        void OpenScope();

        // Closes the current scope of the type name symbol table.
        void CloseScope();

        // Registers the specified identifier as type name, to detect cast expressions.
        void RegisterTypeName(const std::string& ident);

        // Returns true if the specified identifier is a valid type name within the current scope.
        bool IsRegisteredTypeName(const std::string& ident) const;

        // Makes a new alias declaration statement and registers it's identifier in the symbol table.
        AliasDeclStmntPtr MakeAndRegisterAliasDeclStmnt(const DataType dataType, const std::string& ident);

        // Generates all pre defined type aliases (e.g. 'typedef int DWORD').
        void GeneratePreDefinedTypeAliases(Program& ast);

        /* ----- Parsing ----- */

        ProgramPtr                      ParseProgram(const SourceCodePtr& source);

        CodeBlockPtr                    ParseCodeBlock();
        FunctionCallPtr                 ParseFunctionCall(VarIdentPtr varIdent = nullptr);
        FunctionCallPtr                 ParseFunctionCall(const TypeDenoterPtr& typeDenoter);
        VarDeclStmntPtr                 ParseParameter();
        SwitchCasePtr                   ParseSwitchCase();
        SamplerValuePtr                 ParseSamplerValue();
        AttributePtr                    ParseAttribute();
        RegisterPtr                     ParseRegister(bool parseColon = true);
        PackOffsetPtr                   ParsePackOffset(bool parseColon = true);
        ArrayDimensionPtr               ParseArrayDimension(bool allowDynamicDimension = false);
        ExprPtr                         ParseArrayIndex();
        ExprPtr                         ParseInitializer();
        VarIdentPtr                     ParseVarIdent();
        TypeNamePtr                     ParseTypeName(bool parseVoidType = false);

        VarDeclPtr                      ParseVarDecl(VarDeclStmnt* declStmntRef, const TokenPtr& identTkn = nullptr);
        BufferDeclPtr                   ParseBufferDecl(BufferDeclStmnt* declStmntRef, const TokenPtr& identTkn = nullptr);
        SamplerDeclPtr                  ParseSamplerDecl(SamplerDeclStmnt* declStmntRef, const TokenPtr& identTkn = nullptr);
        StructDeclPtr                   ParseStructDecl(bool parseStructTkn = true, const TokenPtr& identTkn = nullptr);
        AliasDeclPtr                    ParseAliasDecl(TypeDenoterPtr typeDenoter);

        StmntPtr                        ParseGlobalStmnt();
        StmntPtr                        ParseGlobalStmntWithTypeName();
        StmntPtr                        ParseGlobalStmntWithSamplerTypeDenoter();
        StmntPtr                        ParseGlobalStmntWithBufferTypeDenoter();
        FunctionDeclPtr                 ParseFunctionDecl(const TypeNamePtr& returnType = nullptr, const TokenPtr& identTkn = nullptr);
        UniformBufferDeclPtr            ParseUniformBufferDecl();
        BufferDeclStmntPtr              ParseBufferDeclStmnt(const BufferTypeDenoterPtr& typeDenoter = nullptr, const TokenPtr& identTkn = nullptr);
        SamplerDeclStmntPtr             ParseSamplerDeclStmnt(const SamplerTypeDenoterPtr& typeDenoter = nullptr, const TokenPtr& identTkn = nullptr);
        VarDeclStmntPtr                 ParseVarDeclStmnt();
        AliasDeclStmntPtr               ParseAliasDeclStmnt();

        StmntPtr                        ParseStmnt(bool allowAttributes = true);
        StmntPtr                        ParseStmntPrimary();
        StmntPtr                        ParseStmntWithStructDecl();
        StmntPtr                        ParseStmntWithVarIdent();
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
        ExprStmntPtr                    ParseExprStmnt();

        ExprPtr                         ParseExpr(bool allowComma = false);
        ExprPtr                         ParsePrimaryExpr() override;
        ExprPtr                         ParseLiteralOrSuffixExpr();
        LiteralExprPtr                  ParseLiteralExpr();
        ExprPtr                         ParseTypeNameOrFunctionCallExpr();
        UnaryExprPtr                    ParseUnaryExpr();
        ExprPtr                         ParseBracketOrCastExpr();
        SuffixExprPtr                   ParseSuffixExpr(const ExprPtr& expr);
        ArrayAccessExprPtr              ParseArrayAccessExpr(const ExprPtr& expr);
        ExprPtr                         ParseVarAccessOrFunctionCallExpr(VarIdentPtr varIdent = nullptr);
        VarAccessExprPtr                ParseVarAccessExpr(const VarIdentPtr& varIdent = nullptr);
        ExprPtr                         ParseFunctionCallExpr(const VarIdentPtr& varIdent = nullptr, const TypeDenoterPtr& typeDenoter = nullptr);
        InitializerExprPtr              ParseInitializerExpr();

        std::vector<VarDeclPtr>         ParseVarDeclList(VarDeclStmnt* declStmntRef, TokenPtr firstIdentTkn = nullptr);
        std::vector<VarDeclStmntPtr>    ParseVarDeclStmntList();
        std::vector<VarDeclStmntPtr>    ParseParameterList();
        std::vector<VarDeclStmntPtr>    ParseAnnotationList();
        std::vector<StmntPtr>           ParseStmntList();
        std::vector<ExprPtr>            ParseExprList(const Tokens listTerminatorToken, bool allowLastComma = false);
        std::vector<ArrayDimensionPtr>  ParseArrayDimensionList(bool allowDynamicDimension = false);
        std::vector<ExprPtr>            ParseArrayIndexList();
        std::vector<ExprPtr>            ParseArgumentList();
        std::vector<ExprPtr>            ParseInitializerList();
        std::vector<RegisterPtr>        ParseRegisterList(bool parseFirstColon = true);
        std::vector<AttributePtr>       ParseAttributeList();
        std::vector<SwitchCasePtr>      ParseSwitchCaseList();
        std::vector<BufferDeclPtr>      ParseBufferDeclList(BufferDeclStmnt* declStmntRef, const TokenPtr& identTkn = nullptr);
        std::vector<SamplerDeclPtr>     ParseSamplerDeclList(SamplerDeclStmnt* declStmntRef, const TokenPtr& identTkn = nullptr);
        std::vector<SamplerValuePtr>    ParseSamplerValueList();
        std::vector<AliasDeclPtr>       ParseAliasDeclList(TypeDenoterPtr typeDenoter);

        std::string                     ParseIdent(const TokenPtr& identTkn = nullptr);
        std::string                     ParseRegister_OBSOLETE(bool parseColon = true);

        TypeDenoterPtr                  ParseTypeDenoter(bool allowVoidType = true, StructDeclPtr* structDecl = nullptr);
        TypeDenoterPtr                  ParseTypeDenoterPrimary(StructDeclPtr* structDecl = nullptr);
        TypeDenoterPtr                  ParseTypeDenoterWithStructDeclOpt(StructDeclPtr& structDecl, bool allowVoidType = true);
        VoidTypeDenoterPtr              ParseVoidTypeDenoter();
        BaseTypeDenoterPtr              ParseBaseTypeDenoter();
        BaseTypeDenoterPtr              ParseBaseVectorTypeDenoter();
        BaseTypeDenoterPtr              ParseBaseMatrixTypeDenoter();
        BufferTypeDenoterPtr            ParseBufferTypeDenoter();
        SamplerTypeDenoterPtr           ParseSamplerTypeDenoter();
        StructTypeDenoterPtr            ParseStructTypeDenoter();
        StructTypeDenoterPtr            ParseStructTypeDenoterWithStructDeclOpt(StructDeclPtr& structDecl);
        AliasTypeDenoterPtr             ParseAliasTypeDenoter(std::string ident = "");

        Variant                         ParseAndEvaluateConstExpr();
        int                             ParseAndEvaluateConstExprInt();
        int                             ParseAndEvaluateVectorDimension();

        void                            ParseAndIgnoreTechnique();

        void                            ParseVarDeclSemantic(VarDecl& varDecl, bool allowPackOffset = true);
        void                            ParseFunctionDeclSemantic(FunctionDecl& funcDecl);

        DataType                        ParseDataType(const std::string& keyword);
        PrimitiveType                   ParsePrimitiveType();
        InterpModifier                  ParseInterpModifier();
        TypeModifier                    ParseTypeModifier();
        StorageClass                    ParseStorageClass();
        UniformBufferType               ParseUniformBufferType();
        BufferType                      ParseBufferType();
        SamplerType                     ParseSamplerType();
        IndexedSemantic                 ParseSemantic(bool parseColon = true);

        std::string                     ParseSamplerStateTextureIdent();

        void                            ParseStmntWithOptionalComment(std::vector<StmntPtr>& stmnts, const std::function<StmntPtr()>& parseFunction);

        bool                            ParseVarDeclStmntModifiers(VarDeclStmnt* ast, bool allowPrimitiveType = false);

        /* === Members === */

        using TypeNameSymbolTable = SymbolTable<bool>;

        // True, if the parser is currently inside a local scope of a function (to detect illegal semantics inside local scopes).
        bool                localScope_             = false;

        // Symbol table for type name (i.e. structure and typedef identifiers) to detect cast expression, which are not context free.
        TypeNameSymbolTable typeNameSymbolTable_;

};


} // /namespace Xsc


#endif



// ================================================================================
