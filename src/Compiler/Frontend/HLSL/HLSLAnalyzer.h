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
        
        void DeclareIntrinsics();

        /*
        Returns the current (top level) function in the call stack
        or null if the AST traversion is in the global scope.
        */
        FunctionCall* CurrentFunction() const;

        /* === Visitor implementation === */

        DECL_VISIT_PROC( Program           );
        DECL_VISIT_PROC( CodeBlock         );
        DECL_VISIT_PROC( FunctionCall      );
        
        DECL_VISIT_PROC( VarDecl           );
        DECL_VISIT_PROC( StructDecl        );
        DECL_VISIT_PROC( AliasDecl         );

        DECL_VISIT_PROC( FunctionDecl      );
        DECL_VISIT_PROC( BufferDeclStmnt   );
        DECL_VISIT_PROC( TextureDeclStmnt  );
        DECL_VISIT_PROC( SamplerDeclStmnt  );
        DECL_VISIT_PROC( StructDeclStmnt   );
        DECL_VISIT_PROC( VarDeclStmnt      );

        DECL_VISIT_PROC( ForLoopStmnt      );
        DECL_VISIT_PROC( WhileLoopStmnt    );
        DECL_VISIT_PROC( DoWhileLoopStmnt  );
        DECL_VISIT_PROC( IfStmnt           );
        DECL_VISIT_PROC( ElseStmnt         );
        DECL_VISIT_PROC( SwitchStmnt       );
        DECL_VISIT_PROC( ExprStmnt         );
        DECL_VISIT_PROC( ReturnStmnt       );

        DECL_VISIT_PROC( VarAccessExpr     );

        DECL_VISIT_PROC( VarType           );

        /* --- Helper functions for context analysis --- */

        void DecorateEntryInOut(VarDeclStmnt* ast, bool isInput);
        void DecorateEntryInOut(VarType* ast, bool isInput);

        bool FetchSystemValueSemantic(const std::vector<VarSemanticPtr>& varSemantics, std::string& semanticName) const;
        bool IsSystemValueSemnatic(std::string semantic) const;

        void AnalyzeFunctionCallStandard(FunctionCall* ast);
        void AnalyzeFunctionCallIntrinsic(FunctionCall* ast, const HLSLIntrinsicEntry& intr);

        void AnalyzeVarIdent(VarIdent* varIdent);
        void AnalyzeVarIdentWithSymbol(VarIdent* varIdent, AST* symbol);
        void AnalyzeVarIdentWithSymbolVarDecl(VarIdent* varIdent, VarDecl* varDecl);
        void AnalyzeVarIdentWithSymbolTextureDecl(VarIdent* varIdent, TextureDecl* textureDecl);
        void AnalyzeVarIdentWithSymbolSamplerDecl(VarIdent* varIdent, SamplerDecl* samplerDecl);

        /* === Members === */

        Program*                    program_        = nullptr;

        ShaderInput                 inputDesc_;
        ShaderOutput                outputDesc_;

        std::string                 entryPoint_;
        ShaderTarget                shaderTarget_   = ShaderTarget::VertexShader;
        InputShaderVersion          versionIn_      = InputShaderVersion::HLSL5;
        ShaderVersion               shaderModel_    { 5, 0 };
        std::string                 localVarPrefix_;

        // Structure stack to collect all members with system value semantic (SV_...).
        std::vector<StructDecl*>    structStack_;

};


} // /namespace Xsc


#endif



// ================================================================================