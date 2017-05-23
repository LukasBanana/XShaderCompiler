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
        void Print(std::ostream& stream, const AssemblyDescriptor& desc);

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

        struct TypeInt
        {
            std::uint32_t width;
            std::uint32_t sign;
        };

        struct TypeFloat
        {
            std::uint32_t width;
        };

        /* === Functions === */

        // Returns true if the current instruction has remaining operands (determined by next offset).
        bool HasRemainingOperands() const;

        void AddOperandId(std::uint32_t offset = ~0);
        void AddOperandLiteral(std::uint32_t offset = ~0);
        void AddOperandASCII(std::uint32_t offset = ~0);

        template <typename T>
        void AddOperandEnum(const std::function<const char*(T e)>& enumToString, std::uint32_t offset = ~0);

        template <typename T>
        void AddOperandEnumFlags(const std::function<const char*(T e)>& enumToString, std::uint32_t offset = ~0);

        template <typename T>
        void AddOperandConstant(std::uint32_t offset = ~0);

        void AddOperandLiteralDecoration(const spv::Decoration decoration);
        void AddOperandLiteralExecutionMode(const spv::ExecutionMode mode);

        // Adds each remaining operand as Id.
        void AddRemainingOperandsId();

        // Adds each remaining operand as litearl.
        void AddRemainingOperandsLiteral();

        // Adjusts the specified offset.
        void NextOffset(std::uint32_t& offset);

        // Sets the current offset to the end to skip all remaining operands as standard output.
        void SkipOperands();

        Printable& MakePrintable();

        void AddPrintable(const Instruction& inst, std::uint32_t& byteOffset);

        void PrintAll(std::ostream& stream);
        void PrintOperand(std::ostream& stream, const std::string& s);

        /* === Members === */

        AssemblyDescriptor              desc_;

        std::string                     versionStr_;
        std::string                     generatorStr_;
        std::string                     boundStr_;
        std::string                     schemaStr_;

        std::vector<Instruction>        instructions_;
        std::vector<Printable>          printables_;

        const Instruction*              currentInst_    = nullptr;
        Printable*                      currentPrt_     = nullptr;
        std::uint32_t                   nextOffset_     = 0;

        std::map<spv::Id, TypeInt>      typesInt_;
        std::map<spv::Id, TypeFloat>    typesFloat_;

};


} // /namespace Xsc


#endif



// ================================================================================
