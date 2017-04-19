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

If the input expression is ``null``, the type denoter itself is returned with ``std::enable_shared_from_this<TypeDenoter>::shared_from_this()``







