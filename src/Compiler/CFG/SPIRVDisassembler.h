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

        // Reads the SPIR-V binary code from the specified input stream, and clears all previously added instructions.
        void Parse(std::istream& stream);

        // Prints the human readable code of all instructions.
        void Print(std::ostream& stream, char idPrefixChar = '%');

        // Adds the specified instruction manually to the print output.
        void Add(const Instruction& inst);

        // Adds the specified instruction manually to the print output with move semantics.
        void Add(Instruction&& inst);

        // Clears all internal instructions.
        void Clear();

    private:

        // Human readable SPIR-V instruction.
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
