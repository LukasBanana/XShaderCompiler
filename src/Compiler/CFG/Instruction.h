/*
 * Instruction.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_INSTRUCTION_H
#define XSC_INSTRUCTION_H


#include <spirv/1.2/spirv.hpp11>
#include <vector>
#include <cstdint>
#include <string>


namespace Xsc
{


// CFG instruction class (SPIR-V encoded).
struct Instruction
{
    Instruction() = default;
    Instruction(const Instruction&) = default;
    Instruction(Instruction&&) = default;

    Instruction(const spv::Op opCode);

    /* ----- Binary format ----- */

    // Writes this instruction into the specified SPIR-V binary format buffer.
    void WriteTo(std::vector<std::uint32_t>& buffer);

    // Reads an instruction from the specified SPIR-V binary format buffer.
    void ReadFrom(std::vector<std::uint32_t>::const_iterator& bufferIter);

    /* ----- Operands ----- */

    // Adds the specified string as ASCII operand (operands of variable size).
    Instruction& AddOperandASCII(const std::string& s);

    // Adds the specified integral value to the operands.
    Instruction& AddOperandUInt32(std::uint32_t i);

    // Returns the specified operand as 32-bit unsigned integral value, or throws an out-of-bounds exception on failure.
    std::uint32_t GetOperandUInt32(std::uint32_t offset) const;

    // Returns the specified operand as 64-bit unsigned integral value, or throws an out-of-bounds exception on failure.
    std::uint64_t GetOperandUInt64(std::uint32_t offset) const;

    // Returns the specified operand as (decompressed) 16-bit floating-point value, or throws an out-of-bounds exception on failure.
    float GetOperandFloat16(std::uint32_t offset) const;

    // Returns the specified operand as 32-bit floating-point value, or throws an out-of-bounds exception on failure.
    float GetOperandFloat32(std::uint32_t offset) const;

    // Returns the specified operand as 64-bit floating-point value, or throws an out-of-bounds exception on failure.
    double GetOperandFloat64(std::uint32_t offset) const;

    // Returns the operands as ASCII string with the specified offset, or throws an out-of-bounds exception on failure..
    const char* GetOperandASCII(std::uint32_t offset) const;

    // Returns the operand offset after the end of the ASCII string operands beginning at the specified offset.
    std::uint32_t FindOperandASCIIEndOffset(std::uint32_t offset) const;

    /* ----- Misc ----- */

    // Returns true if the specified operands are equal to the operands of this instruction.
    bool EqualsOperands(const std::vector<spv::Id>& rhsOperands, std::uint32_t offset = 0) const;

    // Returns the count of words required for the whole instruction.
    std::uint32_t WordCount() const;

    // Returns the number of operands.
    std::uint32_t NumOperands() const;

    /* === Members === */

    spv::Op                 opCode      = spv::Op::OpNop;   // Instruction op-code. By default OpNop.
    spv::Id                 type        = 0;                // Type ID number. By default 0.
    spv::Id                 result      = 0;                // Result ID number. By default 0.
    std::vector<spv::Id>    operands;                       // Operand ID numbers.
};


} // /namespace Xsc


#endif



// ================================================================================
