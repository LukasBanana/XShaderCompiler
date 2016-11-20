/*
 * GLSLConverter.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_GLSL_CONVERTER_H
#define XSC_GLSL_CONVERTER_H


#include "Visitor.h"
#include <Xsc/Targets.h>
#include <functional>
#include <set>


namespace Xsc
{


// GLSL AST converter.
class GLSLConverter : public Visitor
{
    
    public:
        
        // Converts the specified AST for GLSL.
        void Convert(
            Program& program,
            const ShaderTarget shaderTarget,
            const std::string& nameManglingPrefix
        );

    private:
        
        /* --- Visitor implementation --- */

        DECL_VISIT_PROC( FunctionCall );
        DECL_VISIT_PROC( VarIdent     );

        DECL_VISIT_PROC( VarDecl      );
        DECL_VISIT_PROC( StructDecl   );

        DECL_VISIT_PROC( FunctionDecl );

        DECL_VISIT_PROC( ExprStmnt    );

        /* --- Helper functions for conversion --- */

        // Returns true if the specified expression contains a sampler object.
        bool ExprContainsSampler(Expr& ast) const;

        // Returns true if the specified variable type is a sampler.
        bool VarTypeIsSampler(VarType& ast) const;

        // Returns true if the specified structure declaration must be resolved (i.e. structure is removed, and its members are used as global variables).
        bool MustResolveStruct(StructDecl* ast) const;

        // Returns true if the specified variable declaration must be renamed.
        bool MustRenameVarDecl(VarDecl* ast) const;

        // Renames the specified variable declaration with name mangling.
        void RenameVarDecl(VarDecl* ast);

        /* === Members === */

        ShaderTarget                shaderTarget_           = ShaderTarget::VertexShader;

        std::string                 nameManglingPrefix_;

        /*
        List of all reserved variable identifiers that come from a structure that must be resolved.
        If a local variable uses a name from this list, it name must be modified with name mangling.
        */
        std::vector<std::string>    reservedLocalVarIdents_;

        bool                        localScope_             = false;

};


} // /namespace Xsc


#endif



// ================================================================================