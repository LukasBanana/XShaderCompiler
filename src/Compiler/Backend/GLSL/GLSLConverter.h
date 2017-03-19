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
#include "SymbolTable.h"
#include "Identifier.h"
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
            const Options& options,
            bool isVKSL
        );

    private:
        
        /* ----- Visitor implementation ----- */

        DECL_VISIT_PROC( Program          );
        DECL_VISIT_PROC( CodeBlock        );
        DECL_VISIT_PROC( FunctionCall     );
        DECL_VISIT_PROC( SwitchCase       );

        DECL_VISIT_PROC( VarDecl          );
        DECL_VISIT_PROC( BufferDecl       );
        DECL_VISIT_PROC( SamplerDecl      );
        DECL_VISIT_PROC( StructDecl       );

        DECL_VISIT_PROC( FunctionDecl     );
        DECL_VISIT_PROC( VarDeclStmnt     );
        DECL_VISIT_PROC( AliasDeclStmnt   );

        DECL_VISIT_PROC( CodeBlockStmnt   );
        DECL_VISIT_PROC( ForLoopStmnt     );
        DECL_VISIT_PROC( WhileLoopStmnt   );
        DECL_VISIT_PROC( DoWhileLoopStmnt );
        DECL_VISIT_PROC( IfStmnt          );
        DECL_VISIT_PROC( ElseStmnt        );
        DECL_VISIT_PROC( SwitchStmnt      );

        DECL_VISIT_PROC( LiteralExpr      );
        DECL_VISIT_PROC( CastExpr         );
        DECL_VISIT_PROC( FunctionCallExpr );
        DECL_VISIT_PROC( ObjectExpr       );

        /* ----- Scope functions ----- */

        // Opens a new scope in the smybol table.
        void OpenScope();

        // Closes the current scope in the symbol table.
        void CloseScope();

        // Registers the AST node in the current scope with the specified identifier.
        void Register(const std::string& ident);

        // Renames the identifier of the specified declaration object (if required) and registers its identifier.
        void RegisterDeclIdent(Decl* obj, bool global = false);

        // Registers the identifiers of all specified variables (see RegisterDeclIdent).
        void RegisterGlobalDeclIdents(const std::vector<VarDecl*>& varDecls);

        // Tries to fetch an AST node with the specified identifier from the symbol table and reports an error on failure.
        bool FetchFromCurrentScope(const std::string& ident) const;

        /* ----- Helper functions for conversion ----- */

        // Returns true if the specified type denoter is a sampler state type.
        bool IsSamplerStateTypeDenoter(const TypeDenoterPtr& typeDenoter) const;

        // Returns true if the specified variable declaration must be renamed.
        bool MustRenameDeclIdent(const Decl* obj) const;

        // Renames the specified variable declaration with name mangling.
        void RenameIdent(Identifier& ident);
        void RenameDeclIdent(Decl* obj);

        void RenameInOutVarIdents(const std::vector<VarDecl*>& varDecls, bool input, bool useSemanticOnly = false);

        // Labels the specified anonymous structure.
        void LabelAnonymousStructDecl(StructDecl* ast);

        // Returns true if the variable is a global input/output variable declaration.
        bool IsGlobalInOutVarDecl(VarDecl* varDecl) const;

        #if 0//TODO: remove
        
        // Returns true if the variable identifier refers to a global input or output variable declaration.
        bool HasGlobalInOutVarDecl(VarIdent* varIdent) const;

        /*
        Changes the specified variable identifier to a local variable identifier
        (without a leading structure instance name), if it refers to a global input/output variable.
        */
        void PopFrontOfGlobalInOutVarIdent(VarIdent* ast);

        #endif

        /*
        Converts the specified statement to a code block and inserts itself into this code block (if it is a return statement within the entry point).
        This is used to ensure a new scope within a control flow statement (e.g. if-statement).
        */
        void MakeCodeBlockInEntryPointReturnStmnt(StmntPtr& bodyStmnt);

        // Removes all statements that are marked as dead code.
        void RemoveDeadCode(std::vector<StmntPtr>& stmnts);

        // Removes all variable declarations which have a sampler state type.
        void RemoveSamplerStateVarDeclStmnts(std::vector<VarDeclStmntPtr>& stmnts);

        // Renames the specified identifier if it equals a reserved GLSL intrinsic or function name.
        bool RenameReservedKeyword(Identifier& ident);

        void PushSelfParameter(VarDecl* parameter);
        void PopSelfParameter();

        VarDecl* ActiveSelfParameter() const;

        // Function signature compare callback for the function name converter.
        static bool CompareFuncSignatures(const FunctionDecl& lhs, const FunctionDecl& rhs);

        /* ----- Conversion ----- */

        void ConvertFunctionDecl(FunctionDecl* ast);
        void ConvertFunctionDeclDefault(FunctionDecl* ast);
        void ConvertFunctionDeclEntryPoint(FunctionDecl* ast);

        void ConvertIntrinsicCall(FunctionCall* ast);
        void ConvertIntrinsicCallSaturate(FunctionCall* ast);
        void ConvertIntrinsicCallTextureSample(FunctionCall* ast);
        void ConvertIntrinsicCallTextureSampleLevel(FunctionCall* ast);

        void ConvertFunctionCall(FunctionCall* ast);

        void ConvertEntryPointStructPrefix(ExprPtr& expr, ObjectExpr* objectExpr);

        /* ----- Unrolling ----- */

        void UnrollStmnts(std::vector<StmntPtr>& stmnts);
        void UnrollStmntsVarDecl(std::vector<StmntPtr>& unrolledStmnts, VarDeclStmnt* ast);
        void UnrollStmntsVarDeclInitializer(std::vector<StmntPtr>& unrolledStmnts, VarDecl* varDecl);

        /* === Members === */

        // Symbol table to determine which variables must be renamed (scope rules are different between HLSL and GLSL).
        SymbolTable<bool>           symTable_;

        ExprConverter               exprConverter_;

        ShaderTarget                shaderTarget_       = ShaderTarget::VertexShader;
        Program*                    program_            = nullptr;

        NameMangling                nameMangling_;
        Options                     options_;
        bool                        isVKSL_             = false;

        /*
        List of all variables with reserved identifiers that come from a structure that must be resolved.
        If a local variable uses a name from this list, it name must be modified with name mangling.
        */
        std::vector<const Decl*>    globalReservedDecls_;

        // Stack with information of the current 'self' parameter of a member function.
        std::vector<VarDecl*>       selfParamStack_;

        unsigned int                anonymCounter_      = 0;
        unsigned int                obfuscationCounter_ = 0;

};


} // /namespace Xsc


#endif



// ================================================================================