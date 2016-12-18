/*
 * ASTFactory.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_AST_FACTORY_H
#define XSC_AST_FACTORY_H


#include "AST.h"
#include "TypeDenoter.h"


namespace Xsc
{

namespace ASTFactory
{


// Returns the expression which contains a single expression of the specified type (within brackets), or null if not found.
Expr* FindSingleExpr(Expr* ast, const AST::Types searchedExprType);

FunctionCallPtr FindSingleFunctionCall(Expr* ast);
VarIdentPtr FindSingleVarIdent(Expr* ast);

FunctionCallExprPtr MakeIntrinsicCallExpr(
    const Intrinsic intrinsic, const std::string& ident,
    const TypeDenoterPtr& typeDenoter, const std::vector<ExprPtr>& arguments
);

// Converts the specified function call from "sincos(x, s, c)" to "s = sin(x), c = cos(x)".
ListExprPtr MakeSeparatedSinCosFunctionCalls(FunctionCall& funcCall);

CastExprPtr MakeCastExpr(const TypeDenoterPtr& typeDenoter, const ExprPtr& valueExpr);
CastExprPtr MakeLiteralCastExpr(const TypeDenoterPtr& typeDenoter, const DataType literalType, const std::string& literalValue);

ExprPtr ConvertExprBaseType(const DataType dataType, const ExprPtr& subExpr);

AliasDeclStmntPtr MakeBaseTypeAlias(const DataType dataType, const std::string& ident);

VarTypePtr MakeVarType(const StructDeclPtr& structDecl);
VarTypePtr MakeVarType(const TypeDenoterPtr& typeDenoter);
VarTypePtr MakeVarType(const DataType dataType);

VarDeclStmntPtr MakeVarDeclStmnt(const DataType dataType, const std::string& ident);

VarIdentPtr MakeVarIdent(const std::string& ident, AST* symbolRef = nullptr);

VarAccessExprPtr MakeVarAccessExpr(const std::string& ident, AST* symbolRef = nullptr);

// Return a list expression (or only the input expression) for the specified literal expression, so it can be used as constructor for a struct.
ExprPtr MakeConstructorListExpr(const LiteralExprPtr& literalExpr, const std::vector<TypeDenoterPtr>& listTypeDens);


} // /namespace ASTFactory

} // /namespace Xsc


#endif



// ================================================================================
