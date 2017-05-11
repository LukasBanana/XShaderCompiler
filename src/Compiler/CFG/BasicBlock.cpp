/*
 * BasicBlock.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "BasicBlock.h"


namespace Xsc
{


/*
 * Edge structure
 */

BasicBlock::Edge::Edge(BasicBlock& succ, const std::string& label) :
    succ  { succ  },
    label { label }
{
}

bool BasicBlock::Edge::operator == (const BasicBlock* rhs) const
{
    return (&succ == rhs);
}

bool BasicBlock::Edge::operator != (const BasicBlock* rhs) const
{
    return (&succ != rhs);
}


/*
 * BasicBlock class
 */

void BasicBlock::AddSucc(BasicBlock& bb, const std::string& label)
{
    //TODO...
}


} // /namespace Xsc



// ================================================================================
