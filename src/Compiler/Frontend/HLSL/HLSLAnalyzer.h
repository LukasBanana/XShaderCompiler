/*
 * HLSLAnalyzer.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_HLSL_ANALYZER_H
#define XSC_HLSL_ANALYZER_H


#include "Analyzer.h"
#include <map>


namespace Xsc
{


// HLSL context analyzer.
class HLSLAnalyzer : public Analyzer
{
    
    public:
        
        HLSLAnalyzer(Log* log = nullptr);

    private:
        
        using OnOverrideProc = ASTSymbolTable::OnOverrideProc;

        /* === Enumerations === */

        enum class IntrinsicClasses
        {
            Interlocked,
        };

        /* === Functions === */

        void DecorateASTPrimary(
            Program& program,
            const ShaderInput& inputDesc,
            const ShaderOutput& outputDesc
        ) override;
        
        void EstablishMaps();

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
        DECL_VISIT_PROC( AssignStmnt       );
        DECL_VISIT_PROC( ReturnStmnt       );

        DECL_VISIT_PROC( VarAccessExpr     );

        DECL_VISIT_PROC( VarType           );

        /* --- Helper functions for context analysis --- */

        void DecorateEntryInOut(VarDeclStmnt* ast, bool isInput);
        void DecorateEntryInOut(VarType* ast, bool isInput);
        void DecorateVarObject(AST* symbol, VarIdent* varIdent);

        bool FetchSystemValueSemantic(const std::vector<VarSemanticPtr>& varSemantics, std::string& semanticName) const;
        bool IsSystemValueSemnatic(std::string semantic) const;

        /* --- Helper templates for context analysis --- */

        template <typename T>
        void DecorateVarObjectSymbol(T ast);

        /* === Members === */

        Program*                                program_                = nullptr;
        FunctionDecl*                           mainFunction_           = nullptr;

        ShaderInput                             inputDesc_;
        ShaderOutput                            outputDesc_;

        std::string                             entryPoint_;
        ShaderTarget                            shaderTarget_           = ShaderTarget::VertexShader;
        InputShaderVersion                      versionIn_              = InputShaderVersion::HLSL5;
        OutputShaderVersion                     versionOut_             = OutputShaderVersion::GLSL330; //< TODO --> remove this variable from this class!!!
        std::string                             localVarPrefix_;

        std::map<std::string, IntrinsicClasses> intrinsicMap_;

        // Function call stack to join arguments with its function call.
        std::stack<FunctionCall*>               callStack_;

        // Structure stack to collect all members with system value semantic (SV_...).
        std::vector<StructDecl*>                structStack_;

        // True if AST traversal is currently inside any function.
        bool                                    isInsideFunc_          = false;

        // True if AST traversal is currently inside the main entry point (or its sub nodes).
        bool                                    isInsideEntryPoint_    = false;

};


} // /namespace Xsc


#endif



// ================================================================================