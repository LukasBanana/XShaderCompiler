/*
 * Xsc.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_XSC_H
#define XSC_XSC_H


#include "Export.h"
#include "Log.h"
#include "IncludeHandler.h"
#include "Targets.h"
#include "Version.h"
#include "Reflection.h"

#include <string>
#include <vector>
#include <map>
#include <istream>
#include <ostream>
#include <memory>


/**
\mainpage
Welcome to the XShaderCompiler, Version 0.09 Alpha

Here is a quick start example:
\code
#include <Xsc/Xsc.h>
#include <fstream>

int main()
{
    // Open input and output streams
    auto inputStream = std::make_shared<std::ifstream>("Example.hlsl");
    std::ofstream outputStream("Example.VS.vert");

    // Initialize shader input descriptor structure
    Xsc::ShaderInput inputDesc;
    {
        inputDesc.sourceCode     = inputStream;
        inputDesc.shaderVersion  = Xsc::InputShaderVersion::HLSL5;
        inputDesc.entryPoint     = "VS";
        inputDesc.shaderTarget   = Xsc::ShaderTarget::VertexShader;
    }

    // Initialize shader output descriptor structure
    Xsc::ShaderOutput outputDesc;
    {
        outputDesc.sourceCode    = &outputStream;
        outputDesc.shaderVersion = Xsc::OutputShaderVersion::GLSL330;
    }

    // Compile HLSL code into GLSL
    Xsc::StdLog log;
    bool result = Xsc::CompileShader(inputDesc, outputDesc, &log);

    // Show compilation status
    if (result)
        std::cout << "Compilation successful" << std::endl;
    else
        std::cerr << "Compilation failed" << std::endl;

    return 0;
}
\endcode
*/


//! Main XShaderCompiler namespace
namespace Xsc
{


//! Compiler warning flags.
struct Warnings
{
    enum : unsigned int
    {
        Basic                   = (1 << 0), //!< Warning for basic issues (control path, disabled code etc.).
        Syntax                  = (1 << 1), //!< Warning for syntactic issues.
        PreProcessor            = (1 << 2), //!< Warning for pre-processor issues.
        UnusedVariables         = (1 << 3), //!< Warning for unused variables.
        EmptyStatementBody      = (1 << 4), //!< Warning for statements with empty body.
        ImplicitTypeConversions = (1 << 5), //!< Warning for specific implicit type conversions.
        DeclarationShadowing    = (1 << 6), //!< Warning for declarations that shadow a previous local (e.g. for-loops or variables in class hierarchy).
        UnlocatedObjects        = (1 << 7), //!< Warning for optional objects that where not found.
        RequiredExtensions      = (1 << 8), //!< Warning for required extensions in the output code.
        CodeReflection          = (1 << 9), //!< Warning for issues during code reflection.

        All                     = (~0u),    //!< All warnings.
    };
};

//! Formatting descriptor structure for the output shader.
struct Formatting
{
    //! Indentation string for code generation. By default std::string(4, ' ').
    std::string indent              = "    ";

    //! If true, blank lines are allowed. By default true.
    bool        blanks              = true;

    //! If true, line marks are allowed. By default false.
    bool        lineMarks           = false;

    //! If true, wrapper functions for special intrinsics are written in a compact formatting (i.e. all in one line). By default false.
    bool        compactWrappers     = false;

    //! If true, scopes are always written in braces. By default false.
    bool        alwaysBracedScopes  = false;

    //! If true, the '{'-braces for an open scope gets its own line. If false, braces are written like in Java coding conventions. By default true.
    bool        newLineOpenScope    = true;

    //! If true, auto-formatting of line separation is allowed. By default true.
    bool        lineSeparation      = true;
};

//! Structure for additional translation options.
struct Options
{
    //! If true, little code optimizations are performed. By default false.
    bool    optimize                = false;

    //! If true, only the preprocessed source code will be written out. By default false.
    bool    preprocessOnly          = false;

    //! If true, the source code is only validated, but no output code will be generated. By default false.
    bool    validateOnly            = false;

    //! If true, the shader output may contain GLSL extensions, if the target shader version is too low. By default false.
    bool    allowExtensions         = false;

    //! If true, explicit binding slots are enabled. By default false.
    bool    explicitBinding         = false;

    /**
    \brief If true, binding slots for all buffer types will be generated sequentially, starting with index at 'autoBindingStartSlot'. By default false.
    \remarks This will also enable 'explicitBinding'.
    */
    bool    autoBinding             = false;

    //! Index to start generating binding slots from. Only relevant if 'autoBinding' is enabled. By default 0.
    int     autoBindingStartSlot    = 0;

    //! If true, commentaries are preserved for each statement. By default false.
    bool    preserveComments        = false;

    //! If true, intrinsics are prefered to be implemented as wrappers (instead of inlining). By default false.
    bool    preferWrappers          = false;

    //! If true, array initializations will be unrolled. By default false.
    bool    unrollArrayInitializers = false;

    //! If true, matrices have row-major alignment. Otherwise the matrices have column-major alignment. By default false.
    bool    rowMajorAlignment       = false;

    //! If true, code obfuscation is performed. By default false.
    bool    obfuscate               = false;

    //! If true, the AST (Abstract Syntax Tree) will be written to the log output. By default false.
    bool    showAST                 = false;

