/*
 * ReferenceAnalyzer.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_REFERENCE_ANALYZER_H
#define XSC_REFERENCE_ANALYZER_H


#include "VisitorTracker.h"
#include "Token.h"
#include "SymbolTable.h"
#include <Xsc/Targets.h>


namespace Xsc
{


/*
Object reference analyzer.
This helper class for the context analyzer marks all functions
which are used (or rather referenced) from the beginning of the shader entry point.
All other functions will be ignored by the code generator.
*/
class ReferenceAnalyzer : private VisitorTracker
{
    
    public:
        
        // Marks all declarational AST nodes (i.e. function decl, structure decl etc.) that are reachable from the specififed entry point.
        void MarkReferencesFromEntryPoint(Program& program, const ShaderTarget shaderTarget);

    private:
        
        // Marks the specified AST node as reachable and returns false if the AST node has already been marked as reachable.
        bool Reachable(AST* ast);

        void VisitStmntList(const std::vector<StmntPtr>& stmnts);

        void MarkLValueExpr(const Expr* expr);
        void MarkLValueExprObject(const ObjectExpr* objectExpr);

        /* ----- Visitor implementation ----- */

        DECL_VISIT_PROC( CodeBlock         );
        DECL_VISIT_PROC( SwitchCase        );
        DECL_VISIT_PROC( TypeSpecifier     );

        DECL_VISIT_PROC( VarDecl           );
        DECL_VISIT_PROC( StructDecl        );
        DECL_VISIT_PROC( BufferDecl        );
        DECL_VISIT_PROC( SamplerDecl       );

        DECL_VISIT_PROC( FunctionDecl      );
        DECL_VISIT_PROC( UniformBufferDecl );
        DECL_VISIT_PROC( BufferDeclStmnt   );
        DECL_VISIT_PROC( SamplerDeclStmnt  );
        DECL_VISIT_PROC( VarDeclStmnt      );

        DECL_VISIT_PROC( UnaryExpr         );
        DECL_VISIT_PROC( PostUnaryExpr     );
        DECL_VISIT_PROC( CallExpr          );
        DECL_VISIT_PROC( ObjectExpr        );
        DECL_VISIT_PROC( AssignExpr        );

        /* === Members === */

        Program*                program_        = nullptr;
        ShaderTarget            shaderTarget_   = ShaderTarget::VertexShader;
        
        std::vector<CallExpr*>  callExprStack_;

};


} // /namespace Xsc


#endif



// ================================================================================