/*
 * SourcePosition.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_SOURCE_POSITION_H
#define XSC_SOURCE_POSITION_H


#include <string>
#include <memory>


namespace Xsc
{


/*
Source code origin with filename and line offset.
This is used to track the filename and correct source position line for each AST within a pre-processed source code.
This is necessary because the pre-processsor eliminates all include directives.
*/
struct SourceOrigin
{
    std::string filename;
    int         lineOffset;
};

using SourceOriginPtr = std::shared_ptr<SourceOrigin>;


// This class stores the position in a source code file.
class SourcePosition
{
    
    public:
        
        // Invalid source position.
        static const SourcePosition ignore;

        SourcePosition() = default;
        SourcePosition(unsigned int row, unsigned int column, const SourceOriginPtr& origin = nullptr);

        // Returns the source position as string in the format "Row:Column", e.g. "75:10".
        std::string ToString(bool printFilename = true) const;

        // Increases the row by 1 and sets the column to 0.
        void IncRow();

        // Increases the column by 1.
        void IncColumn();

        // Returns ture if this is a valid source position. False if row and column are 0.
        bool IsValid() const;

        // Reste the source position to (0:0).
        void Reset();

        // Makes a strict-weak-order comparison between the two source positions.
        bool operator < (const SourcePosition& rhs) const;

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

        // Sets the new source origin.
        inline void SetOrigin(const SourceOriginPtr& origin)
        {
            origin_ = origin;
        }

    private:
        
        unsigned int    row_    = 0,
                        column_ = 0;

        SourceOriginPtr origin_;

};


} // /namespace Xsc


#endif



// ================================================================================