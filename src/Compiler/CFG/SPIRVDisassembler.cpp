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
    typesInt_.clear();
    typesFloat_.clear();
    idNames_.clear();
    constants_.clear();
}


/*
 * ======= Private: =======
 */

#define ADD_OPERAND_ENUM(ENUM_NAME) \
    AddOperandEnum<ENUM_NAME>(ENUM_NAME##ToString)

#define ADD_OPERAND_ENUM_FLAGS(ENUM_NAME) \
    AddOperandEnumFlags<ENUM_NAME>(ENUM_NAME##ToString)

bool SPIRVDisassembler::HasRemainingOperands() const
{
    return (nextOffset_ < currentInst_->NumOperands());
}

void SPIRVDisassembler::AddOperandId(std::uint32_t offset, spv::Id* output)
{
    /* Get next operand offset */
    NextOffset(offset);

    /* Add operand as ID number (e.g. "%1") */
    spv::Id value = currentInst_->GetOperandUInt32(offset);
    currentPrt_->operands.push_back(desc_.idPrefixChar + GetName(value));

    if (output)
        *output = value;

    /* Set next operand offset */
    nextOffset_ = offset + 1;
}

void SPIRVDisassembler::AddOperandLiteral(std::uint32_t offset, std::uint32_t* output)
{
    /* Get next operand offset */
    NextOffset(offset);

    /* Add operand as value */
    std::uint32_t value = currentInst_->GetOperandUInt32(offset);
    currentPrt_->operands.push_back(std::to_string(value));

    if (output)
        *output = value;

    /* Set next operand offset */
    nextOffset_ = offset + 1;
}

void SPIRVDisassembler::AddOperandASCII(std::uint32_t offset, std::string* output)
{
    /* Get next operand offset */
    NextOffset(offset);

    /* Add operand as ASCII string */
    std::string value = currentInst_->GetOperandASCII(offset);
    currentPrt_->operands.push_back('\"' + value + '\"');
    
    if (output)
        *output = std::move(value);

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
void SPIRVDisassembler::AddOperandEnumFlags(const std::function<const char*(T e)>& enumToString, std::uint32_t offset)
{
    /* Get next operand offset */
    NextOffset(offset);

    /* Add operand as enumeration entry flags name */
    std::string s;

    auto flags = currentInst_->GetOperandUInt32(offset);

    if (flags != 0)
    {
        for (std::size_t i = 0; i < sizeof(std::uint32_t) * 8; ++i)
        {
            const auto currentFlag = (flags & (1u << i));
            if (currentFlag != 0)
            {
                if (!s.empty())
                    s += '|';
                s += enumToString(static_cast<T>(currentFlag));
            }
        }
    }
    else
        s = enumToString(static_cast<T>(flags));

    currentPrt_->operands.push_back(s);

    /* Set next operand offset */
    nextOffset_ = offset + 1;
}

template <typename T>
void SPIRVDisassembler::AddOperandConstant(std::uint32_t offset, std::string* output)
{
    /* Get next operand offset */
    NextOffset(offset);

    /* Add operand as enumeration entry name */
    std::string value = std::to_string(static_cast<T>(currentInst_->GetOperandUInt32(offset)));
    currentPrt_->operands.push_back(value);

    if (output)
        *output = std::move(value);

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
            ADD_OPERAND_ENUM_FLAGS(spv::FPFastMathModeMask);
            break;
        case spv::Decoration::LinkageAttributes:
            AddOperandASCII();
            ADD_OPERAND_ENUM(spv::LinkageType);
            break;
        default:
            AddOperandLiteral();
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
            AddOperandLiteral();
            break;
    }
}

void SPIRVDisassembler::AddRemainingOperandsId()
{
    while (HasRemainingOperands())
        AddOperandId();
}

void SPIRVDisassembler::AddRemainingOperandsLiteral()
{
    while (HasRemainingOperands())
        AddOperandLiteral();
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
    using Op = spv::Op;

    /* Store references */
    currentInst_ = (&inst);
    auto& prt = MakePrintable();

    /* Print offset */
    prt.offset = ToHexString(byteOffset);
    byteOffset += inst.WordCount() * 4;

    /* Set result names */
    if (desc_.showNames)
    {
        switch (inst.opCode)
        {
            case Op::OpTypeVoid:
                SetName(inst.result, "void");
                break;

            case Op::OpTypeBool:
                SetName(inst.result, "bool");
                break;

            case Op::OpTypeInt:
                if (inst.GetOperandUInt32(1) == 0)
                {
                    switch (inst.GetOperandUInt32(0))
                    {
                        case 8:
                            SetName(inst.result, "uchar");
                            break;
                        case 16:
                            SetName(inst.result, "ushort");
                            break;
                        case 32:
                            SetName(inst.result, "uint");
                            break;
                        case 64:
                            SetName(inst.result, "ulong");
                            break;
                    }
                }
                else
                {
                    switch (inst.GetOperandUInt32(0))
                    {
                        case 8:
                            SetName(inst.result, "char");
                            break;
                        case 16:
                            SetName(inst.result, "short");
                            break;
                        case 32:
                            SetName(inst.result, "int");
                            break;
                        case 64:
                            SetName(inst.result, "long");
                            break;
                    }
                }
                break;

            case Op::OpTypeFloat:
                switch (inst.GetOperandUInt32(0))
                {
                    case 16:
                        SetName(inst.result, "half");
                        break;
                    case 32:
                        SetName(inst.result, "float");
                        break;
                    case 64:
                        SetName(inst.result, "double");
                        break;
                }
                break;

            case Op::OpTypeVector:
                SetName(inst.result, GetName(inst.GetOperandUInt32(0)) + std::to_string(inst.GetOperandUInt32(1)));
                break;

            case Op::OpTypeMatrix:
                SetName(inst.result, GetName(inst.GetOperandUInt32(0)) + 'x' + std::to_string(inst.GetOperandUInt32(1)));
                break;

            case Op::OpTypeImage:
            case Op::OpTypeSampler:
            case Op::OpTypeSampledImage:
                break;

            case Op::OpTypeArray:
                SetName(inst.result, GetName(inst.GetOperandUInt32(0)) + '[' + GetConstant(inst.GetOperandUInt32(1)) + ']');
                break;

            case Op::OpTypeRuntimeArray:
                SetName(inst.result, GetName(inst.GetOperandUInt32(0)) + "[]");
                break;

            case Op::OpTypeStruct:
            case Op::OpTypeOpaque:
                break;

            case Op::OpTypePointer:
                SetName(
                    inst.result,
                    (
                        std::string(spv::StorageClassToString(static_cast<spv::StorageClass>(inst.GetOperandUInt32(0)))) +
                        '<' + GetName(inst.GetOperandUInt32(1)) + '>'
                    )
                );
                break;

            case Op::OpTypeFunction:
            case Op::OpTypeEvent:
            case Op::OpTypeDeviceEvent:
            case Op::OpTypeReserveId:
            case Op::OpTypeQueue:
            case Op::OpTypePipe:
            case Op::OpTypeForwardPointer:
                break;

            case Op::OpName:
                SetName(inst.GetOperandUInt32(0), inst.GetOperandASCII(1));
                break;

            default:
                break;
        }
    }

    /* Print result */
    if (inst.result)
        prt.result = (desc_.idPrefixChar + GetName(inst.result));

    /* Print op-code */
    prt.opCode = spv::OpToString(inst.opCode);

    /* Print type */
    if (inst.type)
        prt.operands.push_back(desc_.idPrefixChar + GetName(inst.type));

    /* Print operands */
    switch (inst.opCode)
    {
        case Op::OpSizeOf:
        {
            AddOperandId(); // Pointer
        }
        break;

        case Op::OpSourceContinued:
        {
            AddOperandASCII(); // Continued Source
        }
        break;

        case Op::OpSource:
        {
            ADD_OPERAND_ENUM(spv::SourceLanguage);  // Source Language
            AddOperandLiteral();                    // Version
            if (HasRemainingOperands())
            {
                AddOperandId();                     // File
                if (HasRemainingOperands())
                    AddOperandASCII();              // Source
            }
        }
        break;

        case Op::OpSourceExtension:
        {
            AddOperandASCII(); // Extension
        }
        break;

        case Op::OpName:
        {
            spv::Id target = 0;
            std::string name;

            AddOperandId(~0, &target);  // Target
            AddOperandASCII(~0, &name); // Name

            SetName(target, name);
        }
        break;

        case Op::OpMemberName:
        {
            std::uint32_t member = 0;
            std::string name;

            AddOperandLiteral(~0, &member); // Member
            AddOperandASCII(~0, &name);     // Name

            SetMemberName(inst.type, member, name);
        }
        break;

        case Op::OpString:
        {
            AddOperandASCII(); // String
        }
        break;

        case Op::OpLine:
        {
            AddOperandId();         // File
            AddOperandLiteral();    // Line
            AddOperandLiteral();    // Column
        }
        break;

        case Op::OpModuleProcessed:
        {
            AddOperandASCII(); // Process
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
            AddOperandLiteral(); // Literal number
            ADD_OPERAND_ENUM(spv::Decoration);
            while (HasRemainingOperands())
                AddOperandLiteralDecoration(static_cast<spv::Decoration>(inst.GetOperandUInt32(2)));
        }
        break;

        case Op::OpGroupDecorate:
        {
            AddOperandId();             // Decoration Group
            AddRemainingOperandsId();   // Targets
        }
        break;

        case Op::OpGroupMemberDecorate:
        {
            AddOperandId();                 // Decoration Group
            while (HasRemainingOperands())
            {
                AddOperandId();             // Target Id
                AddOperandLiteral();        // Target Literal
            }
        }
        break;

        case Op::OpDecorateId:
        {
            AddOperandId();                     // Target
            ADD_OPERAND_ENUM(spv::Decoration);  // Decoration
            while (HasRemainingOperands())
                AddOperandLiteralDecoration(static_cast<spv::Decoration>(inst.GetOperandUInt32(2)));
        }
        break;

        case Op::OpExtension:
        {
            AddOperandASCII(); // Extension
        }
        break;

        case Op::OpExtInstImport:
        {
            AddOperandASCII(); // Name
        }
        break;

        case Op::OpExtInst:
        {
            AddOperandId();             // Set
            AddOperandLiteral();        // Instruction
            AddRemainingOperandsId();   // Operands
        }
        break;

        case Op::OpMemoryModel:
        {
            ADD_OPERAND_ENUM(spv::AddressingModel); // Addressing Model
            ADD_OPERAND_ENUM(spv::MemoryModel);     // Memory Model
        }
        break;

        case Op::OpEntryPoint:
        {
            ADD_OPERAND_ENUM(spv::ExecutionModel);  // Execution Model
            AddOperandId();                         // Entry Point
            AddOperandASCII();                      // Name
            AddRemainingOperandsId();               // Interface
        }
        break;

        case Op::OpExecutionMode:
        {
            AddOperandId();                         // Entry Point
            ADD_OPERAND_ENUM(spv::ExecutionMode);   // Mode
            while (HasRemainingOperands())
                AddOperandLiteralExecutionMode(static_cast<spv::ExecutionMode>(inst.GetOperandUInt32(1)));
        }
        break;

        case Op::OpCapability:
        {
            ADD_OPERAND_ENUM(spv::Capability); // Capability
        }
        break;

        case Op::OpExecutionModeId:
        {
            AddOperandId();                         // Entry Point
            ADD_OPERAND_ENUM(spv::ExecutionMode);   // Mode
            while (HasRemainingOperands())
                AddOperandLiteralExecutionMode(static_cast<spv::ExecutionMode>(inst.GetOperandUInt32(1)));
        }
        break;

        case Op::OpTypeInt:
        {
            AddOperandLiteral();    // Width
            AddOperandLiteral();    // Signedness

            /* Store type */
            typesInt_[inst.result] = { inst.GetOperandUInt32(0), inst.GetOperandUInt32(1) };
        }
        break;

        case Op::OpTypeFloat:
        {
            AddOperandLiteral();    // Width

            /* Store type */
            typesFloat_[inst.result] = { inst.GetOperandUInt32(0) };
        }
        break;

        case Op::OpTypeVector:
        {
            AddOperandId();         // Component Type
            AddOperandLiteral();    // Component Count
        }
        break;

        case Op::OpTypeMatrix:
        {
            AddOperandId();         // Component Type
            AddOperandLiteral();    // Column Count
        }
        break;

        case Op::OpTypeImage:
        {
            AddOperandId();                             // Sampled Type
            ADD_OPERAND_ENUM(spv::Dim);                 // Dimension
            AddOperandLiteral();                        // Depth
            AddOperandLiteral();                        // Arrayed
            AddOperandLiteral();                        // Multi-sampled
            AddOperandLiteral();                        // Sampled
            ADD_OPERAND_ENUM(spv::ImageFormat);         // Image Format
            if (HasRemainingOperands())
                ADD_OPERAND_ENUM(spv::AccessQualifier); // Access Qualifier
        }
        break;

        case Op::OpTypeSampledImage:
        {
            AddOperandId(); // Image Type
        }
        break;

        case Op::OpTypeArray:
        {
            AddOperandId(); // Element Type
            AddOperandId(); // Length
        }
        break;

        case Op::OpTypeRuntimeArray:
        {
            AddOperandId(); // Element Type
        }
        break;

        case Op::OpTypeOpaque:
        {
            AddOperandASCII(); // Name of the opaque type
        }
        break;

        case Op::OpTypePointer:
        {
            ADD_OPERAND_ENUM(spv::StorageClass);    // Storage Class
            AddOperandId();                         // Type
        }
        break;

        case Op::OpTypeFunction:
        {
            AddOperandId();             // Return Type
            AddRemainingOperandsId();   // Parameter Types
        }
        break;

        case Op::OpTypePipe:
        {
            ADD_OPERAND_ENUM(spv::AccessQualifier); // Access Qualifier
        }
        break;

        case Op::OpTypeForwardPointer:
        {
            ADD_OPERAND_ENUM(spv::StorageClass); // Storage Class
        }
        break;

        case Op::OpConstant:
        case Op::OpSpecConstant:
        {
            std::string value;

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
                            AddOperandConstant<std::int16_t>(0, &value);
                            break;
                        case 32:
                            AddOperandConstant<std::int32_t>(0, &value);
                            break;
                        case 64:
                            value = std::to_string(static_cast<std::int64_t>(inst.GetOperandUInt64(0)));
                            prt.operands.push_back(value);
                            break;
                    }
                }
                else
                {
                    switch (type.width)
                    {
                        case 16:
                            AddOperandConstant<std::uint16_t>(0, &value);
                            break;
                        case 32:
                            AddOperandConstant<std::uint32_t>(0, &value);
                            break;
                        case 64:
                            value = std::to_string(inst.GetOperandUInt64(0));
                            prt.operands.push_back(value);
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
                            value = std::to_string(inst.GetOperandFloat16(0));
                            break;
                        case 32:
                            value = std::to_string(inst.GetOperandFloat32(0));
                            break;
                        case 64:
                            value = std::to_string(inst.GetOperandFloat64(0));
                            break;
                    }

                    prt.operands.push_back(value);

                    SkipOperands();
                }
            }

            /* Store constant value */
            SetConstant(inst.result, value);
        }
        break;

        case Op::OpConstantSampler:
        {
            ADD_OPERAND_ENUM(spv::SamplerAddressingMode);   // Sampling Addressing Mode
            AddOperandLiteral();                            // Param
            ADD_OPERAND_ENUM(spv::SamplerFilterMode);       // Sampler Filter Mode
        }
        break;

        case Op::OpSpecConstantOp:
        {
            AddOperandLiteral();        // Opcode
            AddRemainingOperandsId();   // Operands
        }
        break;

        case Op::OpVariable:
        {
            ADD_OPERAND_ENUM(spv::StorageClass);    // Storage Class
            if (HasRemainingOperands())
                AddOperandId();                     // Initializer
        }
        break;

        case Op::OpLoad:
        {
            AddOperandId();                                     // Pointer
            if (HasRemainingOperands())
                ADD_OPERAND_ENUM_FLAGS(spv::MemoryAccessMask);  // Memory Access
        }
        break;

        case Op::OpStore:
        {
            AddOperandId();                                     // Pointer
            AddOperandId();                                     // Object
            if (HasRemainingOperands())
                ADD_OPERAND_ENUM_FLAGS(spv::MemoryAccessMask);  // Memory Access
        }
        break;

        case Op::OpCopyMemory:
        {
            AddOperandId();                                     // Target
            AddOperandId();                                     // Source
            if (HasRemainingOperands())
                ADD_OPERAND_ENUM_FLAGS(spv::MemoryAccessMask);  // Memory Access
        }
        break;

        case Op::OpCopyMemorySized:
        {
            AddOperandId();                                     // Target
            AddOperandId();                                     // Source
            AddOperandId();                                     // Size
            if (HasRemainingOperands())
                ADD_OPERAND_ENUM_FLAGS(spv::MemoryAccessMask);  // Memory Access
        }
        break;

        case Op::OpArrayLength:
        {
            AddOperandId();         // Structure
            AddOperandLiteral();    // Array Member
        }
        break;

        case Op::OpGenericPtrMemSemantics:
        {
            AddOperandId(); // Pointer
        }
        break;

        case Op::OpFunction:
        {
            ADD_OPERAND_ENUM_FLAGS(spv::FunctionControlMask);   // Function Control
            AddOperandId();                                     // Function Type
        }
        break;

        case Op::OpImageSampleImplicitLod:
        case Op::OpImageSampleProjImplicitLod:
        case Op::OpImageFetch:
        case Op::OpImageRead:
        case Op::OpImageSparseSampleImplicitLod:
        case Op::OpImageSparseSampleProjImplicitLod:
        case Op::OpImageSparseFetch:
        case Op::OpImageSparseRead:
        {
            AddOperandId();                                     // Image
            AddOperandId();                                     // Coordinate
            if (HasRemainingOperands())
            {
                ADD_OPERAND_ENUM_FLAGS(spv::ImageOperandsMask); // Image Operands
                AddRemainingOperandsId();
            }
        }
        break;

        case Op::OpImageSampleExplicitLod:
        case Op::OpImageSampleProjExplicitLod:
        case Op::OpImageSparseSampleExplicitLod:
        case Op::OpImageSparseSampleProjExplicitLod:
        {
            AddOperandId();                                 // Image
            AddOperandId();                                 // Coordinate
            ADD_OPERAND_ENUM_FLAGS(spv::ImageOperandsMask); // Image Operands
            AddOperandId();
            AddRemainingOperandsId();
        }
        break;

        case Op::OpImageSampleDrefImplicitLod:
        case Op::OpImageSampleProjDrefImplicitLod:
        case Op::OpImageGather:
        case Op::OpImageDrefGather:
        case Op::OpImageWrite:
        case Op::OpImageSparseSampleDrefImplicitLod:
        case Op::OpImageSparseSampleProjDrefImplicitLod:
        case Op::OpImageSparseGather:
        case Op::OpImageSparseDrefGather:
        {
            AddOperandId();                                     // Image
            AddOperandId();                                     // Coordinate
            AddOperandId();                                     // D_ref/ Component/ Texel
            if (HasRemainingOperands())
            {
                ADD_OPERAND_ENUM_FLAGS(spv::ImageOperandsMask); // Image Operands
                AddRemainingOperandsId();
            }
        }
        break;

        case Op::OpImageSampleDrefExplicitLod:
        case Op::OpImageSampleProjDrefExplicitLod:
        case Op::OpImageSparseSampleDrefExplicitLod:
        case Op::OpImageSparseSampleProjDrefExplicitLod:
        {
            AddOperandId();                                 // Image
            AddOperandId();                                 // Coordinate
            AddOperandId();                                 // D_ref
            ADD_OPERAND_ENUM_FLAGS(spv::ImageOperandsMask); // Image Operands
            AddOperandId();
            AddRemainingOperandsId();
        }
        break;

        case Op::OpGenericCastToPtrExplicit:
        {
            AddOperandId();                         // Pointer
            ADD_OPERAND_ENUM(spv::StorageClass);    // Storage Class
        }
        break;

        case Op::OpVectorShuffle:
        {
            AddOperandId();                 // Vector 1
            AddOperandId();                 // Vector 2
            AddRemainingOperandsLiteral();  // Components
        }
        break;

        case Op::OpCompositeExtract:
        {
            AddOperandId();                 // Composite
            AddRemainingOperandsLiteral();  // Indexes
        }
        break;

        case Op::OpCompositeInsert:
        {
            AddOperandId();                 // Object
            AddOperandId();                 // Composite
            AddRemainingOperandsLiteral();  // Indexes
        }
        break;

        case Op::OpLoopMerge:
        {
            AddOperandId();                                 // Merge Block
            AddOperandId();                                 // Continue Target
            ADD_OPERAND_ENUM_FLAGS(spv::LoopControlMask);   // Loop Control
            AddRemainingOperandsLiteral();                  // Loop Control Parameters
        }
        break;

        case Op::OpSelectionMerge:
        {
            AddOperandId();                                     // Merge Block
            ADD_OPERAND_ENUM_FLAGS(spv::SelectionControlMask);  // Selection Control
        }
        break;

        case Op::OpBranchConditional:
        {
            AddOperandId();                 // Condition
            AddOperandId();                 // True Label
            AddOperandId();                 // False Label
            AddRemainingOperandsLiteral();  // Branch Weights
        }
        break;

        case Op::OpSwitch:
        {
            AddOperandId();                 // Selector
            AddOperandId();                 // Default
            while (HasRemainingOperands())
            {
                AddOperandLiteral();        // Target Case
                AddOperandId();             // Target Label
            }
        }
        break;

        case Op::OpLifetimeStart:
        case Op::OpLifetimeStop:
        {
            AddOperandId();         // Pointer
            AddOperandLiteral();    // Size
        }
        break;

        case Op::OpAtomicLoad:
        case Op::OpAtomicIIncrement:
        case Op::OpAtomicIDecrement:
        case Op::OpAtomicFlagTestAndSet:
        case Op::OpAtomicFlagClear:
        case Op::OpControlBarrier:
        case Op::OpMemoryNamedBarrier:
        {
            AddOperandId();                                     // Pointer/ Execution
            AddOperandId();                                     // Scope/ Memory
            ADD_OPERAND_ENUM_FLAGS(spv::MemorySemanticsMask);   // Semantics
        }
        break;

        case Op::OpAtomicStore:
        case Op::OpAtomicExchange:
        case Op::OpAtomicIAdd:
        case Op::OpAtomicISub:
        case Op::OpAtomicSMin:
        case Op::OpAtomicUMin:
        case Op::OpAtomicSMax:
        case Op::OpAtomicUMax:
        case Op::OpAtomicAnd:
        case Op::OpAtomicOr:
        case Op::OpAtomicXor:
        {
            AddOperandId();                                     // Pointer
            AddOperandId();                                     // Scope
            ADD_OPERAND_ENUM_FLAGS(spv::MemorySemanticsMask);   // Memory Semantics
            AddOperandId();                                     // Value
        }
        break;

        case Op::OpAtomicCompareExchange:
        case Op::OpAtomicCompareExchangeWeak:
        {
            AddOperandId();                                     // Pointer
            AddOperandId();                                     // Scope
            ADD_OPERAND_ENUM_FLAGS(spv::MemorySemanticsMask);   // Equal
            ADD_OPERAND_ENUM_FLAGS(spv::MemorySemanticsMask);   // Unequal
            AddOperandId();                                     // Value
            AddOperandId();                                     // Comparator
        }
        break;

        case Op::OpMemoryBarrier:
        {
            AddOperandId();                                     // Memory
            ADD_OPERAND_ENUM_FLAGS(spv::MemorySemanticsMask);   // Semantics
        }
        break;

        case Op::OpGroupIAdd:
        case Op::OpGroupFAdd:
        case Op::OpGroupFMin:
        case Op::OpGroupUMin:
        case Op::OpGroupSMin:
        case Op::OpGroupFMax:
        case Op::OpGroupUMax:
        case Op::OpGroupSMax:
        {
            AddOperandId();                         // Execution
            ADD_OPERAND_ENUM(spv::GroupOperation);  // Operation
            AddOperandId();                         // X
        }
        break;

        case Op::OpConstantPipeStorage:
        {
            AddRemainingOperandsLiteral(); // Packet Size, Packet Alignment, Capacity
        }
        break;

        default:
        break;
    }

    /* Append all remaining operands as ID numbers */
    AddRemainingOperandsId();
}

