/*
 * DebuggerView.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "DebuggerView.h"
#include <Xsc/Version.h>
#include <sstream>
#include <memory>
#include <vector>
#include <fstream>
#include <cctype>
#include <wx/menu.h>
#include <wx/mimetype.h>
#include <wx/msgdlg.h>


namespace Xsc
{


static long DebuggerViewStyle()
{
    return (wxSYSTEM_MENU | wxCAPTION | wxCLIP_CHILDREN | wxMINIMIZE_BOX | wxMAXIMIZE_BOX | wxRESIZE_BORDER | wxCLOSE_BOX);
}

DebuggerView::DebuggerView(const wxPoint& pos, const wxSize& size) :
    wxFrame { nullptr, wxID_ANY, "Xsc Debugger", pos, size, DebuggerViewStyle() }
{
    #ifdef _WIN32
    SetIcon(wxICON(APP_ICON));
    #endif

    CreateLayout();

    Centre();

    Bind(wxEVT_CLOSE_WINDOW, &DebuggerView::OnClose, this);

    /* Initialize descriptor structure */
    shaderInput_.shaderTarget = ShaderTarget::VertexShader;

    inputSourceView_->SetFocus();
}

static const std::string settingsFilename("XscDebuggerSettings");
static const std::string codeFilename("XscDebuggerCode");

static void RemoveCharFromString(std::string& s, char c)
{
    for (auto it = s.begin(); it != s.end();)
    {
        if (*it == c)
            it = s.erase(it);
        else
            ++it;
    }
}

void DebuggerView::SaveSettings()
{
    std::ofstream settingsFile(settingsFilename);
    if (settingsFile.good())
    {
        settingsFile << propGrid_->SaveEditableState().ToStdString() << std::endl;
        settingsFile << propGrid_->GetPropertyValueAsString("entry").ToStdString() << std::endl;
    }

    std::ofstream codeFile(codeFilename);
    if (codeFile.good())
    {
        auto sourceCode = inputSourceView_->GetText().ToStdString();
        RemoveCharFromString(sourceCode, '\r');
        codeFile << sourceCode;
    }
}

void DebuggerView::LoadSettings()
{
    std::ifstream settingsFile(settingsFilename);
    if (settingsFile.good())
    {
        std::string state;
        std::getline(settingsFile, state);
        propGrid_->RestoreEditableState(state);

        std::getline(settingsFile, state);
        propGrid_->SetPropertyValueString("entry", state);
        shaderInput_.entryPoint = state;
    }

    std::ifstream codeFile(codeFilename);
    if (codeFile.good())
    {
        inputSourceView_->SetText(
            std::string(
                ( std::istreambuf_iterator<char>(codeFile) ),
                ( std::istreambuf_iterator<char>() )
            )
        );
    }
}


/*
 * ======= Private: =======
 */

void DebuggerView::CreateLayout()
{
    mainSplitter_ = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_LIVE_UPDATE);

    CreateLayoutPropertyGrid();
    CreateLayoutSubSplitter();

    mainSplitter_->SplitVertically(propGrid_, subSplitter_, 300);

    CreateLayoutStatusBar();
    CreateLayoutMenuBar();
}

void DebuggerView::CreateLayoutPropertyGrid()
{
    propGrid_ = new wxPropertyGrid(mainSplitter_, wxID_ANY, wxDefaultPosition, wxSize(200, 600), wxPG_SPLITTER_AUTO_CENTER);

    CreateLayoutPropertyGridShaderInput(*propGrid_);
    CreateLayoutPropertyGridShaderOutput(*propGrid_);
    CreateLayoutPropertyGridOptions(*propGrid_);
    CreateLayoutPropertyGridFormatting(*propGrid_);
    CreateLayoutPropertyGridNameMangling(*propGrid_);

    propGrid_->Bind(wxEVT_PG_CHANGED, &DebuggerView::OnPropertyGridChange, this);
}

