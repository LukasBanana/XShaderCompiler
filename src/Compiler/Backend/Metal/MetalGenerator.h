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

        /* === Functions === */

        void GenerateCodePrimary(
            Program&            program,
            const ShaderInput&  inputDesc,
            const ShaderOutput& outputDesc
        ) override;

        // Returns the Metal keyword for the specified buffer type or reports and error.
        const std::string* BufferTypeToKeyword(const BufferType bufferType, const AST* ast = nullptr);

        // Returns the Metal keyword for the specified sampler type or reports and error.
        const std::string* SamplerTypeToKeyword(const SamplerType samplerType, const AST* ast = nullptr);

        // Error for intrinsics, that can not be mapped to Metal keywords.
        void ErrorIntrinsic(const std::string& intrinsicName, const AST* ast = nullptr);

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

        /* ----- Basics ----- */

        // Writes a comment (single or multi-line comments).
        void WriteComment(const std::string& text);

        /* ----- Program ----- */

        void WriteProgramHeader();
        void WriteProgramHeaderComment();
        void WriteProgramHeaderInclude();

        /* ----- Object expression ----- */

        void WriteObjectExpr(const ObjectExpr& objectExpr);
        void WriteObjectExprIdent(const ObjectExpr& objectExpr, bool writePrefix = true);

        /* ----- Array expression ----- */

        void WriteArrayExpr(const ArrayExpr& arrayExpr);

        void WriteArrayIndices(const std::vector<ExprPtr>& arrayIndices);

        /* ----- Type denoter ----- */

        void WriteStorageClasses(const std::set<StorageClass>& storageClasses, const AST* ast = nullptr);
        void WriteInterpModifiers(const std::set<InterpModifier>& interpModifiers, const AST* ast = nullptr);
        void WriteTypeModifiers(const std::set<TypeModifier>& typeModifiers, const TypeDenoterPtr& typeDenoter = nullptr);
        void WriteTypeModifiersFrom(const TypeSpecifierPtr& typeSpecifier);

        void WriteDataType(DataType dataType, const AST* ast = nullptr);

        void WriteTypeDenoter(const TypeDenoter& typeDenoter, const AST* ast = nullptr);

        /* ----- Function declaration ----- */

        void WriteFunction(FunctionDecl* ast);
        void WriteFunctionEntryPoint(FunctionDecl* ast);
        void WriteFunctionEntryPointType(FunctionDecl* ast);

        /* ----- Call expressions ----- */

        void AssertIntrinsicNumArgs(CallExpr* callExpr, std::size_t numArgsMin, std::size_t numArgsMax = ~0);

        void WriteCallExprStandard(CallExpr* callExpr);
        void WriteCallExprIntrinsicMul(CallExpr* callExpr);

        void WriteCallExprArguments(CallExpr* callExpr, std::size_t firstArgIndex = 0, std::size_t numWriteArgs = ~0u);

        /* ----- Structure ----- */

        bool WriteStructDecl(StructDecl* structDecl, bool endWithSemicolon);

        /* ----- Misc ----- */

        void WriteStmntComment(Stmnt* ast, bool insertBlank = false);

        template <typename T>
        void WriteStmntList(const std::vector<T>& stmnts, bool isGlobalScope = false);

        void WriteParameter(VarDeclStmnt* ast);
        void WriteScopedStmnt(Stmnt* ast);

        void WriteLiteral(const std::string& value, const DataType& dataType, const AST* ast = nullptr);

    private:

        OutputShaderVersion versionOut_             = OutputShaderVersion::Metal;

        bool                preserveComments_       = false;
        bool                alwaysBracedScopes_     = false;
        bool                writeHeaderComment_     = true;
        
};


} // /namespace Xsc


#endif



// ================================================================================
