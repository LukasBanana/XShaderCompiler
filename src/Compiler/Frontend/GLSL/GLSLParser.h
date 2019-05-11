/*
 * GLSLParser.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_GLSL_PARSER_H
#define XSC_GLSL_PARSER_H


#include "SLParser.h"
#include "GLSLScanner.h"
#include "SymbolTable.h"
#include <map>


namespace Xsc
{


// Syntax parser class for the shading language GLSL.
class GLSLParser : public SLParser
{

    public:

        GLSLParser(Log* log = nullptr);

        ProgramPtr ParseSource(
            const SourceCodePtr&        source,
            const NameMangling&         nameMangling,
            const InputShaderVersion    versionIn,
            bool                        enableWarnings = false
        );

    private:

        ScannerPtr MakeScanner() override;

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

        // Overrides the token accept function to process all directives before the actual parsing.
        TokenPtr AcceptIt() override;

        // Processes the specified directive (only '#line'-directive are allowed after pre-processing).
        void ProcessDirective(const std::string& ident);
        void ProcessDirectiveLine();
        void ProcessDirectiveVersion();
        void ProcessDirectiveExtension();

        /* ----- Symbol table ----- */

        // Opens a new scope of the type name symbol table.
        void OpenScope();

        // Closes the current scope of the type name symbol table.
        void CloseScope();

        // Registers the specified identifier as type name, to detect cast expressions.
        void RegisterTypeName(const std::string& ident);

        // Returns true if the specified identifier is a valid type name within the current scope.
        bool IsRegisteredTypeName(const std::string& ident) const;

        /* ----- Parsing ----- */

        ProgramPtr                      ParseProgram(const SourceCodePtr& source);

        CodeBlockPtr                    ParseCodeBlock() override;
        VarDeclStmtPtr                  ParseParameter() override;
        StmtPtr                         ParseLocalStmt() override;
        StmtPtr                         ParseForStmtInitializer() override;
        SwitchCasePtr                   ParseSwitchCase() override;
        VarDeclPtr                      ParseVarDecl(VarDeclStmt* declStmtRef, const TokenPtr& identTkn = nullptr) override;

        AttributePtr                    ParseAttribute();
        TypeSpecifierPtr                ParseTypeSpecifier(bool parseVoidType = false, const TokenPtr& inputTkn = nullptr);

        #if 0
        BufferDeclPtr                   ParseBufferDecl(BufferDeclStmt* declStmtRef, const TokenPtr& identTkn = nullptr);
        SamplerDeclPtr                  ParseSamplerDecl(SamplerDeclStmt* declStmtRef, const TokenPtr& identTkn = nullptr);
        #endif
        StructDeclPtr                   ParseStructDecl(bool parseStructTkn = true, const TokenPtr& identTkn = nullptr);
        FunctionDeclPtr                 ParseFunctionDecl(BasicDeclStmt* declStmtRef, const TypeSpecifierPtr& returnType = nullptr, const TokenPtr& identTkn = nullptr);
        UniformBufferDeclPtr            ParseUniformBufferDecl(const TokenPtr& identTkn = nullptr);

        StmtPtr                         ParseGlobalStmt();
        StmtPtr                         ParseGlobalStmtPrimary(bool hasAttribs = false);
        StmtPtr                         ParseGlobalStmtWithTypeSpecifier(const TokenPtr& inputTkn = nullptr);
        StmtPtr                         ParseGlobalStmtWithLayoutQualifier();
        #if 0
        StmtPtr                         ParseGlobalStmtWithSamplerTypeDenoter();
        StmtPtr                         ParseGlobalStmtWithBufferTypeDenoter();
        #endif
        BasicDeclStmtPtr                ParseFunctionDeclStmt(const TypeSpecifierPtr& returnType = nullptr, const TokenPtr& identTkn = nullptr);
        StmtPtr                         ParseUniformDeclStmt();
        BasicDeclStmtPtr                ParseUniformBufferDeclStmt(const TokenPtr& identTkn = nullptr);
        #if 0
        BufferDeclStmtPtr               ParseBufferDeclStmt(const BufferTypeDenoterPtr& typeDenoter = nullptr, const TokenPtr& identTkn = nullptr);
        SamplerDeclStmtPtr              ParseSamplerDeclStmt(const SamplerTypeDenoterPtr& typeDenoter = nullptr, const TokenPtr& identTkn = nullptr);
        #endif
        VarDeclStmtPtr                  ParseVarDeclStmt(bool isUniform = false, const TokenPtr& identTkn = nullptr);

        StmtPtr                         ParseStmt();
        StmtPtr                         ParseStmtWithStructDecl();
        #if 0
        StmtPtr                         ParseStmtWithIdent();
        #endif

        ExprPtr                         ParsePrimaryExpr() override;
        ExprPtr                         ParsePrimaryExprPrefix();
        ExprPtr                         ParseExprWithSuffixOpt(ExprPtr expr);
        LiteralExprPtr                  ParseLiteralExpr();
        ExprPtr                         ParseTypeSpecifierOrCallExpr();
        TypeSpecifierExprPtr            ParseTypeSpecifierExpr();
        UnaryExprPtr                    ParseUnaryExpr();
        PostUnaryExprPtr                ParsePostUnaryExpr(const ExprPtr& expr);
        BracketExprPtr                  ParseBracketExpr();
        IdentExprPtr                    ParseIdentExpr(const ExprPtr& prefixExpr = nullptr);
        AssignExprPtr                   ParseAssignExpr(const ExprPtr& lvalueExpr);
        ExprPtr                         ParseIdentOrCallExpr(const ExprPtr& prefixExpr = nullptr);
        CallExprPtr                     ParseCallExpr(const IdentExprPtr& identExpr = nullptr, const TypeDenoterPtr& typeDenoter = nullptr);
        CallExprPtr                     ParseCallExprWithPrefixOpt(const ExprPtr& prefixExpr = nullptr, bool isStatic = false, const TokenPtr& identTkn = nullptr);
        CallExprPtr                     ParseCallExprAsTypeCtor(const TypeDenoterPtr& typeDenoter);

        std::vector<StmtPtr>            ParseGlobalStmtList();
        std::vector<AttributePtr>       ParseAttributeList();
        #if 0
        std::vector<BufferDeclPtr>      ParseBufferDeclList(BufferDeclStmt* declStmtRef, const TokenPtr& identTkn = nullptr);
        std::vector<SamplerDeclPtr>     ParseSamplerDeclList(SamplerDeclStmt* declStmtRef, const TokenPtr& identTkn = nullptr);
        #endif

        TypeDenoterPtr                  ParseTypeDenoter(bool allowVoidType = true, StructDeclPtr* structDecl = nullptr);
        TypeDenoterPtr                  ParseTypeDenoterPrimary(StructDeclPtr* structDecl = nullptr);
        TypeDenoterPtr                  ParseTypeDenoterWithStructDeclOpt(StructDeclPtr& structDecl, bool allowVoidType = true);
        VoidTypeDenoterPtr              ParseVoidTypeDenoter();
        BaseTypeDenoterPtr              ParseBaseTypeDenoter();
        BufferTypeDenoterPtr            ParseBufferTypeDenoter();
        SamplerTypeDenoterPtr           ParseSamplerTypeDenoter();
        StructTypeDenoterPtr            ParseStructTypeDenoter();
        StructTypeDenoterPtr            ParseStructTypeDenoterWithStructDeclOpt(StructDeclPtr& structDecl);

        DataType                        ParseDataType(const std::string& keyword);
        PrimitiveType                   ParsePrimitiveType();
        InterpModifier                  ParseInterpModifier();
        TypeModifier                    ParseTypeModifier();
        StorageClass                    ParseStorageClass();
        SamplerType                     ParseSamplerType();

        bool                            ParseModifiers(TypeSpecifier* typeSpecifier, bool allowPrimitiveType = false, const TokenPtr& inputTkn = nullptr);

    private:

        using TypeNameSymbolTable = SymbolTable<bool>;

        // Symbol table for type name (i.e. structure and typedef identifiers) to detect cast expression, which are not context free.
        TypeNameSymbolTable typeNameSymbolTable_;

        int                 version_        = 0;
        bool                isESSL_         = false;
        bool                isCoreProfile_  = false;

};


} // /namespace Xsc


#endif



// ================================================================================
