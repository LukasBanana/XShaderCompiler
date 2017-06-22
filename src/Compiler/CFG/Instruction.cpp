/*
 * Instruction.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "Instruction.h"
#include "ReportIdents.h"
#include "SPIRVHelper.h"
#include "Float16Compressor.h"
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

    /* Fill remaining bytes with zeros */
    if (numCharsWithNUL % 4 != 0)
    {
        for (std::size_t i = 0; i < (4 - numCharsWithNUL % 4); ++i, ++dst)
            *dst = 0;
    }

    return *this;
}

Instruction& Instruction::AddOperandUInt32(std::uint32_t i)
{
    operands.push_back(i);
    return *this;
}

std::uint32_t Instruction::GetOperandUInt32(std::uint32_t offset) const
{
    if (offset < NumOperands())
        return operands[offset];
    else
        throw std::out_of_range(R_NotEnoughOperandsInInst());
}

std::uint64_t Instruction::GetOperandUInt64(std::uint32_t offset) const
{
    if (offset + 1 < NumOperands())
    {
        /* Extract 64-bit integral */
        std::uint64_t ui = 0;

        ui = operands[offset];
        ui <<= 32;
        ui |= operands[offset + 1];

        return ui;
    }
    else
        throw std::out_of_range(R_NotEnoughOperandsInInst());
}

float Instruction::GetOperandFloat16(std::uint32_t offset) const
{
    return DecompressFloat16(static_cast<std::uint16_t>(GetOperandUInt32(offset)));
}

float Instruction::GetOperandFloat32(std::uint32_t offset) const
{
    if (offset < NumOperands())
    {
        /* Extract 32-bit floating-point */
        union
        {
            std::uint32_t   ui;
            float           f;
        }
        data;

        data.ui = operands[offset];

        return data.f;
    }
    else
        throw std::out_of_range(R_NotEnoughOperandsInInst());
}

double Instruction::GetOperandFloat64(std::uint32_t offset) const
{
    if (offset + 1 < NumOperands())
    {
        /* Extract 32-bit floating-point */
        union
        {
            std::uint64_t   ui;
            double          f;
        }
        data;

        data.ui = operands[offset];
        data.ui <<= 32;
        data.ui |= operands[offset + 1];

        return data.f;
    }
    else
        throw std::out_of_range(R_NotEnoughOperandsInInst());
}

const char* Instruction::GetOperandASCII(std::uint32_t offset) const
{
    if (offset < NumOperands())
        return reinterpret_cast<const char*>(&(operands[offset]));
    else
        throw std::out_of_range(R_NotEnoughOperandsInInst());
}

std::uint32_t Instruction::FindOperandASCIIEndOffset(std::uint32_t offset) const
{
    for (; offset < NumOperands(); ++offset)
    {
        /* Check for null terminator in current word */
        auto word = operands[offset];
        for (int i = 0; i < 4; ++i)
        {
            /* Check if current byte is zero */
            if ((word & 0xff) == 0)
                return offset + 1;

            /* Shift word to check next byte */
            word >>= 8;
        }
    }
    return offset;
}

bool Instruction::EqualsOperands(const std::vector<spv::Id>& rhsOperands, std::uint32_t offset) const
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

std::uint32_t Instruction::WordCount() const
{
    std::uint32_t wordCount = 1;

    if (type)
        ++wordCount;
    if (result)
        ++wordCount;

    wordCount += static_cast<std::uint32_t>(operands.size());

    return wordCount;
}

std::uint32_t Instruction::NumOperands() const
{
    return static_cast<std::uint32_t>(operands.size());
}


} // /namespace Xsc



// ================================================================================
