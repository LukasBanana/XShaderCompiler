/*
 * ControlPathAnalyzer.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2018 by Lukas Hermanns)
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

        void VisitStmtList(const std::vector<StmtPtr>& stmts);

        /* ----- Visitor implementation ----- */

        DECL_VISIT_PROC( CodeBlock         );

        DECL_VISIT_PROC( FunctionDecl      );
        DECL_VISIT_PROC( UniformBufferDecl );

        DECL_VISIT_PROC( BufferDeclStmt    );
        DECL_VISIT_PROC( SamplerDeclStmt   );
        DECL_VISIT_PROC( VarDeclStmt       );
        DECL_VISIT_PROC( AliasDeclStmt     );
        DECL_VISIT_PROC( BasicDeclStmt     );

        DECL_VISIT_PROC( NullStmt          );
        DECL_VISIT_PROC( ScopeStmt         );
        DECL_VISIT_PROC( ForStmt           );
        DECL_VISIT_PROC( WhileStmt         );
        DECL_VISIT_PROC( DoWhileStmt       );
        DECL_VISIT_PROC( IfStmt            );
        DECL_VISIT_PROC( SwitchStmt        );
        DECL_VISIT_PROC( ExprStmt          );
        DECL_VISIT_PROC( ReturnStmt        );
        DECL_VISIT_PROC( JumpStmt          );

    private:

        std::stack<bool> returnPathStack_;

};


} // /namespace Xsc


#endif



// ================================================================================