/*
 * SPIRVDisassembler.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "SPIRVDisassembler.h"
#include "Exception.h"
#include "ReportIdents.h"
#include "Helper.h"
#include <Xsc/ConsoleManip.h>
#include <algorithm>
#include <iomanip>

// Include and define SPIRV "ToString" functions
#define SPIRV_DEF static
#define SPIRV_STRINGS_IMPLEMENT
#include <spirv/1.2/spirv_strings.hpp11>


namespace Xsc
{


using namespace ConsoleManip;

static std::uint32_t SwapEndian(std::uint32_t i)
{
    return
    (
        ((i >> 24) & 0x000000ff) |
        ((i >>  8) & 0x0000ff00) |
        ((i <<  8) & 0x00ff0000) |
        ((i << 24) & 0xff000000)
    );
}

void SPIRVDisassembler::Parse(std::istream& stream)
{
    /* Clear previous instruction cache */
    Clear();

    if (!stream.good())
        InvalidArg(R_InvalidInputStream);

    /* Read entire byte stream */
    std::vector<char> buffer(
        (std::istreambuf_iterator<char>(stream)),
        (std::istreambuf_iterator<char>())
    );

    if (buffer.size() % 4 != 0)
        RuntimeErr(R_SPIRVByteStreamNotWordAligned);

    /* Copy byte stream into word stream */
    std::vector<std::uint32_t> wordStream;
    wordStream.resize(buffer.size() / 4);

    memcpy(wordStream.data(), buffer.data(), wordStream.size() * sizeof(std::uint32_t));

    if (wordStream.size() < 5)
        RuntimeErr(R_SPIRVFileTooSmall);

    /* Parse magic number */
    auto wordStreamIt = wordStream.begin();

    auto ReadWord = [&]() -> std::uint32_t
    {
        return *(wordStreamIt++);
    };

    const auto magicNumber = ReadWord();

    if (magicNumber == SwapEndian(spv::MagicNumber))
    {
        /* Transform byte-order of word stream */
        std::transform(std::begin(wordStream), std::end(wordStream), std::begin(wordStream), SwapEndian);
    }
    else if (magicNumber != spv::MagicNumber)
    {
        /* Wrong magic number -> throw error */
        RuntimeErr(R_SPIRVInvalidMagicNumber(ToHexString(spv::MagicNumber), ToHexString(magicNumber)));
    }

    /* Parse SPIR-V version */
    const auto versionNo = ReadWord();

    switch (versionNo)
    {
        case 0x00010000:
            versionStr_ = "1.0";
            break;
        case 0x00010100:
            versionStr_ = "1.1";
            break;
        case 0x00010200:
            versionStr_ = "1.2";
            break;
        default:
            RuntimeErr(R_SPIRVUnknownVersionNumber(ToHexString(versionNo)));
            break;
    }

    /* Parse generator magic number (see https://www.khronos.org/registry/spir-v/api/spir-v.xml) */
    const auto generatorMagic = ReadWord();

    const auto generatorVendorId    = (generatorMagic >> 16);
    const auto generatorVersionNo   = (generatorMagic & 0xffff);

    switch (generatorVendorId)
    {
        case 0:
            generatorStr_ = "Khronos"; // "Reserved by Khronos"
            break;
        case 1:
            generatorStr_ = "LunarG"; // "Contact TBD"
            break;
        case 2:
            generatorStr_ = "Valve"; // "Contact TBD"
            break;
        case 3:
            generatorStr_ = "Codeplay"; // "Contact Neil Henning, neil@codeplay.com"
            break;
        case 4:
            generatorStr_ = "NVIDIA"; // "Contact Kerch Holt, kholt@nvidia.com"
            break;
        case 5:
            generatorStr_ = "ARM"; // "Contact Alexander Galazin, alexander.galazin@arm.com"
            break;
        case 6:
            generatorStr_ = "Khronos LLVM/SPIR-V Translator"; // "Contact Yaxun (Sam) Liu, yaxun.liu@amd.com"
            break;
        case 7:
            generatorStr_ = "Khronos SPIR-V Tools Assembler"; // "Contact David Neto, dneto@google.com"
            break;
        case 8:
            generatorStr_ = "Khronos Glslang Reference Front End"; // "Contact John Kessenich, johnkessenich@google.com"
            break;
        case 9:
            generatorStr_ = "Qualcomm"; // "Contact weifengz@qti.qualcomm.com"
            break;
        case 10:
            generatorStr_ = "AMD"; // "Contact Daniel Rakos, daniel.rakos@amd.com"
            break;
        case 11:
            generatorStr_ = "Intel"; // "Contact Alexey, alexey.bader@intel.com"/>
            break;
        default:
            generatorStr_ = "Unknown";
            break;
    }

    generatorStr_ += " (Version " + std::to_string(generatorVersionNo) + ")";

    /* Parse ID bound */
    boundStr_ = std::to_string(ReadWord());

    /* Parse instruction schema (always 0) */
    schemaStr_ = std::to_string(ReadWord());

    /* Parse instructions */
    while (wordStreamIt != wordStream.end())
    {
        Instruction inst;
        inst.ReadFrom(wordStreamIt);
        Add(std::move(inst));
    }
}

