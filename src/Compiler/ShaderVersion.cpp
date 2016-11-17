/*
 * ShaderVersion.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "ShaderVersion.h"


namespace Xsc
{


ShaderVersion::ShaderVersion(int major, int minor) :
    major_{ major },
    minor_{ minor }
{
}

std::string ShaderVersion::ToString() const
{
    return (std::to_string(major_) + "." + std::to_string(minor_));
}


bool operator == (const ShaderVersion& lhs, const ShaderVersion& rhs)
{
    return (lhs.Major() == rhs.Major() && lhs.Minor() == rhs.Minor());
}

bool operator != (const ShaderVersion& lhs, const ShaderVersion& rhs)
{
    return !(lhs == rhs);
}

bool operator < (const ShaderVersion& lhs, const ShaderVersion& rhs)
{
    if (lhs.Major() == rhs.Major())
        return (lhs.Minor() < rhs.Minor());
    return (lhs.Major() < rhs.Major());
}

bool operator <= (const ShaderVersion& lhs, const ShaderVersion& rhs)
{
    return (lhs == rhs || lhs < rhs);
}

bool operator > (const ShaderVersion& lhs, const ShaderVersion& rhs)
{
    return (rhs < lhs);
}

bool operator >= (const ShaderVersion& lhs, const ShaderVersion& rhs)
{
    return (lhs == rhs || rhs < lhs);
}


} // /namespace Xsc



// ================================================================================