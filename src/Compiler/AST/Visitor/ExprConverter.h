/*
 * ExprConverter.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_EXPR_CONVERTER_H
#define XSC_EXPR_CONVERTER_H


#include "Visitor.h"
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
class ExprConverter : public Visitor
{
    
    public:
        
        // Conversion flags enumeration.
        enum : unsigned int
        {
            ConvertVectorSubscripts     = (1 << 0),
            ConvertVectorCompare        = (1 << 1),
            ConvertImageAccess          = (1 << 2),
            ConvertImplicitCasts        = (1 << 3),
            ConvertInitializer          = (1 << 4),
            ConvertLog10                = (1 << 5), // Converts "log10(x)" to "(log(x) / log(10))"
            ConvertUnaryExpr            = (1 << 6), // Wraps an unary expression if it's parent expression is also an unary expression (e.g. "-+x" to "-(+x)")
            ConvertSamplerBufferAccess  = (1 << 7),
            ConvertMatrixLayout         = (1 << 8), // Converts expressions that depend on the matrix layout (e.g. the argument order of "mul" intrinsic calls).

            // All conversion flags commonly used before visiting the sub nodes.
            AllPreVisit                 = (ConvertVectorCompare | ConvertImageAccess | ConvertLog10 | ConvertSamplerBufferAccess),

            // All conversion flags commonly used after visiting the sub nodes.
            AllPostVisit                = (ConvertVectorSubscripts),

            // All conversion flags.
            All                         = (~0u),
        };

        // Converts the expressions in the specified AST.
        void Convert(Program& program, const Flags& conversionFlags);

        void ConvertExprIfCastRequired(ExprPtr& expr, const DataType targetType, bool matchTypeSize = true);
        void ConvertExprIfCastRequired(ExprPtr& expr, const TypeDenoter& targetTypeDen, bool matchTypeSize = true);

        // Returns the texture dimension of the specified expression.
        static int GetTextureDimFromExpr(Expr* expr, const AST* ast = nullptr);

    private:
        
        /* === Functions === */

        // Returns the data type to which an expression must be casted, if the target data type and the source data type are incompatible.
        std::unique_ptr<DataType> MustCastExprToDataType(const DataType targetType, const DataType sourceType, bool matchTypeSize);
        std::unique_ptr<DataType> MustCastExprToDataType(const TypeDenoter& targetTypeDen, const TypeDenoter& sourceTypeDen, bool matchTypeSize);

        TypeDenoterPtr MakeBufferAccessCallTypeDenoter(const DataType genericDataType);

        /* ----- Conversion ----- */

        // Converts the expression according to the specified flags (if enabled in the current conversion).
        void ConvertExpr(ExprPtr& expr, const Flags& flags);

        // Converts the list of expressions (see ConvertExpr).
        void ConvertExprList(std::vector<ExprPtr>& exprList, const Flags& flags);

        // Converts the expression if a vector subscript is used on a scalar type expression.
        void ConvertExprVectorSubscript(ExprPtr& expr);
        void ConvertExprVectorSubscriptObject(ExprPtr& expr, ObjectExpr* objectExpr);
        
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

        // Convert a the initializer into GLSL-ready form.
        void ConvertExprFormatInitializer(ExprPtr& expr, InitializerExpr* initExpr, const TypeDenoter& targetTypeDen);

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

        DECL_VISIT_PROC( TernaryExpr      );
        DECL_VISIT_PROC( BinaryExpr       );
        DECL_VISIT_PROC( UnaryExpr        );
        DECL_VISIT_PROC( CallExpr         );
        DECL_VISIT_PROC( BracketExpr      );
        DECL_VISIT_PROC( CastExpr         );
        DECL_VISIT_PROC( ObjectExpr       );
        DECL_VISIT_PROC( AssignExpr       );
        DECL_VISIT_PROC( ArrayExpr        );

        /* === Members === */

        Flags conversionFlags_;

};


} // /namespace Xsc


#endif



// ================================================================================