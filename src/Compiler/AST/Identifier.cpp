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


Identifier& Identifier::operator = (const Identifier& rhs)
{
    *this = rhs.Final();
    return *this;
}

Identifier& Identifier::operator = (const std::string& s)
{
    if (!originalSet_)
    {
        /* Set original identifier for the first time */
        originalSet_ = true;
        original_ = s;
    }
    else
    {
        /* Set renamed identifier */
        renamedSet_ = true;
        renamed_ = s;
    }
    return *this;
}

Identifier& Identifier::AppendPrefix(const std::string& prefix)
{
    if (!prefix.empty() && Final().compare(0, prefix.size(), prefix) == 0)
    {
        /* Remove previous prefix */
        RemovePrefix(prefix);

        /* Increase prefix counter */
        ++counter_;
        return (*this = prefix + std::to_string(counter_) + Final());
    }
    else
    {
        /* Append prefix and reset counter */
        counter_ = 0;
        return (*this = prefix + Final());
    }
}

Identifier& Identifier::RemovePrefix(const std::string& prefix)
{
    if (!prefix.empty() && Final().compare(0, prefix.size(), prefix) == 0)
    {
        auto prefixLen = prefix.size();
        if (counter_ > 0)
            prefixLen += NumDigits(counter_);
        *this = Final().substr(prefixLen);
    }
    return *this;
}

const std::string& Identifier::Final() const
{
    return (renamedSet_ ? renamed_ : original_);
}


} // /namespace Xsc



// ================================================================================
