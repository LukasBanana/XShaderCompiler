/*
 * ConsoleManip.h
 * 
 * This file is part of the "HLSL Translator" (Copyright (c) 2014 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef HTLIB_CONSOLE_MANIP_H
#define HTLIB_CONSOLE_MANIP_H


#include "Export.h"
#include <ostream>


namespace HTLib
{

namespace ConsoleManip
{


//! Console color enumeration.
struct ColorFlags
{
    enum
    {
        Red     = (1 << 0),
        Green   = (1 << 1),
        Blue    = (1 << 2),

        Intens  = (1 << 3),

        Black   = 0,
        Gray    = (Red | Green | Blue),
        White   = (Gray | Intens),

        Yellow  = (Red | Green | Intens),
        Pink    = (Red | Blue | Intens),
        Cyan    = (Green | Blue | Intens),
    };
};


//! Enables or disables console manipulation. By default enabled.
void HTLIB_EXPORT Enable(bool enable);

//! Returns true if console manipulation is enabled.
bool HTLIB_EXPORT IsEnabled();

//! Push the specified front color onto the stack.
void HTLIB_EXPORT PushColor(std::ostream& stream, long front);

//! Push the specified front and back color onto the stack.
void HTLIB_EXPORT PushColor(std::ostream& stream, long front, long back);

//! Pops the previous front and back colors from the stack.
void HTLIB_EXPORT PopColor(std::ostream& stream);


//! Helper class for scoped color stack operations.
class ScopedColor
{
    
    public:
    
        inline ScopedColor(std::ostream& stream, long front) :
            stream_{ stream }
        {
            PushColor(stream_, front);
        }

        inline ScopedColor(std::ostream& stream, long front, long back) :
            stream_{ stream }
        {
            PushColor(stream_, front, back);
        }

        inline ~ScopedColor()
        {
            PopColor(stream_);
        }
        
    private:
        
        std::ostream& stream_;
        
};


} // /namespace ConsoleManip

} // /namespace HTLib


#endif



// ================================================================================