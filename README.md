# XShaderCompiler ("Cross Shader Compiler") #

Features
--------

* Cross (or trans-) compiles HLSL shader code (Shader Model 4 and 5) into GLSL
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

Offline Compiler
----------------

The following command line translates the "Example.hlsl" shader file:

```
xsc -E VS -T vertex Example.hlsl -E PS -T fragment Example.hlsl
```

The result are two GLSL shader files: "Example.VS.glsl" and "Example.PS.glsl".

Library Usage
-------------

```cpp
#include <Xsc/Xsc.h>
#include <fstream>

/* ... */

auto inputStream = std::make_shared<std::ifstream>("Example.hlsl");
std::ofstream outputStream("Example.vertex.glsl");

Xsc::ShaderInput inputDesc;
inputDesc.sourceCode     = inputStream;
inputDesc.shaderVersion  = Xsc::InputShaderVersions::HLSL5;
inputDesc.entryPoint     = "VS";
inputDesc.shaderTarget   = Xsc::ShaderTargets::GLSLVertexShader;

Xsc::ShaderOutput outputDesc;
outputDesc.sourceCode    = &outputStream;
outputDesc.shaderVersion = Xsc::OutputShaderVersions::GLSL330;

// Translate HLSL code into GLSL
Xsc::StdLog log;
bool result = Xsc::CompileShader(inputDesc, outputDesc, &log);
```

Output Example
--------------

<p align="center">Meaningful output messages with line marker:</p>
<p align="center"><img src="docu/screenshot_01.png" alt="docu/screenshot_01.png"/></p>
