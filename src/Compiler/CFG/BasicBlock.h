/*
 * BasicBlock.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_BASIC_BLOCK_H
#define XSC_BASIC_BLOCK_H


#include <spirv/1.1/spirv.hpp11>
#include <vector>
#include <memory>


namespace Xsc
{


// A basic block represents a node in the control-flow-graph (CFG).
class BasicBlock
{

    public:

        /* ----- Structures ----- */

        struct Edge
        {
            Edge() = default;
            Edge(BasicBlock& succ, const std::string& label = "");

            bool operator == (const BasicBlock* rhs) const;
            bool operator != (const BasicBlock* rhs) const;

            inline BasicBlock* operator -> ()
            {
                return &succ;
            }

            inline const BasicBlock* operator -> () const
            {
                return &succ;
            }

            inline BasicBlock& operator * ()
            {
                return succ;
            }

            inline const BasicBlock& operator * () const
            {
                return succ;
            }

            BasicBlock& succ;
            std::string label;
        };

        using BlockList = std::vector<BasicBlock*>;
        using EdgeList = std::vector<Edge>;

        /* ----- Functions ----- */

        // Adds the specified 
        void AddSucc(BasicBlock& bb, const std::string& label = "");

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
        std::string             label;

        // SPIR-V instruction op-codes.
        std::vector<spv::Op>    opCodes;

    private:

        BlockList   pred_; // Predecessor nodes.
        EdgeList    succ_; // Successor nodes.

};


} // /namespace Xsc


#endif



// ================================================================================
