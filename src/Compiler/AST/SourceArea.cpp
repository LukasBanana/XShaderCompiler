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

void SourceArea::Update(const AST& ast)
{
    Update(ast.area);
}


} // /namespace Xsc



// ================================================================================
