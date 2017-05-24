/*
 * InstructionFactory.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "InstructionFactory.h"
#include "BasicBlock.h"
#include "ReportIdents.h"
#include "Exception.h"
#include "Helper.h"
#include "Float16Compressor.h"


using namespace spv;

namespace Xsc
{


static Instruction* FindInstruction(const std::vector<Instruction*>& instrList, const Op opCode, const std::vector<std::uint32_t>& operands)
{
    for (auto inst : instrList)
    {
        if (inst->opCode == opCode && inst->EqualsOperands(operands))
            return inst;
    }
    return nullptr;
}

void InstructionFactory::Push(BasicBlock* basicBlock)
{
    basicBlockStack_.push(basicBlock);
}

void InstructionFactory::Pop()
{
    if (basicBlockStack_.empty())
        RuntimeErr(R_StackUnderflow(R_InstructionFactory));
    else
        basicBlockStack_.pop();
}

Id InstructionFactory::UniqueId()
{
    return (++uniqueId_);
}

/* ----- Instruction creation functions ----- */

void InstructionFactory::MakeNop()
{
    Put(Op::OpNop);
}

Id InstructionFactory::MakeUndefined(Id type)
{
    return Put(Op::OpUndef, type, UniqueId()).result;
}

void InstructionFactory::MakeName(Id id, const std::string& name)
{
    Put(Op::OpName)
        .AddOperandUInt32(id)
        .AddOperandASCII(name)
    ;
}

void InstructionFactory::MakeMemberName(Id id, Id memberId, const std::string& name)
{
    Put(Op::OpName)
        .AddOperandUInt32(id)
        .AddOperandUInt32(memberId)
        .AddOperandASCII(name)
    ;
}

Id InstructionFactory::MakeTypeVoid()
{
    if (!bufferedTypeVoidInstr_)
        bufferedTypeVoidInstr_ = &(Put(Op::OpTypeVoid, 0, UniqueId()));
    return bufferedTypeVoidInstr_->result;
}

Id InstructionFactory::MakeTypeBool()
{
    if (!bufferedTypeBoolInstr_)
        bufferedTypeBoolInstr_ = &(Put(Op::OpTypeBool, 0, UniqueId()));
    return bufferedTypeBoolInstr_->result;
}

Id InstructionFactory::MakeTypeSampler()
{
    if (!bufferedTypeSamplerInstr_)
        bufferedTypeSamplerInstr_ = &(Put(Op::OpTypeSampler, 0, UniqueId()));
    return bufferedTypeSamplerInstr_->result;
}

Id InstructionFactory::MakeTypeInt(std::uint32_t width)
{
    return MakeTypeIntPrimary(width, true);
}

Id InstructionFactory::MakeTypeUInt(std::uint32_t width)
{
    return MakeTypeIntPrimary(width, false);
}

Id InstructionFactory::MakeTypeFloat(std::uint32_t width)
{
    /* If type was already created, return its result ID */
    if (auto inst = FindInstruction(bufferedTypeInstrs_, Op::OpTypeFloat, { width }))
        return inst->result;

    /* Otherwise, create and buffer new type instruction */
    auto& inst = PutAndBuffer(bufferedTypeInstrs_, Op::OpTypeFloat, 0, UniqueId())
        .AddOperandUInt32(width)
    ;

    /* Store required capabilites */
    if (width == 16)
        AddCapability(Capability::Float16);
    else if (width == 64)
        AddCapability(Capability::Float64);

    return inst.result;
}

Id InstructionFactory::MakeTypeStruct(const std::vector<Id>& memberTypes, const std::string& name)
{
    /* Always create new struct type with all members */
    auto& inst = Put(Op::OpTypeStruct, 0, UniqueId());

    for (auto id : memberTypes)
        inst.AddOperandUInt32(id);

    /* Add name to type */
    MakeName(inst.result, name);

    return inst.result;
}

