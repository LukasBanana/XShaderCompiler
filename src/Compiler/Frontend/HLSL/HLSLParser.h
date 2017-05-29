/*
 * HLSLParser.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
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

        ProgramPtr ParseSource(
            const SourceCodePtr& source,
            const NameMangling& nameMangling,
            const InputShaderVersion versionIn,
            bool rowMajorAlignment = false,
            bool enableWarnings = false
        );

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

        // Returns true if the current token is a modifier of a type specifier.
        bool IsModifier() const;

        // Converts the specified expression to a type name expression if it is a left-hand-side of a cast expression.
        TypeSpecifierPtr MakeTypeSpecifierIfLhsOfCastExpr(const ExprPtr& expr);

        // Overrides the token accept function to process all directives before the actual parsing.
        TokenPtr AcceptIt() override;

        // Processes the specified directive (only '#line'-directive are allowed after pre-processing).
        void ProcessDirective(const std::string& ident);
        void ProcessDirectiveLine();
        void ProcessDirectivePragma();

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
        AliasDeclStmntPtr MakeAndRegisterBuildinAlias(const DataType dataType, const std::string& ident);

        // Generates all pre defined type aliases (e.g. 'typedef int DWORD').
        void GeneratePreDefinedTypeAliases(Program& ast);

        // Creates a new var-decl statement with the current matrix pack alignment type modifier.
        TypeSpecifierPtr MakeTypeSpecifierWithPackAlignment();

        /* ----- Parsing ----- */

        ProgramPtr                      ParseProgram(const SourceCodePtr& source);

        CodeBlockPtr                    ParseCodeBlock();
        VarDeclStmntPtr                 ParseParameter();
        SwitchCasePtr                   ParseSwitchCase();
        SamplerValuePtr                 ParseSamplerValue();
        AttributePtr                    ParseAttribute();
        RegisterPtr                     ParseRegister(bool parseColon = true);
        PackOffsetPtr                   ParsePackOffset(bool parseColon = true);
        ArrayDimensionPtr               ParseArrayDimension(bool allowDynamicDimension = false);
        ExprPtr                         ParseArrayIndex();
        ExprPtr                         ParseInitializer();
        TypeSpecifierPtr                ParseTypeSpecifier(bool parseVoidType = false);

        VarDeclPtr                      ParseVarDecl(VarDeclStmnt* declStmntRef, const TokenPtr& identTkn = nullptr);
        BufferDeclPtr                   ParseBufferDecl(BufferDeclStmnt* declStmntRef, const TokenPtr& identTkn = nullptr);
        SamplerDeclPtr                  ParseSamplerDecl(SamplerDeclStmnt* declStmntRef, const TokenPtr& identTkn = nullptr);
        StructDeclPtr                   ParseStructDecl(bool parseStructTkn = true, const TokenPtr& identTkn = nullptr);
        AliasDeclPtr                    ParseAliasDecl(TypeDenoterPtr typeDenoter);
        FunctionDeclPtr                 ParseFunctionDecl(BasicDeclStmnt* declStmntRef, const TypeSpecifierPtr& returnType = nullptr, const TokenPtr& identTkn = nullptr);
        UniformBufferDeclPtr            ParseUniformBufferDecl();

        StmntPtr                        ParseGlobalStmnt();
        StmntPtr                        ParseGlobalStmntPrimary();
        StmntPtr                        ParseGlobalStmntWithTypeSpecifier();
        StmntPtr                        ParseGlobalStmntWithSamplerTypeDenoter();
        StmntPtr                        ParseGlobalStmntWithBufferTypeDenoter();
        BasicDeclStmntPtr               ParseFunctionDeclStmnt(const TypeSpecifierPtr& returnType = nullptr, const TokenPtr& identTkn = nullptr);
        BasicDeclStmntPtr               ParseUniformBufferDeclStmnt();
        BufferDeclStmntPtr              ParseBufferDeclStmnt(const BufferTypeDenoterPtr& typeDenoter = nullptr, const TokenPtr& identTkn = nullptr);
        SamplerDeclStmntPtr             ParseSamplerDeclStmnt(const SamplerTypeDenoterPtr& typeDenoter = nullptr, const TokenPtr& identTkn = nullptr);
        VarDeclStmntPtr                 ParseVarDeclStmnt();
        AliasDeclStmntPtr               ParseAliasDeclStmnt();

        StmntPtr                        ParseStmnt(bool allowAttributes = true);
        StmntPtr                        ParseStmntPrimary();
        StmntPtr                        ParseStmntWithStructDecl();
        StmntPtr                        ParseStmntWithIdent();
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

        ExprPtr                         ParseExpr();
        ExprPtr                         ParseExprWithSequenceOpt();
        ExprPtr                         ParsePrimaryExpr() override;
        ExprPtr                         ParsePrimaryExprPrefix();
        ExprPtr                         ParseExprWithSuffixOpt(ExprPtr expr);
        LiteralExprPtr                  ParseLiteralExpr();
        ExprPtr                         ParseTypeSpecifierOrCallExpr();
        TypeSpecifierExprPtr            ParseTypeSpecifierExpr();
        UnaryExprPtr                    ParseUnaryExpr();
        PostUnaryExprPtr                ParsePostUnaryExpr(const ExprPtr& expr);
        ExprPtr                         ParseExprWithBracketPrefix();
        ObjectExprPtr                   ParseObjectExpr(const ExprPtr& expr = nullptr);
        AssignExprPtr                   ParseAssignExpr(const ExprPtr& expr);
        ExprPtr                         ParseObjectOrCallExpr(const ExprPtr& expr = nullptr);
        ArrayExprPtr                    ParseArrayExpr(const ExprPtr& expr);
        CallExprPtr                     ParseCallExpr(const ObjectExprPtr& objectExpr = nullptr, const TypeDenoterPtr& typeDenoter = nullptr);
        CallExprPtr                     ParseCallExprWithPrefixOpt(const ExprPtr& prefixExpr = nullptr, bool isStatic = false, const TokenPtr& identTkn = nullptr);
        CallExprPtr                     ParseCallExprAsTypeCtor(const TypeDenoterPtr& typeDenoter);
        InitializerExprPtr              ParseInitializerExpr();
        SequenceExprPtr                 ParseSequenceExpr(const ExprPtr& firstExpr);

        std::vector<StmntPtr>           ParseLocalStmntList();
        std::vector<VarDeclPtr>         ParseVarDeclList(VarDeclStmnt* declStmntRef, TokenPtr firstIdentTkn = nullptr);
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

        std::string                     ParseIdent(TokenPtr identTkn = nullptr, SourceArea* area = nullptr);
        std::string                     ParseIdentWithNamespaceOpt(ObjectExprPtr& namespaceExpr, TokenPtr identTkn = nullptr, SourceArea* area = nullptr);

        TypeDenoterPtr                  ParseTypeDenoter(bool allowVoidType = true, StructDeclPtr* structDecl = nullptr);
        TypeDenoterPtr                  ParseTypeDenoterPrimary(StructDeclPtr* structDecl = nullptr);
        TypeDenoterPtr                  ParseTypeDenoterWithStructDeclOpt(StructDeclPtr& structDecl, bool allowVoidType = true);
        TypeDenoterPtr                  ParseTypeDenoterWithArrayOpt(const TypeDenoterPtr& baseTypeDenoter);
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

        void                            ParseAndIgnoreTechniquesAndNullStmnts();
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

        void                            ParseStmntWithCommentOpt(std::vector<StmntPtr>& stmnts, const std::function<StmntPtr()>& parseFunction);

        bool                            ParseModifiers(TypeSpecifier* typeSpecifier, bool allowPrimitiveType = false);

        /* === Members === */

        using TypeNameSymbolTable = SymbolTable<bool>;

        // Symbol table for type name (i.e. structure and typedef identifiers) to detect cast expression, which are not context free.
        TypeNameSymbolTable typeNameSymbolTable_;

        // True, if semantics are parsed for D3D10+ shader.
        bool                useD3D10Semantics_      = true;

        // True, if Cg keyword extensions are enabled.
        bool                enableCgKeywords_       = false;

        // True, if matrix packing is globally set to row major.
        bool                rowMajorAlignment_      = false;

};


} // /namespace Xsc


#endif



// ================================================================================
