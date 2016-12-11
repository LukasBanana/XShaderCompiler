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
#include <set>
#include <vector>


namespace Xsc
{


struct TypeDenoter;
struct BaseTypeDenoter;

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

        // Returns true if the specified AST structure must be resolved.
        bool MustResolveStruct(StructDecl* ast) const;

        // Returns the GLSL keyword for the specified system value semantic (special case is Semantic::Target).
        std::unique_ptr<std::string> SystemValueToKeyword(const IndexedSemantic& semantic) const;

        // Returns true if there is a wrapper function for the specified intrinsic (e.g. "clip" intrinsic).
        bool IsWrappedIntrinsic(const Intrinsic intrinsic) const;

        // Returns true if the output shader language is ESSL (for OpenGL ES 2+).
        bool IsESSL() const;

        // Returns true if the output shader language is VKSL (for Vulkan/SPIR-V).
        bool IsVKSL() const;

        // Returns the GLSL keyword for the specified buffer type or reports and error.
        const std::string* BufferTypeToKeyword(const BufferType bufferType, const AST* ast = nullptr);

        // Returns the GLSL keyword for the specified sampler type or reports and error.
        const std::string* SamplerTypeToKeyword(const SamplerType samplerType, const AST* ast = nullptr);

        // Returns true if the specified type denoter is compatible with the semantic (e.g. 'SV_VertexID' is incompatible with 'UInt').
        bool IsTypeCompatibleWithSemantic(const Semantic semantic, const TypeDenoter& typeDenoter);

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
        DECL_VISIT_PROC( UniformBufferDecl );
        DECL_VISIT_PROC( BufferDeclStmnt   );
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

        /* --- Basics --- */

        // Writes a comment (single or multi-line comments).
        void WriteComment(const std::string& text);

        void WriteLineMark(int lineNumber);
        void WriteLineMark(const TokenPtr& tkn);
        void WriteLineMark(const AST* ast);

        void WriteScopeOpen(bool compact = false);
        void WriteScopeClose(bool compact = false, bool semicolon = false);

        /* --- Program --- */

        void WriteProgramHeader();
        void WriteProgramHeaderVersion();
        void WriteProgramHeaderExtension(const std::string& extensionName);

        /* --- Attributes --- */

        void WriteGlobalLayouts();
        void WriteGlobalLayoutFragCoord();

        bool WriteEntryPointAttributes();

        void WriteAttribute(Attribute* ast);
        void WriteAttributeNumThreads(Attribute* ast);
        void WriteAttributeNumThreadsArgument(Expr* ast);
        void WriteAttributeEarlyDepthStencil();

        /* --- Input semantics --- */

        void WriteLocalInputSemantics();
        void WriteLocalInputSemanticsVarDecl(VarDecl* varDecl);
        
        void WriteGlobalInputSemantics();
        void WriteGlobalInputSemanticsVarDecl(VarDecl* varDecl);

        /* --- Output semantics --- */

        void WriteLocalOutputSemantics();
        void WriteLocalOutputSemanticsVarDecl(VarDecl* varDecl);
        
        void WriteGlobalOutputSemantics();
        void WriteGlobalOutputSemanticsVarDecl(VarDecl* varDecl, bool useSemanticName = false);
        void WriteGlobalOutputSemanticsSlot(VarType* varType, const IndexedSemantic& semantic, const std::string& ident);

        void WriteOutputSemanticsAssignment(Expr* ast);

        /* --- Uniforms --- */

        void WriteGlobalUniforms();
        void WriteGlobalUniformsParameter(VarDeclStmnt* param);

        /* --- VarIdent --- */

        // Returns the final identifier string from the specified variable identifier.
        const std::string& FinalIdentFromVarIdent(VarIdent* ast);

        void WriteVarIdent(VarIdent* ast, bool recursive = true);

        void WriteSuffixVarIdentBegin(const TypeDenoter& lhsTypeDen, VarIdent* ast);
        void WriteSuffixVarIdentEnd(const TypeDenoter& lhsTypeDen, VarIdent* ast);

        /* --- Type denoter --- */

        void WriteTypeModifiers(const std::set<TypeModifier>& typeModifiers);
        void WriteDataType(DataType dataType, bool writePrecisionSpecifier = false, const AST* ast = nullptr);
        void WriteTypeDenoter(const TypeDenoter& typeDenoter, bool writePrecisionSpecifier = false, const AST* ast = nullptr);

        /* --- Function call --- */

        void AssertIntrinsicNumArgs(FunctionCall* ast, std::size_t numArgsMin, std::size_t numArgsMax = ~0);

        void WriteFunctionCallStandard(FunctionCall* ast);
        void WriteFunctionCallIntrinsicMul(FunctionCall* ast);
        void WriteFunctionCallIntrinsicRcp(FunctionCall* ast);
        void WriteFunctionCallIntrinsicClip(FunctionCall* ast);
        void WriteFunctionCallIntrinsicAtomic(FunctionCall* ast);

        /* --- Intrinsics wrapper functions --- */

        // Writes all required wrapper functions for referenced intrinsics.
        void WriteWrapperIntrinsics();
        void WriteWrapperIntrinsicsClip(const IntrinsicUsage& usage);

        /* --- Structure --- */

        bool WriteStructDecl(StructDecl* ast, bool writeSemicolon, bool allowNestedStruct = false);
        bool WriteStructDeclStandard(StructDecl* ast, bool writeSemicolon);
        bool WriteStructDeclInputOutputBlock(StructDecl* ast);
        void WriteStructDeclMembers(StructDecl* ast);

        /* --- Misc --- */

        void WriteStmntComment(Stmnt* ast, bool insertBlank = false);

        void WriteStmntList(const std::vector<StmntPtr>& stmnts, bool isGlobalScope = false);

        void WriteParameter(VarDeclStmnt* ast);
        void WriteScopedStmnt(Stmnt* ast);

        void WriteArrayDims(const std::vector<ExprPtr>& arrayDims);

        void WriteLiteral(const std::string& value, const BaseTypeDenoter& baseTypeDen, const AST* ast = nullptr);

        /* === Members === */

        ShaderTarget        shaderTarget_           = ShaderTarget::VertexShader;
        OutputShaderVersion versionOut_             = OutputShaderVersion::GLSL;
        bool                allowExtensions_        = false;
        bool                explicitBinding_        = false;
        bool                preserveComments_       = false;
        bool                allowLineMarks_         = false;
        bool                compactWrappers_        = true;
        bool                newLineOpenScope_       = true;
        std::string         nameManglingPrefix_     = "xsc_";

        bool                isInsideEntryPoint_     = false;
        bool                isInsideInterfaceBlock_ = false;

};


} // /namespace Xsc


#endif



// ================================================================================