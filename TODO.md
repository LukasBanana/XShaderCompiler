TODO List
---------

| Feature | Priority | Remarks |
|---------|:--------:|---------|
| Output semantic from casted return value in entry point | Very high | Is currently ignored in GLSL output. |
| Braced initializer | High | Translation of braced initializers are incomplete. |
| Tessellation shader support | High | Tessellation shader attributes are done, but in/out semantics are incomplete. |
| Type denoter analysis clean-up | Medium | Few more things should be moved from AST nodes into TypeDenoter classes (e.g. constness). |
| 'interface' and 'class' declarations | Low | Interfaces and classes can currently not even be parsed. |


Known Issues
------------

| Issue | Remarks |
|-------|---------|
| Implicit type conversion | In progress |
| Intrinsic argument type matching | Type matching is incomplete for intrinsic function calls |
| Vector less-then comparison | 'vec_a < vec_b' must be converted to 'lessThan(vec_a, vec_b)' for instance |
| Sampler types for DX9 shaders | Samplers (e.g. `sampler2D`) are ignored, even for HLSL3 |
