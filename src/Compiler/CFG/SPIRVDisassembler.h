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
#include <string>


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

        struct Printable
        {
            std::string                 offset;
            std::string                 result;
            std::string                 opCode;
            std::vector<std::string>    operands;
        };

        void AddPrintable(char idPrefixChar, const Instruction& inst, std::uint32_t& byteOffset);

        void PrintAll(std::ostream& stream, char idPrefixChar);
        void PrintOperand(std::ostream& stream, char idPrefixChar, const std::string& s);

        std::string                 versionStr_;
        std::string                 generatorStr_;
        std::string                 boundStr_;
        std::string                 schemaStr_;

        std::vector<Instruction>    instructions_;
        std::vector<Printable>      printables_;

};


} // /namespace Xsc


#endif



// ================================================================================
