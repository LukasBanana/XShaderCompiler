TODO List
---------

| Feature | Priority | Remarks |
|---------|:--------:|---------|
| Output semantic from casted return value in entry point | Very high | Is currently ignored in GLSL output |
| Default parameters | Very high | Default parameters must be resolved in GLSL output |
| Matrix packing | High | Matrix packing (row- or column major) must be changable by pre-processor, type modifier, and compiler option |
| Geometry shader support | High | Geometry shaders can currently be parsed, but analysis and translation is incomplete. |
| Tessellation shader support | High | Tessellation shader attributes are done, but in/out semantics are incomplete. |
| Type denoter analysis clean-up | Medium | Few more things should be moved from the AST nodes into the TypeDenoter classes. |
| 'interface' and 'class' declarations | Low | Interfaces and classes can currently not even be parsed. |
