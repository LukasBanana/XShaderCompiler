/*
 * SourceArea.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_SOURCE_AREA_H
#define XSC_SOURCE_AREA_H


#include "SourcePosition.h"


namespace Xsc
{


class Token;
struct AST;

// Source area structure with position and length.
struct SourceArea
{
    // Invalid source area.
    static const SourceArea ignore;

    SourceArea() = default;
    SourceArea(const SourceArea&) = default;
    SourceArea& operator = (const SourceArea&) = default;

    SourceArea(const SourcePosition& pos, unsigned int length);

    // Returns ture if this is a valid source area. False if the position is invalid or the length is 0.
    bool IsValid() const;

    // Updates the source area from the specified other area.
    void Update(const SourceArea& area);

    // Updates the source area length from the specified identifier.
    void Update(const std::string& lengthFromIdent);

    // Updates the source area from the specified token.
    void Update(const Token& tkn);

    // Updates the source area from the specified AST node.
    void Update(const AST& ast);

    // Source area start position.
    SourcePosition  pos;

    // Source area length.
    unsigned int    length  = 0;
};


} // /namespace Xsc


#endif



// ================================================================================