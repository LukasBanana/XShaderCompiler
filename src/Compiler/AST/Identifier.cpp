/*
 * Identifier.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "Identifier.h"


namespace Xsc
{


Identifier::Identifier(const std::string& original) :
    original_{ original }
{
}

Identifier& Identifier::operator = (const Identifier& rhs)
{
    *this = rhs.Final();
    return *this;
}

Identifier& Identifier::operator = (const std::string& s)
{
    if (original_.empty())
        original_ = s;
    else
        renamed_ = s;
    return *this;
}

Identifier& Identifier::AppendPrefix(const std::string& prefix)
{
    if (Final().compare(0, prefix.size(), prefix) != 0)
        renamed_ = prefix + Final();
    return *this;
}

const std::string& Identifier::Final() const
{
    return (renamed_.empty() ? original_ : renamed_);
}


} // /namespace Xsc



// ================================================================================
