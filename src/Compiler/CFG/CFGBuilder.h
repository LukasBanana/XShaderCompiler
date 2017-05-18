/*
 * CFGBuilder.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_CFG_BUILDER_H
#define XSC_CFG_BUILDER_H


#include "VisitorTracker.h"
#include "Module.h"


namespace Xsc
{


// CFG builder class.
class CFGBuilder : public VisitorTracker
{
    
    public:
        
        //CFGBuilder(Log* log);

    private:
        
        struct CFG
        {
            BasicBlock* in;
            BasicBlock* out;
        };

        /* === Functions === */

        // Makes a CFG with an input and output basic block.
        CFG MakeCFG(const std::string& name);

        // Pushes the specified input/output basic blocks to the CFG stack.
        void PushCFG(const CFG& cfg);
        
        // Pops and returns the top most input/output basic blocks from the CFG stack.
        CFG PopCFG();

        // Pushes the specified basic block to the break block stack (e.g. "endif", "endfor" etc.).
        void PushBreak(BasicBlock* bb);

        // Pops the basic block from the break block stack.
        void PopBreak();

        // Returns the basic block from the break block stack.
        BasicBlock* TopBreak() const;

        /* --- Visitor implementation --- */

        DECL_VISIT_PROC( Program           );
        DECL_VISIT_PROC( CodeBlock         );
        DECL_VISIT_PROC( SwitchCase        );
        DECL_VISIT_PROC( ArrayDimension    );
        DECL_VISIT_PROC( TypeSpecifier     );

        DECL_VISIT_PROC( VarDecl           );
        DECL_VISIT_PROC( StructDecl        );
        DECL_VISIT_PROC( SamplerDecl       );

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

        DECL_VISIT_PROC( SequenceExpr      );
        DECL_VISIT_PROC( LiteralExpr       );
        DECL_VISIT_PROC( TypeSpecifierExpr );
        DECL_VISIT_PROC( TernaryExpr       );
        DECL_VISIT_PROC( BinaryExpr        );
        DECL_VISIT_PROC( UnaryExpr         );
        DECL_VISIT_PROC( PostUnaryExpr     );
        DECL_VISIT_PROC( CallExpr          );
        DECL_VISIT_PROC( BracketExpr       );
        DECL_VISIT_PROC( ObjectExpr        );
        DECL_VISIT_PROC( AssignExpr        );
        DECL_VISIT_PROC( ArrayExpr         );
        DECL_VISIT_PROC( CastExpr          );
        DECL_VISIT_PROC( InitializerExpr   );

        /* --- Helper functions for CFG builder --- */



        /* === Members === */

        Module                  module_;
        ModuleFunction*         moduleFunc_         = nullptr;  // Active module function.

        std::stack<CFG>         cfgStack_;
        std::stack<BasicBlock*> breakBlockStack_;

};


} // /namespace Xsc


#endif



// ================================================================================
