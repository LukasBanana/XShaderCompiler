/*
 * ReferenceAnalyzer.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
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
        void MarkReferencesFromEntryPoint(FunctionDecl* entryPoint);

    private:
        
        using OnOverrideProc = ASTSymbolTable::OnOverrideProc;

        bool Reachable(AST* ast);

        /* ----- Visitor implementation ----- */

        DECL_VISIT_PROC( FunctionCall      );
        DECL_VISIT_PROC( VarType           );
        DECL_VISIT_PROC( VarIdent          );

        DECL_VISIT_PROC( VarDecl           );
        DECL_VISIT_PROC( StructDecl        );
        DECL_VISIT_PROC( TextureDecl       );

        DECL_VISIT_PROC( FunctionDecl      );
        DECL_VISIT_PROC( BufferDeclStmnt   );
        DECL_VISIT_PROC( TextureDeclStmnt  );

};


} // /namespace Xsc


#endif



// ================================================================================