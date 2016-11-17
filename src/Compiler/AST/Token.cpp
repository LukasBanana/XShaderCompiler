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
    return
    {
        Pos(),
        static_cast<unsigned int>(Spell().size())
    };
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
        case Types::StringType:         return "string type denoter";
        case Types::ScalarType:         return "scalar type denoter";
        case Types::VectorType:         return "vector type denoter";
        case Types::MatrixType:         return "matrix type denoter";
        case Types::Void:               return "'void' type denoter";
        case Types::Reserved:           return "reserved keyword";
        case Types::Vector:             return "'vector' generic type denoter";
        case Types::Matrix:             return "'matrix' generic type denoter";
        case Types::Sampler:            return "sampler state type denoter";
        case Types::Texture:            return "texture type denoter";
        case Types::StorageBuffer:      return "storage buffer type denoter";
        case Types::UniformBuffer:      return "uniform buffer type denoter";
        case Types::Do:                 return "'do' keyword";
        case Types::While:              return "'while' keyword";
        case Types::For:                return "'for' keyword";
        case Types::If:                 return "'if' keyword";
        case Types::Else:               return "'else' keyword";
        case Types::Switch:             return "'switch' keyword";
        case Types::Case:               return "'case' keyword";
        case Types::Default:            return "'default' keyword";
        case Types::Typedef:            return "'typedef' keyword";
        case Types::Struct:             return "'struct' keyword";
        case Types::Register:           return "'register' keyword";
        case Types::PackOffset:         return "'packoffset' keyword";
        case Types::CtrlTransfer:       return "control transfer";
        case Types::Return:             return "'return' keyword";
        case Types::InputModifier:      return "input modifier";
        case Types::StorageModifier:    return "storage modifier";
        case Types::TypeModifier:       return "type modifier";
        case Types::Directive:          return "preprocessor directive";
        case Types::DirectiveConcat:    return "preprocessor directive concatenation";
        case Types::Comment:            return "comment";
        case Types::WhiteSpaces:        return "white spaces";
        case Types::NewLines:           return "new-line characters";
        case Types::LineBreak:          return "preprocessor line break";
        case Types::VarArg:             return "variadic argument specifier";
        case Types::Misc:               return "miscellaneous";
        case Types::EndOfStream:        return "end-of-stream";
        default:                        break;
    }
    return "";
}

std::string Token::SpellContent() const
{
    if (Type() == Types::StringLiteral && Spell().size() >= 2)
        return Spell().substr(1, Spell().size() - 2);
    else
        return Spell();
}


} // /namespace Xsc



// ================================================================================
