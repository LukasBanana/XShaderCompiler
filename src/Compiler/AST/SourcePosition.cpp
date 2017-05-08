/*
 * SourcePosition.cpp
 * 
 * This file is part of the "XieXie 2.0 Project" (Copyright (c) 2014 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "SourcePosition.h"


namespace Xsc
{


const SourcePosition SourcePosition::ignore {};

SourcePosition::SourcePosition(unsigned int row, unsigned int column, const SourceOriginPtr& origin) :
    row_    { row    },
    column_ { column },
    origin_ { origin }
{
}

std::string SourcePosition::ToString(bool printFilename) const
{
    std::string s;

    auto r = row_;
    auto c = column_;

    if (origin_)
    {
        if (printFilename && !origin_->filename.empty())
        {
            s += origin_->filename;
            s += ':';
        }
        s += std::to_string(static_cast<int>(r) + origin_->lineOffset);
    }
    else
        s += std::to_string(r);

    s += ':';
    s += std::to_string(c);

    return s;
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

bool SourcePosition::operator < (const SourcePosition& rhs) const
{
    if (origin_.get() < rhs.origin_.get())
        return true;
    else if (origin_.get() > rhs.origin_.get())
        return false;

    if (row_ < rhs.row_)
        return true;
    if (row_ > rhs.row_)
        return false;

    return (column_ < rhs.column_);
}


} // /namespace Xsc



// ================================================================================
