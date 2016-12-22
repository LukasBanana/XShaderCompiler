TODO List
---------

| Feature | Priority | Remarks |
|---------|:--------:|---------|
| Common HLSL I/O semantics to GLSL transformation | Very high | Few cases left, where the in/out semantics are translated incorrectly. |
| Tessellation shader support | High | Tessellation shader attributes are done, but in/out semantics are incomplete. |
| Geometry shader support | High | Geometry shaders can currently be parsed, but analysis and translation is incomplete. |
| Type denoter analysis clean-up | Medium | Few more things should be moved from the AST nodes into the TypeDenoter classes. |
| 'interface' and 'class' declarations | Low | Interfaces and classes can currently not even be parsed. |
