/*
 * ExprConverter.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_EXPR_CONVERTER_H
#define XSC_EXPR_CONVERTER_H


#include "VisitorTracker.h"
#include "TypeDenoter.h"
#include "Flags.h"
#include <Xsc/Xsc.h>
#include <functional>
#include <set>


namespace Xsc
{


/*
Common AST expression converter.
This helper class modifies the AST after context analysis and supports the following conversions:
1. Eliminate vector subscripts from scalar types
2. Convert implicit casts to explicit casts
3. Wrap nested unary expression into brackets (e.g. "- - a" -> "-(-a)")
4. Convert access to 'image' types through array indexers to imageStore/imageLoad calls (e.g. myImage[index] = 5 -> imageStore(myImage, index, 5))
*/
class ExprConverter : public VisitorTracker
{

    public:

        // Conversion flags enumeration.
        enum : unsigned int
        {
            ConvertVectorSubscripts     = (1 <<  0), // Converts certain vector subscripts to type constructors.
            ConvertVectorCompare        = (1 <<  1),
            ConvertImageAccess          = (1 <<  2),
            ConvertImplicitCasts        = (1 <<  3), // Converts implicit type casts to explicit type casts.
            ConvertInitializerToCtor    = (1 <<  4), // Converts initializer expressions to type constructors (e.g. "{ 1, 2, 3 }" to "float3(1, 2, 3)").
            ConvertLog10                = (1 <<  5), // Converts "log10(x)" to "(log(x) / log(10))".
            ConvertUnaryExpr            = (1 <<  6), // Wraps an unary expression if it's parent expression is also an unary expression (e.g. "-+x" to "-(+x)").
            ConvertSamplerBufferAccess  = (1 <<  7),
            ConvertMatrixLayout         = (1 <<  8), // Converts expressions that depend on the matrix layout (e.g. the argument order of "mul" intrinsic calls).
            ConvertTextureBracketOp     = (1 <<  9), // Converts Texture Operator[] accesses into "Load" intrinsic calls.
            ConvertTextureIntrinsicVec4 = (1 << 10), // Converts Texture intrinsic calls whose return type is a non-4D-vector.
            ConvertMatrixSubscripts     = (1 << 11), // Converts matrix subscripts into function calls to the respective wrapper function.
            ConvertCompatibleStructs    = (1 << 12), // Converts type denoters and struct members when the underlying struct type has a compatible struct.
            ConvertLiteralHalfToFloat   = (1 << 13), // Converts all half literals to float literals (e.g. "1.5h to 1.5f").

            // All conversion flags commonly used before visiting the sub nodes.
            AllPreVisit                 = (
                ConvertVectorCompare        |
                ConvertImageAccess          |
                ConvertLog10                |
                ConvertSamplerBufferAccess  |
                ConvertTextureBracketOp     |
                ConvertCompatibleStructs
            ),

            // All conversion flags commonly used after visiting the sub nodes.
            AllPostVisit                = (
                ConvertVectorSubscripts     |
                ConvertMatrixSubscripts     |
                ConvertTextureIntrinsicVec4
            ),

            // All conversion flags.
            All                         = (~0u),
        };

        // Converts the expressions in the specified AST.
        void Convert(Program& program, const Flags& conversionFlags, const NameMangling& nameMangling);

        static void ConvertExprIfCastRequired(ExprPtr& expr, const DataType targetType, bool matchTypeSize = true);
        static void ConvertExprIfCastRequired(ExprPtr& expr, const TypeDenoter& targetTypeDen, bool matchTypeSize = true);

        // Returns the texture dimension of the specified expression.
        static int GetTextureDimFromExpr(Expr* expr, const AST* ast = nullptr);

        // Returns the identifier used for matrix subscript wrapper functions.
        static std::string GetMatrixSubscriptWrapperIdent(const NameMangling& nameMangling, const MatrixSubscriptUsage& subscriptUsage);

    private:

        /* === Functions === */

        /* ----- Visitor implementation ----- */

        DECL_VISIT_PROC( VarDecl          );

        DECL_VISIT_PROC( FunctionDecl     );

        DECL_VISIT_PROC( ForLoopStmnt     );
        DECL_VISIT_PROC( WhileLoopStmnt   );
        DECL_VISIT_PROC( DoWhileLoopStmnt );
        DECL_VISIT_PROC( IfStmnt          );
        DECL_VISIT_PROC( SwitchStmnt      );
        DECL_VISIT_PROC( ExprStmnt        );
        DECL_VISIT_PROC( ReturnStmnt      );

