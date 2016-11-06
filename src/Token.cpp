/*
 * Token.cpp
 * 
 * This file is part of the "HLSL Translator" (Copyright (c) 2014 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "Token.h"


namespace HTLib
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
    SourceArea area;
    area.pos    = Pos();
    area.length = Spell().size();
    return area;
}


} // /namespace HTLib



// ================================================================================