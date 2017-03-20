/*
 * Token.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "Token.h"
#include "ReportIdents.h"

#ifdef XSC_ENABLE_MEMORY_POOL
#include "MemoryPool.h"
#endif


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
        case Types::Ident:              return R_Identifier;
        case Types::BoolLiteral:        return R_BoolLiteral;
        case Types::IntLiteral:         return R_IntLiteral;
        case Types::FloatLiteral:       return R_FloatLiteral;
        case Types::StringLiteral:      return R_StringLiteral;
        case Types::CharLiteral:        return R_CharLiteral;
        case Types::NullLiteral:        return R_NullLiteral;
        case Types::AssignOp:           return R_AssignOp;
        case Types::BinaryOp:           return R_BinaryOp;
        case Types::UnaryOp:            return R_UnaryOp;
        case Types::TernaryOp:          return R_TernaryOp;
        case Types::Dot:                return "'.'";
        case Types::Colon:              return "':'";
        case Types::DColon:             return "'::'";
        case Types::Semicolon:          return "';'";
        case Types::Comma:              return "','";
        case Types::LBracket:           return "'('";
        case Types::RBracket:           return "')'";
        case Types::LCurly:             return "'{'";
        case Types::RCurly:             return "'}'";
        case Types::LParen:             return "'['";
        case Types::RParen:             return "']'";
        case Types::StringType:         return R_StringTypeDen;
        case Types::ScalarType:         return R_ScalarTypeDen;
        case Types::VectorType:         return R_VectorTypeDen;
        case Types::MatrixType:         return R_MatrixTypeDen;
        case Types::Void:               return R_VoidTypeDen;
        case Types::PrimitiveType:      return R_PrimitiveTypeDen;
        case Types::Reserved:           return R_ReservedWord;
        case Types::Vector:             return R_VectorGenericTypeDen;
        case Types::Matrix:             return R_MatrixGenericTypeDen;
        case Types::Sampler:            return R_SamplerTypeDen;
        case Types::SamplerState:       return R_SamplerState;
        case Types::Buffer:             return R_BufferTypeDen;
        case Types::UniformBuffer:      return R_UniformBufferTypeDen;
        case Types::Do:                 return R_KeywordDo;
        case Types::While:              return R_KeywordWhile;
        case Types::For:                return R_KeywordFor;
        case Types::If:                 return R_KeywordIf;
        case Types::Else:               return R_KeywordElse;
        case Types::Switch:             return R_KeywordSwitch;
        case Types::Case:               return R_KeywordCase;
        case Types::Default:            return R_KeywordDefault;
        case Types::Typedef:            return R_KeywordTypedef;
        case Types::Struct:             return R_KeywordStruct;
        case Types::Register:           return R_KeywordRegister;
        case Types::PackOffset:         return R_KeywordPackOffset;
        case Types::CtrlTransfer:       return R_CtrlTransfer;
        case Types::Return:             return R_KeywordReturn;
        case Types::InputModifier:      return R_InputModifier;
        case Types::InterpModifier:     return R_InterpModifier;
        case Types::TypeModifier:       return R_TypeModifier;
        case Types::StorageClass:       return R_StorageClass;
        case Types::Inline:             return R_KeywordInline;
        case Types::Technique:          return R_KeywordTechnique;
        case Types::Pass:               return R_KeywordPass;
        case Types::Compile:            return R_KeywordCompile;
        case Types::Directive:          return R_PPDirective;
        case Types::DirectiveConcat:    return R_PPDirectiveConcat;
        case Types::Comment:            return R_Comment;
        case Types::WhiteSpaces:        return R_WhiteSpaces;
        case Types::NewLines:           return R_NewLineChars;
        case Types::LineBreak:          return R_PPLineBreak;
        case Types::VarArg:             return R_VarArgSpecifier;
        case Types::Misc:               return R_Misc;
        case Types::EndOfStream:        return R_EndOfStream;
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

#ifdef XSC_ENABLE_MEMORY_POOL

void* Token::operator new (std::size_t count)
{
    return MemoryPool::Instance().Alloc(count);
}

void Token::operator delete (void* ptr)
{
    MemoryPool::Instance().Free(ptr);
}

#endif


} // /namespace Xsc



// ================================================================================
