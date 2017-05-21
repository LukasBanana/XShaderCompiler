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
    instructions_.clear();

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

void SPIRVDisassembler::Print(std::ostream& stream, char idPrefixChar)
{
    if (!stream.good())
        InvalidArg(R_InvalidOutputStream);

    for (const auto& inst : instructions_)
        PrintInst(stream, idPrefixChar, inst);
}


/*
 * ======= Private: =======
 */

void SPIRVDisassembler::PrintInst(std::ostream& stream, char idPrefixChar, const Instruction& inst)
{
    /* Print result */
    const std::size_t idPadding = 6;

    if (inst.result)
    {
        const auto resultStr = std::to_string(inst.result);
        stream << std::string(idPadding - resultStr.size(), ' ') << idPrefixChar << resultStr << " = ";
    }
    else
        stream << std::string(idPadding + 4, ' ');

    /* Print op-code */
    {
        ScopedColor scopedColor(ColorFlags::Yellow | ColorFlags::Intens, stream);
        stream << spv::OpToString(inst.opCode);
    }

    /* Print type */
    if (inst.type)
        stream << ' ' << idPrefixChar << inst.type;

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
