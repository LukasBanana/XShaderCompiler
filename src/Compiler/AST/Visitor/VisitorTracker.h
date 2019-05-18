/*
 * VisitorTracker.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_VISITOR_TRACKER_H
#define XSC_VISITOR_TRACKER_H


#include "Visitor.h"
#include "Flags.h"
#include <vector>
#include <stack>


namespace Xsc
{


// Extended visitor class with AST tracking functions.
class VisitorTracker : public Visitor
{

    protected:

        struct ExprFlags
        {
            Expr* expr = nullptr;
            Flags flags;
        };

    protected:

        /* ----- Global scope tracker ----- */

        bool InsideGlobalScope() const;

        /* ----- Function declaration tracker ----- */

        void PushFunctionDecl(FunctionDecl* funcDecl);
        void PopFunctionDecl();

        // Returns true if the visitor is currently inside a function declaration.
        bool InsideFunctionDecl() const;

        // Returns true if the visitor is currently inside the main entry point.
        bool InsideEntryPoint() const;

        // Returns true if the visitor is currently inside the secondary entry point.
        bool InsideSecondaryEntryPoint() const;

        // Returns the active (inner most) function declaration or null if the analyzer is currently not inside a function declaration.
        FunctionDecl* ActiveFunctionDecl() const;

        // Returns the structure the active (inner most) member function declaration belongs to or null if no such structure exists.
        StructDecl* ActiveFunctionStructDecl() const;

        /* ----- Call expression tracker ----- */

        void PushCallExpr(CallExpr* callExpr);
        void PopCallExpr();

        // Returns the active (inner most) call expression or null if the visitor is currently not inside a function call.
        CallExpr* ActiveCallExpr() const;

        /* ----- Expression flags tracker ----- */

        void PushExprFlags(Expr* expr, const Flags& flags);
        void PopExprFlags();

        // Returns the active (inner most) l-value expression or null (can be AssignExpr or UnaryExpr).
        ExprFlags ActiveExprFlags() const;

        /* ----- Structure declaration tracker ----- */

        void PushStructDecl(StructDecl* structDecl);
        void PopStructDecl();

        // Returns true if the analyzer is currently inside a structure declaration.
        bool InsideStructDecl() const;

        // Returns the active (inner most) structure declaration or null if the visitor is currently not inside a structure declaration.
        StructDecl* ActiveStructDecl() const;

        // Returns the stack (or rather the list) of all current, nested structure declarations.
        inline const std::vector<StructDecl*>& GetStructDeclStack() const
        {
            return structDeclStack_;
        }

        /* ----- Uniform buffer declaration tracker ----- */

        void PushUniformBufferDecl(UniformBufferDecl* uniformBufferDecl);
        void PopUniformBufferDecl();

        // Returns true if the analyzer is currently inside a uniform buffer declaration.
        bool InsideUniformBufferDecl() const;

        // Returns the stack (or rather the list) of all current, nested structure declarations.
        inline const std::vector<UniformBufferDecl*>& GetUniformBufferDeclStack() const
        {
            return uniformBufferDeclStack_;
        }

        /* ----- Variable declaration statement tracker ----- */

        void PushVarDeclStmt(VarDeclStmt* varDeclStmt);
        void PopVarDeclStmt();

        // Returns true if the visitor is currently inside a variable declaration statement.
        bool InsideVarDeclStmt() const;

        // Returns the active (inner most) variable declaration statement.
        VarDeclStmt* ActiveVarDeclStmt() const;

        /* ----- Alias declaration statement tracker ----- */

        void PushAliasDeclStmt(AliasDeclStmt* aliasDeclStmt);
        void PopAliasDeclStmt();

        // Returns true if the visitor is currently inside an alias declaration statement.
        bool InsideAliasDeclStmt() const;

        // Returns the active (inner most) alias declaration statement.
        AliasDeclStmt* ActiveAliasDeclStmt() const;

    private:

        // Function declaration stack.
        std::stack<FunctionDecl*>       funcDeclStack_;

        // Call expression stack to join arguments with its function call.
        std::stack<CallExpr*>           callExprStack_;

        // Stack of expressions with flags.
        std::stack<ExprFlags>           epxrFlagsStack_;

        // Structure stack to collect all members with system value semantic (SV_...), and detect all nested structures.
        std::vector<StructDecl*>        structDeclStack_;

        // Uniform buffer declaration stack.
        std::vector<UniformBufferDecl*> uniformBufferDeclStack_;

        // Variable declaration stack.
        std::stack<VarDeclStmt*>        varDeclStmtStack_;

        // Alias declaration stack.
        std::stack<AliasDeclStmt*>      aliasDeclStmtStack_;

        // Function declaration level of the main entry point.
        std::size_t                     stackLevelOfEntryPoint_     = ~0;

        // Function declaration level of the secondary entry point.
        std::size_t                     stackLevelOf2ndEntryPoint_  = ~0;

};

#undef VISITOR_VISIT_PROC


} // /namespace Xsc


#endif



// ================================================================================
