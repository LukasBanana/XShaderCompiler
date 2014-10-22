/*
 * GLSLGenerator.h
 * 
 * This file is part of the "HLSL Translator" (Copyright (c) 2014 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __HT_GLSL_GENERATOR_H__
#define __HT_GLSL_GENERATOR_H__


#include "HT/Translator.h"
#include "CodeWriter.h"
#include "Visitor.h"
#include "Token.h"

#include <map>


namespace HTLib
{


//! GLSL output code generator.
class GLSLGenerator : private Visitor
{
    
    public:
        
        GLSLGenerator(
            Logger* log = nullptr,
            IncludeHandler* includeHandler = nullptr,
            const Options& options = {}
        );

        bool GenerateCode(
            Program* program,
            std::ostream& output,
            const std::string& entryPoint,
            const ShaderTargets shaderTarget,
            const ShaderVersions shaderVersion
        );

    private:
        
        /* === Structures === */

        struct SemanticStage
        {
            SemanticStage(const std::string& semantic);
            SemanticStage(
                const std::string& vertex,
                const std::string& geometry,
                const std::string& tessControl,
                const std::string& tessEvaluation,
                const std::string& fragment,
                const std::string& compute
            );

            //! \throws std::out_of_range If 'target' is out of range.
            const std::string& operator [] (const ShaderTargets target) const;

            std::string vertex;
            std::string geometry;
            std::string tessControl;
            std::string tessEvaluation;
            std::string fragment;
            std::string compute;
        };

        /* === Functions === */

        void EstablishMaps();

        void Error(const std::string& msg, const AST* ast = nullptr);
        void ErrorInvalidNumArgs(const std::string& functionName, const AST* ast = nullptr);

        void BeginLn();
        void EndLn();
        
        void Write(const std::string& text);
        void WriteLn(const std::string& text);

        void IncTab();
        void DecTab();
        
        void PushOptions(const CodeWriter::Options& options);
        void PopOptions();

        //! Writes a new single line comment.
        void Comment(const std::string& text);
        //! Writes a "#version" directive.
        void Version(int versionNumber);
        //! Writes a "#line" directive.
        void Line(int lineNumber);
        void Line(const TokenPtr& tkn);
        void Line(const AST* ast);
        //! Writes a blank line.
        void Blank();
        //! Writes a new extensions
        void Extension(const std::string& extensionName);

        void AppendRequiredExtensions(Program* ast);
        void AppendCommonMacros();
        void AppendInterlockedMacros();
        //void AppendMulFunctions();
        void AppendRcpFunctions();

        //! Opens a new scope with '{'.
        void OpenScope();
        //! Closes the current scope with '}'.
        void CloseScope(bool semicolon = false);

        void ValidateRegisterPrefix(const std::string& registerName, char prefix);
        int RegisterIndex(const std::string& registerName);

        std::string BRegister(const std::string& registerName);
        std::string TRegister(const std::string& registerName);
        std::string SRegister(const std::string& registerName);
        std::string URegister(const std::string& registerName);

        //! Returns true if the target version is greater than or equal to the specified version number.
        bool IsVersion(int version) const;

        //! Returns true if the specified AST structure must be resolved.
        bool MustResolveStruct(Structure* ast) const;

        /* --- Visitor implementation --- */

        DECL_VISIT_PROC( Program           );
        DECL_VISIT_PROC( CodeBlock         );
        //DECL_VISIT_PROC( BufferDeclIdent   );
        DECL_VISIT_PROC( FunctionCall      );
        DECL_VISIT_PROC( Structure         );
        DECL_VISIT_PROC( SwitchCase        );

        DECL_VISIT_PROC( FunctionDecl      );
        DECL_VISIT_PROC( UniformBufferDecl );
        DECL_VISIT_PROC( TextureDecl       );
        //DECL_VISIT_PROC( SamplerDecl       );
        DECL_VISIT_PROC( StructDecl        );
        //DECL_VISIT_PROC( DirectiveDecl     );

        DECL_VISIT_PROC( NullStmnt         );
        DECL_VISIT_PROC( DirectiveStmnt    );
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
        DECL_VISIT_PROC( TypeNameExpr      );
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

        /* --- Helper functions for code generation --- */

        void VisitAttribute(FunctionCall* ast);
        void WriteAttributeNumThreads(FunctionCall* ast);
        void WriteEntryPointParameter(VarDeclStmnt* ast);

        void VisitParameter(VarDeclStmnt* ast);
        void VisitScopedStmnt(Stmnt* ast);

        /* === Members === */

        CodeWriter      writer_;
        IncludeHandler* includeHandler_     = nullptr;
        Logger*         log_                = nullptr;

        std::string     entryPoint_;
        ShaderTargets   shaderTarget_       = ShaderTargets::GLSLVertexShader;
        ShaderVersions  shaderVersion_      = ShaderVersions::GLSL110;
        std::string     localVarPrefix_;
        bool            allowBlanks_        = true;

        bool            isInsideEntryPoint_ = false; //!< True if AST traversal is currently inside the main entry point (or its sub nodes).

        std::map<std::string, std::string> typeMap_;        // <hlsl-type, glsl-type>
        std::map<std::string, std::string> intrinsicMap_;   // <hlsl-intrinsic, glsl-intrinsic>
        std::map<std::string, std::string> modifierMap_;    // <hlsl-modifier, glsl-qualifier>
        std::map<std::string, SemanticStage> semanticMap_;  // <hlsl-semantic, glsl-keyword>

};


} // /namespace HTLib


#endif



// ================================================================================