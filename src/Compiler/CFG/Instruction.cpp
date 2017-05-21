/*
 * Instruction.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "Instruction.h"
#include "ReportIdents.h"
#include "SPIRVHelper.h"
#include <stdexcept>


namespace Xsc
{


Instruction::Instruction(const spv::Op opCode) :
    opCode { opCode }
{
}

void Instruction::WriteTo(std::vector<std::uint32_t>& buffer)
{
    /* Determine total words */
    std::uint32_t wordCount = 1;

    if (type)
        ++wordCount;
    if (result)
        ++wordCount;

    wordCount += static_cast<std::uint32_t>(operands.size());

    buffer.reserve(buffer.size() + wordCount);

    /* Write word count and op-code */
    buffer.push_back((wordCount << spv::WordCountShift) | (static_cast<std::uint32_t>(opCode) & spv::OpCodeMask));

    /* Write type and result (if used) */
    if (type)
        buffer.push_back(type);
    if (result)
        buffer.push_back(result);

    /* Write operand words */
    for (auto id : operands)
        buffer.push_back(id);
}

void Instruction::ReadFrom(std::vector<std::uint32_t>::const_iterator& bufferIter)
{
    using Op = spv::Op;

    /* Read word count and op-code */
    auto ReadUInt32 = [&bufferIter]() -> std::uint32_t
    {
        return *(bufferIter++);
    };

    auto firstWord = ReadUInt32();

    auto wordCount  = (firstWord >> spv::WordCountShift);
    this->opCode    = static_cast<spv::Op>(firstWord & spv::OpCodeMask);
    
    --wordCount;

    /* Read type (if used) */
    if (wordCount > 0 && SPIRVHelper::HasTypeId(this->opCode))
    {
        this->type = ReadUInt32();
        --wordCount;
    }

    /* Read result (if used) */
    if (wordCount > 0 && SPIRVHelper::HasResultId(this->opCode))
    {
        this->result = ReadUInt32();
        --wordCount;
    }

    /* Read operand words */
    while (wordCount-- > 0)
        operands.push_back(ReadUInt32());
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

bool Instruction::EqualsOperands(const std::vector<spv::Id>& rhsOperands, std::size_t offset) const
{
    if (NumOperands() >= (offset + rhsOperands.size()))
    {
        for (std::size_t i = 0, n = rhsOperands.size(); i < n; ++i)
        {
            if (operands[offset + i] != rhsOperands[i])
                return false;
        }
        return true;
    }
    return false;
}


} // /namespace Xsc



// ================================================================================
