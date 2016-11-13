/*
 * Helper.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_HELPER_H
#define XSC_HELPER_H


#include <string>
#include <sstream>
#include <cmath>
#include <type_traits>


namespace Xsc
{


// Converts the specified string into a value from type T.
template <typename T>
T FromString(const std::string& s)
{
    T value = T(0);
    std::stringstream stream;
    stream << s;
    stream >> value;
    return value;
}


} // /namespace Xsc


#endif



// ================================================================================