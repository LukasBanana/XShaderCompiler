/*
 * XscCSharp.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <Xsc/Xsc.h>
#include <sstream>
#include <vcclr.h>

#using <System.dll>
#using <System.Core.dll>
#using <System.Runtime.InteropServices.dll>


using namespace System;
using namespace System::Runtime::InteropServices;


static std::string ToStdString(String^ s)
{
    std::string out;

    if (s != nullptr)
    {
        auto inputSourceCodePtr = Marshal::StringToHGlobalAnsi(s);
        {
            out = (char*)(void*)inputSourceCodePtr;
        }
        Marshal::FreeHGlobal(inputSourceCodePtr);
    }

    return out;
}

/// <summary>XscCompiler library main class.</summary>
public ref class XscCompiler
{

    public:

        /// <summary>Shader target enumeration.</summary>
        enum class ShaderTarget
        {
            Undefined,                      //!< Undefined shader target.

            VertexShader,                   //!< Vertex shader.
            TessellationControlShader,      //!< Tessellation-control (also Hull-) shader.
            TessellationEvaluationShader,   //!< Tessellation-evaluation (also Domain-) shader.
            GeometryShader,                 //!< Geometry shader.
            FragmentShader,                 //!< Fragment (also Pixel-) shader.
            ComputeShader,                  //!< Compute shader.
        };

        /// <summary>Input shader version enumeration.</summary>
        enum class InputShaderVersion
        {
            Cg      = 2,            //!< Cg (C for graphics) is a slightly extended HLSL3.

            HLSL3   = 3,            //!< HLSL Shader Model 3.0 (DirectX 9).
            HLSL4   = 4,            //!< HLSL Shader Model 4.0 (DirectX 10).
            HLSL5   = 5,            //!< HLSL Shader Model 5.0 (DirectX 11).
            HLSL6   = 6,            //!< HLSL Shader Model 6.0 (DirectX 12).

            GLSL    = 0x0000ffff,   //!< GLSL (OpenGL).
            ESSL    = 0x0001ffff,   //!< GLSL (OpenGL ES).
            VKSL    = 0x0002ffff,   //!< GLSL (Vulkan).
        };

        /// <summary>Output shader version enumeration.</summary>
        enum class OutputShaderVersion
        {
            GLSL110 = 110,                  //!< GLSL 1.10 (OpenGL 2.0).
            GLSL120 = 120,                  //!< GLSL 1.20 (OpenGL 2.1).
            GLSL130 = 130,                  //!< GLSL 1.30 (OpenGL 3.0).
            GLSL140 = 140,                  //!< GLSL 1.40 (OpenGL 3.1).
            GLSL150 = 150,                  //!< GLSL 1.50 (OpenGL 3.2).
            GLSL330 = 330,                  //!< GLSL 3.30 (OpenGL 3.3).
            GLSL400 = 400,                  //!< GLSL 4.00 (OpenGL 4.0).
            GLSL410 = 410,                  //!< GLSL 4.10 (OpenGL 4.1).
            GLSL420 = 420,                  //!< GLSL 4.20 (OpenGL 4.2).
            GLSL430 = 430,                  //!< GLSL 4.30 (OpenGL 4.3).
            GLSL440 = 440,                  //!< GLSL 4.40 (OpenGL 4.4).
            GLSL450 = 450,                  //!< GLSL 4.50 (OpenGL 4.5).
            GLSL    = 0x0000ffff,           //!< Auto-detect minimal required GLSL version (for OpenGL 2+).

            ESSL100 = (0x00010000 + 100),   //!< ESSL 1.00 (OpenGL ES 2.0). \note Currently not supported!
            ESSL300 = (0x00010000 + 300),   //!< ESSL 3.00 (OpenGL ES 3.0). \note Currently not supported!
            ESSL310 = (0x00010000 + 310),   //!< ESSL 3.10 (OpenGL ES 3.1). \note Currently not supported!
            ESSL320 = (0x00010000 + 320),   //!< ESSL 3.20 (OpenGL ES 3.2). \note Currently not supported!
            ESSL    = 0x0001ffff,           //!< Auto-detect minimum required ESSL version (for OpenGL ES 2+). \note Currently not supported!

            VKSL450 = (0x00020000 + 450),   //!< VKSL 4.50 (Vulkan 1.0).
            VKSL    = 0x0002ffff,           //!< Auto-detect minimum required VKSL version (for Vulkan/SPIR-V).
        };

        /// <summary>Sampler filter enumeration (D3D11_FILTER).</summary>
        enum class Filter
        {
            MinMagMipPoint                          = 0,
            MinMagPointMipLinear                    = 0x1,
            MinPointMagLinearMipPoint               = 0x4,
            MinPointMagMipLinear                    = 0x5,
            MinLinearMagMipPoint                    = 0x10,
            MinLinearMagPointMipLinear              = 0x11,
            MinMagLinearMipPoint                    = 0x14,
            MinMagMipLinear                         = 0x15,
            Anisotropic                             = 0x55,
            ComparisonMinMagMipPoint                = 0x80,
            ComparisonMinMagPointMipLinear          = 0x81,
            ComparisonMinPointMagLinearMipPoint     = 0x84,
            ComparisonMinPointMagMipLinear          = 0x85,
            ComparisonMinLinearMagMipPoint          = 0x90,
            ComparisonMinLinearMagPointMipLinear    = 0x91,
            ComparisonMinMagLinearMipPoint          = 0x94,
            ComparisonMinMagMipLinear               = 0x95,
            ComparisonAnisotropic                   = 0xd5,
            MinimumMinMagMipPoint                   = 0x100,
            MinimumMinMagPointMipLinear             = 0x101,
            MinimumMinPointMagLinearMipPoint        = 0x104,
            MinimumMinPointMagMipLinear             = 0x105,
            MinimumMinLinearMagMipPoint             = 0x110,
            MinimumMinLinearMagPointMipLinear       = 0x111,
            MinimumMinMagLinearMipPoint             = 0x114,
            MinimumMinMagMipLinear                  = 0x115,
            MinimumAnisotropic                      = 0x155,
            MaximumMinMagMipPoint                   = 0x180,
            MaximumMinMagPointMipLinear             = 0x181,
            MaximumMinPointMagLinearMipPoint        = 0x184,
            MaximumMinPointMagMipLinear             = 0x185,
            MaximumMinLinearMagMipPoint             = 0x190,
            MaximumMinLinearMagPointMipLinear       = 0x191,
            MaximumMinMagLinearMipPoint             = 0x194,
            MaximumMinMagMipLinear                  = 0x195,
            MaximumAnisotropic                      = 0x1d5,
        };

        /// <summary>Texture address mode enumeration (D3D11_TEXTURE_ADDRESS_MODE).</summary>
        enum class TextureAddressMode
        {
            Wrap        = 1,
            Mirror      = 2,
            Clamp       = 3,
            Border      = 4,
            MirrorOnce  = 5,
        };

        /// <summary>Sample comparison function enumeration (D3D11_COMPARISON_FUNC).</summary>
        enum class ComparisonFunc
        {
            Never           = 1,
            Less            = 2,
            Equal           = 3,
            LessEqual       = 4,
            Greater         = 5,
            NotEqual        = 6,
            GreaterEqual    = 7,
            Always          = 8,
        };

        /// <summary>Resource type enumeration.</summary>
        /// <see cref="Resource.Type"/>
        enum class ResourceType
        {
            Undefined,

            Texture1D,
            Texture2D,
            Texture3D,
            TextureCube,
            Texture1DArray,
            Texture2DArray,
            TextureCubeArray,
            Texture2DMS,
            Texture2DMSArray,

            RWTexture1D,
            RWTexture2D,
            RWTexture3D,
            RWTextureCube,
            RWTexture1DArray,
            RWTexture2DArray,
            RWTextureCubeArray,
            RWTexture2DMS,
            RWTexture2DMSArray,

            Sampler1D,
            Sampler2D,
            Sampler3D,
            SamplerCube,
            Sampler1DArray,
            Sampler2DArray,
            SamplerCubeArray,
            Sampler2DMS,
            Sampler2DMSArray,
            Sampler2DRect,

            Buffer,
            ByteAddressBuffer,
            StructuredBuffer,
            AppendStructuredBuffer,
            ConsumeStructuredBuffer,

            RWBuffer,
            RWByteAddressBuffer,
            RWStructuredBuffer,

            ConstantBuffer,
            TextureBuffer,
            SamplerState,
            SamplerComparisonState,
        };

        /// <summary>Compiler warning flags.</summary>
        [Flags]
        enum class Warnings : System::UInt32
        {
            Disabled                = 0,         // No warnings.

            Basic                   = (1 <<  0), // Warning for basic issues (control path, disabled code etc.).
            Syntax                  = (1 <<  1), // Warning for syntactic issues.
            PreProcessor            = (1 <<  2), // Warning for pre-processor issues.
            UnusedVariables         = (1 <<  3), // Warning for unused variables.
            EmptyStatementBody      = (1 <<  4), // Warning for statements with empty body.
            ImplicitTypeConversions = (1 <<  5), // Warning for specific implicit type conversions.
            DeclarationShadowing    = (1 <<  6), // Warning for declarations that shadow a previous local (e.g. for-loops or variables in class hierarchy).
            UnlocatedObjects        = (1 <<  7), // Warning for optional objects that where not found.
            RequiredExtensions      = (1 <<  8), // Warning for required extensions in the output code.
            CodeReflection          = (1 <<  9), // Warning for issues during code reflection.
            IndexBoundary           = (1 << 10), // Warning for index boundary violations.

            All                     = (~0u),     // All warnings.
        };

        /// <summary>Language extension flags.</summary>
        /// <remarks> This is only supported, if the compiler was build with the 'XSC_ENABLE_LANGUAGE_EXT' macro.</remarks>
        [Flags]
        enum class Extensions : System::UInt32
        {
            Disabled        = 0,        // No extensions.

            LayoutAttribute = (1 << 0), //!< Enables the 'layout' attribute extension (e.g. "[layout(rgba8)]").
            SpaceAttribute  = (1 << 1), //!< Enables the 'space' attribute extension for a stronger type system (e.g. "[space(OBJECT, MODEL)]").

            All             = (~0u)     //!< All extensions.
        };

        /// <summary>Static sampler state descriptor structure (D3D11_SAMPLER_DESC).</summary>
        /// <remarks>All members and enumerations have the same values like the one in the "D3D11_SAMPLER_DESC" structure respectively.
        /// Thus, they can all be statically casted from and to the original D3D11 values.</remarks>
        /// <see cref="https://msdn.microsoft.com/en-us/library/windows/desktop/ff476207(v=vs.85).aspx"/>
        ref class SamplerStateDesc
        {

            public:

                SamplerStateDesc()
                {
                    TextureFilter   = Filter::MinMagMipLinear;
                    AddressU        = TextureAddressMode::Clamp;
                    AddressV        = TextureAddressMode::Clamp;
                    AddressW        = TextureAddressMode::Clamp;
                    MipLODBias      = 0.0f;
                    MaxAnisotropy   = 1u;
                    ComparisonFunc  = XscCompiler::ComparisonFunc::Never;
                    BorderColor     = gcnew array<float> { 0.0f, 0.0f, 0.0f, 0.0f };
                    MinLOD          = -std::numeric_limits<float>::max();
                    MaxLOD          = std::numeric_limits<float>::max();
                }

                property Filter             TextureFilter;
                property TextureAddressMode AddressU;
                property TextureAddressMode AddressV;
                property TextureAddressMode AddressW;
                property float              MipLODBias;
                property unsigned int       MaxAnisotropy;
                property ComparisonFunc     ComparisonFunc;
                property array<float>^      BorderColor;
                property float              MinLOD;
                property float              MaxLOD;

        };

        /// <summary>Input/output attribute and uniform reflection structure.</summary>
        /// <see cref="ReflectionData.InputAttributes"/>
        /// <see cref="ReflectionData.OutputAttributes"/>
        ref class Attribute
        {

            public:

                Attribute()
                {
                    Name = nullptr;
                    Slot = -1;
                }

                Attribute(String^ name, int slot)
                {
                    Name = name;
                    Slot = slot;
                }

                /// <summary>Name of the attribute.</summary>
                property String^    Name;

                /// <summary>Zero-based attribute slot number. If this is -1, the binding slot was not specified. By default -1.</summary>
                property int        Slot;

        };

        /// <summary>Resource reflection structure for textures, combined texture samplers, and buffers.</summary>
        /// <see cref="ReflectionData.Resources"/>
        ref class Resource
        {

            public:

                Resource()
                {
                    Type = ResourceType::Undefined;
                    Name = nullptr;
                    Slot = -1;
                }

                /// <summary>Resource type. By default ResourceType::Undefined.</summary>
                property ResourceType   Type;

                /// <summary>Name of the resource.</summary>
                property String^        Name;

                /// <summary>Zero-based binding slot number. If this is -1, the binding slot was not specified. By default -1.</summary>
                property int            Slot;

        };

        /// <summary>Constant buffer reflection structure.</summary>
        /// <see cref="ReflectionData.ConstantBuffers"/>
        ref class ConstantBuffer
        {

            public:

                ConstantBuffer()
                {
                    Type    = ResourceType::Undefined;
                    Name    = nullptr;
                    Slot    = -1;
                    Size    = 0;
                    Padding = 0;
                }

                /// <summary>Resource type. By default ResourceType::Undefined.</summary>
                property ResourceType   Type;

                /// <summary>Name of the constant buffer.</summary>
                property String^        Name;

                /// <summary>Zero-based binding slot number. If this is -1, the binding slot was not specified. By default -1.</summary>
                property int            Slot;

                /// <summary>Size (in bytes) of the constant buffer with a 16-byte alignment. If this is 0xFFFFFFFF, the buffer size could not be determined. By default 0.</summary>
                property unsigned int   Size;

                /// <summary>Size (in bytes) of the padding that is added to the constant buffer. By default 0.</summary>
                property unsigned int   Padding;

        };

        /// <summary>Sampler state reflection structure.</summary>
        /// <see cref="ReflectionData.SamplerStates"/>
        ref class SamplerState
        {

            public:

                SamplerState()
                {
                    Type = ResourceType::Undefined;
                    Name = nullptr;
                    Slot = -1;
                }

                /// <summary>Resource type. By default ResourceType::Undefined.</summary>
                property ResourceType   Type;

                /// <summary>Name of the sampler state.</summary>
                property String^        Name;

                /// <summary>Zero-based binding slot number. If this is -1, the binding slot was not specified. By default -1.</summary>
                property int            Slot;

        };

        /// <summary>Static sampler state reflection structure.</summary>
        /// <see cref="ReflectionData.StaticSamplerStates"/>
        ref class StaticSamplerState
        {

            public:

                StaticSamplerState()
                {
                    Type = ResourceType::Undefined;
                    Name = nullptr;
                    Desc = nullptr;
                }

                /// <summary>Resource type. By default ResourceType::Undefined.</summary>
                property ResourceType       Type;

                /// <summary>Name of the static sampler state.</summary>
                property String^            Name;

                /// <summary>Descriptor of the sampler state.</summary>
                property SamplerStateDesc^  Desc;

        };

        /// <summary>Number of threads within each work group of a compute shader.</summary>
        ref class ComputeThreads
        {

            public:

                ComputeThreads()
                {
                    X = 0;
                    Y = 0;
                    Z = 0;
                }

                ComputeThreads(int x, int y, int z)
                {
                    X = x;
                    Y = y;
                    Z = z;
                }

                /// <summary>Number of shader compute threads in X dimension.</summary>
                property int X;

                /// <summary>Number of shader compute threads in Y dimension.</summary>
                property int Y;

                /// <summary>Number of shader compute threads in Z dimension.</summary>
                property int Z;

        };

        /// <summary>Structure for shader output statistics (e.g. texture/buffer binding points).</summary>
        ref class ReflectionData
        {

            public:

                /// <summary>All defined macros after pre-processing.</summary>
                property Collections::Generic::List<String^>^               Macros;

                /// <summary>Shader input attributes.</summary>
                property Collections::Generic::List<Attribute^>^            InputAttributes;

                /// <summary>Shader output attributes.</summary>
                property Collections::Generic::List<Attribute^>^            OutputAttributes;

                /// <summary>Single shader uniforms.</summary>
                property Collections::Generic::List<Attribute^>^            Uniforms;

                /// <summary>Texture bindings.</summary>
                property Collections::Generic::List<Resource^>^             Resources;

                /// <summary>Constant buffer bindings.</summary>
                property Collections::Generic::List<ConstantBuffer^>^       ConstantBuffers;

                /// <summary>Dynamic sampler states.</summary>
                property Collections::Generic::List<SamplerState^>^         SamplerStates;

                /// <summary>Static sampler states.</summary>
                property Collections::Generic::List<StaticSamplerState^>^   StaticSamplerStates;

                /// <summary>Number of local threads in a compute shader.</summary>
                property ComputeThreads^                                    NumThreads;

        };

        /// <summary>Formatting descriptor structure for the output shader.</summary>
        ref class OutputFormatting
        {

            public:

                OutputFormatting()
                {
                    AlwaysBracedScopes  = false;
                    Blanks              = true;
                    CompactWrappers     = false;
                    Indent              = gcnew String("    ");
                    LineMarks           = false;
                    LineSeparation      = true;
                    NewLineOpenScope    = true;
                }

                /// <summary>If true, scopes are always written in braces. By default false.</summary>
                property bool       AlwaysBracedScopes;

                /// <summary>If true, blank lines are allowed. By default true.</summary>
                property bool       Blanks;

                /// <summary>If true, wrapper functions for special intrinsics are written in a compact formatting (i.e. all in one line). By default false.</summary>
                property bool       CompactWrappers;

                /// <summary>Indentation string for code generation. By default 4 spaces.</summary>
                property String^    Indent;

                /// <summary>If true, line marks are allowed. By default false.</summary>
                property bool       LineMarks;

                /// <summary>If true, auto-formatting of line separation is allowed. By default true.</summary>
                property bool       LineSeparation;

                /// <summary>If true, the '{'-braces for an open scope gets its own line. If false, braces are written like in Java coding conventions. By default true.</summary>
                property bool       NewLineOpenScope;

        };

        /// <summary>Structure for additional translation options.</summary>
        ref class OutputOptions
        {

            public:

                OutputOptions()
                {
                    AllowExtensions         = false;
                    AutoBinding             = false;
                    AutoBindingStartSlot    = 0;
                    ExplicitBinding         = false;
                    Obfuscate               = false;
                    Optimize                = false;
                    PreferWrappers          = false;
                    PreprocessOnly          = false;
                    PreserveComments        = false;
                    RowMajorAlignment       = false;
                    SeparateSamplers        = true;
                    SeparateShaders         = false;
                    ShowAST                 = false;
                    ShowTimes               = false;
                    UnrollArrayInitializers = false;
                    ValidateOnly            = false;
                    WriteGeneratorHeader    = true;
                }

                /// <summary>If true, the shader output may contain GLSL extensions, if the target shader version is too low. By default false.</summary>
                property bool   AllowExtensions;

                /// <summary>If true, binding slots for all buffer types will be generated sequentially, starting with index at 'AutoBindingStartSlot'. By default false.</summary>
                /// <remarks> This will also enable 'ExplicitBinding'.</remarks>
                property bool   AutoBinding;

                /// <summary>Index to start generating binding slots from. Only relevant if 'AutoBinding' is enabled. By default 0.</summary>
                property int    AutoBindingStartSlot;

                /// <summary>If true, explicit binding slots are enabled. By default false.</summary>
                property bool   ExplicitBinding;

                /// <summary>If true, code obfuscation is performed. By default false.</summary>
                property bool   Obfuscate;

                /// <summary>If true, little code optimizations are performed. By default false.</summary>
                property bool   Optimize;

                /// <summary>If true, intrinsics are prefered to be implemented as wrappers (instead of inlining). By default false.</summary>
                property bool   PreferWrappers;

                /// <summary>If true, only the preprocessed source code will be written out. By default false.</summary>
                property bool   PreprocessOnly;

                /// <summary>If true, commentaries are preserved for each statement. By default false.</summary>
                property bool   PreserveComments;

                /// <summary>If true, matrices have row-major alignment. Otherwise the matrices have column-major alignment. By default false.</summary>
                property bool   RowMajorAlignment;

                /// <summary>If true, generated GLSL code will contain separate sampler and texture objects when supported. By default true.</summary>
                property bool   SeparateSamplers;

                /// <summary>If true, generated GLSL code will support the 'ARB_separate_shader_objects' extension. By default false.</summary>
                property bool   SeparateShaders;

                /// <summary>If true, the AST (Abstract Syntax Tree) will be written to the log output. By default false.</summary>
                property bool   ShowAST;

                /// <summary>If true, the timings of the different compilation processes are written to the log output. By default false.</summary>
                property bool   ShowTimes;

                /// <summary>If true, array initializations will be unrolled. By default false.</summary>
                property bool   UnrollArrayInitializers;

                /// <summary>If true, the source code is only validated, but no output code will be generated. By default false.</summary>
                property bool   ValidateOnly;

                /// <summary>If true, the generator header with metadata is written as first comment to the output. By default true.</summary>
                property bool   WriteGeneratorHeader;

        };

        /// <summary>Name mangling descriptor structure for shader input/output variables (also referred to as "varyings"), temporary variables, and reserved keywords.</summary>
        ref class OutputNameMangling
        {

            public:

                OutputNameMangling()
                {
                    InputPrefix         = gcnew String("xsv_");
                    OutputPrefix        = gcnew String("xsv_");
                    ReservedWordPrefix  = gcnew String("xsr_");
                    TemporaryPrefix     = gcnew String("xst_");
                    NamespacePrefix     = gcnew String("xsn_");
                    UseAlwaysSemantics  = false;
                    RenameBufferFields  = false;
                }

                /// <summary>Name mangling prefix for shader input variables. By default "xsv_".</summary>
                /// <remarks> This can also be empty or equal to "outputPrefix".</remarks>
                property String^    InputPrefix;

                /// <summary>Name mangling prefix for shader output variables. By default "xsv_".</summary>
                /// <remarks> This can also be empty or equal to "inputPrefix".</remarks>
                property String^    OutputPrefix;

                /// <summary>Name mangling prefix for reserved words (such as "texture", "main", "sin" etc.). By default "xsr_".</summary>
                /// <remarks> This must not be equal to any of the other prefixes and it must not be empty.</remarks>
                property String^    ReservedWordPrefix;

                /// <summary>Name mangling prefix for temporary variables. By default "xst_".</summary>
                /// <remarks> This must not be equal to any of the other prefixes and it must not be empty.</remarks>
                property String^    TemporaryPrefix;

                /// <summary>Name mangling prefix for namespaces like structures or classes. By default "xsn_".</summary>
                /// <remarks> This can also be empty, but if it's not empty it must not be equal to any of the other prefixes.</remarks>
                property String^    NamespacePrefix;

                /// <summary>
                /// If true, shader input/output variables are always renamed to their semantics,
                /// even for vertex input and fragment output. Otherwise, their original identifiers are used. By default false.
                /// </summary>
                property bool       UseAlwaysSemantics;

                /// <summary>If true, the data fields of a 'buffer'-objects is renamed rather than the outer identifier. By default false.</summary>
                /// <remarks> This can be useful for external diagnostic tools, to access the original identifier.</remarks>
                property bool       RenameBufferFields;

        };

        /// <summary>Shader source include handler interface.</summary>
        ref class SourceIncludeHandler abstract
        {

            public:

                /// <summary>Returns an input stream for the specified filename.</summary>
                /// <param name="includeName">Specifies the include filename.</param>
                /// <param name="useSearchPathsFirst">Specifies whether to first use the search paths to find the file.</param>
                /// <returns>Content of the new input file.</returns>
                virtual String^ Include(String^ filename, bool useSearchPathsFirst) = 0;

        };

        /// <summary>Shader input descriptor structure.</summary>
        ref class ShaderInput
        {

            public:

                ShaderInput()
                {
                    Filename            = nullptr;
                    SourceCode          = gcnew String("");
                    ShaderVersion       = InputShaderVersion::HLSL5;
                    Target              = ShaderTarget::Undefined;
                    EntryPoint          = gcnew String("main");
                    SecondaryEntryPoint = nullptr;
                    WarningFlags        = Warnings::Disabled;
                    IncludeHandler      = nullptr;
                    ExtensionFlags      = Extensions::Disabled;
                }

                /// <summary>Specifies the filename of the input shader code. This is an optional attribute, and only a hint to the compiler.</summary>
                property String^                        Filename;

                /// <summary>Specifies the input source code stream.</summary>
                property String^                        SourceCode;

                /// <summary>Specifies the input shader version (e.g. InputShaderVersion.HLSL5 for "HLSL 5"). By default InputShaderVersion.HLSL5.</summary>
                property InputShaderVersion             ShaderVersion;

                /// <summary>Specifies the target shader (Vertex, Fragment etc.). By default ShaderTarget::Undefined.</summary>
                property ShaderTarget                   Target;

                /// <summary>Specifies the HLSL shader entry point. By default "main".</summary>
                property String^                        EntryPoint;

                /// <summary>Specifies the secondary HLSL shader entry point.</summary>
                /// <remarks>
                /// This is only used for a Tessellation-Control Shader (alias Hull Shader) entry point,
                /// when a Tessellation-Control Shader (alias Domain Shader) is the output target.
                /// This is required to translate all Tessellation-Control attributes (i.e. "partitioning" and "outputtopology")
                /// to the Tessellation-Evaluation output shader. If this is empty, the default values for these attributes are used.
                /// </remarks>
                property String^                        SecondaryEntryPoint;

                /// <summary>Compiler warning flags. This can be a bitwise OR combination of the "Warnings" enumeration entries. By default 0.</summary>
                /// <see cref="Warnings"/>
                property Warnings                       WarningFlags;

                /// <summary>Language extension flags. This can be a bitwise OR combination of the "Extensions" enumeration entries. By default 0.</summary>
                /// <remarks>This is ignored, if the compiler was not build with the 'XSC_ENABLE_LANGUAGE_EXT' macro.</remarks>
                /// <see cref="Extensions"/>
                property Extensions                     ExtensionFlags;

                /// <summary>Optional handler to handle '#include'-directives. By default null.</summary>
                /// <remarks>If this is null, the default include handler will be used, which will include files with the STL input file streams.</remarks>
                property SourceIncludeHandler^          IncludeHandler;

        };

        /// <summary>Vertex shader semantic (or rather attribute) layout structure.</summary>
        ref class VertexSemantic
        {

            public:

                VertexSemantic()
                {
                    Semantic = nullptr;
                    Location = 0;
                }

                /// <summary>Specifies the shader semantic (or rather attribute).</summary>
                property String^    Semantic;

                /// <summary>Specifies the binding location.</summary>
                property int        Location;

        };

        /// <summary>Shader output descriptor structure.</summary>
        ref class ShaderOutput
        {

            public:

                ShaderOutput()
                {
                    Filename        = nullptr;
                    SourceCode      = gcnew String("");
                    ShaderVersion   = OutputShaderVersion::GLSL;
                    VertexSemantics = gcnew Collections::Generic::List<VertexSemantic^>();
                    Options         = gcnew OutputOptions();
                    Formatting      = gcnew OutputFormatting();
                    NameMangling    = gcnew OutputNameMangling();
                }

                /// <summary>Specifies the filename of the output shader code. This is an optional attribute, and only a hint to the compiler.</summary>
                property String^                                        Filename;

                /// <summary>Specifies the output source code stream. This will contain the output code. This must not be null when passed to the "CompileShader" function!</summary>
                property String^                                        SourceCode;

                /// <summary>Specifies the output shader version. By default OutputShaderVersion::GLSL (to auto-detect minimum required version).</summary>
                property OutputShaderVersion                            ShaderVersion;

                /// <summary>Optional list of vertex semantic layouts, to bind a vertex attribute (semantic name) to a location index (only used when 'ExplicitBinding' is true).</summary>
                property Collections::Generic::List<VertexSemantic^>^   VertexSemantics;

                /// <summary>Additional options to configure the code generation.</summary>
                property OutputOptions^                                 Options;

                /// <summary>Output code formatting descriptor.</summary>
                property OutputFormatting^                              Formatting;

                /// <summary>Specifies the options for name mangling.</summary>
                property OutputNameMangling^                            NameMangling;

        };

        /// <summary>Report types enumeration.</summary>
        enum class ReportTypes
        {
            Info,       //!< Standard information.
            Warning,    //!< Warning message.
            Error       //!< Error message.
        };

        /// <summary>Compiler report class.</summary>
        ref class Report
        {

            public:

                Report(ReportTypes type, String^ message)
                {
                    Type    = type;
                    Message = message;
                }

                Report(ReportTypes type, String^ message, String^ context)
                {
                    Type    = type;
                    Context = context;
                    Message = message;
                }

                Report(ReportTypes type, String^ message, String^ line, String^ marker)
                {
                    Type    = type;
                    Message = message;
                    Line    = line;
                    Marker  = marker;
                }

                Report(ReportTypes type, String^ message, String^ line, String^ marker, String^ context)
                {
                    Type    = type;
                    Context = context;
                    Message = message;
                    Line    = line;
                    Marker  = marker;
                }

                Report(ReportTypes type, String^ message, String^ line, String^ marker, String^ context, Collections::Generic::List<String^>^ hints)
                {
                    Type    = type;
                    Context = context;
                    Message = message;
                    Line    = line;
                    Marker  = marker;
                    Hints   = hints;
                }

                /// <summary>Specifies the type of this report.</summary>
                property ReportTypes                            Type;

                /// <summary>Returns the context description string (e.g. a function name where the report occured). This may also be empty.</summary>
                property String^                                Context;

                /// <summary>Returns the message string.</summary>
                property String^                                Message;

                /// <summary>Returns the line string where the report occured. This line never has new-line characters at its end.</summary>
                property String^                                Line;

                /// <summary>Returns the line marker string to highlight the area where the report occured.</summary>
                property String^                                Marker;

                /// <summary>Returns the list of optional hints of the report.</summary>
                property Collections::Generic::List<String^>^   Hints;

                /// <summary>Returns true if this report has a line with line marker.</summary>
                /// <see cref="Line"/>
                /// <see cref="Marker"/>
                property bool HasLine
                {
                    bool get()
                    {
                        return (Line != nullptr && !Line->Empty);
                    }
                }

        };

        /// <summary>Log base class.</summary>
        ref class Log abstract
        {

            public:

                /// <summary>Submits the specified report with the current indentation.</summary>
                virtual void SubmitReport(Report^ report, String^ indent) = 0;

                /// <summary>Prints all submitted reports to the standard output.</summary>
                void PrintAll()
                {
                    PrintAll(true);
                }

                /// <summary>Prints all submitted reports to the standard output.</summary>
                virtual void PrintAll(bool verbose) = 0;

        };

    private:

        /// <summary>Standard output log (uses standard output to submit a report).</summary>
        ref class StdLog : public Log
        {

            public:

                StdLog()
                {
                    stdLog_ = new Xsc::StdLog();
                }

                ~StdLog()
                {
                    delete stdLog_;
                }

                void SubmitReport(Report^ report, String^ indent) override
                {
                    if (report != nullptr && indent != nullptr)
                    {
                        /* Copy managed report into native report */
                        Xsc::Report r(
                            static_cast<Xsc::ReportTypes>(report->Type),
                            ToStdString(report->Message),
                            ToStdString(report->Line),
                            ToStdString(report->Marker),
                            ToStdString(report->Context)
                        );

                        /* Copy hint array into managed report */
                        if (report->Hints != nullptr)
                        {
                            std::vector<std::string> hints(report->Hints->Count);

                            for (int i = 0; i < report->Hints->Count; ++i)
                                hints[i] = ToStdString(report->Hints[i]);

                            r.TakeHints(std::move(hints));
                        }

                        /* Submit report to native standard log and print immediately */
                        stdLog_->SubmitReport(r);
                    }
                }

                void PrintAll(bool verbose) override
                {
                    stdLog_->PrintAll(verbose);
                }

            private:

                Xsc::StdLog* stdLog_ = nullptr;

        };

    public:

        XscCompiler()
        {
            StandardLog = gcnew StdLog();
        }

        /// <summary>Cross compiles the shader code from the specified input stream into the specified output shader code.</summary>
        /// <param name="inputDesc">Input shader code descriptor.</param>
        /// <param name="outputDesc">Output shader code descriptor.</param>
        /// <param name="log">Optional output log. Inherit from the "Log" class interface. By default null.</param>
        /// <param name="reflectionData">Optional code reflection data. By default null.</param>
        /// <returns>True if the code has been translated successfully.</returns>
        /// <exception cref="ArgumentNullException">If either the input or output streams are null.</exception>
        /// <see cref="ShaderInput"/>
        /// <see cref="ShaderOutput"/>
        /// <see cref="Log"/>
        /// <see cref="ReflectionData"/>
        bool CompileShader(ShaderInput^ inputDesc, ShaderOutput^ outputDesc, Log^ log, ReflectionData^ reflectionData);

        /// <see cref="CompileShader(ShaderInput^, ShaderOutput^, Log^, ReflectionData^)"/>
        bool CompileShader(ShaderInput^ inputDesc, ShaderOutput^ outputDesc, Log^ log)
        {
            return CompileShader(inputDesc, outputDesc, log, nullptr);
        }

        /// <see cref="CompileShader(ShaderInput^, ShaderOutput^, Log^, ReflectionData^)"/>
        bool CompileShader(ShaderInput^ inputDesc, ShaderOutput^ outputDesc)
        {
            return CompileShader(inputDesc, outputDesc, nullptr, nullptr);
        }

        /// <summary>Returns the compiler version.</summary>
        property String ^ Version
        {
            String ^ get()
            {
                return gcnew String(XSC_VERSION_STRING);
            }
        }

        /// <summary>Returns a dictionary of all supported GLSL extensions with their minimum required version number.</summary>
        property Collections::Generic::Dictionary<String^, int>^ GLSLExtensionEnumeration
        {
            Collections::Generic::Dictionary<String^, int>^ get()
            {
                auto dict = gcnew Collections::Generic::Dictionary<String^, int>();

                for (const auto it : Xsc::GetGLSLExtensionEnumeration())
                    dict->Add(gcnew String(it.first.c_str()), it.second);

                return dict;
            }
        }

        /// <summary>Returns the standard log.</summary>
        property Log^ StandardLog
        {
            Log^ get()
            {
                return standardLog_;
            }
            private: void set(Log^ value)
            {
                standardLog_ = value;
            }
        }

    private:

        Log^ standardLog_;

};


