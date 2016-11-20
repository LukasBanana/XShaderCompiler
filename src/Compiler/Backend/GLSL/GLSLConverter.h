/*
 * GLSLConverter.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_GLSL_CONVERTER_H
#define XSC_GLSL_CONVERTER_H


#include "Visitor.h"
#include <functional>


namespace Xsc
{


// GLSL AST converter.
class GLSLConverter : public Visitor
{
    
    public:
        
        // Converts the specified AST for GLSL.
        void Convert(Program& program);

    private:
        
        /* --- Visitor implementation --- */

        DECL_VISIT_PROC( FunctionCall );

        DECL_VISIT_PROC( FunctionDecl );

        DECL_VISIT_PROC( ExprStmnt    );

        /* --- Helper functions for conversion --- */

        // Returns true if the specified expression contains a sampler object.
        bool ExprContainsSampler(Expr& ast);

        // Returns true if the specified variable type is a sampler.
        bool VarTypeIsSampler(VarType& ast);

};


} // /namespace Xsc


#endif



// ================================================================================