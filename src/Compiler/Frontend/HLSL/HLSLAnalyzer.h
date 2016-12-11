/*
 * HLSLAnalyzer.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_HLSL_ANALYZER_H
#define XSC_HLSL_ANALYZER_H


#include "Analyzer.h"
#include "ShaderVersion.h"
#include "Variant.h"
#include <map>


namespace Xsc
{


struct HLSLIntrinsicEntry;

// HLSL context analyzer.
class HLSLAnalyzer : public Analyzer
{
    
    public:
        
        HLSLAnalyzer(Log* log = nullptr);

    private:
        
        using OnOverrideProc = ASTSymbolTable::OnOverrideProc;

        /* === Functions === */

        void DecorateASTPrimary(
            Program& program,
            const ShaderInput& inputDesc,
            const ShaderOutput& outputDesc
        ) override;
        
        /* === Visitor implementation === */

        DECL_VISIT_PROC( Program           );
        DECL_VISIT_PROC( CodeBlock         );
        DECL_VISIT_PROC( FunctionCall      );
        DECL_VISIT_PROC( VarType           );
        
        DECL_VISIT_PROC( VarDecl           );
        DECL_VISIT_PROC( BufferDecl        );
        DECL_VISIT_PROC( SamplerDecl       );
        DECL_VISIT_PROC( StructDecl        );
        DECL_VISIT_PROC( AliasDecl         );

        DECL_VISIT_PROC( FunctionDecl      );
        DECL_VISIT_PROC( BufferDeclStmnt   );
        DECL_VISIT_PROC( UniformBufferDecl );
      //DECL_VISIT_PROC( VarDeclStmnt      );

        DECL_VISIT_PROC( ForLoopStmnt      );
        DECL_VISIT_PROC( WhileLoopStmnt    );
        DECL_VISIT_PROC( DoWhileLoopStmnt  );
        DECL_VISIT_PROC( IfStmnt           );
        DECL_VISIT_PROC( ElseStmnt         );
        DECL_VISIT_PROC( SwitchStmnt       );
        DECL_VISIT_PROC( ExprStmnt         );
        DECL_VISIT_PROC( ReturnStmnt       );

        DECL_VISIT_PROC( TypeNameExpr      );
        DECL_VISIT_PROC( SuffixExpr        );
        DECL_VISIT_PROC( VarAccessExpr     );

        /* --- Helper functions for context analysis --- */

        void AnalyzeFunctionCallStandard(FunctionCall* ast);
        void AnalyzeFunctionCallIntrinsic(FunctionCall* ast, const HLSLIntrinsicEntry& intr);

        void AnalyzeIntrinsicWrapperInlining(FunctionCall* ast);

        void AnalyzeVarIdent(VarIdent* varIdent);
        void AnalyzeVarIdentWithSymbol(VarIdent* varIdent, AST* symbol);
        void AnalyzeVarIdentWithSymbolVarDecl(VarIdent* varIdent, VarDecl* varDecl);
        void AnalyzeVarIdentWithSymbolBufferDecl(VarIdent* varIdent, BufferDecl* bufferDecl);
        void AnalyzeVarIdentWithSymbolSamplerDecl(VarIdent* varIdent, SamplerDecl* samplerDecl);

        void AnalyzeEntryPoint(FunctionDecl* funcDecl);
        void AnalyzeEntryPointParameter(FunctionDecl* funcDecl, VarDeclStmnt* param);
        void AnalyzeEntryPointParameterInOut(FunctionDecl* funcDecl, VarDecl* varDecl, bool input);
        void AnalyzeEntryPointStructInOut(FunctionDecl* funcDecl, StructDecl* structDecl, const std::string& structAliasName, bool input);
        void AnalyzeEntryPointAttributes(const std::vector<AttributePtr>& attribs);
        void AnalyzeEntryPointAttributesComputeShader(const std::vector<AttributePtr>& attribs);

        void AnalyzeAttribute(Attribute* ast);

        void AnalyzeSemantic(IndexedSemantic& semantic);

        void AnalyzeEndOfScopes(FunctionDecl& funcDecl);

        /* === Members === */

        Program*            program_        = nullptr;

        std::string         entryPoint_;
        ShaderTarget        shaderTarget_   = ShaderTarget::VertexShader;
        InputShaderVersion  versionIn_      = InputShaderVersion::HLSL5;
        ShaderVersion       shaderModel_    = { 5, 0 };
        bool                preferWrappers_ = false;

};


} // /namespace Xsc


#endif



// ================================================================================