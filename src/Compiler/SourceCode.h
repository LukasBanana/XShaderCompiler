/*
 * SourceCode.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_SOURCE_CODE_H
#define XSC_SOURCE_CODE_H


#include "SourceArea.h"

#include <istream>
#include <string>
#include <memory>
#include <vector>


namespace Xsc
{


// Source code stream class.
class SourceCode
{

    public:

        SourceCode(const std::shared_ptr<std::istream>& stream);

        // Returns true if this is a valid source code stream.
        bool IsValid() const;

        // Returns the next character from the source.
        char Next();

        // Fetches the line with the marker string of the specified source position.
        bool FetchLineMarker(const SourceArea& area, std::string& line, std::string& marker);

        // Sets the new source origin for the current source position (see "Pos()").
        void NextSourceOrigin(const std::string& filename, int lineOffset);

        // Ignores the current character.
        inline void Ignore()
        {
            Next();
        }

        // Returns the current source position.
        inline const SourcePosition& Pos() const
        {
            return pos_;
        }

        // Returns the current source line.
        inline const std::string& Line() const
        {
            return currentLine_;
        }

        // Returns the filename of the current source position (see SourcePosition::GetOrigin).
        std::string Filename() const;

    protected:

        SourceCode() = default;

        // Returns the line (if it has already been read) by the zero-based line index.
        std::string GetLine(std::size_t lineIndex) const;

        std::shared_ptr<std::istream>   stream_;
        std::string                     currentLine_;
        std::vector<std::string>        lines_;
        SourcePosition                  pos_;

};

using SourceCodePtr = std::shared_ptr<SourceCode>;


} // /namespace Xsc


#endif



// ================================================================================