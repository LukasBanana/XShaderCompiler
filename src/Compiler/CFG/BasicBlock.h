/*
 * BasicBlock.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_BASIC_BLOCK_H
#define XSC_BASIC_BLOCK_H


#include "Instruction.h"
#include <vector>
#include <memory>


namespace Xsc
{


// A basic block represents a node in the control-flow-graph (CFG).
class BasicBlock
{

    public:

        /* ----- Structures ----- */

        // Edge to the next basic block (successor).
        struct Edge
        {
            Edge(const Edge&) = default;
            Edge(BasicBlock* succ, const std::string& label = "");

            bool operator == (const BasicBlock* rhs) const;
            bool operator != (const BasicBlock* rhs) const;

            inline BasicBlock* operator -> ()
            {
                return succ;
            }

            inline const BasicBlock* operator -> () const
            {
                return succ;
            }

            inline BasicBlock& operator * ()
            {
                return *succ;
            }

            inline const BasicBlock& operator * () const
            {
                return *succ;
            }

            BasicBlock* succ;
            std::string label;
        };

        using BlockList = std::vector<BasicBlock*>;
        using EdgeList = std::vector<Edge>;

        /* ----- Functions ----- */

        // Adds the specified 
        void AddSucc(BasicBlock& bb, const std::string& label = "");

        // Removes the specified successor from this basic block and concatenates the next successors to this basic block.
        void RemoveSuccAndJoin(BasicBlock& bb);

        // Removes the specified successor from this basic block.
        void RemoveSucc(BasicBlock& bb);

        // Returns true if this basic block is a direct successor of the specified basic block.
        bool IsSuccOf(const BasicBlock& bb) const;

        // Returns true if this basic block is a direct predecessor of the specified basic block.
        bool IsPredOf(const BasicBlock& bb) const;

        // Returns the list of all predecessor nodes.
        inline const BlockList& Pred() const
        {
            return pred_;
        }

        // Returns the list of all successor nodes.
        inline const EdgeList& Succ() const
        {
            return succ_;
        }

        /* ----- Members ----- */

        // Basic block label (for debugging).
        std::string                 label;

        // SPIR-V instruction op-codes.
        std::vector<Instruction>    instructions;

    private:

        void ReplacePred(const BasicBlock& bb, BasicBlock* bbToReplace);

        void RemovePred(const BasicBlock& bb);

        void RemoveSucc(BasicBlock& bb, bool join);

        BlockList   pred_; // Predecessor nodes.
        EdgeList    succ_; // Successor nodes.

};


} // /namespace Xsc


#endif



// ================================================================================
