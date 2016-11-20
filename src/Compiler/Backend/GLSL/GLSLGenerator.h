/*
 * GLSLGenerator.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_GLSL_GENERATOR_H
#define XSC_GLSL_GENERATOR_H


#include <Xsc/Xsc.h>
#include "Generator.h"
#include "Visitor.h"
#include "Token.h"
#include "ASTEnums.h"

#include <map>
#include <vector>


namespace Xsc
{


struct TypeDenoter;

// GLSL output code generator.
class GLSLGenerator : public Generator
{
    
    public:
        
        GLSLGenerator(Log* log);

    private:
        
        /* === Functions === */

        void GenerateCodePrimary(
            Program& program,
            const ShaderInput& inputDesc,
            const ShaderOutput& outputDesc
        ) override;

        void EstablishMaps();

        // Writes a new single line comment.
        void Comment(const std::string& text);

        // Writes a "#version" directive.
        void Version(int versionNumber);

        // Writes a "#line" directive.
        void Line(int lineNumber);
        void Line(const TokenPtr& tkn);
        void Line(const AST* ast);

        // Writes a new extensions
        void AppendExtension(const std::string& extensionName);
        void AppendRequiredExtensions(Program& ast);
        void AppendAllReferencedIntrinsics(Program& ast);
        void AppendClipIntrinsics();

        // Opens a new scope with '{'.
        void OpenScope();

        // Closes the current scope with '}'.
        void CloseScope(bool semicolon = false);

        void ValidateRegisterPrefix(const std::string& registerName, char prefix, const AST* ast = nullptr);
        int RegisterIndex(const std::string& registerName);

        std::string BRegister(const std::string& registerName, const AST* ast = nullptr);
        std::string TRegister(const std::string& registerName, const AST* ast = nullptr);
        std::string SRegister(const std::string& registerName, const AST* ast = nullptr);
        std::string URegister(const std::string& registerName, const AST* ast = nullptr);

        // Returns true if the specified AST structure must be resolved (i.e. structure is removed, and its members are used as global variables).
        bool MustResolveStruct(StructDecl* ast) const;

        // Returns true if the target version is greater than or equal to the specified version number.
        bool IsVersionOut(int version) const;

        /* --- Visitor implementation --- */

        DECL_VISIT_PROC( Program           );
        DECL_VISIT_PROC( CodeBlock         );
        DECL_VISIT_PROC( FunctionCall      );
        DECL_VISIT_PROC( StructDecl        );
        DECL_VISIT_PROC( SwitchCase        );
        DECL_VISIT_PROC( VarType           );
        DECL_VISIT_PROC( VarIdent          );

        DECL_VISIT_PROC( FunctionDecl      );
        DECL_VISIT_PROC( VarDecl           );
        DECL_VISIT_PROC( BufferDeclStmnt   );
        DECL_VISIT_PROC( TextureDeclStmnt  );
        DECL_VISIT_PROC( StructDeclStmnt   );
        DECL_VISIT_PROC( VarDeclStmnt      );
        DECL_VISIT_PROC( AliasDeclStmnt    );

        DECL_VISIT_PROC( NullStmnt         );
        DECL_VISIT_PROC( CodeBlockStmnt    );
        DECL_VISIT_PROC( ForLoopStmnt      );
        DECL_VISIT_PROC( WhileLoopStmnt    );
        DECL_VISIT_PROC( DoWhileLoopStmnt  );
        DECL_VISIT_PROC( IfStmnt           );
        DECL_VISIT_PROC( ElseStmnt         );
        DECL_VISIT_PROC( SwitchStmnt       );
        DECL_VISIT_PROC( ExprStmnt         );
        DECL_VISIT_PROC( ReturnStmnt       );
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
        DECL_VISIT_PROC( SuffixExpr        );
        DECL_VISIT_PROC( ArrayAccessExpr   );
        DECL_VISIT_PROC( CastExpr          );
        DECL_VISIT_PROC( VarAccessExpr     );
        DECL_VISIT_PROC( InitializerExpr   );

        /* --- Helper functions for code generation --- */

        void WriteAttribute(Attribute* ast);
        void WriteAttributeNumThreads(Attribute* ast);
        void WriteAttributeEarlyDepthStencil();

        void WriteInputSemantics();
        bool WriteInputSemanticsParameter(VarDeclStmnt* ast);
        bool WriteInputSemanticsParameterVarDecl(VarDecl* varDecl);
        bool WriteInputSemanticsGlobalVarDecl(VarDecl* varDecl);

        void WriteOutputSemantics(Expr* ast);

        void WriteFragmentShaderOutput();

        void VisitStructDeclMembers(StructDecl* ast);

        // Returns the first VarIdent AST node which has a system value semantic, or null if no such AST node was found.
        VarIdent* FindSystemValueVarIdent(VarIdent* ast);

        // Writes the specified variable identifier or a system value if the VarIdent has a system value semantic.
        void WriteVarIdentOrSystemValue(VarIdent* ast);

        void VisitParameter(VarDeclStmnt* ast);
        void VisitScopedStmnt(Stmnt* ast);

        bool HasSystemValueSemantic(const std::vector<VarSemanticPtr>& semantics) const;

        void WriteArrayDims(const std::vector<ExprPtr>& arrayDims);
        void WriteTypeDenoter(const TypeDenoter& typeDenoter, const AST* ast);

        // Writes the specified identifier with possible name manging (if the name is reserved in GLSL).
        void WriteIdent(const std::string& ident);

        void AssertIntrinsicNumArgs(FunctionCall* ast, std::size_t numArgsMin, std::size_t numArgsMax = ~0);

        void WriteFunctionCallStandard(FunctionCall* ast);
        void WriteFunctionCallIntrinsicMul(FunctionCall* ast);
        void WriteFunctionCallIntrinsicRcp(FunctionCall* ast);
        void WriteFunctionCallIntrinsicAtomic(FunctionCall* ast);
        void WriteFunctionCallIntrinsicTex(FunctionCall* ast);

        /* === Members === */

        ShaderTarget            shaderTarget_           = ShaderTarget::VertexShader;
        OutputShaderVersion     versionOut_             = OutputShaderVersion::GLSL330;
        std::string             localVarPrefix_;
        bool                    allowLineMarks_         = true;
        Statistics*             stats_                  = nullptr;

        bool                    isInsideEntryPoint_     = false; //< True if AST traversal is currently inside the main entry point (or its sub nodes).
        bool                    isInsideInterfaceBlock_ = false;

        #if 1//TODO: remove all HLSL type mappings from here
        std::map<std::string, std::string> texFuncMap_;         // <hlsl-function, glsl-function>
        #endif

};


} // /namespace Xsc


#endif



// ================================================================================