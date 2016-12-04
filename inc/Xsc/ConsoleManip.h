/*
 * ConsoleManip.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_CONSOLE_MANIP_H
#define XSC_CONSOLE_MANIP_H


#include "Export.h"
#include <ostream>


namespace Xsc
{

namespace ConsoleManip
{


//! Output stream color flags enumeration.
struct ColorFlags
{
    enum
    {
        Red     = (1 << 0),                 //!< Red color flag.
        Green   = (1 << 1),                 //!< Green color flag.
        Blue    = (1 << 2),                 //!< Blue color flag.

        Intens  = (1 << 3),                 //!< Intensity color flag.

        Black   = 0,                        //!< Black color flag.
        Gray    = (Red | Green | Blue),     //!< Gray color flag (Red | Green | Blue).
        White   = (Gray | Intens),          //!< White color flag (Gray | Intens).

        Yellow  = (Red | Green | Intens),   //!< Yellow color flag (Red | Green | Intens).
        Pink    = (Red | Blue | Intens),    //!< Pink color flag (Red | Blue | Intens).
        Cyan    = (Green | Blue | Intens),  //!< Cyan color flag (Green | Blue | Intens).
    };
};


//! Enables or disables console manipulation. By default enabled.
void XSC_EXPORT Enable(bool enable);

//! Returns true if console manipulation is enabled.
bool XSC_EXPORT IsEnabled();

//! Push the specified front color onto the stack.
void XSC_EXPORT PushColor(std::ostream& stream, long front);

//! Push the specified front and back color onto the stack.
void XSC_EXPORT PushColor(std::ostream& stream, long front, long back);

//! Pops the previous front and back colors from the stack.
void XSC_EXPORT PopColor(std::ostream& stream);


//! Helper class for scoped color stack operations.
class ScopedColor
{
    
    public:
    
        /**
        \brief Constructor with output stream and front color flags.
        \param[in,out] stream Specifies the output stream for which the scope is to be changed. This is only used for Unix systems.
        \param[in] front Specifies the front color flags. This can be a bitwise OR combination of the entries of the ColorFlags enumeration.
        \see ColorFlags
        \see PushColor(std::ostream&, long)
        */
        inline ScopedColor(std::ostream& stream, long front) :
            stream_{ stream }
        {
            PushColor(stream_, front);
        }

        /**
        \brief Constructor with output stream, and front- and back color flags.
        \param[in,out] stream Specifies the output stream for which the scope is to be changed. This is only used for Unix systems.
        \param[in] front Specifies the front color flags. This can be a bitwise OR combination of the entries of the ColorFlags enumeration.
        \param[in] back Specifies the back color flags. This can be a bitwise OR combination of the entries of the ColorFlags enumeration.
        \see ColorFlags
        \see PushColor(std::ostream&, long, long)
        */
        inline ScopedColor(std::ostream& stream, long front, long back) :
            stream_{ stream }
        {
            PushColor(stream_, front, back);
        }

        /**
        \brief Destructor which will reset the previous color from the output stream.
        \see PopColor
        */
        inline ~ScopedColor()
        {
            PopColor(stream_);
        }
        
    private:
        
        std::ostream& stream_;
        
};


} // /namespace ConsoleManip

} // /namespace Xsc


#endif



// ================================================================================