void DebuggerView::CreateLayoutPropertyGridShaderInput(wxPropertyGrid& pg)
{
    pg.Append(new wxPropertyCategory("Shader Input"));

    wxPGChoices choices0;
    {
        choices0.Add("Cg");
        choices0.Add("HLSL3");
        choices0.Add("HLSL4");
        choices0.Add("HLSL5");
        choices0.Add("HLSL6");

        choices0.Add("GLSL");
        choices0.Add("ESSL");
        choices0.Add("VKSL");
    }
    pg.Append(new wxEnumProperty("Shader Version", "inputVersion", choices0, 3));

    wxPGChoices choices1;
    {
        choices1.Add("Vertex Shader");
        choices1.Add("Tessellation-Control Shader");
        choices1.Add("Tessellation-Evaluation Shader");
        choices1.Add("Geometry Shader");
        choices1.Add("Fragment Shader");
        choices1.Add("Compute Shader");
    }
    pg.Append(new wxEnumProperty("Shader Target", "target", choices1));

    pg.Append(new wxStringProperty("Entry Point", "entry", ""));
    pg.Append(new wxStringProperty("Secondary Entry Point", "secondaryEntry", ""));
    pg.Append(new wxBoolProperty("Enable Warnings", "warnings"));

    pg.Append(new wxBoolProperty("Language Extensions", "langExtensions"))
    #ifndef XSC_ENABLE_LANGUAGE_EXT
        ->Enable(false);
    #endif
    ;
}

void DebuggerView::CreateLayoutPropertyGridShaderOutput(wxPropertyGrid& pg)
{
    pg.Append(new wxPropertyCategory("Shader Output"));

    wxPGChoices choices0;
    {
        choices0.Add("GLSL (Auto-Detect)");
        choices0.Add("GLSL110");
        choices0.Add("GLSL120");
        choices0.Add("GLSL130");
        choices0.Add("GLSL140");
        choices0.Add("GLSL150");
        choices0.Add("GLSL330");
        choices0.Add("GLSL400");
        choices0.Add("GLSL410");
        choices0.Add("GLSL420");
        choices0.Add("GLSL430");
        choices0.Add("GLSL440");
        choices0.Add("GLSL450");

        choices0.Add("ESSL (Auto-Detect)");
        choices0.Add("ESSL100");
        choices0.Add("ESSL300");
        choices0.Add("ESSL310");
        choices0.Add("ESSL320");

        choices0.Add("VKSL (Auto-Detect)");
        choices0.Add("VKSL450");
    }
    pg.Append(new wxEnumProperty("Shader Version", "outputVersion", choices0));
}

void DebuggerView::CreateLayoutPropertyGridOptions(wxPropertyGrid& pg)
{
    pg.Append(new wxPropertyCategory("Options"));

    pg.Append(new wxBoolProperty("Allow Extensions", "extensions"));
    pg.Append(new wxBoolProperty("Auto. Binding", "autoBinding"));
    pg.Append(new wxIntProperty("Auto. Binding Start Slot", "autoBindingStartSlot"));
    pg.Append(new wxBoolProperty("Explicit Binding", "binding"));
    pg.Append(new wxBoolProperty("Obfuscate", "obfuscate"));
    pg.Append(new wxBoolProperty("Optimize", "optimize"));
    pg.Append(new wxBoolProperty("Prefer Wrappers", "wrappers"));
    pg.Append(new wxBoolProperty("Preprocess Only", "preprocess"));
    pg.Append(new wxBoolProperty("Preserve Comments", "comments"));
    pg.Append(new wxBoolProperty("Row-Major Alignment", "rowMajor"));
    pg.Append(new wxBoolProperty("Separate Samplers", "separateSamplers", true));
    pg.Append(new wxBoolProperty("Separate Shaders", "separateShaders"));
    pg.Append(new wxBoolProperty("Show AST", "showAST"));
    pg.Append(new wxBoolProperty("Unroll Array Initializers", "unrollInitializers"));
    pg.Append(new wxBoolProperty("Validate Only", "validate"));
    pg.Append(new wxBoolProperty("Write Generator Header", "generatorHeader"));
}

void DebuggerView::CreateLayoutPropertyGridFormatting(wxPropertyGrid& pg)
{
    pg.Append(new wxPropertyCategory("Formatting"));

    pg.Append(new wxStringProperty("Indentation", "indent", "    "));
    pg.Append(new wxBoolProperty("Always Braced Scopes", "alwaysBracedScopes"));
    pg.Append(new wxBoolProperty("Blanks", "blanks", true));
    pg.Append(new wxBoolProperty("Compact Wrappers", "compactWrappers", true));
    pg.Append(new wxBoolProperty("Line Marks", "lineMarks"));
    pg.Append(new wxBoolProperty("Line Separation", "lineSeparation", true));
    pg.Append(new wxBoolProperty("New-Line Open Scope", "newLineOpenScope", true));
}

