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
#include "Flags.h"
#include <map>
#include <set>
#include <vector>
#include <initializer_list>
#include <functional>


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

        // Function callback interface for entries in a layout qualifier.
        using LayoutEntryFunctor = std::function<void()>;

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

        // Returns true if the output shader language is GLSL (for OpenGL 2+).
        bool IsGLSL() const;

        // Returns true if the output shader language is ESSL (for OpenGL ES 2+).
        bool IsESSL() const;

        // Returns true if the output shader language is VKSL (for Vulkan/SPIR-V).
        bool IsVKSL() const;

        // Returns true if the 'GL_ARB_shading_language_420pack' is explicitly available.
        bool HasShadingLanguage420Pack() const;

        // Returns true if separate objects for samplers & textures should be used.
        bool UseSeparateSamplers() const;

        // Returns the GLSL keyword for the specified buffer type or reports and error.
        const std::string* BufferTypeToKeyword(const BufferType bufferType, const AST* ast = nullptr);

        // Returns the GLSL keyword for the specified sampler type or reports and error.
        const std::string* SamplerTypeToKeyword(const SamplerType samplerType, const AST* ast = nullptr);

        // Returns true if the specified type denoter is compatible with the semantic (e.g. 'SV_VertexID' is incompatible with 'UInt').
        bool IsTypeCompatibleWithSemantic(const Semantic semantic, const TypeDenoter& typeDenoter);

        // Report warning of optional reminaing feedback.
        void ReportOptionalFeedback();

        // Error for intrinsics, that can not be mapped to GLSL keywords.
        void ErrorIntrinsic(const std::string& intrinsicName, const AST* ast = nullptr);

        // Returns the number of binding locations required by the specified type, or -1 if type is invalid.
        int GetNumBindingLocations(const TypeDenoter* typeDenoter);

        // Attempts to find an empty binding location for the specified type, or returns -1 if it cannot find one. 
        int GetBindingLocation(const TypeDenoter* typeDenoter, bool input);

        /* --- Visitor implementation --- */

        DECL_VISIT_PROC( Program           );
        DECL_VISIT_PROC( CodeBlock         );
        DECL_VISIT_PROC( SwitchCase        );
        DECL_VISIT_PROC( ArrayDimension    );
        DECL_VISIT_PROC( TypeSpecifier     );

        DECL_VISIT_PROC( VarDecl           );
        DECL_VISIT_PROC( StructDecl        );
        DECL_VISIT_PROC( SamplerDecl       );

        DECL_VISIT_PROC( FunctionDecl      );
        DECL_VISIT_PROC( UniformBufferDecl );
        DECL_VISIT_PROC( BufferDeclStmnt   );
        DECL_VISIT_PROC( SamplerDeclStmnt  );
        DECL_VISIT_PROC( VarDeclStmnt      );
        DECL_VISIT_PROC( AliasDeclStmnt    );
        DECL_VISIT_PROC( BasicDeclStmnt    );

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

        DECL_VISIT_PROC( SequenceExpr      );
        DECL_VISIT_PROC( LiteralExpr       );
        DECL_VISIT_PROC( TypeSpecifierExpr );
        DECL_VISIT_PROC( TernaryExpr       );
        DECL_VISIT_PROC( BinaryExpr        );
        DECL_VISIT_PROC( UnaryExpr         );
        DECL_VISIT_PROC( PostUnaryExpr     );
        DECL_VISIT_PROC( CallExpr          );
        DECL_VISIT_PROC( BracketExpr       );
        DECL_VISIT_PROC( ObjectExpr        );
        DECL_VISIT_PROC( AssignExpr        );
        DECL_VISIT_PROC( ArrayExpr         );
        DECL_VISIT_PROC( CastExpr          );
        DECL_VISIT_PROC( InitializerExpr   );

        /* --- Helper functions for code generation --- */

        /* ----- Pre processing AST ----- */

        void PreProcessAST(const ShaderInput& inputDesc, const ShaderOutput& outputDesc);
        void PreProcessStructParameterAnalyzer(const ShaderInput& inputDesc);
        void PreProcessTypeConverter();
        void PreProcessExprConverterPrimary();
        void PreProcessGLSLConverter(const ShaderInput& inputDesc, const ShaderOutput& outputDesc);
        void PreProcessFuncNameConverter();
        void PreProcessReferenceAnalyzer(const ShaderInput& inputDesc);
        void PreProcessExprConverterSecondary();

        /* ----- Basics ----- */

        // Writes a comment (single or multi-line comments).
        void WriteComment(const std::string& text);

        void WriteLineMark(int lineNumber);
        void WriteLineMark(const TokenPtr& tkn);
        void WriteLineMark(const AST* ast);

        /* ----- Program ----- */

        void WriteProgramHeader();
        void WriteProgramHeaderVersion();
        void WriteProgramHeaderComment();
        void WriteProgramHeaderExtension(const std::string& extensionName);

        /* ----- Global layouts ----- */

        void WriteGlobalLayouts();
        bool WriteGlobalLayoutsTessControl(const Program::LayoutTessControlShader& layout);
        bool WriteGlobalLayoutsTessEvaluation(const Program::LayoutTessEvaluationShader& layout);
        bool WriteGlobalLayoutsGeometry(const Program::LayoutGeometryShader& layout);
        bool WriteGlobalLayoutsFragment(const Program::LayoutFragmentShader& layout);
        bool WriteGlobalLayoutsCompute(const Program::LayoutComputeShader& layout);

        /* ----- Built-in block redeclarations ----- */

        void WriteBuiltinBlockRedeclarations();
        void WriteBuiltinBlockRedeclarationsPerVertex(bool input, const std::string& name = "");

        /* ----- Layout ----- */

        void WriteLayout(const std::initializer_list<LayoutEntryFunctor>& entryFunctors);
        void WriteLayout(const std::string& value);
        void WriteLayoutGlobal(const std::initializer_list<LayoutEntryFunctor>& entryFunctors, const LayoutEntryFunctor& varFunctor, const std::string& modifier);
        void WriteLayoutGlobalIn(const std::initializer_list<LayoutEntryFunctor>& entryFunctors, const LayoutEntryFunctor& varFunctor = nullptr);
        void WriteLayoutGlobalOut(const std::initializer_list<LayoutEntryFunctor>& entryFunctors, const LayoutEntryFunctor& varFunctor = nullptr);
        void WriteLayoutBinding(const std::vector<RegisterPtr>& slotRegisters);

        /* ----- Input semantics ----- */

        void WriteLocalInputSemantics(FunctionDecl* entryPoint);
        void WriteLocalInputSemanticsVarDecl(VarDecl* varDecl);
        void WriteLocalInputSemanticsStructDeclParam(VarDeclStmnt* param, StructDecl* structDecl);

        void WriteGlobalInputSemantics(FunctionDecl* entryPoint);
        void WriteGlobalInputSemanticsVarDecl(VarDecl* varDecl);

        /* ----- Output semantics ----- */

        void WriteLocalOutputSemantics(FunctionDecl* entryPoint);
        void WriteLocalOutputSemanticsStructDeclParam(VarDeclStmnt* param, StructDecl* structDecl);

        void WriteGlobalOutputSemantics(FunctionDecl* entryPoint);
        void WriteGlobalOutputSemanticsVarDecl(VarDecl* varDecl, bool useSemanticName = false);
        void WriteGlobalOutputSemanticsSlot(TypeSpecifier* typeSpecifier, IndexedSemantic& semantic, const std::string& ident, VarDecl* varDecl = nullptr);

        void WriteOutputSemanticsAssignment(Expr* expr, bool writeAsListedExpr = false);
        void WriteOutputSemanticsAssignmentStructDeclParam(
            const FunctionDecl::ParameterStructure& paramStruct, bool writeAsListedExpr = false, const std::string& tempIdent = "output"
        );

        /* ----- Uniforms ----- */

        void WriteGlobalUniforms();
        void WriteGlobalUniformsParameter(VarDeclStmnt* param);

        // Writes the specified variable identifier or a system value if the identifier has a system value semantic.
        void WriteVarDeclIdentOrSystemValue(VarDecl* varDecl, int arrayIndex = -1);

        /* ----- Object expression ----- */

        void WriteObjectExpr(const ObjectExpr& objectExpr);
        void WriteObjectExprIdent(const ObjectExpr& objectExpr, bool writePrefix = true);
        void WriteObjectExprIdentOrSystemValue(const ObjectExpr& objectExpr, Decl* symbol);

        /* ----- Array expression ----- */

        void WriteArrayExpr(const ArrayExpr& arrayExpr);

        void WriteArrayIndices(const std::vector<ExprPtr>& arrayIndices);

        /* ----- Type denoter ----- */

        void WriteStorageClasses(const std::set<StorageClass>& storageClasses, const AST* ast = nullptr);
        void WriteInterpModifiers(const std::set<InterpModifier>& interpModifiers, const AST* ast = nullptr);
        void WriteTypeModifiers(const std::set<TypeModifier>& typeModifiers, const TypeDenoterPtr& typeDenoter = nullptr);
        void WriteTypeModifiersFrom(const TypeSpecifierPtr& typeSpecifier);

        void WriteDataType(DataType dataType, bool writePrecisionSpecifier = false, const AST* ast = nullptr);

        void WriteTypeDenoter(const TypeDenoter& typeDenoter, bool writePrecisionSpecifier = false, const AST* ast = nullptr);

        /* ----- Function declaration ----- */

        void WriteFunction(FunctionDecl* ast);
        void WriteFunctionEntryPoint(FunctionDecl* ast);
        void WriteFunctionEntryPointBody(FunctionDecl* ast);
        void WriteFunctionSecondaryEntryPoint(FunctionDecl* ast);

        /* ----- Call expressions ----- */

        void AssertIntrinsicNumArgs(CallExpr* callExpr, std::size_t numArgsMin, std::size_t numArgsMax = ~0);

        void WriteCallExprStandard(CallExpr* callExpr);
        void WriteCallExprIntrinsicMul(CallExpr* callExpr);
        void WriteCallExprIntrinsicRcp(CallExpr* callExpr);
        void WriteCallExprIntrinsicClip(CallExpr* callExpr);
        void WriteCallExprIntrinsicAtomic(CallExpr* callExpr);
        void WriteCallExprIntrinsicAtomicCompSwap(CallExpr* callExpr);
        void WriteCallExprIntrinsicImageAtomic(CallExpr* callExpr);
        void WriteCallExprIntrinsicImageAtomicCompSwap(CallExpr* callExpr);
        void WriteCallExprIntrinsicStreamOutputAppend(CallExpr* callExpr);
        void WriteCallExprIntrinsicTextureQueryLod(CallExpr* callExpr, bool clamped);

        void WriteCallExprArguments(CallExpr* callExpr, std::size_t firstArgIndex = 0, std::size_t numWriteArgs = ~0u);

        /* ----- Intrinsics wrapper ----- */

        // Writes all required wrapper functions for referenced intrinsics.
        void WriteWrapperIntrinsics();
        void WriteWrapperIntrinsicsClip(const IntrinsicUsage& usage);
        void WriteWrapperIntrinsicsLit(const IntrinsicUsage& usage);
        void WriteWrapperIntrinsicsSinCos(const IntrinsicUsage& usage);
        void WriteWrapperIntrinsicsMemoryBarrier(const Intrinsic intrinsic, bool groupSync);

        void WriteWrapperMatrixSubscript(const MatrixSubscriptUsage& usage);

        /* ----- Structure ----- */

        bool WriteStructDecl(StructDecl* structDecl, bool endWithSemicolon);

        /* ----- BufferDecl ----- */

        void WriteBufferDecl(BufferDecl* bufferDecl);
        void WriteBufferDeclTexture(BufferDecl* bufferDecl);
        void WriteBufferDeclStorageBuffer(BufferDecl* bufferDecl);

        /* ----- SamplerDecl ----- */

        void WriteSamplerDecl(SamplerDecl& samplerDecl);

        /* ----- Misc ----- */

        void WriteStmntComment(Stmnt* ast, bool insertBlank = false);

        template <typename T>
        void WriteStmntList(const std::vector<T>& stmnts, bool isGlobalScope = false);

        void WriteParameter(VarDeclStmnt* ast);
        void WriteScopedStmnt(Stmnt* ast);

        void WriteLiteral(const std::string& value, const DataType& dataType, const AST* ast = nullptr);

        /* === Members === */

        struct VertexSemanticLoc
        {
            int     location;
            bool    found;
        };

        OutputShaderVersion                     versionOut_             = OutputShaderVersion::GLSL;
        NameMangling                            nameMangling_;
        std::map<CiString, VertexSemanticLoc>   vertexSemanticsMap_;
        std::string                             entryPointName_;

        bool                                    allowExtensions_        = false;
        bool                                    explicitBinding_        = false;
        bool                                    preserveComments_       = false;
        bool                                    allowLineMarks_         = false;
        bool                                    compactWrappers_        = false;
        bool                                    alwaysBracedScopes_     = false;
        bool                                    separateShaders_        = false;
        bool                                    separateSamplers_       = true;
        bool                                    autoBinding_            = false;
        bool                                    writeHeaderComment_     = true;

        std::set<int>                           usedInLocationsSet_;
        std::set<int>                           usedOutLocationsSet_;

        #ifdef XSC_ENABLE_LANGUAGE_EXT

        Flags                                   extensions_;                        // Flags of all enabled language extensions.

        #endif
};


} // /namespace Xsc


#endif



// ================================================================================
