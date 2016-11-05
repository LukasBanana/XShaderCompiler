/*
 * SourceCode.h
 * 
 * This file is part of the "HLSL Translator" (Copyright (c) 2014 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef HTLIB_SOURCE_CODE_H
#define HTLIB_SOURCE_CODE_H


#include "SourcePosition.h"

#include <istream>
#include <string>
#include <memory>


namespace HTLib
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
            return line_;
        }

    protected:
        
        SourceCode() = default;

        std::shared_ptr<std::istream>   stream_;
        std::string                     line_;
        SourcePosition                  pos_;

};


} // /namespace HTLib


#endif



// ================================================================================