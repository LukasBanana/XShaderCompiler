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
#include <initializer_list>


namespace Xsc
{


// CFG instruction class (SPIR-V encoded).
struct Instruction
{
    Instruction() = default;
    Instruction(const Instruction&) = default;
    Instruction(Instruction&&) = default;

    Instruction(const spv::Op opCode);
    Instruction(const spv::Op opCode, const std::initializer_list<spv::Id>& operands);

    // Writes this instruction into the specified SPIR-V binary format buffer.
    void WriteTo(std::vector<unsigned int>& buffer);

    // Returns the instruction as human-readable string.
    std::string ToString() const;

    spv::Op                 opCode      = spv::Op::OpNop;   // Instruction op-code. By default OpNop.
    spv::Id                 type        = 0;                // Type ID number. By default 0.
    spv::Id                 result      = 0;                // Result ID number. By default 0.
    std::vector<spv::Id>    operands;                       // Operand ID numbers.
};


} // /namespace Xsc


#endif



// ================================================================================
