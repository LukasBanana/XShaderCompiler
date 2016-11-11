/*
 * SourcePosition.cpp
 * 
 * This file is part of the "XieXie 2.0 Project" (Copyright (c) 2014 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "SourcePosition.h"


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

SourceArea SourceArea::IncLength(const SourceArea& other) const
{
    SourceArea area;
    
    area.pos = pos;

    if (other.pos.Row() > pos.Row())
        area.length = ~0;
    else if (other.pos.Row() == pos.Row() && other.pos.Column() > pos.Column() + length)
        area.length = (other.pos.Column() - pos.Column() + other.length);

    return area;
}


} // /namespace Xsc



// ================================================================================
