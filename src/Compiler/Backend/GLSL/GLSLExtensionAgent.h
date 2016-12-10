/*
 * GLSLExtensionAgent.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_GLSL_EXTENSION_AGENT_H
#define XSC_GLSL_EXTENSION_AGENT_H


#include <Xsc/Targets.h>
#include "Visitor.h"
#include "ASTEnums.h"
#include <set>
#include <string>
#include <map>


namespace Xsc
{


// GLSL extension type with name and minimum required GLSL version.
struct GLSLExtension
{
    std::string         extensionName;
    OutputShaderVersion requiredVersion;
};

// GLSL extension agent visitor. Determines which GLSL extension are required for a given GLSL target version.
class GLSLExtensionAgent : private Visitor
{
    
    public:
        
        GLSLExtensionAgent();

        // Returns a set of strings with all required extensions for the specified program and target output GLSL version.
        std::set<std::string> DetermineRequiredExtensions(
            Program& program,
            OutputShaderVersion& targetGLSLVersion,
            const ShaderTarget shaderTarget,
            bool allowExtensions,
            bool explicitBinding
        );

    private:
        
        void AcquireExtension(const GLSLExtension& extension);

        /* --- Visitor implementation --- */

        DECL_VISIT_PROC( FunctionCall      );
        DECL_VISIT_PROC( Attribute         );

        DECL_VISIT_PROC( VarDecl           );

        DECL_VISIT_PROC( FunctionDecl      );
        DECL_VISIT_PROC( UniformBufferDecl );
        DECL_VISIT_PROC( BufferDeclStmnt  );

        DECL_VISIT_PROC( BinaryExpr        );
        DECL_VISIT_PROC( UnaryExpr         );
        DECL_VISIT_PROC( VarAccessExpr     );
        DECL_VISIT_PROC( InitializerExpr   );

        /* === Members === */

        ShaderTarget                        shaderTarget_       = ShaderTarget::Undefined;

        // Target output GLSL version.
        OutputShaderVersion                 targetGLSLVersion_  = OutputShaderVersion::GLSL330;

        // Minimum required GLSL version.
        OutputShaderVersion                 minGLSLVersion_     = OutputShaderVersion::GLSL130;

        bool                                allowExtensions_    = false;
        bool                                explicitBinding_    = false;

        // Resulting set of required GLSL extensions.
        std::set<std::string>               extensions_;

        // Intrinsic name to GLSL extension map.
        std::map<Intrinsic, GLSLExtension>  intrinsicExtMap_;

};


} // /namespace Xsc


#endif



// ================================================================================