    //! If true, the timings of the different compilation processes are written to the log output. By default false.
    bool    showTimes               = false;
};

//! Name mangling descriptor structure for shader input/output variables (also referred to as "varyings"), temporary variables, and reserved keywords.
struct NameMangling
{
    /**
    \brief Name mangling prefix for shader input variables. By default "xsv_".
    \remarks This can also be empty or equal to "outputPrefix".
    */
    std::string     inputPrefix         = "xsv_";

    /**
    \brief Name mangling prefix for shader output variables. By default "xsv_".
    \remarks This can also be empty or equal to "inputPrefix".
    */
    std::string     outputPrefix        = "xsv_";

    /**
    \brief Name mangling prefix for reserved words (such as "texture", "main", "sin" etc.). By default "xsr_".
    \remarks This must not be equal to any of the other prefixes and it must not be empty.
    */
    std::string     reservedWordPrefix  = "xsr_";

    /**
    \brief Name mangling prefix for temporary variables. By default "xst_".
    \remarks This must not be equal to any of the other prefixes and it must not be empty.
    */
    std::string     temporaryPrefix     = "xst_";

    /**
    \brief Name mangling prefix for namespaces like structures or classes. By default "xsn_".
    \remarks This can also be empty, but if it's not empty it must not be equal to any of the other prefixes.
    */
    std::string     namespacePrefix     = "xsn_";

    /**
    If true, shader input/output variables are always renamed to their semantics,
    even for vertex input and fragment output. Otherwise, their original identifiers are used. By default false.
    */
    bool            useAlwaysSemantics  = false;
};

//! Shader input descriptor structure.
struct ShaderInput
{
    //! Specifies the filename of the input shader code. This is an optional attribute, and only a hint to the compiler.
    std::string                     filename;

    //! Specifies the input source code stream.
    std::shared_ptr<std::istream>   sourceCode;

    //! Specifies the input shader version (e.g. InputShaderVersion::HLSL5 for "HLSL 5"). By default InputShaderVersion::HLSL5.
    InputShaderVersion              shaderVersion       = InputShaderVersion::HLSL5;
    
    //! Specifies the target shader (Vertex, Fragment etc.). By default ShaderTarget::Undefined.
    ShaderTarget                    shaderTarget        = ShaderTarget::Undefined;

    //! Specifies the HLSL shader entry point. By default "main".
    std::string                     entryPoint          = "main";

    /**
    \brief Specifies the secondary HLSL shader entry point.
    \remarks This is only used for a Tessellation-Control Shader (alias Hull Shader) entry point,
    when a Tessellation-Control Shader (alias Domain Shader) is the output target.
    This is required to translate all Tessellation-Control attributes (i.e. "partitioning" and "outputtopology")
    to the Tessellation-Evaluation output shader. If this is empty, the default values for these attributes are used.
    */
    std::string                     secondaryEntryPoint;

    /**
    \brief Compiler warning flags. This can be a bitwise OR combination of the "Warnings" enumeration entries. By default 0.
    \see Warnings
    */
    unsigned int                    warnings            = 0;

    /**
    \brief Optional pointer to the implementation of the "IncludeHandler" interface. By default null.
    \remarks If this is null, the default include handler will be used, which will include files with the STL input file streams.
    */
    IncludeHandler*                 includeHandler      = nullptr;
};

//! Vertex shader semantic (or rather attribute) layout structure.
struct VertexSemantic
{
    //! Specifies the shader semantic (or rather attribute).
    std::string semantic;

    //! Specifies the binding location.
    int         location;
};

//! Shader output descriptor structure.
struct ShaderOutput
{
    //! Specifies the filename of the output shader code. This is an optional attribute, and only a hint to the compiler.
    std::string                 filename;

    //! Specifies the output source code stream. This will contain the output code. This must not be null when passed to the "CompileShader" function!
    std::ostream*               sourceCode          = nullptr;

    //! Specifies the output shader version. By default OutputShaderVersion::GLSL (to auto-detect minimum required version).
    OutputShaderVersion         shaderVersion       = OutputShaderVersion::GLSL;

    //! Optional list of vertex semantic layouts, to bind a vertex attribute (semantic name) to a location index (only used when 'explicitBinding' is true).
    std::vector<VertexSemantic> vertexSemantics;

    //! Additional options to configure the code generation.
    Options                     options;

    //! Output code formatting descriptor.
    Formatting                  formatting;
    
    //! Specifies the options for name mangling.
    NameMangling                nameMangling;
};

/**
\brief Cross compiles the shader code from the specified input stream into the specified output shader code.
\param[in] inputDesc Input shader code descriptor.
\param[in] outputDesc Output shader code descriptor.
\param[in] log Optional pointer to an output log. Inherit from the "Log" class interface. By default null.
\param[out] reflectionData Optional pointer to a code reflection data structure. By default null.
\return True if the code has been translated successfully.
\throw std::invalid_argument If either the input or output streams are null.
\see ShaderInput
\see ShaderOutput
\see Log
\see ReflectionData
*/
XSC_EXPORT bool CompileShader(
    const ShaderInput&          inputDesc,
    const ShaderOutput&         outputDesc,
    Log*                        log             = nullptr,
    Reflection::ReflectionData* reflectionData  = nullptr
);


} // /namespace Xsc


#endif



// ================================================================================