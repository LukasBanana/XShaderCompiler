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

void Instruction::WriteTo(std::vector<unsigned int>& buffer)
{
    /* Determine total words */
    unsigned int wordCount = 1;

    if (type)
        ++wordCount;
    if (result)
        ++wordCount;

    wordCount += static_cast<unsigned int>(operands.size());

    /* Write word count and op-code */
    buffer.reserve(static_cast<std::size_t>(wordCount));

    buffer.push_back((wordCount << spv::WordCountShift) | (static_cast<unsigned int>(opCode) & spv::OpCodeMask));

    /* Write type and result (if set) */
    if (type)
        buffer.push_back(type);
    if (result)
        buffer.push_back(result);

    /* Write operand words */
    for (auto id : operands)
        buffer.push_back(id);
}

std::string Instruction::ToString() const
{
    //TODO...
    return "";
}


} // /namespace Xsc



// ================================================================================
