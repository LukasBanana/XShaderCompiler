/*
 * HLSL Offline Translator main file
 * 
 * This file is part of the "HLSL Translator" (Copyright (c) 2014 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <HT/Translator.h>
#include <HT/ConsoleManip.h>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <vector>

#ifdef _WIN32
#include <conio.h>
#endif


using namespace HTLib;
using namespace HTLib::ConsoleManip;

/* --- Globals --- */

struct Config
{
    std::string entry;
    std::string target;
    std::string shaderIn    = "HLSL5";
    std::string shaderOut   = "GLSL330";
    std::string output;
    Options     options;
};

static Config g_config;


/* --- Functions --- */

static void PrintLines(const std::vector<std::string>& lines)
{
    for (const auto& line : lines)
        std::cout << line << std::endl;
}

static void ShowHint()
{
    std::cout << "no input : enter \"HLSLOfflineTranslator help\"" << std::endl;
}

static void ShowHelp()
{
    PrintLines(
        {
            "Usage:",
            "  HTLibCmd (OPTION+ FILE)+",
            "Options:",
            "  -entry ENTRY ........... HLSL shader entry point",
            "  -target TARGET ......... Shader target; valid values:",
            "    vertex, fragment, geometry, tess-control, tess-evaluation, compute",
            "  -shaderin VERSION ...... HLSL version; default is HLSL5; valid values:",
            "    HLSL3, HLSL4, HLSL5",
            "  -shaderout VERSION ..... GLSL version; default is GLSL330; valid values:",
            "    GLSL110, GLSL120, GLSL130, GLSL140, GLSL150, GLSL330,",
            "    GLSL400, GLSL410, GLSL420, GLSL430, GLSL440, GLSL450",
            "  -indent INDENT ......... Code indentation string; by default 4 spaces",
            "  -prefix PREFIX ......... Prefix for local variables (use \"<none>\" to disable); by default '_'",
            "  -output FILE ........... GLSL output file; default is '<FILE>.<ENTRY>.glsl'",
            "  -warn [on|off] ......... Enables/disables all warnings; by default off",
            "  -blanks [on|off] ....... Enables/disables generation of blank lines between declarations; by default on",
            "  -line-marks [on|off] ... Enables/disables generation of line marks (e.g. '#line 30'); by default off",
            "  -dump-ast [on|off] ..... Enables/disables debug output for the entire abstract syntax tree (AST); by default off",
            "  -pponly [on|off] ....... Enables/disables to only preprocess source code; by default off",
            "  -comments [on|off] ..... Enables/disables commentaries output kept from the sources; by default on",
            "  --help, help, -h ....... Prints this help reference",
            "  --version, -v .......... Prints the version information",
            "  --pause ................ Waits for user input after the translation process",
            "Example:",
            "  HTLibCmd -entry VS -target vertex Example.hlsl -entry PS -target fragment Example.hlsl",
            "   --> Example.vertex.glsl; Example.fragment.glsl ",
        }
    );
}

static void ShowVersion()
{
    ScopedColor highlight(std::cout, ColorFlags::Green | ColorFlags::Blue);
    std::cout << "HLSL Translator ( Version " << HTLIB_VERSION_STRING << " )" << std::endl;
    std::cout << "Copyright (c) 2014-2016 by Lukas Hermanns" << std::endl;
    std::cout << "3-Clause BSD License" << std::endl;
}

static ShaderTarget TargetFromString(const std::string& target)
{
    if (target.empty())
        return ShaderTarget::CommonShader;
    if (target == "vertex")
        return ShaderTarget::GLSLVertexShader;
    if (target == "fragment")
        return ShaderTarget::GLSLFragmentShader;
    if (target == "geometry")
        return ShaderTarget::GLSLGeometryShader;
    if (target == "tess-control")
        return ShaderTarget::GLSLTessControlShader;
    if (target == "tess-evaluation")
        return ShaderTarget::GLSLTessEvaluationShader;
    if (target == "compute")
        return ShaderTarget::GLSLComputeShader;

    throw std::runtime_error("invalid shader target \"" + target + "\"");
    return ShaderTarget::GLSLVertexShader;
}

static InputShaderVersion InputVersionFromString(const std::string& version)
{
    #define CHECK_IN_VER(n) if (version == #n) return InputShaderVersion::n

    CHECK_IN_VER(HLSL3);
    CHECK_IN_VER(HLSL4);
    CHECK_IN_VER(HLSL5);

    #undef CHECK_IN_VER

    throw std::runtime_error("invalid input shader version \"" + version + "\"");
    return InputShaderVersion::HLSL5;
}

static OutputShaderVersion OutputVersionFromString(const std::string& version)
{
    #define CHECK_OUT_VER(n) if (version == #n) return OutputShaderVersion::n

    CHECK_OUT_VER(GLSL110);
    CHECK_OUT_VER(GLSL120);
    CHECK_OUT_VER(GLSL130);
    CHECK_OUT_VER(GLSL140);
    CHECK_OUT_VER(GLSL150);
    CHECK_OUT_VER(GLSL330);
    CHECK_OUT_VER(GLSL400);
    CHECK_OUT_VER(GLSL410);
    CHECK_OUT_VER(GLSL420);
    CHECK_OUT_VER(GLSL430);
    CHECK_OUT_VER(GLSL440);
    CHECK_OUT_VER(GLSL450);

    #undef CHECK_OUT_VER

    throw std::runtime_error("invalid output shader version \"" + version + "\"");
    return OutputShaderVersion::GLSL110;
}