void DebuggerView::CreateLayoutPropertyGridNameMangling(wxPropertyGrid& pg)
{
    pg.Append(new wxPropertyCategory("Name Mangling"));

    pg.Append(new wxStringProperty("Input Prefix", "prefixInput", "xsv_"));
    pg.Append(new wxStringProperty("Output Prefix", "prefixOutput", "xsv_"));
    pg.Append(new wxStringProperty("Reserved Word Prefix", "prefixReserved", "xsr_"));
    pg.Append(new wxStringProperty("Temporary Prefix", "prefixTemp", "xst_"));
    pg.Append(new wxStringProperty("Namespace Prefix", "prefixNamespace", "xsn_"));
    pg.Append(new wxBoolProperty("Use Always Semantics", "useAlwaysSemantics", false));
    pg.Append(new wxBoolProperty("Rename Buffer Fields", "renameBufferFields", false));
}

void DebuggerView::CreateLayoutSubSplitter()
{
    subSplitter_ = new wxSplitterWindow(mainSplitter_, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_LIVE_UPDATE);

    CreateLayoutReportView();
    CreateLayoutSourceSplitter();

    subSplitter_->SplitHorizontally(sourceSplitter_, reportView_, 600);
    subSplitter_->SetSashGravity(1.0);
}

void DebuggerView::CreateLayoutReportView()
{
    reportView_ = new ReportView(subSplitter_, wxDefaultPosition, wxSize(400, 100));
}

void DebuggerView::CreateLayoutSourceSplitter()
{
    sourceSplitter_ = new wxSplitterWindow(subSplitter_, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_LIVE_UPDATE);

    CreateLayoutInputSourceView();
    CreateLayoutOutputSourceView();

    sourceSplitter_->SplitVertically(inputSourceView_, outputSourceView_);
    sourceSplitter_->SetSashGravity(0.5);
}

void DebuggerView::CreateLayoutInputSourceView()
{
    inputSourceView_ = new SourceView(sourceSplitter_, wxDefaultPosition, wxSize(100, 600));
    inputSourceView_->SetLanguage(SourceViewLanguage::HLSL);
    inputSourceView_->SetCharEnterCallback(std::bind(&DebuggerView::OnInputSourceCharEnter, this, std::placeholders::_1));
    inputSourceView_->SetMoveCursorCallback(
        [&](int line, int column)
        {
            SetStatusLine(line);
            SetStatusColumn(column);
        }
    );
}

void DebuggerView::CreateLayoutOutputSourceView()
{
    outputSourceView_ = new SourceView(sourceSplitter_, wxDefaultPosition, wxSize(100, 600));
    outputSourceView_->SetLanguage(SourceViewLanguage::GLSL);
    //outputSourceView_->SetReadOnly(true);
}

void DebuggerView::CreateLayoutStatusBar()
{
    statusBar_ = CreateStatusBar(3);

    int widths[] = { 200, 70, 70 };
    statusBar_->SetStatusWidths(3, widths);

    SetStatusReady(true);
    SetStatusLine(1);
    SetStatusColumn(1);
}

void DebuggerView::CreateLayoutMenuBar()
{
    menuBar_ = new wxMenuBar();

    auto menuDebugger = new wxMenu();
    {
        menuDebugger->Append(wxID_ABOUT, "&About");
        menuDebugger->Append(wxID_HELP, "&Help");
        menuDebugger->AppendSeparator();
        menuDebugger->Append(wxID_EXIT, "&Quit");
    }
    menuBar_->Append(menuDebugger, "&XscDebugger");

    Bind(wxEVT_MENU, &DebuggerView::OnAbout, this, wxID_ABOUT);
    Bind(wxEVT_MENU, &DebuggerView::OnHelp, this, wxID_HELP);
    Bind(wxEVT_MENU, &DebuggerView::OnQuit, this, wxID_EXIT);

    SetMenuBar(menuBar_);
}