Id InstructionFactory::MakeTypeVector(Id componentScalarType, std::uint32_t size)
{
    /* If type was already created, return its result ID */
    if (auto inst = FindInstruction(bufferedTypeInstrs_, Op::OpTypeVector, { componentScalarType, size }))
        return inst->result;

    /* Otherwise, create and buffer new type instruction */
    return PutAndBuffer(bufferedTypeInstrs_, Op::OpTypeVector, 0, UniqueId())
        .AddOperandUInt32(componentScalarType)
        .AddOperandUInt32(size)
        .result
    ;
}

Id InstructionFactory::MakeTypeMatrix(Id componentScalarType, std::uint32_t rows, std::uint32_t cols)
{
    /* Make column vector type */
    auto columnType = MakeTypeVector(componentScalarType, rows);

    /* If type was already created, return its result ID */
    if (auto inst = FindInstruction(bufferedTypeInstrs_, Op::OpTypeMatrix, { columnType, cols }))
        return inst->result;

    /* Otherwise, create and buffer new type instruction */
    return PutAndBuffer(bufferedTypeInstrs_, Op::OpTypeMatrix, 0, UniqueId())
        .AddOperandUInt32(columnType)
        .AddOperandUInt32(cols)
        .result
    ;
}

Id InstructionFactory::MakeTypeArray(Id elementType, std::uint32_t length)
{
    /* If type was already created, return its result ID */
    if (auto inst = FindInstruction(bufferedTypeInstrs_, Op::OpTypeArray, { elementType, length }))
        return inst->result;

    /* Otherwise, create and buffer new type instruction */
    return PutAndBuffer(bufferedTypeInstrs_, Op::OpTypeArray, 0, UniqueId())
        .AddOperandUInt32(elementType)
        .AddOperandUInt32(length)
        .result
    ;
}

Id InstructionFactory::MakeTypeRuntimeArray(Id elementType)
{
    /* If type was already created, return its result ID */
    if (auto inst = FindInstruction(bufferedTypeInstrs_, Op::OpTypeRuntimeArray, { elementType }))
        return inst->result;

    /* Otherwise, create and buffer new type instruction */
    return PutAndBuffer(bufferedTypeInstrs_, Op::OpTypeRuntimeArray, 0, UniqueId())
        .AddOperandUInt32(elementType)
        .result
    ;
}

Id InstructionFactory::MakeTypeFunction(Id returnType, const std::vector<Id>& paramTypes)
{
    /* If type was already created, return its result ID */
    for (auto inst : bufferedTypeInstrs_)
    {
        if ( inst->opCode              == Op::OpTypeFunction &&
             inst->GetOperandUInt32(0) == returnType         &&
             inst->EqualsOperands(paramTypes, 1) )
        {
            return inst->result;
        }
    }

    /* Always create new struct type with all members */
    auto& inst = Put(Op::OpTypeFunction, 0, UniqueId());

    inst.AddOperandUInt32(returnType);
    for (auto id : paramTypes)
        inst.AddOperandUInt32(id);

    return inst.result;
}

Id InstructionFactory::MakeTypePointer(spv::StorageClass storageClass, Id subType)
{
    const auto storageClassWord = static_cast<std::uint32_t>(storageClass);

    /* If type was already created, return its result ID */
    if (auto inst = FindInstruction(bufferedTypeInstrs_, Op::OpTypePointer, { storageClassWord, subType }))
        return inst->result;

    /* Otherwise, create and buffer new type instruction */
    return PutAndBuffer(bufferedTypeInstrs_, Op::OpTypePointer, 0, UniqueId())
        .AddOperandUInt32(storageClassWord)
        .AddOperandUInt32(subType)
        .result
    ;
}

Id InstructionFactory::MakeConstantInt16(std::int16_t value, bool uniqueInst)
{
    return MakeConstantUInt32Primary(MakeTypeInt(16), static_cast<std::uint32_t>(static_cast<std::int32_t>(value)), uniqueInst);
}

