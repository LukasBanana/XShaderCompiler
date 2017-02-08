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
*/
class ExprConverter : public Visitor
{
    
    public:
        
        // Conversion flags enumeration.
        enum : unsigned int
        {
            ConvertVectorSubscripts = (1 << 0),
            ConvertImplicitCasts    = (1 << 1),
            WrapUnaryExpr           = (1 << 2),
            All                     = (ConvertVectorSubscripts | ConvertImplicitCasts | WrapUnaryExpr),
        };

        // Converts the expressions in the specified AST.
        void Convert(Program& program, const Flags& conversionFlags);

        void ConvertExprVectorSubscript(ExprPtr& expr);

        void ConvertExprIfCastRequired(ExprPtr& expr, const DataType targetType, bool matchTypeSize = true);
        void ConvertExprIfCastRequired(ExprPtr& expr, const TypeDenoter& targetTypeDen, bool matchTypeSize = true);

        // Converts the expression to a type constructor (i.e. function call) if it's an initializer expression.
        //void ConvertExprIfConstructorRequired(ExprPtr& expr);

        void ConvertExprIntoBracket(ExprPtr& expr);

    private:
        
        /* === Functions === */

        // Returns the data type to which an expression must be casted, if the target data type and the source data type are incompatible in GLSL.
        std::unique_ptr<DataType> MustCastExprToDataType(const DataType targetType, const DataType sourceType, bool matchTypeSize);
        std::unique_ptr<DataType> MustCastExprToDataType(const TypeDenoter& targetTypeDen, const TypeDenoter& sourceTypeDen, bool matchTypeSize);

        // Converts the specified expression if a vector subscript is used on a scalar type expression.
        void ConvertExprVectorSubscriptSuffix(ExprPtr& expr, SuffixExpr* suffixExpr);
        void ConvertExprVectorSubscriptVarIdent(ExprPtr& expr, VarIdent* varIdent);

        void IfFlaggedConvertExprVectorSubscript(ExprPtr& expr);
        void IfFlaggedConvertExprIfCastRequired(ExprPtr& expr, const TypeDenoter& targetTypeDen);
        void IfFlaggedConvertExprIntoBracket(ExprPtr& expr);

        // Find the best suitable common type denoter for both left and right hand side type denoters.
        TypeDenoterPtr FindCommonTypeDenoter(const TypeDenoterPtr& lhsTypeDen, const TypeDenoterPtr& rhsTypeDen);
        TypeDenoterPtr FindCommonTypeDenoterScalarAndScalar(BaseTypeDenoter* lhsTypeDen, BaseTypeDenoter* rhsTypeDen);
        TypeDenoterPtr FindCommonTypeDenoterScalarAndMatrix(BaseTypeDenoter* lhsTypeDen, BaseTypeDenoter* rhsTypeDen);
        TypeDenoterPtr FindCommonTypeDenoterAnyAndAny(TypeDenoter* lhsTypeDen, TypeDenoter* rhsTypeDen);

        /* ----- Visitor implementation ----- */

        DECL_VISIT_PROC( FunctionCall     );

        DECL_VISIT_PROC( VarDecl          );

        DECL_VISIT_PROC( FunctionDecl     );

        DECL_VISIT_PROC( ForLoopStmnt     );
        DECL_VISIT_PROC( WhileLoopStmnt   );
        DECL_VISIT_PROC( DoWhileLoopStmnt );
        DECL_VISIT_PROC( IfStmnt          );
        DECL_VISIT_PROC( ExprStmnt        );
        DECL_VISIT_PROC( ReturnStmnt      );

        DECL_VISIT_PROC( TernaryExpr      );
        DECL_VISIT_PROC( BinaryExpr       );
        DECL_VISIT_PROC( UnaryExpr        );
        DECL_VISIT_PROC( CastExpr         );
        DECL_VISIT_PROC( VarAccessExpr    );

        /* === Members === */

        Flags conversionFlags_;

};


} // /namespace Xsc


#endif



// ================================================================================