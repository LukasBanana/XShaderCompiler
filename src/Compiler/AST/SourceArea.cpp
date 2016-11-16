/*
 * SourceArea.cpp
 * 
 * This file is part of the "XieXie 2.0 Project" (Copyright (c) 2014 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "SourceArea.h"
#include "Token.h"
#include "AST.h"
#include <algorithm>


namespace Xsc
{


const SourceArea SourceArea::ignore {};

SourceArea::SourceArea(const SourcePosition& pos, unsigned int length, unsigned int offset) :
    pos_    { pos    },
    length_ { length },
    offset_ { offset }
{
}

bool SourceArea::IsValid() const
{
    return (pos_.IsValid() && length_ > 0);
}

void SourceArea::Update(const SourceArea& area)
{
    if (area.pos_.Row() > pos_.Row())
        length_ = ~0;
    else if (area.pos_.Row() == pos_.Row() && area.pos_.Column() + area.length_ > pos_.Column() + length_)
        length_ = (area.pos_.Column() - pos_.Column() + area.length_);
}

void SourceArea::Update(const std::string& lengthFromIdent)
{
    length_ = std::max(length_, static_cast<unsigned int>(lengthFromIdent.size()));
}

void SourceArea::Update(const Token& tkn)
{
    Update(tkn.Area());
}

void SourceArea::Update(const AST& ast)
{
    Update(ast.area);
}

void SourceArea::Offset(unsigned int offset)
{
    offset_ = offset;
}

void SourceArea::Offset(const SourcePosition& pos)
{
    if (pos.Row() == pos_.Row() && pos.Column() >= pos_.Column())
        offset_ = (pos.Column() - pos_.Column());
    else
        offset_ = ~0;
}

unsigned int SourceArea::Offset() const
{
    return (length_ > 0 ? std::min(offset_, length_ - 1) : 0);
}


} // /namespace Xsc



// ================================================================================
