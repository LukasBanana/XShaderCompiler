/*
 * SourceCode.h
 * 
 * This file is part of the "HLSL Translator" (Copyright (c) 2014 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __HT_SOURCE_CODE_H__
#define __HT_SOURCE_CODE_H__


#include "SourcePosition.h"

#include <istream>
#include <string>


namespace HTLib
{


class SourceCode
{
    
    public:
        
        SourceCode(std::istream& stream);

        //! Returns the next character from the source.
        char Next();

        //! Ignores the current character.
        inline void Ignore()
        {
            Next();
        }

        //! Returns the current source position.
        inline const SourcePosition& Pos() const
        {
            return pos_;
        }
        //! Returns the current source line.
        inline const std::string& Line() const
        {
            return line_;
        }

    protected:
        
        SourceCode() = default;

        std::istream*   stream_ = nullptr;
        std::string     line_;
        SourcePosition  pos_;

};


} // /namespace HTLib


#endif



// ================================================================================