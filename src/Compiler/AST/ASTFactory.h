/*
 * ASTFactory.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_AST_FACTORY_H
#define XSC_AST_FACTORY_H


#include "AST.h"
#include "TypeDenoter.h"


namespace Xsc
{

class Variant;

namespace ASTFactory
{


/* ----- Make functions ----- */

FunctionCallExprPtr MakeIntrinsicCallExpr(
    const Intrinsic intrinsic, const std::string& ident,
    const TypeDenoterPtr& typeDenoter, const std::vector<ExprPtr>& arguments
);

CastExprPtr MakeCastExpr(const TypeDenoterPtr& typeDenoter, const ExprPtr& valueExpr);
CastExprPtr MakeLiteralCastExpr(const TypeDenoterPtr& typeDenoter, const DataType literalType, const std::string& literalValue);

SuffixExprPtr MakeSuffixExpr(const ExprPtr& expr, const VarIdentPtr& varIdent);

ExprPtr MakeCastOrSuffixCastExpr(const TypeDenoterPtr& typeDenoter, const ExprPtr& valueExpr, const VarIdentPtr& suffixVarIdent);

LiteralExprPtr MakeLiteralExpr(const DataType literalType, const std::string& literalValue);
LiteralExprPtr MakeLiteralExpr(const Variant& literalValue);

AliasDeclStmntPtr MakeBaseTypeAlias(const DataType dataType, const std::string& ident);

TypeSpecifierPtr MakeTypeSpecifier(const StructDeclPtr& structDecl);
TypeSpecifierPtr MakeTypeSpecifier(const TypeDenoterPtr& typeDenoter);
TypeSpecifierPtr MakeTypeSpecifier(const DataType dataType);

VarDeclStmntPtr MakeVarDeclStmnt(const DataType dataType, const std::string& ident);

VarIdentPtr MakeVarIdent(const std::string& ident, AST* symbolRef = nullptr);

VarAccessExprPtr MakeVarAccessExpr(const std::string& ident, AST* symbolRef = nullptr);

// Return a list expression (or only the input expression) for the specified literal expression, so it can be used as constructor for a struct.
ExprPtr MakeConstructorListExpr(const LiteralExprPtr& literalExpr, const std::vector<TypeDenoterPtr>& listTypeDens);

// Makes an statement with an array element assignment for the specified variable identifier, array indices, and value expression.
ExprStmntPtr MakeArrayAssignStmnt(VarDecl* varDecl, const std::vector<int>& arrayIndices, const ExprPtr& assignExpr);

ArrayDimensionPtr MakeArrayDimension(int arraySize);

FunctionDeclPtr MakeIntrinsic(Intrinsic intrinsic, const TypeSpecifierPtr& returnType, const std::vector<VarDeclStmntPtr>& parameters);

/* ----- Make list functions ----- */

std::vector<ExprPtr> MakeArrayIndices(const std::vector<int>& arrayIndices);

std::vector<ArrayDimensionPtr> MakeArrayDimensionList(const std::vector<int>& arraySizes);

/* ----- Convert functions ----- */

ExprPtr ConvertExprBaseType(const DataType dataType, const ExprPtr& subExpr);

ArrayDimensionPtr ConvertExprToArrayDimension(const ExprPtr& expr);

std::vector<ArrayDimensionPtr> ConvertExprListToArrayDimensionList(const std::vector<ExprPtr>& exprs);


} // /namespace ASTFactory

} // /namespace Xsc


#endif



// ================================================================================