void DebuggerView::OnPropertyGridChange(wxPropertyGridEvent& event)
{
    auto p = event.GetProperty();
    auto name = p->GetName();

    auto ValueStr = [p]()
    {
        return p->GetValueAsString().ToStdString();
    };

    auto ValueInt = [p]()
    {
        return p->GetValue().GetInteger();
    };

    auto ValueBool = [p]()
    {
        return p->GetValue().GetBool();
    };

    auto GetInputVersion = [](int idx) -> InputShaderVersion
    {
        using T = InputShaderVersion;

        static const T versions[] =
        {
            T::Cg,

            T::HLSL3,
            T::HLSL4,
            T::HLSL5,
            T::HLSL6,

            T::GLSL,
            T::ESSL,
            T::VKSL,
        };

        return (idx >= 0 && idx <= 7 ? versions[idx] : T::HLSL5);
    };

    auto GetOutputVersion = [](int idx) -> OutputShaderVersion
    {
        using T = OutputShaderVersion;

        static const T versions[] =
        {
            T::GLSL,
            T::GLSL110,
            T::GLSL120,
            T::GLSL130,
            T::GLSL140,
            T::GLSL150,
            T::GLSL330,
            T::GLSL400,
            T::GLSL410,
            T::GLSL420,
            T::GLSL430,
            T::GLSL440,
            T::GLSL450,

            T::ESSL,
            T::ESSL100,
            T::ESSL300,
            T::ESSL310,
            T::ESSL320,

            T::VKSL,
            T::VKSL450,
        };

        return (idx >= 0 && idx < 20 ? versions[idx] : T::GLSL);
    };

    /* --- Main options --- */
    if (name == "entry")
        shaderInput_.entryPoint = ValueStr();
    else if (name == "inputVersion")
        shaderInput_.shaderVersion = GetInputVersion(ValueInt());
    else if (name == "secondaryEntry")
        shaderInput_.secondaryEntryPoint = ValueStr();
    else if (name == "warnings")
        shaderInput_.warnings = (ValueBool() ? Warnings::All : 0);
    else if (name == "target")
        shaderInput_.shaderTarget = static_cast<ShaderTarget>(static_cast<long>(ShaderTarget::VertexShader) + ValueInt());
    else if (name == "outputVersion")
        shaderOutput_.shaderVersion = GetOutputVersion(ValueInt());
    else if (name == "langExtensions")
        shaderInput_.extensions = (ValueBool() ? Extensions::All : 0);

    /* --- Common options --- */
    else if (name == "indent")
        shaderOutput_.formatting.indent = ValueStr();
    else if (name == "extensions")
        shaderOutput_.options.allowExtensions = ValueBool();
    else if (name == "binding")
        shaderOutput_.options.explicitBinding = ValueBool();
    else if (name == "optimize")
        shaderOutput_.options.optimize = ValueBool();
    else if (name == "wrappers")
        shaderOutput_.options.preferWrappers = ValueBool();
    else if (name == "preprocess")
        shaderOutput_.options.preprocessOnly = ValueBool();
    else if (name == "comments")
        shaderOutput_.options.preserveComments = ValueBool();
    else if (name == "unrollInitializers")
        shaderOutput_.options.unrollArrayInitializers = ValueBool();
    else if (name == "rowMajor")
        shaderOutput_.options.rowMajorAlignment = ValueBool();
    else if (name == "obfuscate")
        shaderOutput_.options.obfuscate = ValueBool();
    else if (name == "showAST")
        shaderOutput_.options.showAST = ValueBool();
    else if (name == "autoBinding")
        shaderOutput_.options.autoBinding = ValueBool();
    else if (name == "autoBindingStartSlot")
        shaderOutput_.options.autoBindingStartSlot = ValueInt();
    else if (name == "separateShaders")
        shaderOutput_.options.separateShaders = ValueBool();
    else if (name == "separateSamplers")
        shaderOutput_.options.separateSamplers = ValueBool();
    else if (name == "validate")
        shaderOutput_.options.validateOnly = ValueBool();
    else if (name == "generatorHeader")
        shaderOutput_.options.writeGeneratorHeader = ValueBool();

    /* --- Formatting --- */
    else if (name == "blanks")
        shaderOutput_.formatting.blanks = ValueBool();
    else if (name == "lineMarks")
        shaderOutput_.formatting.lineMarks = ValueBool();
    else if (name == "compactWrappers")
        shaderOutput_.formatting.compactWrappers = ValueBool();
    else if (name == "alwaysBracedScopes")
        shaderOutput_.formatting.alwaysBracedScopes = ValueBool();
    else if (name == "newLineOpenScope")
        shaderOutput_.formatting.newLineOpenScope = ValueBool();
    else if (name == "lineSeparation")
        shaderOutput_.formatting.lineSeparation = ValueBool();

    /* --- Name mangling --- */
    else if (name == "prefixInput")
        shaderOutput_.nameMangling.inputPrefix = ValueStr();
    else if (name == "prefixOutput")
        shaderOutput_.nameMangling.outputPrefix = ValueStr();
    else if (name == "prefixReserved")
        shaderOutput_.nameMangling.reservedWordPrefix = ValueStr();
    else if (name == "prefixTemp")
        shaderOutput_.nameMangling.temporaryPrefix = ValueStr();
    else if (name == "prefixNamespace")
        shaderOutput_.nameMangling.namespacePrefix = ValueStr();
    else if (name == "useAlwaysSemantics")
        shaderOutput_.nameMangling.useAlwaysSemantics = ValueBool();
    else if (name == "renameBufferFields")
        shaderOutput_.nameMangling.renameBufferFields = ValueBool();

    TranslateInputToOutput();
}

