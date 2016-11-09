/*
 * Token.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "Token.h"


namespace Xsc
{


Token::Token(Token&& other) :
    type_   { other.type_             },
    pos_    { other.pos_              },
    spell_  { std::move(other.spell_) }
{
}

Token::Token(const SourcePosition& pos, const Types type) :
    type_   { type },
    pos_    { pos  }
{
}

Token::Token(const SourcePosition& pos, const Types type, const std::string& spell) :
    type_   { type  },
    pos_    { pos   },
    spell_  { spell }
{
}

Token::Token(const SourcePosition& pos, const Types type, std::string&& spell) :
    type_   { type             },
    pos_    { pos              },
    spell_  { std::move(spell) }
{
}

SourceArea Token::Area() const
{
    /* Initialize source area by token position and length of spelling */
    SourceArea area;
    
    area.pos    = Pos();
    area.length = Spell().size();

    /* Handle special cases */
    switch (Type())
    {
        case Types::StringLiteral:
            area.length += 2;
            break;
    }

    return area;
}

std::string Token::TypeToString(const Types type)
{
    switch (type)
    {
        case Types::Ident:              return "identifier";
        case Types::BoolLiteral:        return "boolean literal";
        case Types::IntLiteral:         return "integer literal";
        case Types::FloatLiteral:       return "floating-point literal";
        case Types::StringLiteral:      return "string literal";
        case Types::AssignOp:           return "assign operator";
        case Types::BinaryOp:           return "binary operator";
        case Types::UnaryOp:            return "unary operator";
        case Types::TernaryOp:          return "ternary operator";
        case Types::Dot:                return "'.'";
        case Types::Colon:              return "':'";
        case Types::Semicolon:          return "';'";
        case Types::Comma:              return "','";
        case Types::LBracket:           return "'('";
        case Types::RBracket:           return "')'";
        case Types::LCurly:             return "'{'";
        case Types::RCurly:             return "'}'";
        case Types::LParen:             return "'['";
        case Types::RParen:             return "']'";
        case Types::Void:               return "'void'";
        case Types::ScalarType:         return "scalar type";
        case Types::VectorType:         return "vector type";
        case Types::MatrixType:         return "matrix type";
        case Types::Do:                 return "'do'";
        case Types::While:              return "'while'";
        case Types::For:                return "'for'";
        case Types::If:                 return "'if'";
        case Types::Else:               return "'else'";
        case Types::Switch:             return "'switch'";
        case Types::Case:               return "'case'";
        case Types::Default:            return "'default'";
        case Types::Struct:             return "'struct'";
        case Types::Register:           return "'register'";
        case Types::PackOffset:         return "'packoffset'";
        case Types::Sampler:            return "sampler state";
        case Types::Texture:            return "texture type";
        case Types::StorageBuffer:      return "read/write buffer";
        case Types::UniformBuffer:      return "constant buffer";
        case Types::CtrlTransfer:       return "control transfer keyword";
        case Types::Return:             return "'return'";
        case Types::InputModifier:      return "input modifier";
        case Types::StorageModifier:    return "storage modifier";
        case Types::TypeModifier:       return "type modifier";
        case Types::Directive:          return "preprocessor directive";
        case Types::Comment:            return "comment";
        case Types::WhiteSpaces:        return "white spaces";
        case Types::NewLines:           return "new-line characters";
        case Types::LineBreak:          return "preprocessor line break";
        case Types::Misc:               return "miscellaneous";
        case Types::EndOfStream:        return "end-of-stream";
        default:                        break;
    }
    return "";
}


} // /namespace Xsc



// ================================================================================
