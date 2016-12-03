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

        // Writes the specified extension with ": enable"
        void WriteExtension(const std::string& extensionName);

        // Write GLSL version and required extensions
        void WriteVersionAndExtensions(Program& ast);

        void WriteReferencedIntrinsics(Program& ast);
        void WriteClipIntrinsics();

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

        // Returns true if the specified AST structure must be resolved.
        bool MustResolveStruct(StructDecl* ast) const;

        // Returns true if the target version is greater than or equal to the specified version number.
        bool IsVersionOut(int version) const;

        /* --- Visitor implementation --- */

        DECL_VISIT_PROC( Program           );
        DECL_VISIT_PROC( CodeBlock         );
        DECL_VISIT_PROC( FunctionCall      );
        DECL_VISIT_PROC( SwitchCase        );
        DECL_VISIT_PROC( VarType           );
        DECL_VISIT_PROC( VarIdent          );

        DECL_VISIT_PROC( VarDecl           );
        DECL_VISIT_PROC( StructDecl        );

        DECL_VISIT_PROC( FunctionDecl      );
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

        /* --- Attribute --- */

        void WriteAttribute(Attribute* ast);
        void WriteAttributeNumThreads(Attribute* ast);
        void WriteAttributeEarlyDepthStencil();

        /* --- Input semantics --- */

        void WriteLocalInputSemantics();
        bool WriteLocalInputSemanticsVarDecl(VarDecl* varDecl);
        
        void WriteGlobalInputSemantics();
        bool WriteGlobalInputSemanticsVarDecl(VarDecl* varDecl);

        /* --- Output semantics --- */

        void WriteLocalOutputSemantics();
        bool WriteLocalOutputSemanticsVarDecl(VarDecl* varDecl);
        
        void WriteGlobalOutputSemantics();
        bool WriteGlobalOutputSemanticsVarDecl(VarDecl* varDecl);

        void WriteOutputSemanticsAssignment(Expr* ast);

        //void WriteFragmentShaderOutput();

        /* --- VarIdent --- */

        // Returns the final identifier string from the specified variable identifier.
        const std::string& FinalIdentFromVarIdent(VarIdent* ast);

        void WriteVarIdent(VarIdent* ast, bool recursive = true);

        void WriteSuffixVarIdentBegin(const TypeDenoter& lhsTypeDen, VarIdent* ast);
        void WriteSuffixVarIdentEnd(const TypeDenoter& lhsTypeDen, VarIdent* ast);

        /* --- Type denoter --- */

        void WriteDataType(DataType dataType, const AST* ast = nullptr);
        void WriteTypeDenoter(const TypeDenoter& typeDenoter, const AST* ast = nullptr);

        /* --- Function call --- */

        void AssertIntrinsicNumArgs(FunctionCall* ast, std::size_t numArgsMin, std::size_t numArgsMax = ~0);

        void WriteFunctionCallStandard(FunctionCall* ast);
        void WriteFunctionCallIntrinsicMul(FunctionCall* ast);
        void WriteFunctionCallIntrinsicRcp(FunctionCall* ast);
        void WriteFunctionCallIntrinsicAtomic(FunctionCall* ast);
        void WriteFunctionCallIntrinsicTex(FunctionCall* ast);

        /* --- Structure --- */

        void WriteStructDecl(StructDecl* ast, bool writeSemicolon, bool allowNestedStruct = false);
        void WriteStructDeclMembers(StructDecl* ast);

        /* --- Misc --- */

        void WriteParameter(VarDeclStmnt* ast);
        void WriteScopedStmnt(Stmnt* ast);

        void WriteArrayDims(const std::vector<ExprPtr>& arrayDims);

        /* === Members === */

        ShaderTarget        shaderTarget_           = ShaderTarget::VertexShader;
        OutputShaderVersion versionOut_             = OutputShaderVersion::GLSL;
        bool                allowExtensions_        = false;
        bool                allowLineMarks_         = true;
        std::string         nameManglingPrefix_;
        Statistics*         stats_                  = nullptr;

        bool                isInsideEntryPoint_     = false;
        bool                isInsideInterfaceBlock_ = false;

        #if 1//TODO: remove all HLSL type mappings from here
        std::map<std::string, std::string> texFuncMap_;         // <hlsl-function, glsl-function>
        #endif

};


} // /namespace Xsc


#endif



// ================================================================================