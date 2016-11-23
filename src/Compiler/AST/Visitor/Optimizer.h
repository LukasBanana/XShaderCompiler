/*
 * Optimizer.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_OPTIMIZER_H
#define XSC_OPTIMIZER_H


#include "Visitor.h"
#include <vector>


namespace Xsc
{


// This AST optimizer supports only little optimizations such as null-statement removal.
class Optimizer : private Visitor
{
    
    public:
        
        // Optimizes the specified program AST.
        void Optimize(Program& program);

    private:
        
        void OptimizeStmntList(std::vector<StmntPtr>& stmnts);

        bool CanRemoveStmnt(const Stmnt& ast) const;

        /* ----- Visitor implementation ----- */

        DECL_VISIT_PROC( CodeBlock  );
        DECL_VISIT_PROC( SwitchCase );

};


} // /namespace Xsc


#endif



// ================================================================================