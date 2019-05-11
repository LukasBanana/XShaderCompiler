/*
 * HLSLAnalyzer.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_HLSL_ANALYZER_H
#define XSC_HLSL_ANALYZER_H


#include "Analyzer.h"
#include "ShaderVersion.h"
#include "Variant.h"
#include "Flags.h"
#include <map>
#include <set>


namespace Xsc
{


struct HLSLIntrinsicEntry;

// HLSL context analyzer.
class HLSLAnalyzer : public Analyzer
{

    public:

        HLSLAnalyzer(Log* log = nullptr);

    private:

        using OnOverrideProc            = ASTSymbolTable::OnOverrideProc;
        using OnValidAttributeValueProc = std::function<bool(const AttributeValue)>;
        using OnAssignTypeDenoterProc   = std::function<void(const TypeDenoterPtr&)>;

        struct PrefixArgs
        {
            bool        inIsPostfixStatic;
            StructDecl* outPrefixBaseStruct;
        };

        void DecorateASTPrimary(
            Program&            program,
            const ShaderInput&  inputDesc,
            const ShaderOutput& outputDesc
        ) override;

        void ErrorIfAttributeNotFound(bool found, const std::string& attribDesc);

        // Returns true, if the input shader version if either HLSL3 or Cg.
        bool IsD3D9ShaderModel() const;

        /* === Visitor implementation === */

        DECL_VISIT_PROC( Program           );
        DECL_VISIT_PROC( CodeBlock         );
        DECL_VISIT_PROC( Attribute         );
        DECL_VISIT_PROC( ArrayDimension    );
        DECL_VISIT_PROC( TypeSpecifier     );

        DECL_VISIT_PROC( VarDecl           );
        DECL_VISIT_PROC( BufferDecl        );
        DECL_VISIT_PROC( SamplerDecl       );
        DECL_VISIT_PROC( StructDecl        );
        DECL_VISIT_PROC( AliasDecl         );
        DECL_VISIT_PROC( FunctionDecl      );

        DECL_VISIT_PROC( BufferDeclStmt   );
        DECL_VISIT_PROC( UniformBufferDecl );
        DECL_VISIT_PROC( VarDeclStmt      );
        DECL_VISIT_PROC( BasicDeclStmt    );

        DECL_VISIT_PROC( CodeBlockStmt    );
        DECL_VISIT_PROC( ForLoopStmt      );
        DECL_VISIT_PROC( WhileLoopStmt    );
        DECL_VISIT_PROC( DoWhileLoopStmt  );
        DECL_VISIT_PROC( IfStmt           );
        DECL_VISIT_PROC( SwitchStmt       );
        DECL_VISIT_PROC( ExprStmt         );
        DECL_VISIT_PROC( ReturnStmt       );

        DECL_VISIT_PROC( UnaryExpr         );
        DECL_VISIT_PROC( PostUnaryExpr     );
        DECL_VISIT_PROC( CallExpr          );
        DECL_VISIT_PROC( AssignExpr        );
        DECL_VISIT_PROC( ObjectExpr        );
        DECL_VISIT_PROC( ArrayExpr         );

        /* ----- Declarations ----- */

        void AnalyzeVarDecl(VarDecl* varDecl);
        void AnalyzeVarDeclLocal(VarDecl* varDecl, bool registerVarIdent = true);
        void AnalyzeVarDeclStaticMember(VarDecl* varDecl);

        /* ----- Call expressions ----- */

        void AnalyzeCallExpr(CallExpr* callExpr);
        void AnalyzeCallExprPrimary(CallExpr* callExpr, const TypeDenoter* prefixTypeDenoter = nullptr);
        void AnalyzeCallExprFunction(CallExpr* callExpr, bool isStatic = false, const Expr* prefixExpr = nullptr, const TypeDenoter* prefixTypeDenoter = nullptr);
        void AnalyzeCallExprIntrinsic(CallExpr* callExpr, const HLSLIntrinsicEntry& intr, bool isStatic = false, const TypeDenoter* prefixTypeDenoter = nullptr);
        void AnalyzeCallExprIntrinsicPrimary(CallExpr* callExpr, const HLSLIntrinsicEntry& intr);
        void AnalyzeCallExprIntrinsicFromBufferType(const CallExpr* callExpr, const BufferType bufferType);

        void AnalyzeIntrinsicWrapperInlining(CallExpr* callExpr);

        /* ----- Object expressions ----- */

        void AnalyzeObjectExpr(ObjectExpr* expr, PrefixArgs* args);
        void AnalyzeObjectExprVarDeclFromStruct(ObjectExpr* expr, StructDecl* baseStructDecl, const StructTypeDenoter& structTypeDen);
        void AnalyzeObjectExprBaseStructDeclFromStruct(ObjectExpr* expr, PrefixArgs& outputArgs, const StructTypeDenoter& structTypeDen);

        bool AnalyzeStaticAccessExpr(const Expr* prefixExpr, bool isStatic, const AST* ast = nullptr);
        bool AnalyzeStaticTypeSpecifier(const TypeSpecifier* typeSpecifier, const std::string& ident, const Expr* expr, bool isStatic);

        void AnalyzeLValueExpr(Expr* expr, const AST* ast = nullptr, VarDecl* param = nullptr);
        void AnalyzeLValueExprObject(ObjectExpr* objectExpr, const AST* ast = nullptr, VarDecl* param = nullptr);

        /* ----- Array expressions ----- */

        void AnalyzeArrayExpr(ArrayExpr* expr);

        /* ----- Entry point ----- */

        void AnalyzeEntryPoint(FunctionDecl* funcDecl);
        void AnalyzeEntryPointInputOutput(FunctionDecl* funcDecl);
        //void AnalyzeEntryPointInputOutputGeometryShader(FunctionDecl* funcDecl);

        void AnalyzeEntryPointParameter(FunctionDecl* funcDecl, VarDeclStmt* param);
        void AnalyzeEntryPointParameterInOut(FunctionDecl* funcDecl, VarDecl* varDecl, bool input, TypeDenoterPtr varTypeDen = nullptr);
        void AnalyzeEntryPointParameterInOutVariable(FunctionDecl* funcDecl, VarDecl* varDecl, bool input);
        void AnalyzeEntryPointParameterInOutStruct(FunctionDecl* funcDecl, StructDecl* structDecl, bool input);
        void AnalyzeEntryPointParameterInOutBuffer(FunctionDecl* funcDecl, VarDecl* varDecl, BufferTypeDenoter* bufferTypeDen, bool input);

        void AnalyzeEntryPointAttributes(const std::vector<AttributePtr>& attribs);
        void AnalyzeEntryPointAttributesTessControlShader(const std::vector<AttributePtr>& attribs);
        void AnalyzeEntryPointAttributesTessEvaluationShader(const std::vector<AttributePtr>& attribs);
        void AnalyzeEntryPointAttributesGeometryShader(const std::vector<AttributePtr>& attribs);
        void AnalyzeEntryPointAttributesFragmentShader(const std::vector<AttributePtr>& attribs);
        void AnalyzeEntryPointAttributesComputeShader(const std::vector<AttributePtr>& attribs);

        void AnalyzeEntryPointSemantics(FunctionDecl* funcDecl, const std::vector<Semantic>& inSemantics, const std::vector<Semantic>& outSemantics);

        void AnalyzeEntryPointOutput(Expr* expr);

        /* ----- Secondary entry point ----- */

        void AnalyzeSecondaryEntryPoint(FunctionDecl* funcDecl, bool isPatchConstantFunc = false);
        void AnalyzeSecondaryEntryPointAttributes(const std::vector<AttributePtr>& attribs);
        void AnalyzeSecondaryEntryPointAttributesTessEvaluationShader(const std::vector<AttributePtr>& attribs);

        /* ----- Attributes ----- */

        bool AnalyzeNumArgsAttribute(Attribute* attrib, std::size_t minNumArgs, std::size_t maxNumArgs, bool required);
        bool AnalyzeNumArgsAttribute(Attribute* attrib, std::size_t expectedNumArgs, bool required = true);

        void AnalyzeAttributeDomain(Attribute* attrib, bool required = true);
        void AnalyzeAttributeOutputTopology(Attribute* attrib, bool required = true);
        void AnalyzeAttributePartitioning(Attribute* attrib, bool required = true);
        void AnalyzeAttributeOutputControlPoints(Attribute* attrib);
        void AnalyzeAttributePatchConstantFunc(Attribute* attrib);

        void AnalyzeAttributeMaxVertexCount(Attribute* attrib);

        void AnalyzeAttributeNumThreads(Attribute* attrib);
        void AnalyzeAttributeNumThreadsArgument(Expr* expr, unsigned int& value);

        void AnalyzeAttributeValue(
            Expr*                               argExpr,
            AttributeValue&                     value,
            const OnValidAttributeValueProc&    expectedValueFunc,
            const std::string&                  expectationDesc,
            bool                                required            = true
        );

        bool AnalyzeAttributeValuePrimary(
            Expr*                               argExpr,
            AttributeValue&                     value,
            const OnValidAttributeValueProc&    expectedValueFunc,
            std::string&                        literalValue
        );

        /* ----- Semantic ----- */

        void AnalyzeSemantic(IndexedSemantic& semantic);
        void AnalyzeSemanticSM3(IndexedSemantic& semantic, bool input);
        void AnalyzeSemanticSM3Remaining();
        void AnalyzeSemanticVarDecl(IndexedSemantic& semantic, VarDecl* varDecl);
        void AnalyzeSemanticFunctionReturn(IndexedSemantic& semantic);

        /* ----- Language extensions ----- */

        #ifdef XSC_ENABLE_LANGUAGE_EXT

        void AnalyzeExtAttributes(std::vector<AttributePtr>& attribs, const TypeDenoterPtr& typeDen);

        void AnalyzeAttributeLayout(Attribute* attrib, const TypeDenoterPtr& typeDen);

        void AnalyzeAttributeSpace(Attribute* attrib, const TypeDenoterPtr& typeDen);
        bool AnalyzeAttributeSpaceIdent(Attribute* attrib, std::size_t argIndex, std::string& ident);

        void AnalyzeVectorSpaceAssign(
            TypedAST*                       lhs,
            const TypeDenoter&              rhsTypeDen,
            const OnAssignTypeDenoterProc&  assignTypeDenProc   = nullptr,
            bool                            swapAssignOrder     = false
        );

        #endif

        /* ----- Misc ----- */

        void AnalyzeArrayDimensionList(const std::vector<ArrayDimensionPtr>& arrayDims);

        void AnalyzeParameter(VarDeclStmt* param);

    private:

        Program*            program_                    = nullptr;

        std::string         entryPoint_;
        std::string         secondaryEntryPoint_;
        bool                secondaryEntryPointFound_   = false;

        ShaderTarget        shaderTarget_               = ShaderTarget::VertexShader;
        InputShaderVersion  versionIn_                  = InputShaderVersion::HLSL5;
        ShaderVersion       shaderModel_                = { 5, 0 };
        bool                preferWrappers_             = false;

        std::set<VarDecl*>  varDeclSM3Semantics_;

        #ifdef XSC_ENABLE_LANGUAGE_EXT

        Flags               extensions_;

        #endif

};


} // /namespace Xsc


#endif



// ================================================================================