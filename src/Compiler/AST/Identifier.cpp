/*
 * Identifier.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "Identifier.h"
#include "Helper.h"


namespace Xsc
{


Identifier::Identifier(const std::string& original) :
    original_ { original }
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
    if (Final().compare(0, prefix.size(), prefix) == 0)
    {
        /* Remove previous prefix */
        RemovePrefix(prefix);

        /* Increase prefix counter */
        ++counter_;
        renamed_ = prefix + std::to_string(counter_) + Final();
    }
    else
    {
        /* Append prefix and reset counter */
        renamed_ = prefix + Final();
        counter_ = 0;
    }
    return *this;
}

Identifier& Identifier::RemovePrefix(const std::string& prefix)
{
    if (Final().compare(0, prefix.size(), prefix) == 0)
    {
        auto prefixLen = prefix.size();
        if (counter_ > 0)
            prefixLen += NumDigits(counter_);
        renamed_ = Final().substr(prefixLen);
    }
    return *this;
}

const std::string& Identifier::Final() const
{
    return (renamed_.empty() ? original_ : renamed_);
}


} // /namespace Xsc



// ================================================================================