void SPIRVDisassembler::Print(std::ostream& stream, char idPrefixChar)
{
    if (!stream.good())
        InvalidArg(R_InvalidOutputStream);

    std::uint32_t byteOffset = sizeof(std::uint32_t) * 5;

    for (const auto& inst : instructions_)
        AddPrintable(idPrefixChar, inst, byteOffset);

    PrintAll(stream, idPrefixChar);

    printables_.clear();
}

void SPIRVDisassembler::Add(const Instruction& inst)
{
    instructions_.push_back(inst);
}

void SPIRVDisassembler::Add(Instruction&& inst)
{
    instructions_.emplace_back(std::move(inst));
}

void SPIRVDisassembler::Clear()
{
    instructions_.clear();
}


/*
 * ======= Private: =======
 */

void SPIRVDisassembler::AddPrintable(char idPrefixChar, const Instruction& inst, std::uint32_t& byteOffset)
{
    Printable prt;

    /* Print offset */
    prt.offset = ToHexString(byteOffset);
    byteOffset += inst.WordCount() * 4;

    /* Print result */
    if (inst.result)
        prt.result = (idPrefixChar + std::to_string(inst.result));

    /* Print op-code */
    prt.opCode = spv::OpToString(inst.opCode);

    /* Print type */
    if (inst.type)
        prt.operands.push_back(idPrefixChar + std::to_string(inst.type));

    /* Print operands */
    using Op = spv::Op;

    auto AddOperandASCII = [&prt,&inst](std::size_t offset)
    {
        prt.operands.push_back('\"' + std::string(inst.GetOperandASCII(offset)) + '\"');
    };

    //TODO: print correct operands, and distinguish between Uint32 and ASCII operands!
    switch (inst.opCode)
    {
        case Op::OpCapability:
            prt.operands.push_back(spv::CapabilityToString(static_cast<spv::Capability>(inst.GetOperandUInt32(0))));
            break;
        case Op::OpExtInstImport:
            AddOperandASCII(0);
            break;
        case Op::OpMemoryModel:
            prt.operands.push_back(spv::AddressingModelToString(static_cast<spv::AddressingModel>(inst.GetOperandUInt32(0))));
            prt.operands.push_back(spv::MemoryModelToString(static_cast<spv::MemoryModel>(inst.GetOperandUInt32(1))));
            break;
        case Op::OpEntryPoint:
            prt.operands.push_back(spv::ExecutionModelToString(static_cast<spv::ExecutionModel>(inst.GetOperandUInt32(0))));
            prt.operands.push_back(std::to_string(inst.GetOperandUInt32(1)));
            AddOperandASCII(2);
            break;
        case Op::OpName:
            AddOperandASCII(1);
            break;
        case Op::OpMemberName:
            prt.operands.push_back(std::to_string(inst.GetOperandUInt32(0)));
            AddOperandASCII(1);
            break;
        default:
            break;
    }

    printables_.emplace_back(std::move(prt));
}

void SPIRVDisassembler::PrintAll(std::ostream& stream, char idPrefixChar)
{
    static const std::size_t idResultPadding = 6;

    /* Print header information */
    {
        ScopedColor scopedColor(ColorFlags::Gray, stream);
        stream << "; SPIR-V " << versionStr_ << std::endl;
        stream << "; Generator: " << generatorStr_ << std::endl;
        stream << "; Bound:     " << boundStr_ << std::endl;
        stream << "; Schema:    " << schemaStr_ << std::endl;
        stream << std::endl;
        stream << "; Offset   Result    OpCode" << std::endl;
        stream << "; " << std::string(50, '-') << std::endl;
    }

    for (const auto& prt : printables_)
    {
        /* Print byte offset */
        {
            ScopedColor scopedColor(ColorFlags::Green, ColorFlags::Black, stream);
            stream << prt.offset;
        }

        stream << "  ";

        /* Print result */
        if (!prt.result.empty())
        {
            {
                ScopedColor scopedColor(ColorFlags::Red | ColorFlags::Intens, stream);
                stream << std::string(idResultPadding - prt.result.size(), ' ') << prt.result;
            }
            stream << " = ";
        }
        else
            stream << std::string(idResultPadding + 3, ' ');

        /* Print op-code */
        {
            ScopedColor scopedColor(ColorFlags::Yellow | ColorFlags::Intens, stream);
            stream << prt.opCode;
        }

        /* Print operands */
        for (const auto& op : prt.operands)
            PrintOperand(stream, idPrefixChar, op);

        stream << std::endl;
    }
}

void SPIRVDisassembler::PrintOperand(std::ostream& stream, char idPrefixChar, const std::string& s)
{
    if (!s.empty())
    {
        stream << ' ';

        if (s[0] == '\"')
        {
            stream << '\"';
            {
                ScopedColor scopedColor(ColorFlags::Cyan, stream);
                stream << s.substr(1, s.size() - 2);
            }
            stream << '\"';
        }
        else if (s[0] == idPrefixChar)
        {
            ScopedColor scopedColor(ColorFlags::Red | ColorFlags::Intens, stream);
            stream << s;
        }
        else
            stream << s;
    }
}


} // /namespace Xsc



// ================================================================================
