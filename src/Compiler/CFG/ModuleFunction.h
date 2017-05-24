/*
 * ModuleFunction.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_MODULE_FUNCTION_H
#define XSC_MODULE_FUNCTION_H


#include "BasicBlock.h"


namespace Xsc
{


class Module;

// CFG module function class (SPIR-V encoded).
class ModuleFunction
{

    public:

        ModuleFunction(Module& parent, const std::string& name);

        // Makes a new basic block.
        BasicBlock* MakeBlock(const std::string& label = "");

        // Returns a reference to the parent module object of this function.
        inline Module& Parent() const
        {
            return parent_;
        }

        // Returns the name of this function (including name mangling).
        inline const std::string& Name() const
        {
            return name_;
        }

    private:

        Module&                                     parent_;

        std::string                                 name_;
        std::vector<std::unique_ptr<BasicBlock>>    basicBlocks_;

};


} // /namespace Xsc


#endif



// ================================================================================
