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


using namespace spv;

namespace Xsc
{


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


/*
 * ======= Private: =======
 */

BasicBlock& InstructionFactory::BB()
{
    if (basicBlockStack_.empty())
        RuntimeErr(R_NoActiveBasicBlock);
    return *(basicBlockStack_.top());
}

Instruction& InstructionFactory::Put(const Op opCode, const Id typeId, const Id resultId)
{
    return Put(opCode, {}, typeId, resultId);
}

Instruction& InstructionFactory::Put(const Op opCode, const std::initializer_list<Id>& operands, const Id typeId, const Id resultId)
{
    auto& bb = BB();

    Instruction inst(opCode, operands);
    {
        inst.type   = typeId;
        inst.result = resultId;
    }
    bb.instructions.push_back(inst);

    return bb.instructions.back();
}


} // /namespace Xsc



// ================================================================================
