/*
 * GLSLGenerator.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_GLSL_GENERATOR_H
#define XSC_GLSL_GENERATOR_H


#include <Xsc/Xsc.h>
#include "AST.h"
#include "Generator.h"
#include "Visitor.h"
#include "Token.h"
#include "ASTEnums.h"
#include "CiString.h"
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
        DECL_VISIT_PROC( ArrayDimension    );
        DECL_VISIT_PROC( TypeName          );
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

        /* --- Program --- */

        void WriteProgramHeader();
        void WriteProgramHeaderVersion();
        void WriteProgramHeaderExtension(const std::string& extensionName);

        /* --- Layouts --- */

        void WriteGlobalLayouts();
        bool WriteGlobalLayoutsTessControl(const Program::LayoutTessControlShader& layout);
        bool WriteGlobalLayoutsTessEvaluation(const Program::LayoutTessEvaluationShader& layout);
        bool WriteGlobalLayoutsGeometry(const Program::LayoutGeometryShader& layout);
        bool WriteGlobalLayoutsFragment(const Program::LayoutFragmentShader& layout);
        bool WriteGlobalLayoutsCompute(const Program::LayoutComputeShader& layout);

        /* --- Input semantics --- */

        void WriteLocalInputSemantics(FunctionDecl* entryPoint);
        void WriteLocalInputSemanticsVarDecl(VarDecl* varDecl);
        void WriteLocalInputSemanticsStructDeclParam(VarDeclStmnt* param, StructDecl* structDecl);
        
        void WriteGlobalInputSemantics(FunctionDecl* entryPoint);
        void WriteGlobalInputSemanticsVarDecl(VarDecl* varDecl);

        /* --- Output semantics --- */

        void WriteLocalOutputSemantics(FunctionDecl* entryPoint);
        void WriteLocalOutputSemanticsVarDecl(VarDecl* varDecl);
        void WriteLocalOutputSemanticsStructDeclParam(VarDeclStmnt* param, StructDecl* structDecl);
        
        void WriteGlobalOutputSemantics(FunctionDecl* entryPoint);
        void WriteGlobalOutputSemanticsVarDecl(VarDecl* varDecl, bool useSemanticName = false);
        void WriteGlobalOutputSemanticsSlot(TypeName* varType, const IndexedSemantic& semantic, const std::string& ident, VarDecl* varDecl = nullptr);

        void WriteOutputSemanticsAssignment(Expr* ast);
        void WriteOutputSemanticsAssignmentStructDeclParam(VarDecl* paramVar, StructDecl* structDecl);

        /* --- Uniforms --- */

        void WriteGlobalUniforms();
        void WriteGlobalUniformsParameter(VarDeclStmnt* param);

        /* --- VarIdent --- */

        // Returns the first VarIdent AST node which has a system value semantic, or null if no such AST node was found.
        VarIdent* FindSystemValueVarIdent(VarIdent* ast);

        // Returns the final identifier string from the specified variable identifier.
        const std::string& FinalIdentFromVarIdent(VarIdent* ast);

        void WriteVarIdent(VarIdent* ast, bool recursive = true);

        // Writes the specified variable identifier or a system value if the VarIdent has a system value semantic.
        void WriteVarIdentOrSystemValue(VarIdent* ast);

        /* --- Type denoter --- */

        void WriteStorageClasses(const std::set<StorageClass>& storageClasses, const AST* ast = nullptr);
        void WriteInterpModifiers(const std::set<InterpModifier>& interpModifiers, const AST* ast = nullptr);
        void WriteTypeModifiers(const std::set<TypeModifier>& typeModifiers, const TypeDenoterPtr& typeDenoter = nullptr);

        void WriteDataType(DataType dataType, bool writePrecisionSpecifier = false, const AST* ast = nullptr);

        void WriteTypeDenoter(const TypeDenoter& typeDenoter, bool writePrecisionSpecifier = false, const AST* ast = nullptr);

        /* --- Function declaration --- */

        void WriteFunction(FunctionDecl* ast);
        void WriteFunctionEntryPoint(FunctionDecl* ast);
        void WriteFunctionEntryPointBody(FunctionDecl* ast);
        void WriteFunctionSecondaryEntryPoint(FunctionDecl* ast);

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
        void WriteWrapperIntrinsicsSinCos(const IntrinsicUsage& usage);

        /* --- Structure --- */

        bool WriteStructDecl(StructDecl* ast, bool writeSemicolon, bool allowNestedStruct = false);
        bool WriteStructDeclStandard(StructDecl* ast, bool endWithSemicolon);
        bool WriteStructDeclInputOutputBlock(StructDecl* ast);
        void WriteStructDeclMembers(StructDecl* ast);

        /* --- BufferDecl --- */

        void WriteBufferDecl(BufferDecl* ast);
        void WriteBufferDeclTexture(BufferDecl* ast);
        void WriteBufferDeclStorageBuffer(BufferDecl* ast);

        /* --- Misc --- */

        void WriteStmntComment(Stmnt* ast, bool insertBlank = false);

        void WriteStmntList(const std::vector<StmntPtr>& stmnts, bool isGlobalScope = false);

        void WriteParameter(VarDeclStmnt* ast);
        void WriteScopedStmnt(Stmnt* ast);

        void WriteArrayIndices(const std::vector<ExprPtr>& arrayDims);

        void WriteLiteral(const std::string& value, const BaseTypeDenoter& baseTypeDen, const AST* ast = nullptr);

        /* === Members === */

        OutputShaderVersion     versionOut_             = OutputShaderVersion::GLSL;
        std::string             nameManglingPrefix_     = "xsc_";
        std::map<CiString, int> vertexSemanticsMap_;

        bool                    allowExtensions_        = false;
        bool                    explicitBinding_        = false;
        bool                    preserveComments_       = false;
        bool                    allowLineMarks_         = false;
        bool                    compactWrappers_        = true;
        bool                    alwaysBracedScopes_     = false;

        bool                    isInsideInterfaceBlock_ = false;
        bool                    isInsideUniformBuffer_  = false;
};


} // /namespace Xsc


#endif



// ================================================================================
