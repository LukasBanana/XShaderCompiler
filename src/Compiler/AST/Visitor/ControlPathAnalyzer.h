/*
 * ControlPathAnalyzer.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_CONTROL_PATH_ANALYZER_H
#define XSC_CONTROL_PATH_ANALYZER_H


#include "Visitor.h"
#include <stack>


namespace Xsc
{


/*
Control path analyzer (must implement visitors for all statements).
This helper class for the context analyzer marks all functions
where not all control paths return a value (if the function is declared to have a return value).
It also marks all statements as dead code, when they appear after a return path.
Marks 'FunctionDecl::hasNonReturnControlPath' and 'AST::isDeadCode' flags.
*/
class ControlPathAnalyzer : private Visitor
{
    
    public:
        
        void MarkControlPathsFromFunction(FunctionDecl& funcDecl);

    private:
        
        void PushReturnPath(bool returnPath);
        bool PopReturnPath();

        void VisitStmntList(const std::vector<StmntPtr>& stmnts);

        /* ----- Visitor implementation ----- */

        DECL_VISIT_PROC( CodeBlock         );

        DECL_VISIT_PROC( FunctionDecl      );
        DECL_VISIT_PROC( UniformBufferDecl );

        DECL_VISIT_PROC( BufferDeclStmnt   );
        DECL_VISIT_PROC( SamplerDeclStmnt  );
        DECL_VISIT_PROC( VarDeclStmnt      );
        DECL_VISIT_PROC( AliasDeclStmnt    );
        DECL_VISIT_PROC( BasicDeclStmnt    );

        DECL_VISIT_PROC( NullStmnt         );
        DECL_VISIT_PROC( CodeBlockStmnt    );
        DECL_VISIT_PROC( ForLoopStmnt      );
        DECL_VISIT_PROC( WhileLoopStmnt    );
        DECL_VISIT_PROC( DoWhileLoopStmnt  );
        DECL_VISIT_PROC( IfStmnt           );
        DECL_VISIT_PROC( ElseStmnt         );
        DECL_VISIT_PROC( SwitchStmnt       );
        DECL_VISIT_PROC( ExprStmnt         );
        DECL_VISIT_PROC( ReturnStmnt       );
        DECL_VISIT_PROC( CtrlTransferStmnt );

        /* === Members === */

        std::stack<bool> returnPathStack_;

};


} // /namespace Xsc


#endif



// ================================================================================