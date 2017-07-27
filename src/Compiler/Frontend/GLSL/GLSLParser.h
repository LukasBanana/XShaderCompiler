/*
 * GLSLParser.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
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
            const SourceCodePtr& source,
            const NameMangling& nameMangling,
            const InputShaderVersion versionIn,
            bool enableWarnings = false
        );

    private:
    
        /* === Functions === */

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
        VarDeclStmntPtr                 ParseParameter() override;
        StmntPtr                        ParseLocalStmnt() override;
        StmntPtr                        ParseForLoopInitializer() override;
        SwitchCasePtr                   ParseSwitchCase() override;
        VarDeclPtr                      ParseVarDecl(VarDeclStmnt* declStmntRef, const TokenPtr& identTkn = nullptr) override;

        AttributePtr                    ParseAttribute();
        TypeSpecifierPtr                ParseTypeSpecifier(bool parseVoidType = false, const TokenPtr& inputTkn = nullptr);

        #if 0
        BufferDeclPtr                   ParseBufferDecl(BufferDeclStmnt* declStmntRef, const TokenPtr& identTkn = nullptr);
        SamplerDeclPtr                  ParseSamplerDecl(SamplerDeclStmnt* declStmntRef, const TokenPtr& identTkn = nullptr);
        #endif
        StructDeclPtr                   ParseStructDecl(bool parseStructTkn = true, const TokenPtr& identTkn = nullptr);
        FunctionDeclPtr                 ParseFunctionDecl(BasicDeclStmnt* declStmntRef, const TypeSpecifierPtr& returnType = nullptr, const TokenPtr& identTkn = nullptr);
        UniformBufferDeclPtr            ParseUniformBufferDecl(const TokenPtr& identTkn = nullptr);

        StmntPtr                        ParseGlobalStmnt();
        StmntPtr                        ParseGlobalStmntPrimary(bool hasAttribs = false);
        StmntPtr                        ParseGlobalStmntWithTypeSpecifier(const TokenPtr& inputTkn = nullptr);
        StmntPtr                        ParseGlobalStmntWithLayoutQualifier();
        #if 0
        StmntPtr                        ParseGlobalStmntWithSamplerTypeDenoter();
        StmntPtr                        ParseGlobalStmntWithBufferTypeDenoter();
        #endif
        BasicDeclStmntPtr               ParseFunctionDeclStmnt(const TypeSpecifierPtr& returnType = nullptr, const TokenPtr& identTkn = nullptr);
        StmntPtr                        ParseUniformDeclStmnt();
        BasicDeclStmntPtr               ParseUniformBufferDeclStmnt(const TokenPtr& identTkn = nullptr);
        #if 0
        BufferDeclStmntPtr              ParseBufferDeclStmnt(const BufferTypeDenoterPtr& typeDenoter = nullptr, const TokenPtr& identTkn = nullptr);
        SamplerDeclStmntPtr             ParseSamplerDeclStmnt(const SamplerTypeDenoterPtr& typeDenoter = nullptr, const TokenPtr& identTkn = nullptr);
        #endif
        VarDeclStmntPtr                 ParseVarDeclStmnt(bool isUniform = false, const TokenPtr& identTkn = nullptr);

        StmntPtr                        ParseStmnt();
        StmntPtr                        ParseStmntWithStructDecl();
        #if 0
        StmntPtr                        ParseStmntWithIdent();
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
        ObjectExprPtr                   ParseObjectExpr(const ExprPtr& expr = nullptr);
        AssignExprPtr                   ParseAssignExpr(const ExprPtr& expr);
        ExprPtr                         ParseObjectOrCallExpr(const ExprPtr& expr = nullptr);
        CallExprPtr                     ParseCallExpr(const ObjectExprPtr& objectExpr = nullptr, const TypeDenoterPtr& typeDenoter = nullptr);
        CallExprPtr                     ParseCallExprWithPrefixOpt(const ExprPtr& prefixExpr = nullptr, bool isStatic = false, const TokenPtr& identTkn = nullptr);
        CallExprPtr                     ParseCallExprAsTypeCtor(const TypeDenoterPtr& typeDenoter);

        std::vector<StmntPtr>           ParseGlobalStmntList();
        std::vector<AttributePtr>       ParseAttributeList();
        #if 0
        std::vector<BufferDeclPtr>      ParseBufferDeclList(BufferDeclStmnt* declStmntRef, const TokenPtr& identTkn = nullptr);
        std::vector<SamplerDeclPtr>     ParseSamplerDeclList(SamplerDeclStmnt* declStmntRef, const TokenPtr& identTkn = nullptr);
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

        /* === Members === */

        using TypeNameSymbolTable = SymbolTable<bool>;

        // Symbol table for type name (i.e. structure and typedef identifiers) to detect cast expression, which are not context free.
        TypeNameSymbolTable typeNameSymbolTable_;

        int                 version_                = 0;
        bool                isCompatibilityProfile_ = false;

};


} // /namespace Xsc


#endif



// ================================================================================