/*
 * Wrapper classes
 */

class IncludeHandlerCSharp : public Xsc::IncludeHandler
{

    private:

        using HandlerRefType = gcroot<XscCompiler::SourceIncludeHandler^>;

        std::unique_ptr<HandlerRefType> handler_;

    public:

        IncludeHandlerCSharp(XscCompiler::SourceIncludeHandler^ handler)
        {
            if (handler != nullptr)
            {
                handler_ = std::unique_ptr<HandlerRefType>(new HandlerRefType());
                *handler_ = handler;
            }
        }

        std::unique_ptr<std::istream> Include(const std::string& filename, bool useSearchPathsFirst) override
        {
            auto stream = std::unique_ptr<std::stringstream>(new std::stringstream());

            if (handler_)
            {
                /* Include from handler and convert to std::string */
                auto sourceCode = (*handler_)->Include(gcnew String(filename.c_str()), useSearchPathsFirst);
                *stream << ToStdString(sourceCode);
            }

            return std::move(stream);
        }

};

class LogCSharp : public Xsc::Log
{

    private:

        using HandlerRefType = gcroot<XscCompiler::Log^>;

        std::unique_ptr<HandlerRefType> handler_;

    public:

        LogCSharp(XscCompiler::Log^ handler)
        {
            if (handler != nullptr)
            {
                handler_ = std::unique_ptr<HandlerRefType>(new HandlerRefType());
                *handler_ = handler;
            }
        }

