/*
 * GLSLConverter.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_GLSL_CONVERTER_H
#define XSC_GLSL_CONVERTER_H


#include "Visitor.h"
#include "TypeDenoter.h"
#include "ExprConverter.h"
#include <Xsc/Xsc.h>
#include <functional>
#include <set>


namespace Xsc
{


/*
GLSL AST converter.
This class modifies the AST after context analysis to be conform with GLSL,
e.g. remove arguments from intrinsic calls, that are not allowed in GLSL, such as sampler state objects.
*/
class GLSLConverter : public Visitor
{
    
    public:
        
        // Converts the specified AST for GLSL.
        void Convert(
            Program& program,
            const ShaderTarget shaderTarget,
            const NameMangling& nameMangling,
            const Options& options
        );

    private:
        
        /* ----- Visitor implementation ----- */

        DECL_VISIT_PROC( Program          );
        DECL_VISIT_PROC( CodeBlock        );
        DECL_VISIT_PROC( FunctionCall     );
        DECL_VISIT_PROC( SwitchCase       );
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

        DECL_VISIT_PROC( LiteralExpr      );
        DECL_VISIT_PROC( CastExpr         );

        /* ----- Helper functions for conversion ----- */

        // Returns true if the specified type denoter is a sampler state type.
        bool IsSamplerStateTypeDenoter(const TypeDenoterPtr& typeDenoter) const;

        // Returns true if the specified variable declaration must be renamed.
        bool MustRenameVarDecl(VarDecl* ast) const;

        // Renames the specified variable declaration with name mangling.
        void RenameVarDecl(VarDecl* ast, const std::string& ident);
        void RenameVarDecl(VarDecl* ast);

        void RenameInOutVarIdents(const std::vector<VarDecl*>& varDecls, bool input, bool useSemanticOnly = false);

        // Labels the specified anonymous structure.
        void LabelAnonymousStructDecl(StructDecl* ast);

        // Returns true if the variable identifier refers to a global input or output variable declaration.
        bool HasGlobalInOutVarDecl(VarIdent* varIdent) const;

        /*
        Changes the specified variable identifier to a local variable identifier
        (without a leading structure instance name), if it refers to a global input/output variable.
        */
        void PopFrontOfGlobalInOutVarIdent(VarIdent* ast);

        /*
        Converts the specified statement to a code block and inserts itself into this code block (if it is a return statement within the entry point).
        This is used to ensure a new scope within a control flow statement (e.g. if-statement).
        */
        void MakeCodeBlockInEntryPointReturnStmnt(StmntPtr& bodyStmnt);

        // Registers the all specified variables as reserved identifiers.
        void RegisterReservedVarIdents(const std::vector<VarDecl*>& varDecls);

        // Removes all statements that are marked as dead code.
        void RemoveDeadCode(std::vector<StmntPtr>& stmnts);

        // Removes all variable declarations which have a sampler state type.
        void RemoveSamplerStateVarDeclStmnts(std::vector<VarDeclStmntPtr>& stmnts);

        // Renames the specified identifier if it equals a reserved GLSL intrinsic or function name.
        bool RenameReservedKeyword(const std::string& ident, std::string& renamedIdent);

        /* ----- Conversion ----- */

        void ConvertFunctionDecl(FunctionDecl* ast);
        void ConvertFunctionDeclDefault(FunctionDecl* ast);
        void ConvertFunctionDeclEntryPoint(FunctionDecl* ast);

        void ConvertIntrinsicCall(FunctionCall* ast);
        void ConvertIntrinsicCallSaturate(FunctionCall* ast);
        void ConvertIntrinsicCallTextureSample(FunctionCall* ast);
        void ConvertIntrinsicCallTextureSampleLevel(FunctionCall* ast);

        /* ----- Unrolling ----- */

        void UnrollStmnts(std::vector<StmntPtr>& stmnts);
        void UnrollStmntsVarDecl(std::vector<StmntPtr>& unrolledStmnts, VarDeclStmnt* ast);
        void UnrollStmntsVarDeclInitializer(std::vector<StmntPtr>& unrolledStmnts, VarDecl* varDecl);

        /* === Members === */

        ExprConverter           exprConverter_;

        ShaderTarget            shaderTarget_       = ShaderTarget::VertexShader;
        Program*                program_            = nullptr;

        NameMangling            nameMangling_;
        Options                 options_;

        /*
        List of all variables with reserved identifiers that come from a structure that must be resolved.
        If a local variable uses a name from this list, it name must be modified with name mangling.
        */
        std::vector<VarDecl*>   reservedVarDecls_;

        unsigned int            anonymCounter_      = 0;
        unsigned int            obfuscationCounter_ = 0;

};


} // /namespace Xsc


#endif



// ================================================================================