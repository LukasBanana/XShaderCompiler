/*
 * Helper.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "Helper.h"
#include <algorithm>


namespace Xsc
{


std::string ToLower(const std::string& s)
{
    std::string t;
    t.resize(s.size());
    std::transform(s.begin(), s.end(), t.begin(), ::tolower);
    return t;
}

std::string ToUpper(const std::string& s)
{
    std::string t;
    t.resize(s.size());
    std::transform(s.begin(), s.end(), t.begin(), ::toupper);
    return t;
}


} // /namespace Xsc



// ================================================================================