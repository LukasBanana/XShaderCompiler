/*
 * Optimizer.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_OPTIMIZER_H
#define XSC_OPTIMIZER_H


#include "Visitor.h"
#include <vector>


namespace Xsc
{


//TODO: replace this class by "ExprConverter".

// This AST optimizer supports only little optimizations such as null-statement removal.
class Optimizer : private Visitor
{

    public:

        // Optimizes the specified program AST.
        void Optimize(Program& program);

    private:

        void OptimizeStmntList(std::vector<StmntPtr>& stmnts);

        void OptimizeExpr(ExprPtr& expr);

        bool CanRemoveStmnt(const Stmnt& ast) const;

        /* ----- Visitor implementation ----- */

        DECL_VISIT_PROC( CodeBlock         );
        DECL_VISIT_PROC( SwitchCase        );
        DECL_VISIT_PROC( ArrayDimension    );

        DECL_VISIT_PROC( VarDecl           );

        DECL_VISIT_PROC( ForLoopStmnt      );
        DECL_VISIT_PROC( WhileLoopStmnt    );
        DECL_VISIT_PROC( DoWhileLoopStmnt  );
        DECL_VISIT_PROC( IfStmnt           );
        DECL_VISIT_PROC( SwitchStmnt       );
        DECL_VISIT_PROC( ExprStmnt         );
        DECL_VISIT_PROC( ReturnStmnt       );

        DECL_VISIT_PROC( SequenceExpr      );
        DECL_VISIT_PROC( TernaryExpr       );
        DECL_VISIT_PROC( BinaryExpr        );
        DECL_VISIT_PROC( UnaryExpr         );
        DECL_VISIT_PROC( PostUnaryExpr     );
        DECL_VISIT_PROC( BracketExpr       );
        DECL_VISIT_PROC( ObjectExpr        );
        DECL_VISIT_PROC( AssignExpr        );
        DECL_VISIT_PROC( ArrayExpr         );
        DECL_VISIT_PROC( CastExpr          );
        DECL_VISIT_PROC( InitializerExpr   );

};


} // /namespace Xsc


#endif



// ================================================================================