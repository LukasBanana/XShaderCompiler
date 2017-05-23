/*
 * SPIRVDisassembler.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "SPIRVDisassembler.h"
#include "SPIRVHelper.h"
#include "Exception.h"
#include "ReportIdents.h"
#include "Helper.h"
#include "Float16Compressor.h"
#include <Xsc/ConsoleManip.h>
#include <algorithm>
#include <iomanip>
#include <cstring>

// Include and define SPIRV "ToString" functions
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

    std::memcpy(wordStream.data(), buffer.data(), wordStream.size() * sizeof(std::uint32_t));

    if (wordStream.size() < 5)
        RuntimeErr(R_SPIRVFileTooSmall);

    /* Parse magic number */
    std::vector<std::uint32_t>::const_iterator wordStreamIt = wordStream.begin();

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

    if (auto s = SPIRVHelper::GetSPIRVVersionStringOrNull(versionNo))
        versionStr_ = s;
    else
        RuntimeErr(R_SPIRVUnknownVersionNumber(ToHexString(versionNo)));

    /* Parse generator magic number (see https://www.khronos.org/registry/spir-v/api/spir-v.xml) */
    const auto generatorMagic = ReadWord();

    const auto generatorVendorId    = (generatorMagic >> 16);
    const auto generatorVersionNo   = (generatorMagic & 0xffff);

    generatorStr_ = SPIRVHelper::GetSPIRVGeneratorNameById(generatorVendorId);
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

void SPIRVDisassembler::Print(std::ostream& stream, const AssemblyDescriptor& desc)
{
    /* Validate arguments */
    if (!stream.good())
        InvalidArg(R_InvalidOutputStream);

    /* Store descriptor parameter */
    desc_ = desc;

    /* Print all instructions */
    std::uint32_t byteOffset = sizeof(std::uint32_t) * 5;

    for (const auto& inst : instructions_)
        AddPrintable(inst, byteOffset);

    PrintAll(stream);

    /* Clear cache */
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
    currentInst_ = nullptr;
    currentPrt_ = nullptr;
    instructions_.clear();
}


/*
 * ======= Private: =======
 */

