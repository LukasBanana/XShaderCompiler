/*
 * StructParameterAnalyzer.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_STRUCT_PARAMETER_ANALYZER_H
#define XSC_STRUCT_PARAMETER_ANALYZER_H


#include "VisitorTracker.h"
#include <Xsc/Targets.h>
#include <set>


namespace Xsc
{


/*
Structure parameter analyzer.
This is a helper class for the context analyzer to determine which
structures are used for another reason than entry-point parameters.
*/
class StructParameterAnalyzer : private VisitorTracker
{

    public:

        // Marks all declarational AST nodes (i.e. function decl, structure decl etc.) that are reachable from the specififed entry point.
        void MarkStructsFromEntryPoint(Program& program, const ShaderTarget shaderTarget);

        // Marks all declarational AST nodes (i.e. function decl, structure decl etc.) that are reachable from the specififed entry point.
        void MarkStructsFromEntryPoint(FunctionDecl& funcDecl, const ShaderTarget shaderTarget = ShaderTarget::Undefined);

    private:

        // Returns true if the specified AST has not yet been visited.
        bool NotVisited(const AST* ast);

        void VisitStmtList(const std::vector<StmtPtr>& stmts);

        // Returns true if the specified variable is a paramter of the entry point.
        bool IsVariableAnEntryPointParameter(VarDeclStmt* var) const;

        // Returns true if the active function declaration is the main entry point.
        bool IsActiveFunctionDeclEntryPoint() const;

        /* ----- Visitor implementation ----- */

        DECL_VISIT_PROC( CodeBlock         );
        DECL_VISIT_PROC( SwitchCase        );
        DECL_VISIT_PROC( TypeSpecifier     );

        DECL_VISIT_PROC( VarDecl           );
        DECL_VISIT_PROC( StructDecl        );
        DECL_VISIT_PROC( BufferDecl        );

        DECL_VISIT_PROC( FunctionDecl      );
        DECL_VISIT_PROC( UniformBufferDecl );
        DECL_VISIT_PROC( BufferDeclStmt    );

        DECL_VISIT_PROC( CallExpr          );
        DECL_VISIT_PROC( ObjectExpr        );

    private:

        FunctionDecl*           entryPoint_     = nullptr;
        ShaderTarget            shaderTarget_   = ShaderTarget::VertexShader;

        std::set<const AST*>    visitSet_;

};


} // /namespace Xsc


#endif



// ================================================================================
