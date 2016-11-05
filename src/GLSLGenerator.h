/*
 * GLSLGenerator.h
 * 
 * This file is part of the "HLSL Translator" (Copyright (c) 2014 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef HTLIB_GLSL_GENERATOR_H
#define HTLIB_GLSL_GENERATOR_H


#include "HT/Translator.h"
#include "CodeWriter.h"
#include "Visitor.h"
#include "Token.h"

#include <map>
#include <vector>


namespace HTLib
{


// GLSL output code generator.
class GLSLGenerator : private Visitor
{
    
    public:
        
        GLSLGenerator(Log* log = nullptr, const Options& options = {});

        bool GenerateCode(
            Program* program,
            std::ostream& output,
            const std::string& entryPoint,
            const ShaderTarget shaderTarget,
            const InputShaderVersion versionIn,
            const OutputShaderVersion versionOut
        );

    private:
        
        /* === Structures === */

        struct SemanticStage
        {
            SemanticStage() = default;
            SemanticStage(const std::string& semantic);
            SemanticStage(
                const std::string& vertex,
                const std::string& geometry,
                const std::string& tessControl,
                const std::string& tessEvaluation,
                const std::string& fragment,
                const std::string& compute
            );

            // \throws std::out_of_range If 'target' is out of range.
            const std::string& operator [] (const ShaderTarget target) const;

            std::string vertex;
            std::string geometry;
            std::string tessControl;
            std::string tessEvaluation;
            std::string fragment;
            std::string compute;
            int         index = 0;      // Semantic index
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

        // Writes a new single line comment.
        void Comment(const std::string& text);
        // Writes a "#version" directive.
        void Version(int versionNumber);
        // Writes a "#line" directive.
        void Line(int lineNumber);
        void Line(const TokenPtr& tkn);
        void Line(const AST* ast);
        // Writes a blank line.
        void Blank();
        // Writes a new extensions
        void Extension(const std::string& extensionName);

        void AppendRequiredExtensions(Program* ast);
        
        void AppendCommonMacros();
        //void AppendInterlockedMacros();

        //void AppendMulIntrinsics();
        void AppendRcpIntrinsics();
        void AppendClipIntrinsics();
        void AppendSinCosIntrinsics();

        // Opens a new scope with '{'.
        void OpenScope();
        // Closes the current scope with '}'.
        void CloseScope(bool semicolon = false);

        void ValidateRegisterPrefix(const std::string& registerName, char prefix);
        int RegisterIndex(const std::string& registerName);

        std::string BRegister(const std::string& registerName);
        std::string TRegister(const std::string& registerName);
        std::string SRegister(const std::string& registerName);
        std::string URegister(const std::string& registerName);

        // Returns true if the specified AST structure must be resolved.
        bool MustResolveStruct(Structure* ast) const;

        // Returns true if the target version is greater than or equal to the specified version number.
        bool IsVersionOut(int version) const;

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
        DECL_VISIT_PROC( DirectiveDecl     );

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
        DECL_VISIT_PROC( AssignStmnt       );
        DECL_VISIT_PROC( ExprStmnt         );
        DECL_VISIT_PROC( FunctionCallStmnt );
        DECL_VISIT_PROC( ReturnStmnt       );
        //DECL_VISIT_PROC( StructDeclStmnt   );
        DECL_VISIT_PROC( CtrlTransferStmnt );

        DECL_VISIT_PROC( ListExpr          );
        DECL_VISIT_PROC( LiteralExpr       );
        DECL_VISIT_PROC( TypeNameExpr      );
        DECL_VISIT_PROC( TernaryExpr       );
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

        void WriteEntryPointParameter(VarDeclStmnt* ast, size_t& writtenParamCounter);
        void WriteEntryPointInputSemantics();
        void WriteEntryPointOutputSemantics(Expr* ast);

        void WriteFragmentShaderOutput();

        VarIdent* FirstSystemSemanticVarIdent(VarIdent* ast);
        void WriteVarIdent(VarIdent* ast);

        void VisitParameter(VarDeclStmnt* ast);
        void VisitScopedStmnt(Stmnt* ast);

        // Returns true if the specified expression contains a sampler object.
        bool ExprContainsSampler(Expr* ast);
        // Returns true if the specified variable type is a sampler.
        bool VarTypeIsSampler(VarType* ast);

        bool FetchSemantic(std::string semanticName, SemanticStage& semantic) const;

        bool IsSystemValueSemantic(const VarSemantic* ast) const;
        bool HasSystemValueSemantic(const std::vector<VarSemanticPtr>& semantics) const;

        /* === Members === */

        CodeWriter              writer_;
        Log*                    log_                    = nullptr;

        Program*                program_                = nullptr;

        std::string             entryPoint_;
        ShaderTarget            shaderTarget_           = ShaderTarget::GLSLVertexShader;
        InputShaderVersion      versionIn_              = InputShaderVersion::HLSL5;
        OutputShaderVersion     versionOut_             = OutputShaderVersion::GLSL330;
        std::string             localVarPrefix_;
        bool                    allowBlanks_            = true;
        bool                    allowLineMarks_         = true;

        bool                    isInsideEntryPoint_     = false; //< True if AST traversal is currently inside the main entry point (or its sub nodes).
        bool                    isInsideInterfaceBlock_ = false;

        std::map<std::string, std::string> typeMap_;            // <hlsl-type, glsl-type>
        std::map<std::string, std::string> intrinsicMap_;       // <hlsl-intrinsic, glsl-intrinsic>
        std::map<std::string, std::string> atomicIntrinsicMap_; // <hlsl-interlocked-intrinsic, glsl-atomic-intrinsic>
        std::map<std::string, std::string> modifierMap_;        // <hlsl-modifier, glsl-qualifier>
        std::map<std::string, std::string> texFuncMap_;         // <hlsl-function, glsl-function>
        std::map<std::string, SemanticStage> semanticMap_;      // <hlsl-semantic, glsl-keyword>

};


} // /namespace HTLib


#endif



// ================================================================================