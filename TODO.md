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
| Implicit type conversion | **in progress** |
| Intrinsic argument type matching | Type matching is incomplete for intrinsic function calls; **in progress** |
| Sampler types for DX9 shaders | Samplers (e.g. `sampler2D`) are ignored, even for HLSL3 |
