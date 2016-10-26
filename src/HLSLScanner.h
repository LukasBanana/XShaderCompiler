/*
 * HLSLScanner.h
 * 
 * This file is part of the "HLSL Translator" (Copyright (c) 2014 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef HTLIB_HLSL_SCANNER_H
#define HTLIB_HLSL_SCANNER_H


#include "HT/Translator.h"
#include "HT/Logger.h"
#include "SourceCode.h"
#include "SourcePosition.h"
#include "Token.h"

#include <string>
#include <functional>


namespace HTLib
{


//! This class stores the position in a source code file.
class HLSLScanner
{
    
    public:
        
        HLSLScanner(Logger* log = nullptr);

        bool ScanSource(const std::shared_ptr<SourceCode>& source);

        //! Scanns the next token.
        TokenPtr Next(bool scanComments = false);

        SourcePosition Pos() const;

        inline SourceCode* Source() const
        {
            return source_.get();
        }

    private:
        
        /* === Functions === */

        char Take(char chr);
        char TakeIt();

        void Error(const std::string& msg);
        void ErrorUnexpected();
        void ErrorUnexpected(char expectedChar);
        void ErrorEOF();
        void ErrorLetterInNumber();

        //! Ignores all characters which comply the specified predicate.
        void Ignore(const std::function<bool (char)>& pred);

        void IgnoreWhiteSpaces();

        TokenPtr ScanCommentLine(bool scanComments);
        TokenPtr ScanCommentBlock(bool scanComments);

        TokenPtr Make(const Token::Types& type, bool takeChr = false);
        TokenPtr Make(const Token::Types& type, std::string& spell, bool takeChr = false);
        TokenPtr Make(const Token::Types& type, std::string& spell, const SourcePosition& pos, bool takeChr = false);

        TokenPtr ScanToken();

        TokenPtr ScanDirective();
        TokenPtr ScanIdentifier();
        TokenPtr ScanAssignShiftRelationOp(const char Chr);
        TokenPtr ScanPlusOp();
        TokenPtr ScanMinusOp();
        TokenPtr ScanNumber();

        void ScanDecimalLiteral(std::string& spell);

        inline bool Is(char chr) const
        {
            return chr_ == chr;
        }

        inline unsigned char UChr() const
        {
            return static_cast<unsigned char>(chr_);
        }

        /* === Members === */

        std::shared_ptr<SourceCode> source_;
        char                        chr_ = 0;

        Logger*                     log_ = nullptr;

};


} // /namespace HTLib


#endif



// ================================================================================