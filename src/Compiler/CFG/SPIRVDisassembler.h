/*
 * SPIRVDisassembler.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_SPIRV_DISASSEMBLER_H
#define XSC_SPIRV_DISASSEMBLER_H


#include "Instruction.h"
#include <Xsc/Xsc.h>
#include <iostream>
#include <vector>
#include <string>
#include <functional>


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

        /* === Structures === */

        // Human readable SPIR-V instruction.
        struct Printable
        {
            std::string                 offset;
            std::string                 result;
            std::string                 opCode;
            std::vector<std::string>    operands;
        };

        /* === Functions === */

        // Returns true if the current instruction has remaining operands (determined by next offset).
        bool HasRemainingOperands() const;

        void AddOperandUInt32(std::uint32_t offset = ~0);
        void AddOperandId(std::uint32_t offset = ~0);
        void AddOperandASCII(std::uint32_t offset = ~0);

        template <typename T>
        void AddOperandEnum(const std::function<const char*(T e)>& enumToString, std::uint32_t offset = ~0);

        void AddOperandLiteralDecoration(const spv::Decoration decoration);

        void NextOffset(std::uint32_t& offset);

        Printable& MakePrintable();

        void AddPrintable(const Instruction& inst, std::uint32_t& byteOffset);

        void PrintAll(std::ostream& stream);
        void PrintOperand(std::ostream& stream, const std::string& s);

        /* === Members === */

        char                        idPrefixChar_   = '%';

        std::string                 versionStr_;
        std::string                 generatorStr_;
        std::string                 boundStr_;
        std::string                 schemaStr_;

        std::vector<Instruction>    instructions_;
        std::vector<Printable>      printables_;

        const Instruction*          currentInst_    = nullptr;
        Printable*                  currentPrt_     = nullptr;
        std::uint32_t               nextOffset_     = 0;

};


} // /namespace Xsc


#endif



// ================================================================================
