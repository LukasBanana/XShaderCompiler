/*
 * Parser.h
 * 
 * This file is part of the "HLSL Translator" (Copyright (c) 2014 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef HTLIB_PARSER_H
#define HTLIB_PARSER_H


#include "HT/Log.h"
#include "HLSLScanner.h"
#include "HLSLErr.h"
#include "ReportHandler.h"
#include "Visitor.h"
#include "Token.h"

#include <vector>
#include <map>
#include <string>
#include <stack>


namespace HTLib
{


// Syntax parser base class.
class Parser
{
    
    public:
        
        virtual ~Parser();

    protected:
        
        using Tokens = Token::Types;

        /* === Functions === */

        Parser(Log* log);

        virtual ScannerPtr MakeScanner() = 0;

        void PushScannerSource(const std::shared_ptr<SourceCode>& source, const std::string& filename = "");
        bool PopScannerSource();

        // Returns the filename for the current scanner source.
        std::string GetCurrentFilename() const;

        void Error(const std::string& msg, Token* tkn, const HLSLErr errorCode = HLSLErr::Unknown);
        void Error(const std::string& msg, bool prevToken = true, const HLSLErr errorCode = HLSLErr::Unknown);

        void ErrorUnexpected(const std::string& hint = "");
        void ErrorUnexpected(const Tokens type);

        void Warning(const std::string& msg, Token* tkn);
        void Warning(const std::string& msg, bool prevToken = true);

        TokenPtr Accept(const Tokens type);
        TokenPtr Accept(const Tokens type, const std::string& spell);
        TokenPtr AcceptIt();

        // Ignores the next tokens if they are white spaces and optionally new lines.
        void IgnoreWhiteSpaces(bool includeNewLines = true);//, bool includeComments = true);
        void IgnoreNewLines();

        // Returns the log pointer or null if no log was defined.
        inline Log* GetLog() const
        {
            return log_;
        }

        // Makes a new shared pointer of the specified AST node class.
        template <typename T, typename... Args>
        std::shared_ptr<T> Make(Args&&... args)
        {
            return std::make_shared<T>(GetScanner().Pos(), args...);
        }

        // Returns the current token.
        inline const TokenPtr& Tkn() const
        {
            return tkn_;
        }

        // Returns the type of the next token.
        inline Tokens Type() const
        {
            return Tkn()->Type();
        }

        // Returns true if the next token is from the specified type.
        inline bool Is(const Tokens type) const
        {
            return (Type() == type);
        }

        // Returns true if the next token is from the specified type and has the specified spelling.
        inline bool Is(const Tokens type, const std::string& spell) const
        {
            return (Type() == type && Tkn()->Spell() == spell);
        }

    private:

        /* === Structures === */

        struct ScannerStackEntry
        {
            ScannerPtr  scanner;
            std::string filename;
            TokenPtr    nextToken;
        };

        /* === Functions === */

        Scanner& GetScanner();

        /* === Members === */

        ReportHandler                   reportHandler_;

        Log*                            log_            = nullptr;
        TokenPtr                        tkn_;

        std::stack<ScannerStackEntry>   scannerStack_;

};


} // /namespace HTLib


#endif



// ================================================================================