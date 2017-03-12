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

//! XscCompiler library main class.
public ref class XscCompiler
{

    public:

        //! Shader target enumeration.
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

        //! Input shader version enumeration.
        enum class InputShaderVersion
        {
            HLSL3   = 3,            //!< HLSL Shader Model 3.0 (DirectX 9).
            HLSL4   = 4,            //!< HLSL Shader Model 4.0 (DirectX 10).
            HLSL5   = 5,            //!< HLSL Shader Model 5.0 (DirectX 11).

            GLSL    = 0x0000ffff,   //!< GLSL (OpenGL).
            ESSL    = 0x0001ffff,   //!< GLSL (OpenGL ES).
            VKSL    = 0x0002ffff,   //!< GLSL (Vulkan).
        };

        //! Output shader version enumeration.
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

        //! Sampler filter enumeration (D3D11_FILTER).
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

        //! Texture address mode enumeration (D3D11_TEXTURE_ADDRESS_MODE).
        enum class TextureAddressMode
        {
            Wrap        = 1,
            Mirror      = 2,
            Clamp       = 3,
            Border      = 4,
            MirrorOnce  = 5,
        };

        //! Sample comparison function enumeration (D3D11_COMPARISON_FUNC).
        enum class Comparison
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

        /**
        \brief Static sampler state descriptor structure (D3D11_SAMPLER_DESC).
        \remarks All members and enumerations have the same values like the one in the "D3D11_SAMPLER_DESC" structure respectively.
        Thus, they can all be statically casted from and to the original D3D11 values.
        \see https://msdn.microsoft.com/en-us/library/windows/desktop/ff476207(v=vs.85).aspx
        */
        ref class SamplerState
        {

            public:

                SamplerState()
                {
                    TextureFilter   = Filter::MinMagMipLinear;
                    AddressU        = TextureAddressMode::Clamp;
                    AddressV        = TextureAddressMode::Clamp;
                    AddressW        = TextureAddressMode::Clamp;
                    MipLODBias      = 0.0f;
                    MaxAnisotropy   = 1u;
                    ComparisonFunc  = Comparison::Never;
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
                property Comparison         ComparisonFunc;
                property array<float>^      BorderColor;
                property float              MinLOD;
                property float              MaxLOD;

        };

        //! Formatting descriptor structure for the output shader.
        ref class OutputFormatting
        {

            public:

                OutputFormatting()
                {
                    Indent              = gcnew String("    ");
                    Blanks              = true;
                    LineMarks           = false;
                    CompactWrappers     = false;
                    AlwaysBracedScopes  = false;
                    NewLineOpenScope    = true;
                    LineSeparation      = true;
                }

                //! Indentation string for code generation. By default 4 spaces.
                property String^    Indent;

                //! If true, blank lines are allowed. By default true.
                property bool       Blanks;

                //! If true, line marks are allowed. By default false.
                property bool       LineMarks;

                //! If true, wrapper functions for special intrinsics are written in a compact formatting (i.e. all in one line). By default false.
                property bool       CompactWrappers;

                //! If true, scopes are always written in braces. By default false.
                property bool       AlwaysBracedScopes;

                //! If true, the '{'-braces for an open scope gets its own line. If false, braces are written like in Java coding conventions. By default true.
                property bool       NewLineOpenScope;

                //! If true, auto-formatting of line separation is allowed. By default true.
                property bool       LineSeparation;

        };

        //! Structure for additional translation options.
        ref class OutputOptions
        {

            public:

                OutputOptions()
                {
                    Warnings                = false;
                    Optimize                = false;
                    PreprocessOnly          = false;
                    ValidateOnly            = false;
                    AllowExtensions         = false;
                    ExplicitBinding         = false;
                    PreserveComments        = false;
                    PreferWrappers          = false;
                    UnrollArrayInitializers = false;
                    RowMajorAlignment       = false;
                    Obfuscate               = false;
                    ShowAST                 = false;
                    ShowTimes               = false;
                }

                //! True if warnings are allowed. By default false.
                property bool Warnings;

                //! If true, little code optimizations are performed. By default false.
                property bool Optimize;

                //! If true, only the preprocessed source code will be written out. By default false.
                property bool PreprocessOnly;

                //! If true, the source code is only validated, but no output code will be generated. By default false.
                property bool ValidateOnly;

                //! If true, the shader output may contain GLSL extensions, if the target shader version is too low. By default false.
                property bool AllowExtensions;

                //! If true, explicit binding slots are enabled. By default false.
                property bool ExplicitBinding;

                //! If true, commentaries are preserved for each statement. By default false.
                property bool PreserveComments;

                //! If true, intrinsics are prefered to be implemented as wrappers (instead of inlining). By default false.
                property bool PreferWrappers;

                //! If true, array initializations will be unrolled. By default false.
                property bool UnrollArrayInitializers;

                //! If true, matrices have row-major alignment. Otherwise the matrices have column-major alignment. By default false.
                property bool RowMajorAlignment;

                //! If true, code obfuscation is performed. By default false.
                property bool Obfuscate;

                //! If true, the AST (Abstract Syntax Tree) will be written to the log output. By default false.
                property bool ShowAST;

                //! If true, the timings of the different compilation processes are written to the log output. By default false.
                property bool ShowTimes;

        };

        //! Name mangling descriptor structure for shader input/output variables (also referred to as "varyings"), temporary variables, and reserved keywords.
        ref class OutputNameMangling
        {

            public:

                OutputNameMangling()
                {
                    InputPrefix         = gcnew String("xsv_");
                    OutputPrefix        = gcnew String("xsv_");
                    ReservedWordPrefix  = gcnew String("xsr_");
                    TemporaryPrefix     = gcnew String("xst_");
                    UseAlwaysSemantics  = false;
                }

                //! Name mangling prefix for shader input variables. By default "xsv_".
                property String^    InputPrefix;

                //! Name mangling prefix for shader output variables. By default "xsv_".
                property String^    OutputPrefix;

                /**
                \brief Name mangling prefix for reserved words (such as "texture", "main", "sin" etc.). By default "xsr_".
                \remarks This must not be equal to any of the other prefixes and it must not be empty.
                */
                property String^    ReservedWordPrefix;

                /**
                \brief Name mangling prefix for temporary variables. By default "xst_".
                \remarks This must not be equal to any of the other prefixes and it must not be empty.
                */
                property String^    TemporaryPrefix;

                /**
                If true, shader input/output variables are always renamed to their semantics,
                even for vertex input and fragment output. Otherwise, their original identifiers are used. By default false.
                */
                property bool       UseAlwaysSemantics;

        };

        //! Shader source include handler interface.
        ref class SourceIncludeHandler abstract
        {

            public:
                
                /**
                \brief Returns an input stream for the specified filename.
                \param[in] includeName Specifies the include filename.
                \param[in] useSearchPathsFirst Specifies whether to first use the search paths to find the file.
                \return Content of the new input file.
                */
                virtual String^ Include(String^ filename, bool useSearchPathsFirst) = 0;

        };

        //! Shader input descriptor structure.
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
                    IncludeHandler      = nullptr;
                }

                //! Specifies the filename of the input shader code. This is an optional attribute, and only a hint to the compiler.
                property String^                        Filename;

                //! Specifies the input source code stream.
                property String^                        SourceCode;

                //! Specifies the input shader version (e.g. InputShaderVersion.HLSL5 for "HLSL 5"). By default InputShaderVersion.HLSL5.
                property InputShaderVersion             ShaderVersion;
    
                //! Specifies the target shader (Vertex, Fragment etc.). By default ShaderTarget::Undefined.
                property ShaderTarget                   Target;

                //! Specifies the HLSL shader entry point. By default "main".
                property String^                        EntryPoint;

                /**
                \brief Specifies the secondary HLSL shader entry point.
                \remarks This is only used for a Tessellation-Control Shader (alias Hull Shader) entry point,
                when a Tessellation-Control Shader (alias Domain Shader) is the output target.
                This is required to translate all Tessellation-Control attributes (i.e. "partitioning" and "outputtopology")
                to the Tessellation-Evaluation output shader. If this is empty, the default values for these attributes are used.
                */
                property String^                        SecondaryEntryPoint;

                /**
                \brief Optional handler to handle '#include'-directives. By default null.
                \remarks If this is null, the default include handler will be used, which will include files with the STL input file streams.
                */
                property SourceIncludeHandler^          IncludeHandler;

        };

        //! Vertex shader semantic (or rather attribute) layout structure.
        ref class VertexSemantic
        {

            public:

                VertexSemantic()
                {
                    Semantic = nullptr;
                    Location = 0;
                }

                //! Specifies the shader semantic (or rather attribute).
                property String^    Semantic;

                //! Specifies the binding location.
                property int        Location;

        };

        //! Shader output descriptor structure.
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

                //! Specifies the filename of the output shader code. This is an optional attribute, and only a hint to the compiler.
                property String^                                        Filename;

                //! Specifies the output source code stream. This will contain the output code. This must not be null when passed to the "CompileShader" function!
                property String^                                        SourceCode;

                //! Specifies the output shader version. By default OutputShaderVersion::GLSL (to auto-detect minimum required version).
                property OutputShaderVersion                            ShaderVersion;
                
                //! Optional list of vertex semantic layouts, to bind a vertex attribute (semantic name) to a location index (only used when 'explicitBinding' is true).
                property Collections::Generic::List<VertexSemantic^>^   VertexSemantics;

                //! Additional options to configure the code generation.
                property OutputOptions^                                 Options;

                //! Output code formatting descriptor.
                property OutputFormatting^                              Formatting;
    
                //! Specifies the options for name mangling.
                property OutputNameMangling^                            NameMangling;

        };

        //! Compiler report class.
        ref class Report
        {

            public:

                //! Report types enumeration.
                enum class Types
                {
                    Info,       //!< Standard information.
                    Warning,    //!< Warning message.
                    Error       //!< Error message.
                };
                
                Report(Types type, String^ message)
                {
                    Type    = type;
                    Message = message;
                }

                Report(Types type, String^ message, String^ context)
                {
                    Type    = type;
                    Context = context;
                    Message = message;
                }

                Report(Types type, String^ message, String^ line, String^ marker)
                {
                    Type    = type;
                    Message = message;
                    Line    = line;
                    Marker  = marker;
                }

                Report(Types type, String^ message, String^ line, String^ marker, String^ context)
                {
                    Type    = type;
                    Context = context;
                    Message = message;
                    Line    = line;
                    Marker  = marker;
                }

                Report(Types type, String^ message, String^ line, String^ marker, String^ context, Collections::Generic::List<String^>^ hints)
                {
                    Type    = type;
                    Context = context;
                    Message = message;
                    Line    = line;
                    Marker  = marker;
                    Hints   = hints;
                }

                //! Specifies the type of this report.
                property Types                                  Type;

                //! Returns the context description string (e.g. a function name where the report occured). This may also be empty.
                property String^                                Context;

                //! Returns the message string.
                property String^                                Message;

                //! Returns the line string where the report occured. This line never has new-line characters at its end.
                property String^                                Line;

                //! Returns the line marker string to highlight the area where the report occured.
                property String^                                Marker;

                //! Returns the list of optional hints of the report.
                property Collections::Generic::List<String^>^   Hints;

                /**
                \brief Returns true if this report has a line with line marker.
                \see Line
                \see Marker
                */
                property bool HasLine
                {
                    bool get()
                    {
                        return (Line != nullptr && !Line->Empty);
                    }
                }

        };

        //! Log base class.
        ref class Log abstract
        {

            public:

                //! Submits the specified report with the current indentation.
                virtual void SubmitReport(Report^ report, String^ indent) = 0;

                //! Prints all submitted reports to the standard output.
                void PrintAll()
                {
                    PrintAll(true, true);
                }

                //! Prints all submitted reports to the standard output.
                virtual void PrintAll(bool verbose, bool warnings) = 0;

        };

    private:

        //! Standard output log (uses standard output to submit a report).
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
                            static_cast<Xsc::Report::Types>(report->Type),
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
                        stdLog_->SumitReport(r);
                    }
                }

                void PrintAll(bool verbose, bool warnings) override
                {
                    stdLog_->PrintAll(verbose, warnings);
                }

            private:

                Xsc::StdLog* stdLog_ = nullptr;

        };

    public:

        XscCompiler()
        {
            StandardLog = gcnew StdLog();
        }

        /**
        \brief Cross compiles the shader code from the specified input stream into the specified output shader code.
        \param[in] inputDesc Input shader code descriptor.
        \param[in] outputDesc Output shader code descriptor.
        \param[in] log Optional output log. Inherit from the "Log" class interface. By default null.
        \param[out] reflectionData Optional pointer to a code reflection data structure. By default null.
        \return True if the code has been translated successfully.
        \throw ArgumentNullException If either the input or output streams are null.
        \see ShaderInput
        \see ShaderOutput
        \see Log
        \see ReflectionData
        */
        bool CompileShader(ShaderInput^ inputDesc, ShaderOutput^ outputDesc, Log^ log);

        //! \see CompileShader(ShaderInput^, ShaderOutput^, Log^)
        bool CompileShader(ShaderInput^ inputDesc, ShaderOutput^ outputDesc)
        {
            return CompileShader(inputDesc, outputDesc, nullptr);
        }

        //! Returns the compiler version.
        property String ^ Version
        {
            String ^ get()
            {
                return gcnew String(XSC_VERSION_STRING);
            }
        }

        //! Returns a dictionary of all supported GLSL extensions with their minimum required version number.
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

        //! Returns the standard log.
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

        void SumitReport(const Xsc::Report& report) override
        {
            if (handler_)
            {
                /* Copy hints into managed hints */
                auto managedHints = gcnew Collections::Generic::List<String^>();

                for (const auto& hint : report.GetHints())
                    managedHints->Add(gcnew String(hint.c_str()));

                /* Copy report into managed report */
                auto managedReport = gcnew XscCompiler::Report(
                    static_cast<XscCompiler::Report::Types>(report.Type()),
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

bool XscCompiler::CompileShader(ShaderInput^ inputDesc, ShaderOutput^ outputDesc, Log^ log)
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
    in.includeHandler       = (&includeHandler);

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
    out.options.warnings                = outputDesc->Options->Warnings;
    out.options.optimize                = outputDesc->Options->Optimize;
    out.options.preprocessOnly          = outputDesc->Options->PreprocessOnly;
    out.options.validateOnly            = outputDesc->Options->ValidateOnly;
    out.options.allowExtensions         = outputDesc->Options->AllowExtensions;
    out.options.explicitBinding         = outputDesc->Options->ExplicitBinding;
    out.options.preserveComments        = outputDesc->Options->PreserveComments;
    out.options.preferWrappers          = outputDesc->Options->PreferWrappers;
    out.options.unrollArrayInitializers = outputDesc->Options->UnrollArrayInitializers;
    out.options.rowMajorAlignment       = outputDesc->Options->RowMajorAlignment;
    out.options.obfuscate               = outputDesc->Options->Obfuscate;
    out.options.showAST                 = outputDesc->Options->ShowAST;
    out.options.showTimes               = outputDesc->Options->ShowTimes;

    /* Copy output formatting descriptor */
    out.formatting.indent               = ToStdString(outputDesc->Formatting->Indent);
    out.formatting.blanks               = outputDesc->Formatting->Blanks;
    out.formatting.lineMarks            = outputDesc->Formatting->LineMarks;
    out.formatting.compactWrappers      = outputDesc->Formatting->CompactWrappers;
    out.formatting.alwaysBracedScopes   = outputDesc->Formatting->AlwaysBracedScopes;
    out.formatting.newLineOpenScope     = outputDesc->Formatting->NewLineOpenScope;
    out.formatting.lineSeparation       = outputDesc->Formatting->LineSeparation;

    /* Copy output name mangling descriptor */
    out.nameMangling.inputPrefix        = ToStdString(outputDesc->NameMangling->InputPrefix);
    out.nameMangling.outputPrefix       = ToStdString(outputDesc->NameMangling->OutputPrefix);
    out.nameMangling.reservedWordPrefix = ToStdString(outputDesc->NameMangling->ReservedWordPrefix);
    out.nameMangling.temporaryPrefix    = ToStdString(outputDesc->NameMangling->TemporaryPrefix);
    out.nameMangling.useAlwaysSemantics = outputDesc->NameMangling->UseAlwaysSemantics;

    /* Compile shader */
    bool result = false;
    LogCSharp logCSharp(log);

    try
    {
        result = Xsc::CompileShader(in, out, (log != nullptr ? &logCSharp : nullptr));
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
        /*if (reflectionData != NULL)
            CopyReflection(g_compilerContext.reflection, reflectionData);*/
    }

    return result;
}




// ================================================================================