Id InstructionFactory::MakeConstantUInt16(std::uint16_t value, bool uniqueInst)
{
    return MakeConstantUInt32Primary(MakeTypeUInt(16), static_cast<std::uint32_t>(value), uniqueInst);
}

Id InstructionFactory::MakeConstantInt32(std::int32_t value, bool uniqueInst)
{
    return MakeConstantUInt32Primary(MakeTypeInt(32), static_cast<std::uint32_t>(value), uniqueInst);
}

Id InstructionFactory::MakeConstantUInt32(std::uint32_t value, bool uniqueInst)
{
    return MakeConstantUInt32Primary(MakeTypeUInt(32), value, uniqueInst);
}

Id InstructionFactory::MakeConstantInt64(std::int64_t value, bool uniqueInst)
{
    return MakeConstantUInt64Primary(MakeTypeInt(65), static_cast<std::uint64_t>(value), uniqueInst);
}

Id InstructionFactory::MakeConstantUInt64(std::uint64_t value, bool uniqueInst)
{
    return MakeConstantUInt64Primary(MakeTypeUInt(65), value, uniqueInst);
}

Id InstructionFactory::MakeConstantFloat16(float value, bool uniqueInst)
{
    /* Generate opcode and type */
    auto opCode = (uniqueInst ? Op::OpSpecConstant : Op::OpConstant);
    auto type   = MakeTypeFloat(16);

    /* Convert float to bitmask */
    auto f16 = CompressFloat16(value);

    /* Check if this constant is already defined */
    if (!uniqueInst)
    {
        if (auto inst = FetchConstant(opCode, type, f16))
            return inst->result;
    }

    /* Create new constant instruction */
    return PutAndBuffer(bufferedConstantInstrs_, opCode, type, UniqueId())
        .AddOperandUInt32(f16)
        .result
    ;
}

Id InstructionFactory::MakeConstantFloat32(float value, bool uniqueInst)
{
    /* Generate opcode and type */
    auto opCode = (uniqueInst ? Op::OpSpecConstant : Op::OpConstant);
    auto type   = MakeTypeFloat(32);

    /* Convert float to bitmask */
    union
    {
        std::uint32_t   ui32;
        float           f32;
    }
    data;

    data.f32 = value;

    /* Check if this constant is already defined */
    if (!uniqueInst)
    {
        if (auto inst = FetchConstant(opCode, type, data.ui32))
            return inst->result;
    }

    /* Create new constant instruction */
    return PutAndBuffer(bufferedConstantInstrs_, opCode, type, UniqueId())
        .AddOperandUInt32(data.ui32)
        .result
    ;
}

Id InstructionFactory::MakeConstantFloat64(double value, bool uniqueInst)
{
    /* Generate opcode and type */
    auto opCode = (uniqueInst ? Op::OpSpecConstant : Op::OpConstant);
    auto type   = MakeTypeFloat(64);

    /* Convert float to bitmask */
    union
    {
        std::uint32_t   ui32[2];
        double          f64;
    }
    data;

    data.f64 = value;

    /* Check if this constant is already defined */
    if (!uniqueInst)
    {
        if (auto inst = FetchConstant(opCode, type, data.ui32[0], data.ui32[1]))
            return inst->result;
    }

    /* Create new constant instruction */
    return PutAndBuffer(bufferedConstantInstrs_, opCode, type, UniqueId())
        .AddOperandUInt32(data.ui32[0])
        .AddOperandUInt32(data.ui32[1])
        .result
    ;
}


/*
 * ======= Private: =======
 */

BasicBlock& InstructionFactory::BB()
{
    if (basicBlockStack_.empty())
        RuntimeErr(R_NoActiveBasicBlock);
    return *(basicBlockStack_.top());
}

