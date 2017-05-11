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


// CFG instruction class.
struct Instruction
{
    Instruction() = default;
    Instruction(const Instruction&) = default;
    Instruction(Instruction&&) = default;

    Instruction(const spv::Op opCode);
    Instruction(const spv::Op opCode, const std::initializer_list<spv::Id>& operands);

    // Returns the instruction as human-readable string.
    std::string ToString() const;

    spv::Op                 opCode      = spv::Op::OpNop;   // Instruction op-code. By default OpNop.
    std::vector<spv::Id>    operands;                       // Operand ID numbers.
};


} // /namespace Xsc


#endif



// ================================================================================
