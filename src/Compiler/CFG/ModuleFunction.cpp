/*
 * ModuleFunction.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "ModuleFunction.h"
#include "Helper.h"


namespace Xsc
{


ModuleFunction::ModuleFunction(Module& parent, const std::string& name) :
    parent_ { parent },
    name_   { name   }
{
}

BasicBlock* ModuleFunction::MakeBlock(const std::string& label)
{
    basicBlocks_.emplace_back(MakeUnique<BasicBlock>());
    basicBlocks_.back()->label = label;
    return basicBlocks_.back().get();
}


} // /namespace Xsc



// ================================================================================
