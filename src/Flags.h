/*
 * Flags.h
 * 
 * This file is part of the "HLSL Translator" (Copyright (c) 2014 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_FLAGS_H
#define XSC_FLAGS_H


namespace Xsc
{


// Common flags class.
class Flags
{
    
    public:
        
        Flags() = default;

        inline Flags(unsigned int flags) :
            bitMask_{ flags }
        {
        }

        inline Flags& operator << (unsigned int flag)
        {
            bitMask_ |= flag;
            return *this;
        }

        inline bool operator () (unsigned int flag) const
        {
            return ((bitMask_ & flag) != 0);
        }

        inline operator unsigned int () const
        {
            return bitMask_;
        }

    private:
        
        unsigned int bitMask_ = 0;

};


} // /namespace Xsc


#endif



// ================================================================================