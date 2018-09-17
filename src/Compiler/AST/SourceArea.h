/*
 * SourceArea.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
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
class SourceArea
{

    public:

        // Invalid source area.
        static const SourceArea ignore;

        SourceArea() = default;
        SourceArea(const SourceArea&) = default;
        SourceArea& operator = (const SourceArea&) = default;

        SourceArea(const SourcePosition& pos, unsigned int length, unsigned int offset = 0);

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

        // Sets the new offset of the marker pointer.
        void Offset(unsigned int offset);

        // Sets the new offset of the marker pointer by a source position.
        void Offset(const SourcePosition& pos);

        // Returns the offset of the marker pointer (e.g. "^~~~") clamped to the range [0, length).
        unsigned int Offset() const;

        // Returns the start position of the source area.
        inline const SourcePosition& Pos() const
        {
            return pos_;
        }

        // Returns the length of the source area.
        inline unsigned int Length() const
        {
            return length_;
        }

    private:

        SourcePosition  pos_;
        unsigned int    length_ = 0;
        unsigned int    offset_ = 0;

};


} // /namespace Xsc


#endif



// ================================================================================