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


Requested Features
------------------

- [x] Warnings for required GLSL extensions (implemented with `-Wextension`; also `-Wall` added like in GCC).

