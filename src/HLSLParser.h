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

        bool ParseSource(const std::shared_ptr<SourceCode>& source);

    private:
        
        /* === Functions === */

        void Error(const std::string& msg);
        void ErrorUnexpected();
        void ErrorUnexpected(const std::string& hint);

        TokenPtr Accept(const Token::Types type);
        TokenPtr AcceptIt();

        /* === Members === */

        HLSLScanner scanner_;

        TokenPtr tkn_;

        Logger* log_ = nullptr;

};


} // /namespace HTLib


#endif



// ================================================================================