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
#include <set>
#include <string>
#include <map>


namespace Xsc
{


// GLSL extension type with name and minimum required GLSL version.
struct GLSLExtension
{
    std::string extensionName;
    int         requiredVersion;
};

// GLSL extension agent visitor. Determines which GLSL extension are required for a given GLSL target version.
class GLSLExtensionAgent : private Visitor
{
    
    public:
        
        GLSLExtensionAgent();

        // Returns a set of strings with all required extensions for the specified program and target output GLSL version.
        std::set<std::string> DetermineRequiredExtensions(Program& program, const OutputShaderVersion targetGLSLVersion);

    private:
        
        void AcquireExtension(const GLSLExtension& extension);

        /* --- Visitor implementation --- */

        DECL_VISIT_PROC( FunctionCall      );

        DECL_VISIT_PROC( FunctionDecl      );
        DECL_VISIT_PROC( TextureDeclStmnt       );

        DECL_VISIT_PROC( BinaryExpr        );
        DECL_VISIT_PROC( VarAccessExpr     );
        DECL_VISIT_PROC( InitializerExpr   );

        /* === Members === */

        // Target output GLSL version number.
        int                                     targetGLSLVersion_  = 0;

        // Resulting set of required GLSL extensions.
        std::set<std::string>                   extensions_;

        // Function (or intrinsic) name to GLSL extension map.
        std::map<std::string, GLSLExtension>    funcToExtMap_;

};


} // /namespace Xsc


#endif



// ================================================================================