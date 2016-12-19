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


/* ----- Find functions ----- */

// Returns the expression which contains a single expression of the specified type (within brackets), or null if not found.
Expr* FindSingleExpr(Expr* ast, const AST::Types searchedExprType);

FunctionCallPtr FindSingleFunctionCall(Expr* ast);
VarIdentPtr FindSingleVarIdent(Expr* ast);

/* ----- Make functions ----- */

FunctionCallExprPtr MakeIntrinsicCallExpr(
    const Intrinsic intrinsic, const std::string& ident,
    const TypeDenoterPtr& typeDenoter, const std::vector<ExprPtr>& arguments
);

// Converts the specified function call from "sincos(x, s, c)" to "s = sin(x), c = cos(x)".
ListExprPtr MakeSeparatedSinCosFunctionCalls(FunctionCall& funcCall);

CastExprPtr MakeCastExpr(const TypeDenoterPtr& typeDenoter, const ExprPtr& valueExpr);
CastExprPtr MakeLiteralCastExpr(const TypeDenoterPtr& typeDenoter, const DataType literalType, const std::string& literalValue);

LiteralExprPtr MakeLiteralExpr(const DataType literalType, const std::string& literalValue);

AliasDeclStmntPtr MakeBaseTypeAlias(const DataType dataType, const std::string& ident);

TypeNamePtr MakeTypeName(const StructDeclPtr& structDecl);
TypeNamePtr MakeTypeName(const TypeDenoterPtr& typeDenoter);
TypeNamePtr MakeTypeName(const DataType dataType);

VarDeclStmntPtr MakeVarDeclStmnt(const DataType dataType, const std::string& ident);

VarIdentPtr MakeVarIdent(const std::string& ident, AST* symbolRef = nullptr);

VarAccessExprPtr MakeVarAccessExpr(const std::string& ident, AST* symbolRef = nullptr);

// Return a list expression (or only the input expression) for the specified literal expression, so it can be used as constructor for a struct.
ExprPtr MakeConstructorListExpr(const LiteralExprPtr& literalExpr, const std::vector<TypeDenoterPtr>& listTypeDens);

std::vector<ExprPtr> MakeArrayIndices(const std::vector<int>& arrayIndices);

// Makes an statement with an array element assignment for the specified variable identifier, array indices, and value expression.
ExprStmntPtr MakeArrayAssignStmnt(VarDecl* varDecl, const std::vector<int>& arrayIndices, const ExprPtr& assignExpr);

ArrayDimensionPtr MakeArrayDimension(int arraySize);

std::vector<ArrayDimensionPtr> MakeArrayDimensionList(const std::vector<int>& arraySizes);

/* ----- Convert functions ----- */

ExprPtr ConvertExprBaseType(const DataType dataType, const ExprPtr& subExpr);

ArrayDimensionPtr ConvertExprToArrayDimension(const ExprPtr& expr);

std::vector<ArrayDimensionPtr> ConvertExprListToArrayDimensionList(const std::vector<ExprPtr>& exprs);


} // /namespace ASTFactory

} // /namespace Xsc


#endif



// ================================================================================
