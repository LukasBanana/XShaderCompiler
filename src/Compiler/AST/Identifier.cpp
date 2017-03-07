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
    renamed_ = rhs.Final();
    return *this;
}

Identifier& Identifier::operator = (const std::string& s)
{
    renamed_ = s;
    return *this;
}

Identifier& Identifier::AppendPrefix(const std::string& prefix)
{
    renamed_ = prefix + Final();
    return *this;
}

const std::string& Identifier::Final() const
{
    return (renamed_.empty() ? original_ : renamed_);
}


} // /namespace Xsc



// ================================================================================
