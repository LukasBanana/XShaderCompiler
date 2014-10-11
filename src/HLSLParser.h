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

        /* === Functions === */

        void Error(const std::string& msg);
        void ErrorUnexpected();
        void ErrorUnexpected(const std::string& hint);

        TokenPtr Accept(const Token::Types type);
        TokenPtr AcceptIt();

        //! Makes a new shared pointer of the specified AST node class.
        template <typename T> std::shared_ptr<T> Make()
        {
            return std::make_shared<T>(scanner_.Pos());
        }

        inline Token::Types Type() const
        {
            return tkn_->Type();
        }

        /* === Parse functions === */

        ProgramPtr          ParseProgram();

        GlobalDeclPtr       ParseGlobalDecl();
        FunctionDeclPtr     ParseFunctionDecl();
        BufferDeclPtr       ParseBufferDecl();
        TextureDeclPtr      ParseTextureDecl();
        SamplerStateDeclPtr ParseSamplerStateDecl();
        StructDeclPtr       ParseStructDecl();

        /* === Members === */

        HLSLScanner scanner_;

        TokenPtr tkn_;

        Logger* log_ = nullptr;

};


} // /namespace HTLib


#endif



// ================================================================================