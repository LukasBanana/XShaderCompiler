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
* Common HLSL IO semantics to GLSL transformation.
* 'typedef' statement.
* Basic pre-processor (*In Progress*).
* Geometry and Tessellation semantics.
* 'interface' and 'class' declarations.

##### Limitations: #####

**UPDATE**:
*The limitations described in this section are about to be solved.
The development of an sophisticated pre-processor for the HLSL translator is in progress.*

Pre-processor directives (beginning with '#') will be translated as something like a dummy statement. Example:
```
#if 1
float4 Function(inout float4 x);
#endif
```
Will be translated to:
```
#if 1
vec4 Function(inout vec4 x);
#endif
```
And the following HLSL code can currently not be translated:
```
#define FOREVER for(;;)
FOREVER
{
	if (Function())
		break;
}
```
That is to say that pre-processor directives may only appear as an own statement
and not as a combination with other statements or expressions.
Only 'include' directives will be parsed and inlined.

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
#include <iostream>

/* ... */

// Implements the output log interface
class StdLog : public HTLib::Log
{
	public:
		void Info(const std::string& message) override
		{
			std::cout << indent << message << std::endl;
		}
		void Warning(const std::string& message) override
		{
			std::cout << indent << message << std::endl;
		}
		void Error(const std::string& message) override
		{
			std::cerr << indent << message << std::endl;
		}
		void IncIndent() override
		{
			indent.push_back(' ');
		}
		void DecIndent() override
		{
			if (!indent.empty())
				indent.pop_back();
		}
	private:
		std::string indent;
};

/* ... */

auto inputStream = std::make_shared<std::ifstream>("Example.hlsl");
std::ofstream outputStream("Example.vertex.glsl");

ShaderIncludeHandler includeHandler;
HTLib::Options options;
StdLog log;

// Translate HLSL code into GLSL
bool result = HTLib::TranslateHLSLtoGLSL(
	inputStream,							// Input HLSL stream
	outputStream,							// Output GLSL stream
	"VS",									// Main entry point
	HTLib::ShaderTargets::GLSLVertexShader,	// Target shader
	HTLib::InputShaderVersions::HLSL5,		// Input shader version: HLSL Shader Model 5
	HTLib::OutputShaderVersions::GLSL330,	// Output shader version: GLSL 3.30
	nullptr,							// Use default include handler
	options,								// Optional translation options
	&log									// Optional output log
);
```
