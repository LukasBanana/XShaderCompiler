TODO List
---------

| Feature | Priority | Remarks |
|---------|:--------:|---------|
| Output semantic from casted return value in entry point | Very high | Is currently ignored in GLSL output. **in progress** |
| Braced initializer | High | Translation of braced initializers are incomplete. **in progress** |
| Tessellation shader support | High | Tessellation shader attributes are done, but in/out semantics are incomplete. |
| Type denoter analysis clean-up | Medium | Few more things should be moved from AST nodes into TypeDenoter classes (e.g. constness). **in progress** |
| GLSL Frontend | Medium | GLSL can currently only be pre-processed. |


Requested Features
------------------

- [x] Warnings for required GLSL extensions (implemented with `-Wextension`; also `-Wall` added like in GCC).
- [ ] Shader Reflection: texture type, constant buffer members

