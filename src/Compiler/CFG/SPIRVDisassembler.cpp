/*
 * SPIRVDisassembler.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "SPIRVDisassembler.h"
#include "Instruction.h"
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

void SPIRVDisassembler::Disassemble(std::istream& streamIn, std::ostream& streamOut)
{
    if (!streamIn.good())
        InvalidArg(R_InvalidInputStream);
    if (!streamOut.good())
        InvalidArg(R_InvalidOutputStream);

    Parse(streamIn);
    Print(streamOut);

    instructions_.clear();
}


/*
 * ======= Private: =======
 */

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
    /* Read words from byte stream */
    std::vector<std::uint32_t> wordStream;

    while (!stream.eof())
    {
        std::uint32_t word = 0;
        stream.read(reinterpret_cast<char*>(&word), sizeof(word));
        wordStream.push_back(word);
    }

    /* Parse magic number */
    auto wordStreamIt = wordStream.begin();

    auto ReadWordOrError = [&](const JoinableString& err) -> std::uint32_t
    {
        if (wordStreamIt == wordStream.end())
            RuntimeErr(err);
        return *(wordStreamIt++);
    };

    const auto magicNumber = ReadWordOrError(R_SPIRVMissingMagicNumber);

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
    versionNo_ = ReadWordOrError(R_SPIRVMissingVersionNumber);

    switch (versionNo_)
    {
        case 0x00010000:
        case 0x00010100:
        case 0x00010200:
            break;
        default:
            RuntimeErr(R_SPIRVUnknownVersionNumber(ToHexString(versionNo_)));
            break;
    }

    #if 1
    //TODO: currently remaining header is ignored
    wordStreamIt += 3;
    #endif

    /* Parse instructions */
    while (wordStreamIt != wordStream.end())
    {
        Instruction inst;
        inst.ReadFrom(wordStreamIt);
        instructions_.emplace_back(std::move(inst));
    }
}

void SPIRVDisassembler::Print(std::ostream& stream)
{
    for (const auto& inst : instructions_)
        PrintInst(stream, inst);
}

void SPIRVDisassembler::PrintInst(std::ostream& stream, const Instruction& inst)
{
    /* Print result */
    if (inst.result)
    {
        const auto resultStr = std::to_string(inst.result);
        stream << std::string(4 - resultStr.size(), ' ') << '%' << resultStr << " = ";
    }
    else
        stream << std::string(8, ' ');

    /* Print op-code */
    {
        ScopedColor scopedColor(ColorFlags::Yellow | ColorFlags::Intens, stream);
        stream << spv::OpToString(inst.opCode);
    }

    /* Print type */
    if (inst.type)
        stream << '%' << inst.type;

    /* Print operands */
    //TODO: print correct operands, and distinguish between Uint32 and ASCII operands!
    #if 0
    for (auto id : inst.operands)
        ;
    #endif

    stream << std::endl;
}


} // /namespace Xsc



// ================================================================================
