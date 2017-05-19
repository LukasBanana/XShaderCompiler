/*
 * Instruction.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_INSTRUCTION_H
#define XSC_INSTRUCTION_H


#include <spirv/1.1/spirv.hpp11>
#include <vector>


namespace Xsc
{


// CFG instruction class (SPIR-V encoded).
struct Instruction
{
    Instruction() = default;
    Instruction(const Instruction&) = default;
    Instruction(Instruction&&) = default;

    Instruction(const spv::Op opCode);

    // Writes this instruction into the specified SPIR-V binary format buffer.
    void WriteTo(std::vector<unsigned int>& buffer);

    // Adds the specified string as ASCII operand (operands of variable size).
    Instruction& AddOperandASCII(const std::string& s);

    // Adds the specified integral value to the operands.
    Instruction& AddOperandUInt32(spv::Id i);

    // Returns the specified operand as integral value, or throws an out-of-bounds exception on failure.
    spv::Id GetOperandUInt32(std::size_t idx) const;

    // Returns the operands as ASCII string with the specified offset, or throws an out-of-bounds exception on failure..
    const char* GetOperandASCII(std::size_t offset = 0) const;

    // Returns the number of operands.
    inline std::size_t NumOperands() const
    {
        return operands.size();
    }

    spv::Op                 opCode      = spv::Op::OpNop;   // Instruction op-code. By default OpNop.
    spv::Id                 type        = 0;                // Type ID number. By default 0.
    spv::Id                 result      = 0;                // Result ID number. By default 0.
    std::vector<spv::Id>    operands;                       // Operand ID numbers.
};


} // /namespace Xsc


#endif



// ================================================================================
