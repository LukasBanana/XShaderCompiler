/*
 * ReferenceAnalyzer.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_REFERENCE_ANALYZER_H
#define XSC_REFERENCE_ANALYZER_H


#include "Visitor.h"
#include "Token.h"
#include "SymbolTable.h"


namespace Xsc
{


/*
Object reference analyzer.
This helper class for the context analyzer marks all functions
which are used from the beginning of the shader entry point.
All other functions will be removed from the code generation.
*/
class ReferenceAnalyzer : private Visitor
{
    
    public:
        
        // Marks all declarational AST nodes (i.e. function decl, structure decl etc.) that are reachable from the specififed entry point.
        void MarkReferencesFromEntryPoint(Program& program, const ShaderTarget shaderTarget);

    private:
        
        // Marks the specified AST node as reachable and returns false if the AST node has already been marked as reachable.
        bool Reachable(AST* ast);

        void VisitStmntList(const std::vector<StmntPtr>& stmnts);

        // Returns true if the specified variable is a paramter of the entry point.
        bool IsVariableAnEntryPointParameter(VarDeclStmnt* var) const;

        void PushFunctionDecl(FunctionDecl* funcDecl);
        void PopFunctionDecl();

        bool IsInsideEntryPoint() const;

        /* ----- Visitor implementation ----- */

        DECL_VISIT_PROC( CodeBlock         );
        DECL_VISIT_PROC( FunctionCall      );
        DECL_VISIT_PROC( SwitchCase        );
        DECL_VISIT_PROC( TypeName          );
        DECL_VISIT_PROC( VarIdent          );

        DECL_VISIT_PROC( VarDecl           );
        DECL_VISIT_PROC( StructDecl        );
        DECL_VISIT_PROC( BufferDecl        );

        DECL_VISIT_PROC( FunctionDecl      );
        DECL_VISIT_PROC( UniformBufferDecl );
        DECL_VISIT_PROC( BufferDeclStmnt   );

        DECL_VISIT_PROC( UnaryExpr         );
        DECL_VISIT_PROC( PostUnaryExpr     );
        DECL_VISIT_PROC( VarAccessExpr     );

        /* === Members === */

        Program*                    program_            = nullptr;
        ShaderTarget                shaderTarget_       = ShaderTarget::VertexShader;
        
        std::stack<FunctionDecl*>   funcDeclStack_;
        std::vector<FunctionCall*>  funcCallStack_;

};


} // /namespace Xsc


#endif



// ================================================================================