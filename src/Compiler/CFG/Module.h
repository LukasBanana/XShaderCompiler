/*
 * Module.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_MODULE_H
#define XSC_MODULE_H


#include "ModuleFunction.h"


namespace Xsc
{


// CFG module (SPIR-V encoded).
class Module
{

    public:

        // Makes a new module function with the specified name.
        ModuleFunction* MakeFunction(const std::string& name);

    private:

        std::vector<std::unique_ptr<ModuleFunction>> functions_;

};


} // /namespace Xsc


#endif



// ================================================================================