        DECL_VISIT_PROC( LiteralExpr      );
        DECL_VISIT_PROC( TernaryExpr      );
        DECL_VISIT_PROC( BinaryExpr       );
        DECL_VISIT_PROC( UnaryExpr        );
        DECL_VISIT_PROC( CallExpr         );
        DECL_VISIT_PROC( BracketExpr      );
        DECL_VISIT_PROC( CastExpr         );
        DECL_VISIT_PROC( ObjectExpr       );
        DECL_VISIT_PROC( AssignExpr       );
        DECL_VISIT_PROC( ArrayExpr        );

        /* ----- Conversion ----- */

        // Converts the expression according to the specified flags (if enabled in the current conversion).
        void ConvertExpr(ExprPtr& expr, const Flags& flags);

        // Converts the list of expressions (see ConvertExpr).
        void ConvertExprList(std::vector<ExprPtr>& exprList, const Flags& flags);

        // Converts the expression if a vector subscript is used on a scalar type expression.
        void ConvertExprVectorSubscript(ExprPtr& expr);
        void ConvertExprVectorSubscriptObject(ExprPtr& expr, ObjectExpr* objectExpr);

        // Converts the expression if a matrix subscript is used.
        void ConvertExprMatrixSubscript(ExprPtr& expr);
        void ConvertExprMatrixSubscriptObject(ExprPtr& expr, ObjectExpr* objectExpr);

        // Converts the expression from a vector comparison to the respective intrinsic call (e.g. "a < b" -> "lessThan(a, b)").
        void ConvertExprVectorCompare(ExprPtr& expr);
        void ConvertExprVectorCompareUnary(ExprPtr& expr, UnaryExpr* unaryExpr);
        void ConvertExprVectorCompareBinary(ExprPtr& expr, BinaryExpr* binaryExpr);
        void ConvertExprVectorCompareTernary(ExprPtr& expr, TernaryExpr* ternaryExpr);

        // Converts the expression from an image access to the respective intrinsic call (e.g. "image[int2(1, 2)]" -> "imageLoad(image, ivec2(1, 2))").
        void ConvertExprImageAccess(ExprPtr& expr);
        void ConvertExprImageAccessAssign(ExprPtr& expr, AssignExpr* assignExpr);
        void ConvertExprImageAccessArray(ExprPtr& expr, ArrayExpr* arrayExpr, AssignExpr* assignExpr = nullptr);

        // Converts the expression from a sampler buffer access to the texelFetch intrinsic call (e.g. "buffer[2]" -> "texelFetch(buffer, 2)").
        void ConvertExprSamplerBufferAccess(ExprPtr& expr);
        void ConvertExprSamplerBufferAccessArray(ExprPtr& expr, ArrayExpr* arrayExpr);

        // Converts the expression by moving its sub expression into a bracket (e.g. "-+x" -> "-(+x)").
        void ConvertExprIntoBracket(ExprPtr& expr);

        // Converts the expression if this is an intrinsic call to "log10" (e.g. "log10(x)" to "(log(x) / log(10))").
        void ConvertExprIntrinsicCallLog10(ExprPtr& expr);

        // Converts the expression to the specified target type and according to the specified flags (if enabled in the current conversion).
        void ConvertExprTargetType(ExprPtr& expr, const TypeDenoter& targetTypeDen, bool matchTypeSize = true);

        // Converts the expression from an initializer list to a type constructor.
        void ConvertExprTargetTypeInitializer(ExprPtr& expr, InitializerExpr* initExpr, const TypeDenoter& targetTypeDen);

        // Converts the initializer into GLSL-ready form.
        void ConvertExprFormatInitializer(ExprPtr& expr, InitializerExpr* initExpr, const TypeDenoter& targetTypeDen);

        // Converts the expression from array access to texture object into a "Load" intrinsic call.
        void ConvertExprTextureBracketOp(ExprPtr& expr);

        // Appends vector subscripts to a texture intrinsic call if the intrinsic return type is not a 4D-vector.
        void ConvertExprTextureIntrinsicVec4(ExprPtr& expr);

        // Converts the specified expression when it refers to a member variable of a struct that has a compatible struct.
        void ConvertExprCompatibleStruct(ExprPtr& expr);

        /* === Members === */

        Flags           conversionFlags_;
        NameMangling    nameMangling_;

};


} // /namespace Xsc


#endif



// ================================================================================