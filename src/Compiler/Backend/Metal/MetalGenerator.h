/*
 * MetalGenerator.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_METAL_GENERATOR_H
#define XSC_METAL_GENERATOR_H


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

// Metal output code generator.
class MetalGenerator : public Generator
{

    public:

        MetalGenerator(Log* log);

    private:

        // Function callback interface for entries in a layout qualifier.
        using LayoutEntryFunctor = std::function<void()>;

        void GenerateCodePrimary(
            Program&            program,
            const ShaderInput&  inputDesc,
            const ShaderOutput& outputDesc
        ) override;

        // Returns the Metal keyword for the specified buffer type or reports an error.
        const std::string* BufferTypeToKeyword(const BufferType bufferType, const AST* ast = nullptr);

        // Returns the Metal keyword for the specified sampler type or reports an error.
        const std::string* SamplerTypeToKeyword(const SamplerType samplerType, const AST* ast = nullptr);

        // Returns the Metal keyword for the specified semantic or reports an error.
        std::unique_ptr<std::string> SemanticToKeyword(const IndexedSemantic& semantic, const AST* ast = nullptr);

        // Error for intrinsics, that can not be mapped to Metal keywords.
        void ErrorIntrinsic(const std::string& intrinsicName, const AST* ast = nullptr);

        // Determines the shader target type for the specified function if it is an entry point.
        ShaderTarget DetermineEntryPointType(FunctionDecl& funcDecl);

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
        DECL_VISIT_PROC( BufferDeclStmt    );
        DECL_VISIT_PROC( SamplerDeclStmt   );
        DECL_VISIT_PROC( VarDeclStmt       );
        DECL_VISIT_PROC( AliasDeclStmt     );
        DECL_VISIT_PROC( BasicDeclStmt     );

        DECL_VISIT_PROC( NullStmt          );
        DECL_VISIT_PROC( ScopeStmt         );
        DECL_VISIT_PROC( ForStmt           );
        DECL_VISIT_PROC( WhileStmt         );
        DECL_VISIT_PROC( DoWhileStmt       );
        DECL_VISIT_PROC( IfStmt            );
        DECL_VISIT_PROC( SwitchStmt        );
        DECL_VISIT_PROC( ExprStmt          );
        DECL_VISIT_PROC( ReturnStmt        );
        DECL_VISIT_PROC( JumpStmt          );

        DECL_VISIT_PROC( SequenceExpr      );
        DECL_VISIT_PROC( LiteralExpr       );
        DECL_VISIT_PROC( TypeSpecifierExpr );
        DECL_VISIT_PROC( TernaryExpr       );
        DECL_VISIT_PROC( BinaryExpr        );
        DECL_VISIT_PROC( UnaryExpr         );
        DECL_VISIT_PROC( CallExpr          );
        DECL_VISIT_PROC( BracketExpr       );
        DECL_VISIT_PROC( IdentExpr         );
        DECL_VISIT_PROC( AssignExpr        );
        DECL_VISIT_PROC( SubscriptExpr     );
        DECL_VISIT_PROC( CastExpr          );
        DECL_VISIT_PROC( InitializerExpr   );

        /* --- Helper functions for code generation --- */

        /* ----- Pre processing AST ----- */

        void PreProcessAST(const ShaderInput& inputDesc, const ShaderOutput& outputDesc);
        void PreProcessReferenceAnalyzer(const ShaderInput& inputDesc);

        /* ----- Basics ----- */

        // Writes a comment (single or multi-line comments).
        void WriteComment(const std::string& text);
        void WriteSemantic(const IndexedSemantic& semantic, const AST* ast = nullptr);

        /* ----- Program ----- */

        void WriteProgramHeader();
        void WriteProgramHeaderComment();
        void WriteProgramHeaderInclude();

        /* ----- Object expression ----- */

        void WriteObjectExpr(const IdentExpr& identExpr);
        void WriteObjectExprIdent(const IdentExpr& identExpr, bool writePrefix = true);

        /* ----- Array expression ----- */

        void WriteArrayExpr(const SubscriptExpr& subscriptExpr);

        void WriteArrayIndices(const std::vector<ExprPtr>& arrayIndices);

        /* ----- Type denoter ----- */

        void WriteStorageClasses(const std::set<StorageClass>& storageClasses, const AST* ast = nullptr);
        void WriteInterpModifiers(const std::set<InterpModifier>& interpModifiers, const AST* ast = nullptr);
        void WriteTypeModifiers(const std::set<TypeModifier>& typeModifiers, const TypeDenoterPtr& typeDenoter = nullptr);
        void WriteTypeModifiersFrom(const TypeSpecifierPtr& typeSpecifier);

        void WriteDataType(DataType dataType, const AST* ast = nullptr);

        void WriteTypeDenoter(const TypeDenoter& typeDenoter, const AST* ast = nullptr);
        void WriteTypeDenoterExt(const TypeDenoter& typeDenoter, bool writeArrayDims, const AST* ast = nullptr);

        /* ----- Attributes ----- */

        void WriteAttribBegin();
        void WriteAttribEnd();
        void WriteAttribNext();
        void WriteAttrib(const std::string& value);

        /* ----- Function declaration ----- */

        void WriteFunction(FunctionDecl* ast);
        void WriteFunctionEntryPointType(const ShaderTarget target);

        /* ----- Call expressions ----- */

        void AssertIntrinsicNumArgs(CallExpr* callExpr, std::size_t numArgsMin, std::size_t numArgsMax = ~0);

        void WriteCallExprStandard(CallExpr* callExpr);
        void WriteCallExprIntrinsicMul(CallExpr* callExpr);

        void WriteCallExprArguments(CallExpr* callExpr);

        /* ----- Structure ----- */

        bool WriteStructDecl(StructDecl* structDecl, bool endWithSemicolon);

        /* ----- Misc ----- */

        void WriteStmtComment(Stmt* ast, bool insertBlank = false);

        template <typename T>
        void WriteStmtList(const std::vector<T>& stmts, bool isGlobalScope = false);

        void WriteParameter(VarDeclStmt* ast);
        void WriteScopedStmt(Stmt* ast);

        void WriteLiteral(const std::string& value, const DataType& dataType, const AST* ast = nullptr);

    private:

        ShaderTarget        shaderTarget_           = ShaderTarget::Undefined;
        OutputShaderVersion versionOut_             = OutputShaderVersion::Metal;

        bool                preserveComments_       = false;
        bool                alwaysBracedScopes_     = false;
        bool                writeHeaderComment_     = true;
        bool                newLineOpenScope_       = true;

        struct AttribList
        {
            bool scheduled  = false;
            bool started    = false;
        }
        attribList_;

};


} // /namespace Xsc


#endif



// ================================================================================
