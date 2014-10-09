/*
 * HLSLScanner.h
 * 
 * This file is part of the "HLSL Translator" (Copyright (c) 2014 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __HT_SCANNER_H__
#define __HT_SCANNER_H__


#include "SourceCode.h"
#include "SourcePosition.h"
#include "Token.h"
#include "Logger.h"

#include <string>
#include <functional>


using namespace HLSLTrans;

//! This class stores the position in a source code file.
class HLSLScanner
{
    
    public:
        
        HLSLScanner(Logger* log = nullptr);

        bool ScanSource(const std::shared_ptr<SourceCode>& source);

        /**
        Scanns the next token with the current scanner state.
        \see state
        */
        TokenPtr Next();

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
        void IgnoreCommentLine();
        void IgnoreCommentBlock();

        TokenPtr Make(const Token::Types& type, bool takeChr = false);
        TokenPtr Make(const Token::Types& type, std::string& spell, bool takeChr = false);
        TokenPtr Make(const Token::Types& type, std::string& spell, const SourcePosition& pos, bool takeChr = false);

        TokenPtr ScanToken();

        TokenPtr ScanIdentifier();
        TokenPtr ScanAssignShiftRelationOp(const char Chr);
        TokenPtr ScanPlusOp();
        TokenPtr ScanMinusOp();
        TokenPtr ScanNumber();
        TokenPtr ScanHexNumber();
        TokenPtr ScanOctNumber();
        TokenPtr ScanBinNumber();

        void ScanDecimalLiteral(std::string& spell);

        bool IsEscapeChar() const;

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

        char chr_ = 0;

        Logger* log_ = nullptr;

};


#endif



// ================================================================================