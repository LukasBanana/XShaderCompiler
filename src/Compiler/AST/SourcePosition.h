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
        
        // Invalid source position.
        static const SourcePosition ignore;

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

    private:
        
        unsigned int row_    = 0,
                     column_ = 0;

};


} // /namespace Xsc


#endif



// ================================================================================