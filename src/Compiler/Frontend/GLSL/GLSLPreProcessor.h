/*
 * GLSLPreProcessor.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_GLSL_PRE_PROCESSOR_H
#define XSC_GLSL_PRE_PROCESSOR_H


#include "PreProcessor.h"


namespace Xsc
{


/*
Pre-processor to substitute macros and include directives.
The preprocessor works on something similar to a Concrete Syntax Tree (CST) rather than an Abstract Syntax Tree (AST).
This is because the output is not an intermediate representation but rather concrete source code.
Therefore, all white spaces and new-line characters must NOT be ignored.
All other parsers and analyzers only work on an AST.
*/
class GLSLPreProcessor : public PreProcessor
{
    
    public:
        
        GLSLPreProcessor(IncludeHandler& includeHandler, Log* log = nullptr);

    private:
        
        bool OnDefineMacro(const Macro& macro) override;
        bool OnRedefineMacro(const Macro& macro, const Macro& previousMacro) override;
        bool OnUndefineMacro(const Macro& macro) override;

        void ParseDirective(const std::string& directive, bool ignoreUnknown) override;

        void ParseDirectiveVersion();
        void ParseDirectiveExtension();

        /* === Members === */

        int versionNo_ = 0;

};


} // /namespace Xsc


#endif



// ================================================================================