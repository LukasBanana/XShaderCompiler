===========
Type System
===========

.. contents::
   :local:
   :depth: 2

TypeDenoter
===========

The main class in the type system is the ``TypeDenoter`` and its sub classes::

 TypeDenoter
  |-VoidTypeDenoter
  |-NullTypeDenoter
  |-BaseTypeDenoter
  |-BufferTypeDenoter
  |-SamplerTypeDenoter
  |-StructTypeDenoter
  |-AliasTypeDenoter
  `-ArrayTypeDenoter

Type Derivation
---------------

The main function in ``TypeDenoter`` to derive a type by an input expression is the ``GetSub`` function::

 shared_ptr<TypeDenoter> GetSub(const Expr* expr = nullptr);

If the input expression is ``null``, the type denoter itself is returned with ``shared_from_this()``,
expect for ``AliasTypeDenoter``, where its aliased sub type is returned!
Otherwise, the type denoter is derived by the expression,
e.g. with an ``ArrayExpr`` this type denoter is expected to be an ``ArrayTypeDenoter`` and its base type is returned.
Here are a few examples (Pseudocode)::

 BaseTypeDenoter( Float4 ).GetSub( ObjectExpr( idenitifer("xy") ) )
  -> BaseTypeDenoter( Float2 )
 
 BaseTypeDenoter( Float4 ).GetSub( ArrayExpr( indices(I) ) )
  -> BaseTypeDenoter( Float )
 
 ArrayTypeDenoter( dimension(N), BaseTypeDenoter( Float4 ) ).GetSub( ArrayExpr( indices(I) ) )
  -> BaseTypeDenoter( Float4 )
 
 ArrayTypeDenoter( dimension(N), BaseTypeDenoter( Float4 ) ).GetSub( ArrayExpr( indices(I, J) ) )
  -> BaseTypeDenoter( Float )
 
 ArrayTypeDenoter( dimension(N), BaseTypeDenoter( Float4 ) ).GetSub( ArrayExpr( indices(I), ObjectExpr( idenitifer("xy") ) ) )
  -> BaseTypeDenoter( Float2 )
 
 ArrayTypeDenoter( dimension(N, M), BaseTypeDenoter( Float ) ).GetSub( ArrayExpr( indices(I) ) )
  -> ArrayTypeDenoter( dimension(N), BaseTypeDenoter( Float ) )

The main function to remove the ``AliasTypeDenoter`` wrapper, and only get the sub type, is the ``GetAliased`` function::

 const TypeDenoter& GetAliased() const;

It should be preferred over ``GetSub`` if only a non-alias type is required which can be constant,
because this function avoids creating new ``shared_ptr`` instances.


