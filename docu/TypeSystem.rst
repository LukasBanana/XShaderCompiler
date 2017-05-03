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

If the input expression is ``nullptr``, the type denoter itself is returned with ``shared_from_this()``,
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

Why is there an ``AliasTypeDenoter`` anyway, you might ask? Because with ``typedef`` a new type is created,
but for contextual analysis, commonly only its sub type is of interest.

Vector Space Extension
----------------------

If the ``attr-space`` language extension is enabled, the ``space`` attribute can be used for a stronger type system::

 // This matrix transforms vectors from MODEL into PROJECTION vector space
 [space(MODEL, PROJECTION)]
 float4x4 wvpMatrix;
 
 // This matrix transforms vectors from MODEL into WORLD vector space
 [space(MODEL, WORLD)]
 float4x4 wMatrix;
 
 // These two vectors are in MODEL vector space
 [space(MODEL)]
 float3 position, normal;
 
 // Transform vertex position from MODEL into PROJECTION space
 float4 projPosition = mul(wvpMatrix, float4(position, 1));
 
 // Transform vertex normal from MODEL into WORLD space
 float3 worldNormal = mul(wMatrix, float4(normal, 0));
 
 // ERROR: inconsistent vector-spaces between type denoters (found 'WORLD' and 'PROJECTION')
 float2 error1 = float2(projPosition.x, worldNormal.y);
 
 // ERROR: illegal assignment of 'MODEL' vector-space to 'WORLD' vector-space
 [space(WORLD)]
 float3 worldPos = position;

 // Ok: declares "modelPos" implicitly to have MODEL vector space
 float3 modelPos = position;

