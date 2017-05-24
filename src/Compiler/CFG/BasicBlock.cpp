/*
 * BasicBlock.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "BasicBlock.h"
#include <algorithm>


namespace Xsc
{


/*
 * Edge structure
 */

BasicBlock::Edge::Edge(BasicBlock* succ, const std::string& label) :
    succ  { succ  },
    label { label }
{
}

bool BasicBlock::Edge::operator == (const BasicBlock* rhs) const
{
    return (succ == rhs);
}

bool BasicBlock::Edge::operator != (const BasicBlock* rhs) const
{
    return (succ != rhs);
}


/*
 * BasicBlock class
 */

void BasicBlock::AddSucc(BasicBlock& bb, const std::string& label)
{
    /* Is block already a succesor to this basic block? */
    if (!IsPredOf(bb))
    {
        /* Add block to successors */
        succ_.push_back({ &bb, label });

        /* Add this block to the predecessors */
        bb.pred_.push_back(this);
    }
}

void BasicBlock::RemoveSuccAndJoin(BasicBlock& bb)
{
    RemoveSucc(bb, true);
}

void BasicBlock::RemoveSucc(BasicBlock& bb)
{
    RemoveSucc(bb, false);
}

bool BasicBlock::IsSuccOf(const BasicBlock& bb) const
{
    return (std::find(bb.succ_.begin(), bb.succ_.end(), this) != bb.succ_.end());
}

bool BasicBlock::IsPredOf(const BasicBlock& bb) const
{
    return (std::find(bb.pred_.begin(), bb.pred_.end(), this) != bb.pred_.end());
}


/*
 * ======= Private: =======
 */

void BasicBlock::ReplacePred(const BasicBlock& bb, BasicBlock* bbToReplace)
{
    auto it = std::find(pred_.begin(), pred_.end(), &bb);
    if (it != pred_.end())
        *it = bbToReplace;
}

void BasicBlock::RemovePred(const BasicBlock& bb)
{
    auto it = std::find(pred_.begin(), pred_.end(), &bb);
    if (it != pred_.end())
        pred_.erase(it);
}

void BasicBlock::RemoveSucc(BasicBlock& bb, bool join)
{
    /* Find block in successor list */
    auto it = std::find(succ_.begin(), succ_.end(), &bb);
    if (it != succ_.end())
    {
        /* Remove this block from the predecessor list of the input block */
        bb.RemovePred(*this);

        /* Remove block from the list */
        it = succ_.erase(it);

        if (join)
        {
            /* Add all successors of the input block and replace its predecessor by this block */
            for (auto& next : bb.succ_)
            {
                next->ReplacePred(bb, this);
                it = succ_.insert(it, next);
                ++it;
            }
        }
    }
}


} // /namespace Xsc



// ================================================================================
