# HLSL (Shader Model 4 & 5) Translator #

Features
--------

* Translates HLSL shader code into GLSL
* Simple to integrate into other projects
* Written in C++11

License
-------

3-Clause BSD License

Status
------

##### TODO List: #####
* Preprocessor (*In Progress*).
* Common HLSL IO semantics to GLSL transformation.
* 'typedef' statement.
* Geometry and Tessellation semantics.
* 'interface' and 'class' declarations.
* Cast expressions to a struct type is currently not supported (e.g. "(VertexOut)0").

Offline Translator
------------------

The following command line translates the "Example.hlsl" shader file:

```
HTLibCmd -entry VS -target vertex Example.hlsl -entry PS -target fragment Example.hlsl
```

The result are two GLSL shader files: "Example.vertex.glsl" and "Example.fragment.glsl".

Library Usage
-------------

```cpp
#include <HT/Translator.h>
#include <fstream>

/* ... */

auto inputStream = std::make_shared<std::ifstream>("Example.hlsl");
std::ofstream outputStream("Example.vertex.glsl");

HTLib::ShaderInput inputDesc;
inputDesc.sourceCode     = inputStream;
inputDesc.shaderVersion  = HTLib::InputShaderVersions::HLSL5;
inputDesc.entryPoint     = "VS";
inputDesc.shaderTarget   = HTLib::ShaderTargets::GLSLVertexShader;

HTLib::ShaderOutput outputDesc;
outputDesc.sourceCode    = &outputStream;
outputDesc.shaderVersion = HTLib::OutputShaderVersions::GLSL330;

// Translate HLSL code into GLSL
HTLib::StdLog log;
bool result = HTLib::TranslateHLSLtoGLSL(inputDesc, outputDesc, &log);
```

Output Example
--------------

<p align="center">Meaningful output messages with line marker:</p>
<p align="center"><img src="docu/screenshot_01.png" alt="docu/screenshot_01.png"/></p>
