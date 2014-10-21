/*
 * HLSLAnalyzer.h
 * 
 * This file is part of the "HLSL Translator" (Copyright (c) 2014 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __HT_HLSL_ANALYZER_H__
#define __HT_HLSL_ANALYZER_H__


#include "HT/Translator.h"
#include "CodeWriter.h"
#include "Visitor.h"
#include "Token.h"
#include "SymbolTable.h"
#include "HLSLTree.h"

#include <map>


namespace HTLib
{


//! HLSL context analyzer.
class HLSLAnalyzer : private Visitor
{
    
    public:
        
        HLSLAnalyzer(Logger* log = nullptr);

        bool DecorateAST(
            Program* program,
            const std::string& entryPoint,
            const ShaderTargets shaderTarget,
            const ShaderVersions shaderVersion,
            const std::string& localVarPrefix
        );

    private:
        
        typedef SymbolTable<AST>::OnOverrideProc OnOverrideProc;

        /* === Enumerations === */

        enum class IntrinsicClasses
        {
            Interlocked,
        };

        /* === Functions === */
        
        void EstablishMaps();

        void Error(const std::string& msg, const AST* ast = nullptr);
        void Warning(const std::string& msg, const AST* ast = nullptr);
        void NotifyUndeclaredIdent(const std::string& ident, const AST* ast = nullptr);

        void OpenScope();
        void CloseScope();

        void Register(const std::string& ident, AST* ast, const OnOverrideProc& overrideProc = nullptr);
        
        AST* Fetch(const std::string& ident) const;
        AST* Fetch(const VarIdentPtr& ident) const;

        void DecorateEntryInOut(VarDeclStmnt* ast, bool isInput);
        void DecorateEntryInOut(VarType* ast, bool isInput);

        void ReportNullStmnt(const StmntPtr& ast, const std::string& stmntTypeName);

        void AcquireExtension(const Program::ARBExtension& extension);

        //! Returns true if the target version is greater than or equal to the specified version number.
        bool IsVersion(int version) const;

        /* === Visitor implementation === */

        DECL_VISIT_PROC( Program           );
        DECL_VISIT_PROC( CodeBlock         );
        //DECL_VISIT_PROC( BufferDeclIdent   );
        DECL_VISIT_PROC( FunctionCall      );
        DECL_VISIT_PROC( Structure         );
        DECL_VISIT_PROC( SwitchCase        );

        DECL_VISIT_PROC( FunctionDecl      );
        DECL_VISIT_PROC( BufferDecl        );
        //DECL_VISIT_PROC( TextureDecl       );
        //DECL_VISIT_PROC( SamplerStateDecl  );
        DECL_VISIT_PROC( StructDecl        );
        //DECL_VISIT_PROC( DirectiveDecl     );

        DECL_VISIT_PROC( CodeBlockStmnt    );
        DECL_VISIT_PROC( ForLoopStmnt      );
        DECL_VISIT_PROC( WhileLoopStmnt    );
        DECL_VISIT_PROC( DoWhileLoopStmnt  );
        DECL_VISIT_PROC( IfStmnt           );
        DECL_VISIT_PROC( ElseStmnt         );
        DECL_VISIT_PROC( SwitchStmnt       );
        DECL_VISIT_PROC( VarDeclStmnt      );
        DECL_VISIT_PROC( AssignSmnt        );
        DECL_VISIT_PROC( FunctionCallStmnt );
        DECL_VISIT_PROC( ReturnStmnt       );
        //DECL_VISIT_PROC( StructDeclStmnt   );
        DECL_VISIT_PROC( CtrlTransferStmnt );

        DECL_VISIT_PROC( ListExpr          );
        DECL_VISIT_PROC( LiteralExpr       );
        DECL_VISIT_PROC( BinaryExpr        );
        DECL_VISIT_PROC( UnaryExpr         );
        DECL_VISIT_PROC( PostUnaryExpr     );
        DECL_VISIT_PROC( FunctionCallExpr  );
        DECL_VISIT_PROC( BracketExpr       );
        DECL_VISIT_PROC( CastExpr          );
        DECL_VISIT_PROC( VarAccessExpr     );
        DECL_VISIT_PROC( InitializerExpr   );

        DECL_VISIT_PROC( PackOffset        );
        DECL_VISIT_PROC( VarSemantic       );
        DECL_VISIT_PROC( VarType           );
        DECL_VISIT_PROC( VarIdent          );
        DECL_VISIT_PROC( VarDecl           );

        /* === Members === */

        Logger*         log_            = nullptr;

        bool            hasErrors_      = false;
        Program*        program_        = nullptr;

        std::string     entryPoint_;
        ShaderTargets   shaderTarget_   = ShaderTargets::GLSLVertexShader;
        ShaderVersions  shaderVersion_  = ShaderVersions::GLSL110;
        std::string     localVarPrefix_;

        std::map<std::string, IntrinsicClasses> intrinsicMap_;
        std::map<std::string, Program::ARBExtension> extensionMap_;

        SymbolTable<AST> symTable_;

        bool isInsideFunc_          = false; //!< True if AST traversal is currently inside any function.
        bool isInsideEntryPoint_    = false; //!< True if AST traversal is currently inside the main entry point (or its sub nodes).

};


} // /namespace HTLib


#endif



// ================================================================================