/*
 * ConsoleManip.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <Xsc/ConsoleManip.h>


namespace Xsc
{

namespace ConsoleManip
{


static bool g_enabled = true;

void XSC_EXPORT Enable(bool enable)
{
    g_enabled = enable;
}

bool XSC_EXPORT IsEnabled()
{
    return g_enabled;
}


} // /namespace ConsoleManip

} // /namespace Xsc



// ================================================================================