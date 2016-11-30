/*
 * GLSLConverter.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_GLSL_CONVERTER_H
#define XSC_GLSL_CONVERTER_H


#include "Visitor.h"
#include "TypeDenoter.h"
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

        DECL_VISIT_PROC( Program          );
        DECL_VISIT_PROC( CodeBlock        );
        DECL_VISIT_PROC( FunctionCall     );
        DECL_VISIT_PROC( SwitchCase       );
        DECL_VISIT_PROC( VarSemantic      );
        DECL_VISIT_PROC( VarIdent         );

        DECL_VISIT_PROC( VarDecl          );
        DECL_VISIT_PROC( StructDecl       );

        DECL_VISIT_PROC( FunctionDecl     );
        DECL_VISIT_PROC( VarDeclStmnt     );
        DECL_VISIT_PROC( AliasDeclStmnt   );

        DECL_VISIT_PROC( ForLoopStmnt     );
        DECL_VISIT_PROC( WhileLoopStmnt   );
        DECL_VISIT_PROC( DoWhileLoopStmnt );
        DECL_VISIT_PROC( IfStmnt          );
        DECL_VISIT_PROC( ElseStmnt        );
        DECL_VISIT_PROC( ExprStmnt        );
        DECL_VISIT_PROC( ReturnStmnt      );

        DECL_VISIT_PROC( LiteralExpr      );
        DECL_VISIT_PROC( BinaryExpr       );
        DECL_VISIT_PROC( UnaryExpr        );
        DECL_VISIT_PROC( VarAccessExpr    );

        /* --- Helper functions for conversion --- */

        void PushStructDeclLevel();
        void PopStructDeclLevel();

        bool IsInsideStructDecl() const;

        // Returns true if the specified expression contains a sampler object.
        bool ExprContainsSampler(Expr& ast) const;

        // Returns true if the specified variable type is a sampler.
        bool VarTypeIsSampler(VarType& ast) const;

        // Returns true if the specified structure declaration must be resolved.
        bool MustResolveStruct(StructDecl* ast) const;

        // Returns true if the specified variable declaration must be renamed.
        bool MustRenameVarDecl(VarDecl* ast) const;

        // Renames the specified variable declaration with name mangling.
        void RenameVarDecl(VarDecl* ast);

        // Returns true if the variable identifier refers to a variable declaration which has a system semantic.
        bool HasVarDeclOfVarIdentSystemSemantic(VarIdent* varIdent) const;

        /*
        Changes the specified variable identifier to a local variable identifier
        (without a leading structure instance name), if it refers to a variable with a system semantic.
        */
        void MakeVarIdentWithSystemSemanticLocal(VarIdent* ast);

        /*
        Converts the specified statement to a code block and inserts itself into this code block (if it is a return statement within the entry point).
        This is used to ensure a new scope within a control flow statement (e.g. if-statement).
        */
        void MakeCodeBlockInEntryPointReturnStmnt(StmntPtr& bodyStmnt);

        // Registers the all specified variables as reserved identifiers.
        void RegisterReservedVarIdents(const std::vector<VarDecl*>& varDecls);

        // Returns the data type to which an expression must be casted, if the target type denoter and the source type denoter are incompatible in GLSL.
        std::unique_ptr<DataType> MustCastExprToDataType(TypeDenoter& targetTypeDen, TypeDenoter& sourceTypeDen);

        // Converts the expression to a cast expression if it is required for the specified target type.
        void ConvertExprIfCastRequired(ExprPtr& expr, TypeDenoter& targetTypeDen);

        // Removes all statements that are marked as dead code.
        void RemoveDeadCode(std::vector<StmntPtr>& stmnts);

        /* === Members === */

        ShaderTarget            shaderTarget_           = ShaderTarget::VertexShader;

        std::string             nameManglingPrefix_;

        /*
        List of all variables with reserved identifiers that come from a structure that must be resolved.
        If a local variable uses a name from this list, it name must be modified with name mangling.
        */
        std::vector<VarDecl*>   reservedVarDecls_;

        unsigned int            structDeclLevel_        = 0;

        // True if AST traversal is currently inside the main entry point (or its sub nodes).
        bool                    isInsideEntryPoint_     = false;

        FunctionDecl*           currentFunctionDecl_    = nullptr;

};


} // /namespace Xsc


#endif



// ================================================================================