        void SubmitReport(const Xsc::Report& report) override
        {
            if (handler_)
            {
                /* Copy hints into managed hints */
                auto managedHints = gcnew Collections::Generic::List<String^>();

                for (const auto& hint : report.GetHints())
                    managedHints->Add(gcnew String(hint.c_str()));

                /* Copy report into managed report */
                auto managedReport = gcnew XscCompiler::Report(
                    static_cast<XscCompiler::ReportTypes>(report.Type()),
                    gcnew String(report.Message().c_str()),
                    gcnew String(report.Line().c_str()),
                    gcnew String(report.Marker().c_str()),
                    gcnew String(report.Context().c_str()),
                    managedHints
                );

                /* Submit report to managed handler */
                (*handler_)->SubmitReport(managedReport, gcnew String(FullIndent().c_str()));
            }
        }

};


/*
 * XscCompiler class implementation
 */

static Collections::Generic::List<String^>^ ToManagedList(const std::vector<std::string>& src)
{
    auto dst = gcnew Collections::Generic::List<String^>();

    for (const auto& s : src)
        dst->Add(gcnew String(s.c_str()));

    return dst;
}

static Collections::Generic::List<XscCompiler::Attribute^>^ ToManagedList(const std::vector<Xsc::Reflection::Attribute>& src)
{
    auto dst = gcnew Collections::Generic::List<XscCompiler::Attribute^>();

    for (const auto& s : src)
        dst->Add(gcnew XscCompiler::Attribute(gcnew String(s.name.c_str()), s.slot));

    return dst;
}

