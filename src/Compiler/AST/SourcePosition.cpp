/*
 * SourcePosition.cpp
 * 
 * This file is part of the "XieXie 2.0 Project" (Copyright (c) 2014 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "SourcePosition.h"
#include "Token.h"
#include <algorithm>


namespace Xsc
{


/*
 * SourcePosition class
 */

const SourcePosition SourcePosition::ignore {};

SourcePosition::SourcePosition(unsigned int row, unsigned int column) :
    row_   { row    },
    column_{ column }
{
}

std::string SourcePosition::ToString() const
{
    return (std::to_string(row_) + ":" + std::to_string(column_ > 0 ? column_ - 1 : 0));
}

void SourcePosition::IncRow()
{
    ++row_;
    column_ = 0;
}

void SourcePosition::IncColumn()
{
    ++column_;
}

bool SourcePosition::IsValid() const
{
    return (row_ > 0 && column_ > 0);
}

void SourcePosition::Reset()
{
    row_ = column_ = 0;
}


/*
 * SourceArea structure
 */

const SourceArea SourceArea::ignore {};

SourceArea::SourceArea(const SourcePosition& pos, unsigned int length) :
    pos     { pos    },
    length  { length }
{
}

bool SourceArea::IsValid() const
{
    return (pos.IsValid() && length > 0);
}

void SourceArea::Update(const SourceArea& area)
{
    if (area.pos.Row() > pos.Row())
        length = ~0;
    else if (area.pos.Row() == pos.Row() && area.pos.Column() > pos.Column() + length)
        length = (area.pos.Column() - pos.Column() + area.length);
}

void SourceArea::Update(const std::string& lengthFromIdent)
{
    length = std::max(length, static_cast<unsigned int>(lengthFromIdent.size()));
}

void SourceArea::Update(const Token& tkn)
{
    Update(tkn.Area());
}


} // /namespace Xsc



// ================================================================================
