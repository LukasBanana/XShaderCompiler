/*
 * Token.h
 * 
 * This file is part of the "HLSL Translator" (Copyright (c) 2014 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __HT_TOKEN_H__
#define __HT_TOKEN_H__


#include "SourcePosition.h"

#include <string>
#include <memory>


namespace HTLib
{


//! Token classes used by the scanner and parser.
class Token
{
    
    public:
        
        //! Token type enumeration.
        enum class Types
        {
            __Unknown__,

            // Identifiers
            Ident = 0,      //!< (letter | '_') (letter | '_' | digit)*

            // Literals,
            BoolLiteral,    //!< true | false
            IntLiteral,     //!< digit+
            FloatLiteral,   //!< digit+ '.' digit+

            // Operators
            AssignOp,       //!< =, +=, -=, *=, /=, %=, <<=, >>=, |= , &=, ^=
            BinaryOp,       //!< |, ^, &, <<, >>, +, -, *, /, %, ==, !=, <, >, <=, >=
            UnaryOp,        //!< !, ~, -, ++, --

            // Punctuation
            Dot,            //!< .
            Colon,          //!< :
            Semicolon,      //!< ;
            Comma,          //!< ,

            // Brackets
            LBracket,       //!< (
            RBracket,       //!< )
            LCurly,         //!< {
            RCurly,         //!< }
            LParen,         //!< [
            RParen,         //!< ]

            // Keywords
            Void,           //!< void

            ScalarType,     //!< bool, int, uint, half, float, double
            VectorType,     //!< ScalarType ('2' | '3' | '4')
            MatrixType,     //!< ('float' | 'double') ('2' | '3' | '4') 'x' ('2' | '3' | '4')

            InputModifier,  //!< in, out, inout, uniform

            Do,             //!< do
            While,          //!< while
            For,            //!< for
            
            If,             //!< if
            Else,           //!< else
            
            Switch,         //!< switch
            Case,           //!< case
            Default,        //!< default

            Struct,         //!< struct
            Register,       //!< register

            // Object keywords
            Texture,        //!< texture, Texture1D, Texture1DArray, Texture2D, Texture2DArray, Texture3D, TextureCube, TextureCubeArray, Texture2DMS, Texture2DMSArray, Buffer
            SamplerState,   //!< SamplerState
            Buffer,         //!< cbuffer, tbuffer

            // Control transfer keywords
            CtrlTransfer,   //!< break, continue, discard
            Return,         //!< return

            // Special tokens
            Directive,      //!< '#' ... (e.g. "#include").
            EndOfStream,
        };

        Token(Token&& other);

        Token(const SourcePosition& pos, const Types type);
        Token(const SourcePosition& pos, const Types type, const std::string& spell);
        Token(const SourcePosition& pos, const Types type, std::string&& spell);

        //! Returns the token type.
        inline Types Type() const
        {
            return type_;
        }
        //! Returns the token source position.
        inline const SourcePosition& Pos() const
        {
            return pos_;
        }
        //! Returns the token spelling.
        inline const std::string& Spell() const
        {
            return spell_;
        }

    private:

        Types           type_;  //!< Type of this token.
        SourcePosition  pos_;  //!< Source area of this token.
        std::string     spell_; //!< Token spelling.

};

typedef std::shared_ptr<Token> TokenPtr;


} // /namespace HTLib


#endif



// ================================================================================