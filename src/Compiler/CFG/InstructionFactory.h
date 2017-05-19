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


namespace Xsc
{


class BasicBlock;

// CFG instruction class (SPIR-V encoded).
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

    private:

        /* === Functions === */

        // Returns the active basic block, or throws an exception if the stack of basic blocks is empty.
        BasicBlock& BB();

        // Makes a new instruction and puts it into the active basic block (see 'BB()').
        Instruction& Put(const spv::Op opCode, const Id type = 0, const Id result = 0);
        Instruction& Put(const spv::Op opCode, const std::initializer_list<Id>& operands, const Id type = 0, const Id result = 0);

        std::stack<BasicBlock*> basicBlockStack_; // Stack of active basic blocks to put instructions into.

        /* === Members === */

        Id uniqueId_ = 0;

};


} // /namespace Xsc


#endif



// ================================================================================
