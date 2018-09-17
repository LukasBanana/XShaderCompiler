/*
 * Reflection.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <Xsc/Reflection.h>
#include "ReflectionPrinter.h"
#include "ASTEnums.h"


namespace Xsc
{


XSC_EXPORT std::string ToString(const Reflection::Filter t)
{
    return FilterToString(t);
}

XSC_EXPORT std::string ToString(const Reflection::TextureAddressMode t)
{
    return TexAddressModeToString(t);
}

XSC_EXPORT std::string ToString(const Reflection::ComparisonFunc t)
{
    return CompareFuncToString(t);
}

XSC_EXPORT std::string ToString(const Reflection::ResourceType t)
{
    return ResourceTypeToString(t);
}

XSC_EXPORT void PrintReflection(std::ostream& stream, const Reflection::ReflectionData& reflectionData)
{
    ReflectionPrinter printer(stream);
    printer.PrintReflection(reflectionData);
}


} // /namespace Xsc



// ================================================================================
