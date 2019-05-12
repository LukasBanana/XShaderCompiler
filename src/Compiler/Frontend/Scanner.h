/*
 * Scanner.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_SCANNER_H
#define XSC_SCANNER_H


#include <Xsc/Xsc.h>
#include <Xsc/Log.h>
#include "SourceCode.h"
#include "SourceArea.h"
#include "Token.h"
#include "TokenString.h"

#include <string>
#include <functional>


namespace Xsc
{


// Scanner base class.
class Scanner
{

    public:

        Scanner(Log* log = nullptr);
        virtual ~Scanner() = default;

        // Starts scanning the specified source code.
        bool ScanSource(const SourceCodePtr& source);

        // Pushes the specified token string onto the stack where further tokens will be parsed from the top of the stack.
        void PushTokenString(const TokenPtrString& tokenString);
        void PopTokenString();

        // Returns the iterator of the top most token string on the stack.
        TokenPtrString::ConstIterator TopTokenStringIterator() const;

        // Scanes the source code for the next token
        virtual TokenPtr Next() = 0;

        // Returns the token previously returned by the "Next" function.
        TokenPtr ActiveToken() const;

        // Returns the token previously returned by the "Next" function.
        TokenPtr PreviousToken() const;

        // Returns the start position of the token previously returned by the "Next" function.
        inline const SourcePosition& Pos() const
        {
            return nextStartPos_;
        }

        // Returns the source code which is currently being scanned.
        inline SourceCode* Source() const
        {
            return source_.get();
        }

        // Returns the source code which is currently being scanned.
        inline const SourceCodePtr& GetSharedSource() const
        {
            return source_;
        }

        // Returns the active commentary string, which is in front of the next token.
        inline const std::string& GetComment() const
        {
            return comment_;
        }

    protected:

        using Tokens = Token::Types;

        TokenPtr NextToken(bool scanComments, bool scanWhiteSpaces);

        void StoreStartPos();

        virtual TokenPtr ScanToken() = 0;

        char Take(char chr);
        char TakeIt();

        TokenPtr Make(const Token::Types& type, bool takeChr = false);
        TokenPtr Make(const Token::Types& type, std::string& spell, bool takeChr = false);
        TokenPtr Make(const Token::Types& type, std::string& spell, const SourcePosition& pos, bool takeChr = false);

        /* ----- Report Handling ----- */

        // Throws an instance of the exception Report class.
        [[noreturn]]
        void Error(const std::string& msg);

        // Throws an 'unexpected character' error.
        [[noreturn]]
        void ErrorUnexpected();

        // Throws an 'unexpected character' error with suggestion which character was expected.
        [[noreturn]]
        void ErrorUnexpected(char expectedChar);

        // Throws an 'unexpected end-of-stream' error.
        [[noreturn]]
        void ErrorUnexpectedEOS();

        /* ----- Scanning ----- */

        // Ignores all characters which comply the specified predicate.
        void        Ignore(const std::function<bool(char)>& pred);
        void        IgnoreWhiteSpaces(bool includeNewLines = true);

        TokenPtr    ScanWhiteSpaces(bool includeNewLines = true);
        TokenPtr    ScanCommentLine(bool scanComments);
        TokenPtr    ScanCommentBlock(bool scanComments);
        TokenPtr    ScanStringLiteral();
        TokenPtr    ScanCharLiteral();
        TokenPtr    ScanNumber(bool startWithPeriod = false, bool acceptInfConst = false);
        TokenPtr    ScanNumberOrDot();
        TokenPtr    ScanVarArg(std::string& spell);

        bool        ScanDigitSequence(std::string& spell);

        /* ----- Helper functions ----- */

        // Returns true if the next character is a new-line character (i.e. '\n' or '\r').
        inline bool IsNewLine() const
        {
            return (chr_ == '\n' || chr_ == '\r');
        }

        // Returns true if the next character is equal to the specified character.
        inline bool Is(char chr) const
        {
            return (chr_ == chr);
        }

        // Returns the next character.
        inline char Chr() const
        {
            return chr_;
        }

        // Returns the next character as 'unsigned char'.
        inline unsigned char UChr() const
        {
            return static_cast<unsigned char>(chr_);
        }

    private:

        TokenPtr NextTokenScan(bool scanComments, bool scanWhiteSpaces);

        void AppendComment(const std::string& s);
        void AppendMultiLineComment(const std::string& s);

    private:

        SourceCodePtr                               source_;
        char                                        chr_                = 0;

        Log*                                        log_                = nullptr;

        SourcePosition                              nextStartPos_;
        TokenPtr                                    activeToken_;
        TokenPtr                                    prevToken_;

        std::vector<TokenPtrString::ConstIterator>  tokenStringItStack_;

        // Active commentary string (in front of the next token).
        std::string                                 comment_;
        unsigned int                                commentStartPos_    = 0;
        bool                                        commentFirstLine_   = false;

};

using ScannerPtr = std::shared_ptr<Scanner>;


} // /namespace Xsc


#endif



// ================================================================================