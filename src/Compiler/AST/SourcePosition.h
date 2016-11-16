/*
 * SourcePosition.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_SOURCE_POSITION_H
#define XSC_SOURCE_POSITION_H


#include <string>


namespace Xsc
{


// This class stores the position in a source code file.
class SourcePosition
{
    
    public:
        
        SourcePosition() = default;
        SourcePosition(unsigned int row, unsigned int column);

        // Returns the source position as string in the format "Row:Column", e.g. "75:10".
        std::string ToString() const;

        // Increases the row by 1 and sets the column to 0.
        void IncRow();

        // Increases the column by 1.
        void IncColumn();

        // Returns ture if this is a valid source position. False if row and column are 0.
        bool IsValid() const;

        // Reste the source position to (0:0).
        void Reset();

        // Returns the row of the source position, beginning with 1.
        inline unsigned int Row() const
        {
            return row_;
        }

        // Returns the colummn of the source position, beginning with 1.
        inline unsigned int Column() const
        {
            return column_;
        }

        // Invalid source position.
        static const SourcePosition ignore;

    private:
        
        unsigned int    row_    = 0,
                        column_ = 0;

};


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