Instruction& InstructionFactory::Put(const Op opCode, const Id type, const Id result)
{
    auto& bb = BB();

    Instruction inst(opCode);
    {
        inst.type   = type;
        inst.result = result;
    }
    bb.instructions.push_back(inst);

    return bb.instructions.back();
}

Instruction& InstructionFactory::PutAndBuffer(std::vector<Instruction*>& buffer, const Op opCode, const Id type, const Id result)
{
    auto& inst = Put(opCode, type, result);
    buffer.push_back(&inst);
    return inst;
}

Instruction* InstructionFactory::FetchConstant(const Op opCode, const Id type, std::uint32_t value0) const
{
    for (auto inst : bufferedConstantInstrs_)
    {
        if ( inst->opCode              == opCode &&
             inst->type                == type   &&
             inst->NumOperands()       == 1      &&
             inst->GetOperandUInt32(0) == value0 )
        {
            return inst;
        }
    }
    return nullptr;
}

Instruction* InstructionFactory::FetchConstant(const Op opCode, const Id type, std::uint32_t value0, std::uint32_t value1) const
{
    for (auto inst : bufferedConstantInstrs_)
    {
        if ( inst->opCode              == opCode &&
             inst->type                == type   &&
             inst->NumOperands()       == 2      &&
             inst->GetOperandUInt32(0) == value0 &&
             inst->GetOperandUInt32(1) == value1 )
        {
            return inst;
        }
    }
    return nullptr;
}

void InstructionFactory::AddCapability(const Capability cap)
{
    capabilites_.insert(cap);
}

/* ----- Instruction creation functions ----- */

Id InstructionFactory::MakeTypeIntPrimary(std::uint32_t width, bool sign)
{
    /* If type was already created, return its result ID */
    const std::uint32_t signWord = (sign ? 1 : 0);

    if (auto inst = FindInstruction(bufferedTypeInstrs_, Op::OpTypeInt, { width, signWord }))
        return inst->result;

    /* Otherwise, create and buffer new type instruction */
    auto& inst = PutAndBuffer(bufferedTypeInstrs_, Op::OpTypeInt, 0, UniqueId())
        .AddOperandUInt32(width)
        .AddOperandUInt32(signWord)
    ;

    /* Store required capabilites */
    if (width == 16)
        AddCapability(Capability::Int16);
    else if (width == 64)
        AddCapability(Capability::Int64);

    return inst.result;
}

Id InstructionFactory::MakeConstantUInt32Primary(Id type, std::uint32_t value, bool uniqueInst)
{
    /* Generate opcode */
    auto opCode = (uniqueInst ? Op::OpSpecConstant : Op::OpConstant);

    /* Check if this constant is already defined */
    if (!uniqueInst)
    {
        if (auto inst = FetchConstant(opCode, type, value))
            return inst->result;
    }

    /* Create new constant instruction */
    return PutAndBuffer(bufferedConstantInstrs_, opCode, type, UniqueId())
        .AddOperandUInt32(value)
        .result
    ;
}

Id InstructionFactory::MakeConstantUInt64Primary(Id type, std::uint64_t value, bool uniqueInst)
{
    /* Generate opcode */
    auto opCode = (uniqueInst ? Op::OpSpecConstant : Op::OpConstant);

    /* Convert float to bitmask */
    union
    {
        std::uint32_t ui32[2];
        std::uint64_t ui64;
    }
    data;

    data.ui64 = value;

    /* Check if this constant is already defined */
    if (!uniqueInst)
    {
        if (auto inst = FetchConstant(opCode, type, data.ui32[0], data.ui32[1]))
            return inst->result;
    }

    /* Create new constant instruction */
    return PutAndBuffer(bufferedConstantInstrs_, opCode, type, UniqueId())
        .AddOperandUInt32(data.ui32[0])
        .AddOperandUInt32(data.ui32[1])
        .result
    ;
}


} // /namespace Xsc



// ================================================================================
