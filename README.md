# HLSL (Shader Model 4 & 5) Translator #

### Features ###

* Translates HLSL shader code into GLSL
* Simple to integrate into other projects
* Written in C++11

### License ###

3-Clause BSD License

### Offline Translator ###

The following command line translates the "Example.hlsl" shader file:

```
HLSLOfflineTranslator -entry VS -target vertex Example.hlsl -entry PS -target fragment Example.hlsl
```

The result are two GLSL shader files: "Example.hlsl.vertex.glsl" and "Example.hlsl.fragment.glsl".

### Library Usage ###

```cpp
#include <HT/Translator.h>
#include <fstream>

/* ... */

// Implements the include handler interface
class ShaderIncludeHandler : public HT::IncludeHandler
{
	public:
		std::shared_ptr<std::istream> Include(const std::string& includeName) override
		{
			return std::make_shared<std::ifstream>(includeName);
		}
};

// Implements the output log interface
class Log : public HT::Logger
{
	public:
		void Info(const std::string& message) override
		{
			std::cout << message << std::endl;
		}
		void Warning(const std::string& message) override
		{
			std::cout << message << std::endl;
		}
		void Error(const std::string& message) override
		{
			std::cerr << message << std::endl;
		}
};

/* ... */

auto inputStream = std::make_shared<std::ifstream>("Example.hlsl");
std::ofstream outputStream("Example.vertex.glsl");

ShaderIncludeHandler includeHandler;
HT::Options options;
Log log;

// Translate HLSL code into GLSL
bool result = TranslateHLSLtoGLSL(
	inputStream,							// Input HLSL stream
	outputStream,							// Output GLSL stream
	"VS",									// Main entry point
	HT::ShaderTargets::GLSLVertexShader,	// Target shader
	HT::InputShaderVersions::HLSL5,			// Input shader version: HLSL Shader Model 5
	HT::OutputShaderVersions::GLSL330,		// Output shader version: GLSL 3.30
	&includeHandler,						// Optional include handler
	options,								// Optional translation options
	&log									// Optional output log
);
```
