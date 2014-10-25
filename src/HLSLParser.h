/*
 * HLSLParser.h
 * 
 * This file is part of the "HLSL Translator" (Copyright (c) 2014 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __HT_HLSL_PARSER_H__
#define __HT_HLSL_PARSER_H__


#include "HT/Logger.h"
#include "HLSLScanner.h"
#include "Visitor.h"
#include "Token.h"

#include <vector>
#include <map>
#include <string>


namespace HTLib
{


//! Syntax parser class.
class HLSLParser
{
    
    public:
        
        HLSLParser(Logger* log = nullptr);

        ProgramPtr ParseSource(const std::shared_ptr<SourceCode>& source);

    private:
        
        typedef Token::Types Tokens;

        /* === Enumerations === */

        //! Variable declaration modifiers.
        enum class VarModifiers
        {
            StorageModifier,    //!< Storage class or interpolation modifier (extern, linear, centroid, nointerpolation, noperspective, sample).
            TypeModifier,       //!< Type modifier (const, row_major, column_major).
        };

        /* === Functions === */

        void EstablishMaps();

        void Error(const std::string& msg);
        void ErrorUnexpected();
        void ErrorUnexpected(const std::string& hint);

        TokenPtr Accept(const Tokens type);
        TokenPtr Accept(const Tokens type, const std::string& spell);
        TokenPtr AcceptIt();

        //! Accepts the semicolon token (Accept(Tokens::Semicolon)).
        void Semi();

        //! Returns true if the current token is a data type.
        bool IsDataType() const;
        //! Returns true if the current token is a literal.
        bool IsLiteral() const;
        //! Returns true if the current token is part of a primary expression.
        bool IsPrimaryExpr() const;

        //! Makes a new shared pointer of the specified AST node class.
        template <typename T, typename... Args> std::shared_ptr<T> Make(Args&&... args)
        {
            return std::make_shared<T>(scanner_.Pos(), args...);
        }

        //! Returns the type of the next token.
        inline Tokens Type() const
        {
            return tkn_->Type();
        }

        //! Returns true if the next token is from the specified type.
        inline bool Is(const Tokens type) const
        {
            return Type() == type;
        }
        //! Returns true if the next token is from the specified type and has the specified spelling.
        inline bool Is(const Tokens type, const std::string& spell) const
        {
            return Type() == type && tkn_->Spell() == spell;
        }

        /* === Parse functions === */

        ProgramPtr                      ParseProgram();

        CodeBlockPtr                    ParseCodeBlock();
        BufferDeclIdentPtr              ParseBufferDeclIdent();
        FunctionCallPtr                 ParseFunctionCall(VarIdentPtr varIdent = nullptr);
        StructurePtr                    ParseStructure();
        VarDeclStmntPtr                 ParseParameter();
        SwitchCasePtr                   ParseSwitchCase();

        GlobalDeclPtr                   ParseGlobalDecl();
        FunctionDeclPtr                 ParseFunctionDecl();
        UniformBufferDeclPtr            ParseUniformBufferDecl();
        TextureDeclPtr                  ParseTextureDecl();
        SamplerDeclPtr                  ParseSamplerDecl();
        StructDeclPtr                   ParseStructDecl();
        DirectiveDeclPtr                ParseDirectiveDecl();

        FunctionCallPtr                 ParseAttribute();
        PackOffsetPtr                   ParsePackOffset(bool parseColon = true);
        ExprPtr                         ParseArrayDimension();
        ExprPtr                         ParseInitializer();
        VarSemanticPtr                  ParseVarSemantic();
        VarIdentPtr                     ParseVarIdent();
        VarTypePtr                      ParseVarType(bool parseVoidType = false);
        VarDeclPtr                      ParseVarDecl();

        StmntPtr                        ParseStmnt();
        NullStmntPtr                    ParseNullStmnt();
        DirectiveStmntPtr               ParseDirectiveStmnt();
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
        ExprPtr                         ParsePrimaryExpr();
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
        std::vector<ExprPtr>            ParseExprList(const Tokens listTerminatorToken);
        std::vector<ExprPtr>            ParseArrayDimensionList();
        std::vector<ExprPtr>            ParseArgumentList();
        std::vector<ExprPtr>            ParseInitializerList();
        std::vector<VarSemanticPtr>     ParseVarSemanticList();
        std::vector<FunctionCallPtr>    ParseAttributeList();
        std::vector<SwitchCasePtr>      ParseSwitchCaseList();
        std::vector<BufferDeclIdentPtr> ParseBufferDeclIdentList();

        std::string                     ParseRegister(bool parseColon = true);
        std::string                     ParseSemantic();

        /* === Members === */

        HLSLScanner scanner_;
        TokenPtr tkn_;

        Logger* log_ = nullptr;

        std::map<std::string, VarModifiers> varModifierMap_;

};


} // /namespace HTLib


#endif



// ================================================================================