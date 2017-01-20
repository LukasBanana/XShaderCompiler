TODO List
---------

| Feature | Priority | Remarks |
|---------|:--------:|---------|
| Output semantic from casted return value in entry point | Very high | Is currently ignored in GLSL output. |
| Braced initializer | High | Translation of braced initializers are incomplete. |
| Geometry shader support | High | Geometry shaders can currently be parsed, but analysis and translation is incomplete. |
| Tessellation shader support | High | Tessellation shader attributes are done, but in/out semantics are incomplete. |
| Type denoter analysis clean-up | Medium | Few more things should be moved from AST nodes into TypeDenoter classes (e.g. constness). |
| 'interface' and 'class' declarations | Low | Interfaces and classes can currently not even be parsed. |
