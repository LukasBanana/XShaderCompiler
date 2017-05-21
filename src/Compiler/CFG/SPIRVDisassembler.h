/*
 * SPIRVDisassembler.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_SPIRV_DISASSEMBLER_H
#define XSC_SPIRV_DISASSEMBLER_H


#include <iostream>
#include <vector>


namespace Xsc
{


struct Instruction;

// SPIR-V disassembler class.
class SPIRVDisassembler
{

    public:

        /*
        Disassembles the SPIR-V binary code from the input stream 'streamIn'
        to human readable code into the output stream 'streamOut',
        or throws an exceptio on failure.
        */
        void Disassemble(std::istream& streamIn, std::ostream& streamOut);

    private:

        void Parse(std::istream& stream);

        void Print(std::ostream& stream);
        void PrintInst(std::ostream& stream, const Instruction& inst);

        std::uint32_t               versionNo_      = 0;

        std::vector<Instruction>    instructions_;

};


} // /namespace Xsc


#endif



// ================================================================================
