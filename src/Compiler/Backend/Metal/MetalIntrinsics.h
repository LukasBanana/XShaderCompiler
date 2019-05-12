/*
 * MetalIntrinsics.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_METAL_INTRINSICS_H
#define XSC_METAL_INTRINSICS_H


#include "ASTEnums.h"
#include <string>


namespace Xsc
{


// Stores the basic information of an intrinsic from the Metal Shading Language.
struct MetalIntrinsic
{
    MetalIntrinsic() = default;
    MetalIntrinsic(const MetalIntrinsic&) = default;
    MetalIntrinsic& operator = (const MetalIntrinsic&) = default;

    inline MetalIntrinsic(const char* ident) :
        ident { ident }
    {
    }

    inline MetalIntrinsic(const char* ident, bool isTemplate) :
        ident      { ident      },
        isTemplate { isTemplate }
    {
    }

    inline operator std::string () const
    {
        return ident;
    }

    std::string ident;                  // Intrinsic identifier
    bool        isTemplate  = false;    // Specifies whether the intrinsic is a template (e.g. 'as_type<int>')
};

// Returns Metal keyword for the specified intrinsic.
const MetalIntrinsic* IntrinsicToMetalKeyword(const Intrinsic intr);


} // /namespace Xsc


#endif



// ================================================================================
