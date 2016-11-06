/*
 * ConsoleManip.cpp
 * 
 * This file is part of the "HLSL Translator" (Copyright (c) 2014 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <HT/ConsoleManip.h>


namespace HTLib
{

namespace ConsoleManip
{


static bool g_enabled = true;

void HTLIB_EXPORT Enable(bool enable)
{
    g_enabled = enable;
}

bool HTLIB_EXPORT IsEnabled()
{
    return g_enabled;
}


} // /namespace ConsoleManip

} // /namespace HTLib



// ================================================================================