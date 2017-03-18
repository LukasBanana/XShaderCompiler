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

FunctionCallExprPtr             MakeIntrinsicCallExpr(
    const Intrinsic intrinsic, const std::string& ident,
    const TypeDenoterPtr& typeDenoter, const std::vector<ExprPtr>& arguments
);

FunctionCallExprPtr             MakeTextureSamplerBindingCallExpr(const ExprPtr& textureObjectExpr, const ExprPtr& samplerObjectExpr);

CastExprPtr                     MakeCastExpr(const TypeDenoterPtr& typeDenoter, const ExprPtr& valueExpr);
CastExprPtr                     MakeLiteralCastExpr(const TypeDenoterPtr& typeDenoter, const DataType literalType, const std::string& literalValue);

#if 0//TODO: remove
SuffixExprPtr                   MakeSuffixExpr(const ExprPtr& expr, const VarIdentPtr& varIdent);

ExprPtr                         MakeCastOrSuffixCastExpr(const TypeDenoterPtr& typeDenoter, const ExprPtr& valueExpr, const VarIdentPtr& suffixVarIdent = nullptr);
#endif

BinaryExprPtr                   MakeBinaryExpr(const ExprPtr& lhsExpr, const BinaryOp op, const ExprPtr& rhsExpr);

LiteralExprPtr                  MakeLiteralExpr(const DataType literalType, const std::string& literalValue);
LiteralExprPtr                  MakeLiteralExpr(const Variant& literalValue);

AliasDeclStmntPtr               MakeBaseTypeAlias(const DataType dataType, const std::string& ident);

TypeSpecifierPtr                MakeTypeSpecifier(const StructDeclPtr& structDecl);
TypeSpecifierPtr                MakeTypeSpecifier(const TypeDenoterPtr& typeDenoter);
TypeSpecifierPtr                MakeTypeSpecifier(const DataType dataType);

VarDeclStmntPtr                 MakeVarDeclStmnt(const TypeSpecifierPtr& typeSpecifier, const std::string& ident);
VarDeclStmntPtr                 MakeVarDeclStmnt(const DataType dataType, const std::string& ident);

#if 0//TODO: remove

VarIdentPtr                     MakeVarIdent(const std::string& ident, AST* symbolRef = nullptr);

// Makes a new VarIdent instance with only the first node of the specified identifier.
VarIdentPtr                     MakeVarIdentFirst(const VarIdent& varIdent);

// Makes a new VarIdent instance with all nodes of the specified identifier except the last one.
VarIdentPtr                     MakeVarIdentWithoutLast(const VarIdent& varIdent);

// Makes a new VarIdent instance by adding the specified new identifier with symbol reference at the front of the specified VarIdent.
VarIdentPtr                     MakeVarIdentPushFront(const std::string& firstIdent, AST* symbolRef, const VarIdentPtr& next);

VarAccessExprPtr                MakeVarAccessExpr(const VarIdentPtr& varIdent);
VarAccessExprPtr                MakeVarAccessExpr(const std::string& ident, AST* symbolRef = nullptr);

#endif

#if 1//TODO: make this standard

ObjectExprPtr                   MakeObjectExpr(const std::string& ident, Decl* symbolRef = nullptr);
ObjectExprPtr                   MakeObjectExpr(Decl* symbolRef);

ArrayAccessExprPtr              MakeArrayAccessExpr(const ExprPtr& prefixExpr, const std::vector<int>& arrayIndices);

#endif

// Makes a new bracket expression with the specified sub expression (source area is copied).
BracketExprPtr                  MakeBracketExpr(const ExprPtr& expr);

// Return a list expression (or only the input expression) for the specified literal expression, so it can be used as constructor for a struct.
ExprPtr                         MakeConstructorListExpr(const LiteralExprPtr& literalExpr, const std::vector<TypeDenoterPtr>& listTypeDens);

// Makes a statement with an array element assignment for the specified variable, array indices, and assignment expression.
ExprStmntPtr                    MakeArrayAssignStmnt(VarDecl* varDecl, const std::vector<int>& arrayIndices, const ExprPtr& assignExpr);

ArrayDimensionPtr               MakeArrayDimension(int arraySize);

/* ----- Make list functions ----- */

std::vector<ExprPtr>            MakeArrayIndices(const std::vector<int>& arrayIndices);

std::vector<ArrayDimensionPtr>  MakeArrayDimensionList(const std::vector<int>& arraySizes);

/* ----- Convert functions ----- */

ExprPtr                         ConvertExprBaseType(const DataType dataType, const ExprPtr& subExpr);

ArrayDimensionPtr               ConvertExprToArrayDimension(const ExprPtr& expr);

std::vector<ArrayDimensionPtr>  ConvertExprListToArrayDimensionList(const std::vector<ExprPtr>& exprs);


} // /namespace ASTFactory

} // /namespace Xsc


#endif



// ================================================================================
