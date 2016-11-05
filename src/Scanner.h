/*
 * Scanner.h
 * 
 * This file is part of the "HLSL Translator" (Copyright (c) 2014 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef HTLIB_SCANNER_H
#define HTLIB_SCANNER_H


#include "HT/Translator.h"
#include "HT/Log.h"
#include "SourceCode.h"
#include "SourcePosition.h"
#include "Token.h"

#include <string>
#include <functional>


namespace HTLib
{


// Scanner base class.
class Scanner
{
    
    public:
        
        Scanner(Log* log = nullptr);
        virtual ~Scanner();

        bool ScanSource(const std::shared_ptr<SourceCode>& source);

        SourcePosition Pos() const;

        inline SourceCode* Source() const
        {
            return source_.get();
        }

    protected:
        
        /* === Functions === */

        TokenPtr NextToken(bool scanComments, bool scanWhiteSpaces);

        virtual TokenPtr ScanToken() = 0;

        char Take(char chr);
        char TakeIt();

        void Error(const std::string& msg);
        void ErrorUnexpected();
        void ErrorUnexpected(char expectedChar);

        // Ignores all characters which comply the specified predicate.
        void Ignore(const std::function<bool(char)>& pred);
        void IgnoreWhiteSpaces();

        TokenPtr ScanWhiteSpaces();
        TokenPtr ScanCommentLine(bool scanComments);
        TokenPtr ScanCommentBlock(bool scanComments);

        TokenPtr Make(const Token::Types& type, bool takeChr = false);
        TokenPtr Make(const Token::Types& type, std::string& spell, bool takeChr = false);
        TokenPtr Make(const Token::Types& type, std::string& spell, const SourcePosition& pos, bool takeChr = false);

        inline bool Is(char chr) const
        {
            return (chr_ == chr);
        }

        inline char Chr() const
        {
            return chr_;
        }

        inline unsigned char UChr() const
        {
            return static_cast<unsigned char>(chr_);
        }

    private:

        /* === Members === */

        std::shared_ptr<SourceCode> source_;
        char                        chr_ = 0;

        Log*                        log_ = nullptr;

};


} // /namespace HTLib


#endif



// ================================================================================