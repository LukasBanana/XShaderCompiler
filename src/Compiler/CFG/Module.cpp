/*
 * Module.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "Module.h"
#include "Helper.h"


namespace Xsc
{


ModuleFunction* Module::MakeFunction(const std::string& name)
{
    functions_.emplace_back(MakeUnique<ModuleFunction>(*this, name));
    return functions_.back().get();
}


} // /namespace Xsc



// ================================================================================