static Collections::Generic::List<XscCompiler::Resource^>^ ToManagedList(const std::vector<Xsc::Reflection::Resource>& src)
{
    auto dst = gcnew Collections::Generic::List<XscCompiler::Resource^>();

    for (const auto& s : src)
    {
        auto entry = gcnew XscCompiler::Resource();
        {
            entry->Type = static_cast<XscCompiler::ResourceType>(s.type);
            entry->Name = gcnew String(s.name.c_str());
            entry->Slot = s.slot;
        }
        dst->Add(entry);
    }

    return dst;
}

static Collections::Generic::List<XscCompiler::ConstantBuffer^>^ ToManagedList(const std::vector<Xsc::Reflection::ConstantBuffer>& src)
{
    auto dst = gcnew Collections::Generic::List<XscCompiler::ConstantBuffer^>();

    for (const auto& s : src)
    {
        auto entry = gcnew XscCompiler::ConstantBuffer();
        {
            entry->Type     = static_cast<XscCompiler::ResourceType>(s.type);
            entry->Name     = gcnew String(s.name.c_str());
            entry->Slot     = s.slot;
            entry->Size     = s.size;
            entry->Padding  = s.padding;
        }
        dst->Add(entry);
    }

    return dst;
}

static Collections::Generic::List<XscCompiler::SamplerState^>^ ToManagedList(const std::vector<Xsc::Reflection::SamplerState>& src)
{
    auto dst = gcnew Collections::Generic::List<XscCompiler::SamplerState^>();

    for (const auto& s : src)
    {
        auto entry = gcnew XscCompiler::SamplerState();
        {
            entry->Type = static_cast<XscCompiler::ResourceType>(s.type);
            entry->Name = gcnew String(s.name.c_str());
            entry->Slot = s.slot;
        }
        dst->Add(entry);
    }

    return dst;
}