void DebuggerView::OnInputSourceCharEnter(char chr)
{
    TranslateInputToOutput();
}

void DebuggerView::OnClose(wxCloseEvent& event)
{
    SaveSettings();
    event.Skip();
}

void DebuggerView::OnAbout(wxCommandEvent& event)
{
    wxMessageBox(
        (
            wxString("XShaderCompiler and XscDebugger\n") +
            wxString("Version ") + wxString(XSC_VERSION_STRING) + wxString("\n\n") +
            wxString("Copyright (c) 2014-2017 by Lukas Hermanns\n\n") +
            wxString("3-Clause BSD License")
        ),
        "About XscDebugger",
        wxICON_INFORMATION | wxOK | wxOK_DEFAULT | wxCENTRE,
        this
    );
}

void DebuggerView::OnHelp(wxCommandEvent& event)
{
    wxMimeTypesManager mimeTypeMngr;
    auto fileType = mimeTypeMngr.GetFileTypeFromExtension("html");
    auto command = fileType->GetOpenCommand("https://github.com/LukasBanana/XShaderCompiler");
    wxExecute(command);
}

void DebuggerView::OnQuit(wxCommandEvent& event)
{
    SaveSettings();
    Close();
}

class DebuggerLog : public Log
{

    public:

        DebuggerLog(ReportView* reportView) :
            reportView_ { reportView }
        {
        }

        void SubmitReport(const Report& report) override
        {
            reportView_->AddReport(report, FullIndent());
        }

    private:

        ReportView* reportView_ = nullptr;

};

void DebuggerView::TranslateInputToOutput()
{
    SetStatusReady(false);

    /* Get input source string */
    auto inputSourceStr = inputSourceView_->GetText().ToStdString();
    RemoveCharFromString(inputSourceStr, '\r');

    /* Initialize input source */
    auto inputSource = std::make_shared<std::stringstream>();
    *inputSource << inputSourceStr;

    shaderInput_.sourceCode = inputSource;
    shaderInput_.filename   = "<unnamed>";

    /* Initialize output source */
    std::stringstream outputSource;
    shaderOutput_.sourceCode = (&outputSource);

    inputSourceView_->ClearAnnotations();
    reportView_->ClearAll();

    try
    {
        /* Compile shader */
        DebuggerLog log(reportView_);
        if (Xsc::CompileShader(shaderInput_, shaderOutput_, &log))
        {
            /* Show output */
            outputSourceView_->SetTextAndRefresh(outputSource.str());
        }

        /* Show annotations */
        for (const auto& err : reportView_->GetReportedErrors())
            inputSourceView_->AddAnnotation(err.line - 1, err.text);
    }
    catch (const std::exception& e)
    {
        /* Show message box with error message */
        std::string s = e.what();
        if (!s.empty())
            s[0] = std::toupper(s[0]);
        s += '!';
        wxMessageBox(s, "Invalid Input", wxOK | wxICON_WARNING, this);
    }

    SetStatusReady(true);
}

void DebuggerView::SetStatusReady(bool isReady)
{
    statusBar_->SetStatusText(isReady ? "Ready" : "Busy", 0);
    statusBar_->Refresh();
}

void DebuggerView::SetStatusLine(int line)
{
    statusBar_->SetStatusText("Ln " + std::to_string(line), 1);
}

void DebuggerView::SetStatusColumn(int column)
{
    statusBar_->SetStatusText("Col " + std::to_string(column), 2);
}


} // /namespace Xsc



// ================================================================================