static std::string NextArg(int& i, int argc, char** argv, const std::string& flag)
{
    if (i + 1 >= argc)
        throw std::runtime_error("missing next argument after flag \"" + flag + "\"");
    return argv[++i];
}

static bool BoolArg(int& i, int argc, char** argv, const std::string& flag)
{
    /* Set default value to true (when used, boolean flags for command lines enable per default) */
    bool value = true;

    /* Check if the there is a next argument "on" or "off" */
    if (i + 1 < argc)
    {
        /* Fetch next argument from 'argv' array */
        auto arg = std::string(argv[i + 1]);

        if (arg == "on")
            value = true;
        else if (arg == "off")
            value = false;
        else
            return true;

        /* Accept this argument by increasing index of 'argv' array */
        ++i;
    }

    return value;
}

static std::string ExtractFilename(const std::string& filename)
{
    auto pos = filename.find_last_of('.');
    return pos == std::string::npos ? filename : filename.substr(0, pos);
}

static void Translate(const std::string& filename)
{
    ShaderInput inputDesc;
    ShaderOutput outputDesc;

    if (g_config.output.empty())
    {
        /* Set default output filename */
        g_config.output = ExtractFilename(filename);
        if (!g_config.target.empty())
            g_config.output += "." + g_config.target;
        g_config.output += ".glsl";
    }

    if (g_config.options.prefix == "<none>")
        g_config.options.prefix.clear();

    /* Ignore entry and target if one of them are empty */
    if (g_config.entry.empty() || g_config.target.empty())
    {
        g_config.entry.clear();
        g_config.target.clear();
    }

    /* Translate HLSL file into GLSL */
    std::cout << "translate from " << filename << " to " << g_config.output << std::endl;

    std::ofstream outputStream(g_config.output);

    inputDesc.sourceCode        = std::make_shared<std::ifstream>(filename);
    inputDesc.shaderVersion     = InputVersionFromString(g_config.shaderIn);
    inputDesc.entryPoint        = g_config.entry;
    inputDesc.shaderTarget      = TargetFromString(g_config.target);

    outputDesc.sourceCode       = &outputStream;
    outputDesc.shaderVersion    = OutputVersionFromString(g_config.shaderOut);
    outputDesc.options          = g_config.options;

    try
    {
        StdLog log;

        auto result = TranslateHLSLtoGLSL(inputDesc, outputDesc, &log);

        log.PrintAll();

        if (result)
            std::cout << "translation successful" << std::endl;
    }
    catch (const std::exception& err)
    {
        std::cerr << err.what() << std::endl;
    }
}


/* --- Main function --- */

int main(int argc, char** argv)
{
    int     translationCounter  = 0;
    bool    showHelp            = false,
            showVersion         = false,
            pauseApp            = false;

    /* Parse program arguments */
    for (int i = 1; i < argc; ++i)
    {
        try
        {
            auto arg = std::string(argv[i]);

            if (arg == "help" || arg == "--help" || arg == "-h")
                showHelp = true;
            else if (arg == "--version" || arg == "-v")
                showVersion = true;
            else if (arg == "--pause")
                pauseApp = true;
            else if (arg == "-warn")
                g_config.options.warnings = BoolArg(i, argc, argv, arg);
            else if (arg == "-blanks")
                g_config.options.blanks = BoolArg(i, argc, argv, arg);
            else if (arg == "-line-marks")
                g_config.options.lineMarks = BoolArg(i, argc, argv, arg);
            else if (arg == "-dump-ast")
                g_config.options.dumpAST = BoolArg(i, argc, argv, arg);
            else if (arg == "-pponly")
                g_config.options.preprocessOnly = BoolArg(i, argc, argv, arg);
            else if (arg == "-comments")
                g_config.options.keepComments = BoolArg(i, argc, argv, arg);
            else if (arg == "-entry")
                g_config.entry = NextArg(i, argc, argv, arg);
            else if (arg == "-target")
                g_config.target = NextArg(i, argc, argv, arg);
            else if (arg == "-shaderin")
                g_config.shaderIn = NextArg(i, argc, argv, arg);
            else if (arg == "-shaderout")
                g_config.shaderOut = NextArg(i, argc, argv, arg);
            else if (arg == "-indent")
                g_config.options.indent = NextArg(i, argc, argv, arg);
            else if (arg == "-prefix")
                g_config.options.prefix = NextArg(i, argc, argv, arg);
            else if (arg == "-output")
                g_config.output = NextArg(i, argc, argv, arg);
            else
            {
                /* Translate input code */
                Translate(arg);
                ++translationCounter;

                /* Reset translation options */
                g_config.output.clear();
                g_config.target.clear();
                g_config.entry.clear();
            }
        }
        catch (const std::exception& err)
        {
            std::cerr << err.what() << std::endl;
            return 0;
        }
    }

    /* Evaluate arguemnts */
    if (showHelp)
        ShowHelp();
    if (showVersion)
        ShowVersion();

    if (translationCounter == 0 && !showHelp && !showVersion)
        ShowHint();

    #ifdef _WIN32
    if (pauseApp)
    {
        std::cout << "press any key to continue ...";
        int i = _getch();
        std::cout << std::endl;
    }
    #endif

    return 0;
}
