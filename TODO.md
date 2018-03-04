TODO List
---------

| Feature | Priority | Remarks |
|---------|:--------:|---------|
| Output semantic from casted return value in entry point | Very high | Is currently ignored in GLSL output. **in progress** |
| Braced initializer | High | Translation of braced initializers are incomplete. **in progress** |
| Tessellation shader support | High | Tessellation shader attributes are done, but in/out semantics are incomplete. |
| Type denoter analysis clean-up | Medium | Few more things should be moved from AST nodes into TypeDenoter classes (e.g. constness). **in progress** |
| GLSL Frontend | Medium | GLSL can currently only be pre-processed. |
| 'interface' and 'class' declarations | Low | Interfaces and classes can currently not even be parsed. |


Known Issues
------------

| Issue | Remarks |
|-------|---------|
| Implicit type conversion | **in progress** |
| Intrinsic argument type matching | Type matching is incomplete for intrinsic function calls. **in progress** |
| Struct used as input *and* ouput | Structures can currently not be used for shader input and output semantics simultaneously. |
| Unicode | The compiler uses the C++ ASCII file streams, that are not aware of unicode characters, neither in the file contents, nor in their filenames. |
| Texture `Operator[]` | The bracket operator is currently not translated for Texture objects, neither [`Operator[]`](https://msdn.microsoft.com/en-us/library/windows/desktop/ff471561(v=vs.85).aspx) nor [`mips.Operator[][]`](https://msdn.microsoft.com/en-us/library/windows/desktop/ff471560(v=vs.85).aspx). |
| `GetDimensions(...)` | Cannot be translated propery (for all texture types) -> use cbuffer to pass dimension data. |
| `atan2(x, y)` | Must be translated to `atan(y, x)`. |
| Boolean in `lerp`/`mix` | Third argument in `lerp`/`mix` intrinsic must not be casted to floating-points. |


Requested Features
------------------

- [x] Warnings for required GLSL extensions (implemented with `-Wextension`; also `-Wall` added like in GCC).
- [ ] Shader Reflection: texture type, constant buffer members