#define ADD_OPERAND_ENUM(ENUM_NAME) \
    AddOperandEnum<ENUM_NAME>(ENUM_NAME##ToString)

bool SPIRVDisassembler::HasRemainingOperands() const
{
    return (nextOffset_ < currentInst_->NumOperands());
}

void SPIRVDisassembler::AddOperandUInt32(std::uint32_t offset)
{
    /* Get next operand offset */
    NextOffset(offset);

    /* Add operand as value */
    currentPrt_->operands.push_back(
        std::to_string(currentInst_->GetOperandUInt32(offset))
    );

    /* Set next operand offset */
    nextOffset_ = offset + 1;
}

void SPIRVDisassembler::AddOperandId(std::uint32_t offset)
{
    /* Get next operand offset */
    NextOffset(offset);

    /* Add operand as ID number (e.g. "%1") */
    currentPrt_->operands.push_back(
        desc_.idPrefixChar + std::to_string(currentInst_->GetOperandUInt32(offset))
    );

    /* Set next operand offset */
    nextOffset_ = offset + 1;
}

void SPIRVDisassembler::AddOperandASCII(std::uint32_t offset)
{
    /* Get next operand offset */
    NextOffset(offset);

    /* Add operand as ASCII string */
    currentPrt_->operands.push_back(
        '\"' + std::string(currentInst_->GetOperandASCII(offset)) + '\"'
    );

    /* Set next operand offset */
    nextOffset_ = currentInst_->FindOperandASCIIEndOffset(offset);
}

template <typename T>
void SPIRVDisassembler::AddOperandEnum(const std::function<const char*(T e)>& enumToString, std::uint32_t offset)
{
    /* Get next operand offset */
    NextOffset(offset);

    /* Add operand as enumeration entry name */
    currentPrt_->operands.push_back(
        enumToString( static_cast<T>(currentInst_->GetOperandUInt32(offset)) )
    );

    /* Set next operand offset */
    nextOffset_ = offset + 1;
}

template <typename T>
void SPIRVDisassembler::AddOperandConstant(std::uint32_t offset)
{
    /* Get next operand offset */
    NextOffset(offset);

    /* Add operand as enumeration entry name */
    currentPrt_->operands.push_back(
        std::to_string( static_cast<T>(currentInst_->GetOperandUInt32(offset)) )
    );

    /* Set next operand offset */
    nextOffset_ = offset + 1;
}

void SPIRVDisassembler::AddOperandLiteralDecoration(const spv::Decoration decoration)
{
    switch (decoration)
    {
        case spv::Decoration::BuiltIn:
            ADD_OPERAND_ENUM(spv::BuiltIn);
            break;
        case spv::Decoration::FuncParamAttr:
            ADD_OPERAND_ENUM(spv::FunctionParameterAttribute);
            break;
        case spv::Decoration::FPRoundingMode:
            ADD_OPERAND_ENUM(spv::FPRoundingMode);
            break;
        case spv::Decoration::FPFastMathMode:
            ADD_OPERAND_ENUM(spv::FPFastMathModeMask);
            break;
        case spv::Decoration::LinkageAttributes:
            AddOperandASCII();
            ADD_OPERAND_ENUM(spv::LinkageType);
            break;
        default:
            AddOperandUInt32();
            break;
    }
}

void SPIRVDisassembler::AddOperandLiteralExecutionMode(const spv::ExecutionMode mode)
{
    switch (mode)
    {
        case spv::ExecutionMode::SubgroupsPerWorkgroupId:
            AddOperandId(); // Subgroups Per Workgroup
            break;
        case spv::ExecutionMode::LocalSizeId:
            AddOperandId(); // X Size
            AddOperandId(); // Y Size
            AddOperandId(); // Z Size
            break;
        case spv::ExecutionMode::LocalSizeHintId:
            AddOperandId(); // Local Size Hint
            break;
        default:
            AddOperandUInt32();
            break;
    }
}

void SPIRVDisassembler::AddRemainingOperandsId()
{
    while (HasRemainingOperands())
        AddOperandId();
}

void SPIRVDisassembler::NextOffset(std::uint32_t& offset)
{
    if (offset == ~0)
        offset = nextOffset_;
}

void SPIRVDisassembler::SkipOperands()
{
    nextOffset_ = currentInst_->NumOperands();
}

SPIRVDisassembler::Printable& SPIRVDisassembler::MakePrintable()
{
    /* Make new printable instance */
    printables_.emplace_back();

    /* Store reference to current printabel and reset offset for subsequent operands */
    currentPrt_ = &(printables_.back());
    nextOffset_ = 0;

    return *currentPrt_;
}

void SPIRVDisassembler::AddPrintable(const Instruction& inst, std::uint32_t& byteOffset)
{
    /* Store references */
    currentInst_ = (&inst);
    auto& prt = MakePrintable();

    /* Print offset */
    prt.offset = ToHexString(byteOffset);
    byteOffset += inst.WordCount() * 4;

    /* Print result */
    if (inst.result)
        prt.result = (desc_.idPrefixChar + std::to_string(inst.result));

    /* Print op-code */
    prt.opCode = spv::OpToString(inst.opCode);

    /* Print type */
    if (inst.type)
        prt.operands.push_back(desc_.idPrefixChar + std::to_string(inst.type));

    /* Print operands */
    using Op = spv::Op;

    switch (inst.opCode)
    {
        case Op::OpCapability:
        {
            ADD_OPERAND_ENUM(spv::Capability);
        }
        break;

        case Op::OpExtInstImport:
        {
            AddOperandASCII();
        }
        break;

        case Op::OpMemoryModel:
        {
            ADD_OPERAND_ENUM(spv::AddressingModel);
            ADD_OPERAND_ENUM(spv::MemoryModel);
        }
        break;

        case Op::OpEntryPoint:
        {
            ADD_OPERAND_ENUM(spv::ExecutionModel);
            AddOperandId();
            AddOperandASCII();
            AddRemainingOperandsId(); // Interface
        }
        break;

        case Op::OpExecutionMode:
        {
            AddOperandId();
            ADD_OPERAND_ENUM(spv::ExecutionMode);
            while (HasRemainingOperands())
                AddOperandLiteralExecutionMode(static_cast<spv::ExecutionMode>(inst.GetOperandUInt32(1)));
        }
        break;

        case Op::OpSource:
        {
            ADD_OPERAND_ENUM(spv::SourceLanguage);
            AddOperandUInt32(); // Version
            if (HasRemainingOperands())
                AddOperandId(); // File
            if (HasRemainingOperands())
                AddOperandASCII(); // Source
        }
        break;

        case Op::OpSourceExtension:
        {
            AddOperandASCII(); // Extension
        }
        break;

        case Op::OpDecorate:
        {
            AddOperandId(); // Target
            ADD_OPERAND_ENUM(spv::Decoration);
            while (HasRemainingOperands())
                AddOperandLiteralDecoration(static_cast<spv::Decoration>(inst.GetOperandUInt32(1)));
        }
        break;

        case Op::OpMemberDecorate:
        {
            AddOperandId(); // Structure type
            AddOperandUInt32(); // Literal number
            ADD_OPERAND_ENUM(spv::Decoration);
            while (HasRemainingOperands())
                AddOperandLiteralDecoration(static_cast<spv::Decoration>(inst.GetOperandUInt32(2)));
        }
        break;

        case Op::OpDecorateId:
        {
            AddOperandId(); // Target
            ADD_OPERAND_ENUM(spv::Decoration);
            while (HasRemainingOperands())
                AddOperandLiteralDecoration(static_cast<spv::Decoration>(inst.GetOperandUInt32(2)));
        }
        break;

        case Op::OpName:
        {
            AddOperandId(); // Target
            AddOperandASCII();
        }
        break;

        case Op::OpMemberName:
        {
            AddOperandUInt32();
            AddOperandASCII();
        }
        break;

        case Op::OpTypeInt:
        {
            /* Store type */
            typesInt_[inst.result] = { inst.GetOperandUInt32(0), inst.GetOperandUInt32(1) };
        }
        break;

        case Op::OpTypeFloat:
        {
            /* Store type */
            typesFloat_[inst.result] = { inst.GetOperandUInt32(0) };
        }
        break;

        case Op::OpConstant:
        {
            /* Search int type */
            auto it = typesInt_.find(inst.type);
            if (it != typesInt_.end())
            {
                /* Add integral constant to output */
                const auto& type = it->second;

                if (type.sign)
                {
                    switch (type.width)
                    {
                        case 16:
                            AddOperandConstant<std::int16_t>(0);
                            break;
                        case 32:
                            AddOperandConstant<std::int32_t>(0);
                            break;
                        case 64:
                            prt.operands.push_back(std::to_string(static_cast<std::int64_t>(inst.GetOperandUInt64(0))));
                            break;
                    }
                }
                else
                {
                    switch (type.width)
                    {
                        case 16:
                            AddOperandConstant<std::uint16_t>(0);
                            break;
                        case 32:
                            AddOperandConstant<std::uint32_t>(0);
                            break;
                        case 64:
                            prt.operands.push_back(std::to_string(inst.GetOperandUInt64(0)));
                            break;
                    }
                }

                SkipOperands();
            }
            else
            {
                /* Search float type */
                auto it = typesFloat_.find(inst.type);
                if (it != typesFloat_.end())
                {
                    /* Add integral constant to output */
                    const auto& type = it->second;

                    switch (type.width)
                    {
                        case 16:
                            prt.operands.push_back(std::to_string(inst.GetOperandFloat16(0)));
                            break;
                        case 32:
                            prt.operands.push_back(std::to_string(inst.GetOperandFloat32(0)));
                            break;
                        case 64:
                            prt.operands.push_back(std::to_string(inst.GetOperandFloat64(0)));
                            break;
                    }

                    SkipOperands();
                }
            }
        }
        break;

        case Op::OpConstantComposite:
        {
            AddRemainingOperandsId(); // Constituents
        }
        break;

        case Op::OpFunction:
        {
            ADD_OPERAND_ENUM(spv::FunctionControlMask);
            AddOperandId(); // Function type
        }
        break;

        default:
        break;
    }

    /* Append all remaining operands */
    while (HasRemainingOperands())
        AddOperandUInt32();
}

void SPIRVDisassembler::PrintAll(std::ostream& stream)
{
    static const std::size_t idResultPadding = 8;
    
    /* Print header information */
    if (desc_.showHeader)
    {
        ScopedColor scopedColor(ColorFlags::Gray, stream);
        stream << "; SPIR-V " << versionStr_ << std::endl;
        stream << "; Generator: " << generatorStr_ << std::endl;
        stream << "; Bound:     " << boundStr_ << std::endl;
        stream << "; Schema:    " << schemaStr_ << std::endl;
        stream << std::endl;
        
        if (desc_.showOffsets)
        {
            stream << "; Offset     Result     OpCode" << std::endl;
            stream << "; --------   --------   ------------------------------" << std::endl;
        }
        else
        {
            stream << "; Result     OpCode" << std::endl;
            stream << "; --------   ------------------------------" << std::endl;
        }
    }

    /* Determine longest opcode name */
    std::size_t maxOpCodeLen = 0;

    for (const auto& prt : printables_)
        maxOpCodeLen = std::max(maxOpCodeLen, prt.opCode.size());

    /* Write all printables out to stream */
    for (const auto& prt : printables_)
    {
        /* Print byte offset */
        if (desc_.showOffsets)
        {
            ScopedColor scopedColor(ColorFlags::Green, ColorFlags::Black, stream);
            stream << prt.offset << ' ';
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
            if (desc_.indentOperands)
                stream << std::string(maxOpCodeLen - prt.opCode.size(), ' ');
        }

        /* Print operands */
        for (const auto& op : prt.operands)
            PrintOperand(stream, op);

        stream << std::endl;
    }
}

void SPIRVDisassembler::PrintOperand(std::ostream& stream, const std::string& s)
{
    if (!s.empty())
    {
        stream << ' ';

        if (s[0] == '\"')
        {
            stream << '\"';
            {
                ScopedColor scopedColor(ColorFlags::Pink, stream);
                stream << s.substr(1, s.size() - 2);
            }
            stream << '\"';
        }
        else if (s[0] == desc_.idPrefixChar)
        {
            ScopedColor scopedColor(ColorFlags::Red | ColorFlags::Intens, stream);
            stream << s;
        }
        else
            stream << s;
    }
}

#undef ADD_OPERAND_ENUM


} // /namespace Xsc



// ================================================================================