static Collections::Generic::List<XscCompiler::StaticSamplerState^>^ ToManagedList(const std::vector<Xsc::Reflection::StaticSamplerState>& src)
{
    auto dst = gcnew Collections::Generic::List<XscCompiler::StaticSamplerState^>();

    for (const auto& s : src)
    {
        auto entry = gcnew XscCompiler::StaticSamplerState();
        {
            entry->Type = static_cast<XscCompiler::ResourceType>(s.type);
            entry->Name = gcnew String(s.name.c_str());
            entry->Desc = gcnew XscCompiler::SamplerStateDesc();
            {
                entry->Desc->TextureFilter  = static_cast<XscCompiler::Filter>(s.desc.filter);
                entry->Desc->AddressU       = static_cast<XscCompiler::TextureAddressMode>(s.desc.addressU);
                entry->Desc->AddressV       = static_cast<XscCompiler::TextureAddressMode>(s.desc.addressV);
                entry->Desc->AddressW       = static_cast<XscCompiler::TextureAddressMode>(s.desc.addressW);
                entry->Desc->MipLODBias     = s.desc.mipLODBias;
                entry->Desc->MaxAnisotropy  = s.desc.maxAnisotropy;
                entry->Desc->ComparisonFunc = static_cast<XscCompiler::ComparisonFunc>(s.desc.comparisonFunc);
                entry->Desc->BorderColor    = gcnew array<float>
                {
                    s.desc.borderColor[0],
                    s.desc.borderColor[1],
                    s.desc.borderColor[2],
                    s.desc.borderColor[3]
                };
                entry->Desc->MinLOD         = s.desc.minLOD;
                entry->Desc->MaxLOD         = s.desc.maxLOD;
            }
        }
        dst->Add(entry);
    }

    return dst;
}

