/*
 * Instruction.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "Instruction.h"


namespace Xsc
{


Instruction::Instruction(const spv::Op opCode) :
    opCode { opCode }
{
}

Instruction::Instruction(const spv::Op opCode, const std::initializer_list<spv::Id>& operands) :
    opCode   { opCode   },
    operands { operands }
{
}

std::string Instruction::ToString() const
{
    //TODO...
    return "";
}


} // /namespace Xsc



// ================================================================================
