/*
 * Converter.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_CONVERTER_H
#define XSC_CONVERTER_H


#include "VisitorTracker.h"
#include "TypeDenoter.h"
#include "SymbolTable.h"
#include "Identifier.h"
#include <Xsc/Xsc.h>
#include <stack>


namespace Xsc
{


/*
GLSL AST converter.
This class modifies the AST after context analysis to be conform with GLSL,
e.g. remove arguments from intrinsic calls, that are not allowed in GLSL, such as sampler state objects.
*/
class Converter : public VisitorTracker
{

    public:

        // Converts the specified AST for the target language.
        bool ConvertAST(
            Program&            program,
            const ShaderInput&  inputDesc,
            const ShaderOutput& outputDesc
        );

    protected:

        virtual void ConvertASTPrimary(
            Program&            program,
            const ShaderInput&  inputDesc,
            const ShaderOutput& outputDesc
        ) = 0;

        /* ----- Symbol table functions ----- */

        // Opens a new scope in the smybol table.
        void OpenScope();

        // Closes the current scope in the symbol table.
        void CloseScope();

        // Registers the AST node in the current scope with the specified identifier.
        void Register(const std::string& ident);

        // Returns the symbol with the specified identifer which is in the deepest scope, or null if there is no such symbol.
        bool Fetch(const std::string& ident) const;

        // Returns the symbol with the specified identifer which is in the current scope, or null if there is no such symbol.
        bool FetchFromCurrentScope(const std::string& ident) const;

        /* ----- Self parameter ----- */

        void PushSelfParameter(VarDecl* parameter);
        void PopSelfParameter();

        VarDecl* ActiveSelfParameter() const;

        /* ----- Name mangling ----- */

        // Renames the specified variable declaration with the name mangling temporary-prefix.
        void RenameIdent(Identifier& ident);

        // Renames the specified identifier to "_{ObfuscationCounter}".
        void RenameIdentObfuscated(Identifier& ident);

        // Renames the identifier of the specified declaration object.
        void RenameIdentOf(Decl* obj);

        // Renames the identifiers of the specified input/output variable declarations.
        void RenameIdentOfInOutVarDecls(const std::vector<VarDecl*>& varDecls, bool input, bool useSemanticOnly = false);

        // Labels the specified anonymous structure.
        void LabelAnonymousDecl(Decl* declObj);

        /* ----- Code injection ----- */

        // Visits the specified statements, and allows insertion of further statements (i.e. replace the single statement by a code block statement).
        void VisitScopedStmt(StmtPtr& stmt, void* args = nullptr);

        // Visits the specified list of statements, and allows insertion of further statements.
        void VisitScopedStmtList(std::vector<StmtPtr>& stmtList, void* args = nullptr);

        // Inserts the specified statement before the current statement.
        void InsertStmtBefore(const StmtPtr& stmt, bool globalScope = false);

        // Inserts the specified statement after the current statement.
        void InsertStmtAfter(const StmtPtr& stmt, bool globalScope = false);

        // Moves all structure declaration within the specified statement list into the respective upper scope.
        void MoveNestedStructDecls(std::vector<StmtPtr>& localStmts, bool globalScope = false);

        /* ----- Misc ----- */

        // Returns true if the variable is a global input/output variable declaration.
        bool IsGlobalInOutVarDecl(VarDecl* varDecl) const;

        // Returns true if the specified type denoter is a sampler state type.
        bool IsSamplerStateTypeDenoter(const TypeDenoterPtr& typeDenoter) const;

        // Removes all statements that are marked as dead code.
        void RemoveDeadCode(std::vector<StmtPtr>& stmts);

        // Returns an identifier for a new temporary variable.
        std::string MakeTempVarIdent();

        // Returns the program AST root node.
        inline Program* GetProgram() const
        {
            return program_;
        }

        // Returns the name mangling settings.
        inline const NameMangling& GetNameMangling() const
        {
            return nameMangling_;
        }

    private:

        // Helper class to handle code injection during traversal.
        class StmtScopeHandler
        {

            public:

                StmtScopeHandler(const StmtScopeHandler&) = default;
                StmtScopeHandler& operator = (const StmtScopeHandler&) = default;

                StmtScopeHandler(StmtPtr& stmt);
                StmtScopeHandler(std::vector<StmtPtr>& stmts);

                // Returns the next statement to visit, or null if there is no more statement.
                Stmt* Next();

                void InsertStmtBefore(const StmtPtr& stmt);
                void InsertStmtAfter(const StmtPtr& stmt);

            private:

                void EnsureStmtList();

                void InsertStmtAt(const StmtPtr& stmt, std::size_t pos);

                StmtPtr*                stmt_       = nullptr;
                std::vector<StmtPtr>*   stmtList_   = nullptr;
                std::size_t             idx_        = 0;

        };

        void VisitScopedStmtsFromHandler(const StmtScopeHandler& handler, void* args);

        StmtScopeHandler& ActiveStmtScopeHandler();

    private:

        // Symbol table to determine which variables must be renamed (scope rules are different between HLSL and GLSL).
        SymbolTable<bool>               symTable_;

        Program*                        program_                    = nullptr;
        NameMangling                    nameMangling_;

        // Stack with information of the current 'self' parameter of a member function.
        std::vector<VarDecl*>           selfParamStack_;

        std::stack<StmtScopeHandler>    stmtScopeHandlerStack_;
        StmtScopeHandler*               stmtScopeHandlerGlobalRef_  = nullptr;

        unsigned int                    anonymCounter_              = 0;
        unsigned int                    obfuscationCounter_         = 0;
        unsigned int                    tempVarCounter_             = 0;

};


} // /namespace Xsc


#endif



// ================================================================================
