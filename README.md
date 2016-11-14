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
* Common HLSL IO semantics to GLSL transformation.
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

A few thoughts on translating HLSL
----------------------------------

Although HLSL lacks lots of features commonly seen in general purpose programming languages like C++ and Java,
HLSL is a very complex language, in both syntax and context!
The XShaderCompiler has to be prepared for a lot of weird corner cases, especially syntactically.
Take a look at the following example of an unnecessarily complex expression:
```hlsl
float f = ((vector<float, (1+4)/5+3>)1).w;
```
The XShaderCompiler is able to translate this to the follwing GLSL code:
```glsl
float f = (vec4(1)).w;
```
Many other features like structure inheritance (which does not seem to be documented in the HLSL manual pages)
must be translated to other constructs in GLSL, because GLSL is a more simpler language -- which pleases the compiler builder ;-).