bool XscCompiler::CompileShader(ShaderInput^ inputDesc, ShaderOutput^ outputDesc, Log^ log, ReflectionData^ reflectionData)
{
    /* Validate input arguments */
    if (inputDesc == nullptr)
        throw gcnew ArgumentNullException(gcnew String("inputDesc"));
    if (outputDesc == nullptr)
        throw gcnew ArgumentNullException(gcnew String("outputDesc"));
    if (inputDesc->SourceCode == nullptr)
        throw gcnew ArgumentNullException(gcnew String("inputDesc.SourceCode"));
    if (outputDesc->Options == nullptr)
        throw gcnew ArgumentNullException(gcnew String("outputDesc.Options"));
    if (outputDesc->Formatting == nullptr)
        throw gcnew ArgumentNullException(gcnew String("outputDesc.Formatting"));
    if (outputDesc->NameMangling == nullptr)
        throw gcnew ArgumentNullException(gcnew String("outputDesc.NameMangling"));

    /* Copy input descriptor */
    Xsc::ShaderInput in;

    IncludeHandlerCSharp includeHandler(inputDesc->IncludeHandler);

    auto inputStream = std::make_shared<std::stringstream>();
    *inputStream << ToStdString(inputDesc->SourceCode);

    in.filename             = ToStdString(inputDesc->Filename);
    in.sourceCode           = inputStream;
    in.shaderVersion        = static_cast<Xsc::InputShaderVersion>(inputDesc->ShaderVersion);
    in.shaderTarget         = static_cast<Xsc::ShaderTarget>(inputDesc->Target);
    in.entryPoint           = ToStdString(inputDesc->EntryPoint);
    in.secondaryEntryPoint  = ToStdString(inputDesc->SecondaryEntryPoint);
    in.warnings             = static_cast<unsigned int>(inputDesc->WarningFlags);
    in.includeHandler       = (&includeHandler);
    in.extensions           = static_cast<unsigned int>(inputDesc->ExtensionFlags);

    /* Copy output descriptor */
    Xsc::ShaderOutput out;

    std::stringstream outputStream;

    out.filename        = ToStdString(outputDesc->Filename);
    out.sourceCode      = (&outputStream);
    out.shaderVersion   = static_cast<Xsc::OutputShaderVersion>(outputDesc->ShaderVersion);

    if (outputDesc->VertexSemantics != nullptr)
    {
        out.vertexSemantics.resize(outputDesc->VertexSemantics->Count);
        for (int i = 0; i < outputDesc->VertexSemantics->Count; ++i)
        {
            out.vertexSemantics[i].semantic = ToStdString(outputDesc->VertexSemantics[i]->Semantic);
            out.vertexSemantics[i].location = outputDesc->VertexSemantics[i]->Location;
        }
    }

    /* Copy output options descriptor */
    out.options.allowExtensions         = outputDesc->Options->AllowExtensions;
    out.options.autoBinding             = outputDesc->Options->AutoBinding;
    out.options.autoBindingStartSlot    = outputDesc->Options->AutoBindingStartSlot;
    out.options.explicitBinding         = outputDesc->Options->ExplicitBinding;
    out.options.obfuscate               = outputDesc->Options->Obfuscate;
    out.options.optimize                = outputDesc->Options->Optimize;
    out.options.preferWrappers          = outputDesc->Options->PreferWrappers;
    out.options.preprocessOnly          = outputDesc->Options->PreprocessOnly;
    out.options.preserveComments        = outputDesc->Options->PreserveComments;
    out.options.rowMajorAlignment       = outputDesc->Options->RowMajorAlignment;
    out.options.separateSamplers        = outputDesc->Options->SeparateSamplers;
    out.options.separateShaders         = outputDesc->Options->SeparateShaders;
    out.options.showAST                 = outputDesc->Options->ShowAST;
    out.options.showTimes               = outputDesc->Options->ShowTimes;
    out.options.unrollArrayInitializers = outputDesc->Options->UnrollArrayInitializers;
    out.options.validateOnly            = outputDesc->Options->ValidateOnly;
    out.options.writeGeneratorHeader    = outputDesc->Options->WriteGeneratorHeader;

    /* Copy output formatting descriptor */
    out.formatting.alwaysBracedScopes   = outputDesc->Formatting->AlwaysBracedScopes;
    out.formatting.blanks               = outputDesc->Formatting->Blanks;
    out.formatting.compactWrappers      = outputDesc->Formatting->CompactWrappers;
    out.formatting.indent               = ToStdString(outputDesc->Formatting->Indent);
    out.formatting.lineMarks            = outputDesc->Formatting->LineMarks;
    out.formatting.lineSeparation       = outputDesc->Formatting->LineSeparation;
    out.formatting.newLineOpenScope     = outputDesc->Formatting->NewLineOpenScope;

    /* Copy output name mangling descriptor */
    out.nameMangling.inputPrefix        = ToStdString(outputDesc->NameMangling->InputPrefix);
    out.nameMangling.outputPrefix       = ToStdString(outputDesc->NameMangling->OutputPrefix);
    out.nameMangling.reservedWordPrefix = ToStdString(outputDesc->NameMangling->ReservedWordPrefix);
    out.nameMangling.temporaryPrefix    = ToStdString(outputDesc->NameMangling->TemporaryPrefix);
    out.nameMangling.namespacePrefix    = ToStdString(outputDesc->NameMangling->NamespacePrefix);
    out.nameMangling.useAlwaysSemantics = outputDesc->NameMangling->UseAlwaysSemantics;
    out.nameMangling.renameBufferFields = outputDesc->NameMangling->RenameBufferFields;

    /* Compile shader */
    bool result = false;
    Xsc::Reflection::ReflectionData reflect;
    LogCSharp logCSharp(log);

    try
    {
        result = Xsc::CompileShader(
            in,
            out,
            (log != nullptr ? &logCSharp : nullptr),
            (reflectionData != nullptr ? &reflect : nullptr)
        );
    }
    catch (const std::exception& e)
    {
        throw gcnew Exception(gcnew String(e.what()));
    }

    /* Copy output code */
    if (result)
    {
        /* Copy output code */
        auto outputCode = outputStream.str();
        outputDesc->SourceCode = gcnew String(outputCode.c_str());

        /* Copy reflection */
        if (reflectionData != nullptr)
        {
            const auto& src = reflect;
            auto dst = reflectionData;

            /* Copy lists in reflection */
            dst->Macros                 = ToManagedList(src.macros);
            dst->InputAttributes        = ToManagedList(src.inputAttributes);
            dst->OutputAttributes       = ToManagedList(src.outputAttributes);
            dst->Uniforms               = ToManagedList(src.uniforms);
            dst->Resources              = ToManagedList(src.resources);
            dst->ConstantBuffers        = ToManagedList(src.constantBuffers);
            dst->SamplerStates          = ToManagedList(src.samplerStates);
            dst->StaticSamplerStates    = ToManagedList(src.staticSamplerStates);

            /* Copy compute threads reflection */
            dst->NumThreads = gcnew ComputeThreads(
                src.numThreads.x,
                src.numThreads.y,
                src.numThreads.z
            );
        }
    }

    return result;
}




// ================================================================================