void SPIRVDisassembler::PrintAll(std::ostream& stream)
{
    static const std::size_t minResultLen       = 8;
    static const std::size_t minOpCodeLen       = 5;
    static const std::size_t minOperandListLen  = 8;

    /* Determine longest opcode name and operand list */
    std::size_t maxResultLen        = minResultLen;
    std::size_t maxOpCodeLen        = minOpCodeLen;
    std::size_t maxOperandListLen   = minOperandListLen;

    for (const auto& prt : printables_)
    {
        /* Get maximal result and opcode length */
        maxResultLen = std::max(maxResultLen, prt.result.size());
        maxOpCodeLen = std::max(maxOpCodeLen, prt.opCode.size());

        /* Get maximal operand list length */
        std::size_t len = 0;

        for (const auto& op : prt.operands)
            len += (op.size() + 1);

        maxOperandListLen = std::max(maxOperandListLen, len);
    }

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
            stream << "; Result     " << std::string(maxResultLen - minResultLen, ' ');
            stream << "OpCode" << std::string(maxOpCodeLen - minOpCodeLen, ' ');
            stream << "Operands" << std::string(maxOperandListLen - minOperandListLen, ' ');
            stream << "Offsets" << std::endl;

            stream << "; " << std::string(maxResultLen + 2, '-');
            stream << ' ' << std::string(maxOpCodeLen, '-');
            stream << ' ' << std::string(maxOperandListLen - 1, '-');
            stream << ' ' << std::string(12, '-') << std::endl;
        }
    }

    /* Write all printables out to stream */
    for (const auto& prt : printables_)
    {
        stream << "  ";

        /* Print result */
        if (!prt.result.empty())
        {
            {
                ScopedColor scopedColor(ColorFlags::Red | ColorFlags::Intens, stream);
                stream << std::string(maxResultLen - prt.result.size(), ' ') << prt.result;
            }
            stream << " = ";
        }
        else
            stream << std::string(maxResultLen + 3, ' ');

        /* Print op-code */
        {
            ScopedColor scopedColor(ColorFlags::Yellow | ColorFlags::Intens, stream);
            stream << prt.opCode;
            if (desc_.indentOperands)
                stream << std::string(maxOpCodeLen - prt.opCode.size(), ' ');
        }

        /* Print operands */
        std::size_t len = 0;

        for (const auto& op : prt.operands)
        {
            PrintOperand(stream, op);
            len += (op.size() + 1);
        }

        /* Print byte offset */
        if (desc_.showOffsets)
        {
            stream << std::string(maxOperandListLen - len, ' ');
            ScopedColor scopedColor(ColorFlags::Gray, stream);
            stream << " ; " << prt.offset << ' ';
        }

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

void SPIRVDisassembler::SetName(spv::Id id, const std::string& name)
{
    if (desc_.showNames && !name.empty())
        idNames_[id].name = name;
}

std::string SPIRVDisassembler::GetName(spv::Id id) const
{
    if (desc_.showNames)
    {
        auto it = idNames_.find(id);
        if (it != idNames_.end())
            return it->second.name;
    }
    return std::to_string(id);
}

void SPIRVDisassembler::SetMemberName(spv::Id id, std::uint32_t index, const std::string& name)
{
    if (desc_.showNames && !name.empty())
        idNames_[id].memberNames[index] = name;
}

std::string SPIRVDisassembler::GetMemberName(spv::Id id, std::uint32_t index) const
{
    if (desc_.showNames)
    {
        auto it = idNames_.find(id);
        if (it != idNames_.end())
        {
            auto itMember = it->second.memberNames.find(index);
            if (itMember != it->second.memberNames.end())
                return itMember->second;
        }
    }
    return std::to_string(id);
}

void SPIRVDisassembler::SetConstant(spv::Id id, const std::string& value)
{
    constants_[id] = value;
}

std::string SPIRVDisassembler::GetConstant(spv::Id id) const
{
    auto it = constants_.find(id);
    if (it != constants_.end())
        return it->second;
    else
        return "";
}

#undef ADD_OPERAND_ENUM
#undef ADD_OPERAND_ENUM_FLAGS


} // /namespace Xsc



// ================================================================================
