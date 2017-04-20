===========
Expressions
===========

.. contents::
   :local:
   :depth: 2

Object Identifiers
==================

Since there is no complete HLSL grammar specification I know of,
the overall structure of the AST comes from many trials and erros (and might still change).

Here is a brief overview how one of the most complex constructs, the **object identifiers**, are structured:
These object identifiers consist of several AST classes and are part of the expressions,
specifically ``ObjectExpr``, ``ArrayExpr``, ``AssignExpr``, and ``CallExpr``.

An ``ObjectExpr`` (for "object expression") holds a reference to an object, which can be
either a **variable** (also parameter), **buffer** (also texture), **sampler** (also sampler state),
**structure**, or **type alias**.

Specification
-------------

::

 ObjectExpr : Expr {
     Expr              prefixExpression [Optional]
     Boolean           isStatic         [Optional]
     String            identifier
     
     Ref<Decl>         symbolRef        [Optional]  --> VarDecl, BufferDecl, SamplerDecl, StructDecl
 }
 
 AssignExpr : Expr {
     Expr              lvalueExpression
     AssinOp           assignOperator
     Expr              rvalueExpression
 }
 
 ArrayExpr : Expr {
     Expr              prefixExpression
     List<Expr>        arrayIndices
 }
 
 CallExpr : Expr {
     Expr              prefixExpression [Optional]
     Boolean           isStatic         [Optional]
     String            identifier
     Ref<FunctionDecl> funcDeclRef      [Optional]
 }
 
 BracketExpr : Expr {
     Expr              subExpression
 }


Example: ``( Scene::getMain().getLights() )[1].material.getShininess()``

::

 expression (CallExpr)
  |-prefixExpression (ObjectExpr)
  |  |-prefixExpression (ArrayExpr)
  |  |  |-prefixExpression (BracketExpr)
  |  |  |  `-subExpression (CallExpr)
  |  |  |     |-prefixExpression (CallExpr)
  |  |  |     |  |-prefixExpression (ObjectExpr)
  |  |  |     |  |  `-identifier = "Scene"
  |  |  |     |  |-isStatic = true
  |  |  |     |  `-identifier = "getMain"
  |  |  |     `-identifier = "getLights"
  |  |  `-arrayIndices[0] (LiteralExpr)
  |  |     `-literal = "1"
  |  `-identifier = "material"
  `-identifier = "getShininess"

Cast Expressions
================

Parsing
-------

Cast expressions are the only part that can *not* be parsed with a **context free grammar**.
Consider the following example::

 int X = 0;
 (X) - (1);

In this example ``X`` specifies a local variable from type ``int``, and the expression ``(X) - (1)`` is a **binary expression**
with the minus operator. Now consider the next example::

 typedef int X;
 (X) - (1);

In this example ``X`` specifies a type alias that refers to the type ``int``, and the expression ``(X) - (1)`` is
equivalent to ``(int)-1`` which is a **cast expression**. This means the parser needs to know all type names
for the respective scopes, which in turn means the parser needs to be context aware.

Entry-Point Return
------------------

When the entry point function return type is a structure (e.g. ``Output``), and a return statement uses a cast expression
(e.g. ``return (Output)1;``) then this cast expression must be transformed for GLSL in different ways.
In the following the structure ``Output`` is assumed to be declared as::

 struct Output {
     float4 position;
     float3 normal;
 };

Return ``ObjectExpr``
---------------------

When an **object expression** is returned, it must be translated to a constructor of the respective function return type::

 return (Output)object;

Must translate to this in GLSL::

 return Output(vec4(object), vec3(object));

Return ``CallExpr``
-------------------

When a **call expression** is returned, a temporary variable must be written out, to avoid multiple calls to the function,
because it might have side affects::

 return (Output)f();

Must translate to this in GLSL::

 f_ReturnType xst_temp = f();
 return Output(vec4(xst_temp), vec3(xst_temp));

Transforming System Value Semantics
===================================

Especially for geometry shaders a system value semantic must be transformed from an array expression like this:

* ``output[0].position`` to ``gl_Position`` (for geometry shader output)
* ``input[0].position`` to ``gl_in[0].gl_Position`` (for geometry shader input)
* ``input[0].normal`` to ``xsv_NORMAL[0]`` (for geometry shader input)

This transformation is currently implemented in the ``GLSLGenerator`` instead of the ``GLSLConverter``.






