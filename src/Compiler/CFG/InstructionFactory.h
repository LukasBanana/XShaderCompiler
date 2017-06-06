/*
 * InstructionFactory.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_INSTRUCTION_FACTORY_H
#define XSC_INSTRUCTION_FACTORY_H


#include "Instruction.h"
#include <stack>
#include <set>
#include <cstdint>


namespace Xsc
{


class BasicBlock;

// CFG instruction factory class (SPIR-V encoded).
class InstructionFactory
{

    public:

        using Id = spv::Id;

        // Pushes the specified basic block onto the stack. Further instructions will be inserted into the top most basic block.
        void Push(BasicBlock* basicBlock);

        // Pop previous basic block from stack.
        void Pop();

        // Returns a new unique ID number.
        Id UniqueId();

        /* ----- Instruction creation functions ----- */

        void    MakeNop();
        Id      MakeUndefined(Id type);
        void    MakeName(Id id, const std::string& name);
        void    MakeMemberName(Id id, Id memberId, const std::string& name);

        Id      MakeTypeVoid();
        Id      MakeTypeBool();
        Id      MakeTypeSampler();
        Id      MakeTypeInt(std::uint32_t width);
        Id      MakeTypeUInt(std::uint32_t width);
        Id      MakeTypeFloat(std::uint32_t width);
        Id      MakeTypeStruct(const std::vector<Id>& memberTypes, const std::string& name);
        Id      MakeTypeVector(Id componentScalarType, std::uint32_t size);
        Id      MakeTypeMatrix(Id componentScalarType, std::uint32_t rows, std::uint32_t cols);
        Id      MakeTypeArray(Id elementType, std::uint32_t length);
        Id      MakeTypeRuntimeArray(Id elementType);
        Id      MakeTypeFunction(Id returnType, const std::vector<Id>& paramTypes);
        Id      MakeTypePointer(spv::StorageClass storageClass, Id subType);

        Id      MakeConstantInt16(std::int16_t value, bool uniqueInst = false);
        Id      MakeConstantUInt16(std::uint16_t value, bool uniqueInst = false);
        Id      MakeConstantInt32(std::int32_t value, bool uniqueInst = false);
        Id      MakeConstantUInt32(std::uint32_t value, bool uniqueInst = false);
        Id      MakeConstantInt64(std::int64_t value, bool uniqueInst = false);
        Id      MakeConstantUInt64(std::uint64_t value, bool uniqueInst = false);
        Id      MakeConstantFloat16(float value, bool uniqueInst = false);
        Id      MakeConstantFloat32(float value, bool uniqueInst = false);
        Id      MakeConstantFloat64(double value, bool uniqueInst = false);

    private:

        /* === Functions === */

        // Returns the active basic block, or throws an exception if the stack of basic blocks is empty.
        BasicBlock& BB();

        // Makes a new instruction and puts it into the active basic block (see 'BB()').
        Instruction& Put(const spv::Op opCode, const Id type = 0, const Id result = 0);

        // Makes a new instruction, pits it into the active basic block (see 'BB()'), and buffers it into the specified container.
        Instruction& PutAndBuffer(std::vector<Instruction*>& buffer, const spv::Op opCode, const Id type = 0, const Id result = 0);

        // Tries to find the instruction with the specified 32-bit constant.
        Instruction* FetchConstant(const spv::Op opCode, const Id type, std::uint32_t value0) const;

        // Tries to find the instruction with the specified 64-bit constant.
        Instruction* FetchConstant(const spv::Op opCode, const Id type, std::uint32_t value0, std::uint32_t value1) const;

        // Adds the specified capability to the capability set.
        void AddCapability(const spv::Capability cap);

        /* ----- Instruction creation functions ----- */

        Id MakeTypeIntPrimary(std::uint32_t width, bool sign);

        Id MakeConstantUInt32Primary(Id type, std::uint32_t value, bool uniqueInst);
        Id MakeConstantUInt64Primary(Id type, std::uint64_t value, bool uniqueInst);

        /* === Members === */

        // Stack of active basic blocks to put instructions into.
        std::stack<BasicBlock*>     basicBlockStack_;

        Id                          uniqueId_                   = 0;

        Instruction*                bufferedTypeVoidInstr_      = nullptr;
        Instruction*                bufferedTypeBoolInstr_      = nullptr;
        Instruction*                bufferedTypeSamplerInstr_   = nullptr;

        std::vector<Instruction*>   bufferedTypeInstrs_,
                                    bufferedConstantInstrs_;

        std::set<spv::Capability>   capabilites_;

};


} // /namespace Xsc


#endif



// ================================================================================
