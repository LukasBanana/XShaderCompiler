/*
 * SPIRVDisassembler.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_SPIRV_DISASSEMBLER_H
#define XSC_SPIRV_DISASSEMBLER_H


#include "Instruction.h"
#include <iostream>
#include <vector>


namespace Xsc
{


struct Instruction;

// SPIR-V disassembler class.
class SPIRVDisassembler
{

    public:

        // Reads the SPIR-V binary code.
        void Parse(std::istream& stream);

        // Prints the human readable code.
        void Print(std::ostream& stream, char idPrefixChar = '%');

    private:

        void PrintInst(std::ostream& stream, char idPrefixChar, const Instruction& inst);

        std::uint32_t               versionNo_      = 0;

        std::vector<Instruction>    instructions_;

};


} // /namespace Xsc


#endif



// ================================================================================
