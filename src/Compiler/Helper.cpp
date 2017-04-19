/*
 * Helper.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "Helper.h"
#include <algorithm>


namespace Xsc
{


void Replace(std::string& s, const std::string& from, const std::string& to)
{
    for (std::size_t pos = 0; (pos = s.find(from, pos)) != std::string::npos; pos += to.size())
        s.replace(pos, from.size(), to);
}


} // /namespace Xsc



// ================================================================================