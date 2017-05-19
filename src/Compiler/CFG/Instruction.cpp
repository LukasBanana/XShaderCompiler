/*
 * Instruction.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "Instruction.h"
#include "ReportIdents.h"
#include <stdexcept>


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

Instruction& Instruction::AddOperandASCII(const std::string& s)
{
    /* Allocate enough operands for string */
    const auto numChars         = s.size();
    const auto numCharsWithNUL  = numChars + 1;
    const auto numWords         = (numCharsWithNUL + 3) / 4;

    const auto prevNumOperands  = operands.size();
    operands.resize(operands.size() + numWords);

    /* Fill operands with ASCII characters */
    auto src = s.c_str();
    auto dst = reinterpret_cast<char*>(&(operands[prevNumOperands]));

    for (std::size_t i = 0; i < numCharsWithNUL; ++i, ++src, ++dst)
        *dst = *src;

    /* Fill remaining bytese with zeros */
    if (numCharsWithNUL % 4 != 0)
    {
        for (std::size_t i = 0; i < (4 - numCharsWithNUL % 4); ++i, ++dst)
            *dst = 0;
    }

    return *this;
}

Instruction& Instruction::AddOperandUInt32(spv::Id i)
{
    operands.push_back(i);
    return *this;
}

spv::Id Instruction::GetOperandUInt32(std::size_t idx) const
{
    if (idx < operands.size())
        return operands[idx];
    else
        throw std::out_of_range(R_NotEnoughOperandsInInst());
}

const char* Instruction::GetOperandASCII(std::size_t offset) const
{
    if (offset < operands.size())
        return reinterpret_cast<const char*>(&(operands[offset]));
    else
        throw std::out_of_range(R_NotEnoughOperandsInInst());
}


} // /namespace Xsc



// ================================================================================
