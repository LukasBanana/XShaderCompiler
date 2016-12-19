/*
 * DebuggerView.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "DebuggerView.h"
#include <sstream>
#include <memory>
#include <vector>
#include <fstream>


namespace Xsc
{


static long DebuggerViewStyle()
{
    return (wxSYSTEM_MENU | wxCAPTION | wxCLIP_CHILDREN | wxMINIMIZE_BOX | wxMAXIMIZE_BOX | wxRESIZE_BORDER | wxCLOSE_BOX);
}

DebuggerView::DebuggerView(const wxPoint& pos, const wxSize& size) :
    wxFrame{ nullptr, wxID_ANY, "Xsc Debugger", pos, size, DebuggerViewStyle() }
{
    #ifdef _WIN32
    SetIcon(wxICON(APP_ICON));
    #endif

    CreateLayout();

    Centre();

    Bind(wxEVT_CLOSE_WINDOW, &DebuggerView::OnClose, this);
}

static const std::string settingsFilename("XscDebuggerSettings");
static const std::string codeFilename("XscDebuggerCode");

void DebuggerView::SaveSettings()
{
    std::ofstream settingsFile(settingsFilename);
    if (settingsFile.good())
        settingsFile << propGrid_->SaveEditableState().ToStdString() << std::endl;

    std::ofstream codeFile(codeFilename);
    if (codeFile.good())
        codeFile << inputSourceView_->GetText();
}

void DebuggerView::LoadSettings()
{
    std::ifstream settingsFile(settingsFilename);
    if (settingsFile.good())
    {
        std::string state;
        std::getline(settingsFile, state);
        propGrid_->RestoreEditableState(state);
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
}

void DebuggerView::CreateLayoutPropertyGrid()
{
    propGrid_ = new wxPropertyGrid(mainSplitter_, wxID_ANY, wxDefaultPosition, wxSize(200, 600), wxPG_SPLITTER_AUTO_CENTER);

    CreateLayoutPropertyGridShaderInput(*propGrid_);
    CreateLayoutPropertyGridShaderOutput(*propGrid_);
    CreateLayoutPropertyGridOptions(*propGrid_);
    CreateLayoutPropertyGridFormatting(*propGrid_);

    propGrid_->Bind(wxEVT_PG_CHANGED, &DebuggerView::OnPropertyGridChange, this);
}

void DebuggerView::CreateLayoutPropertyGridShaderInput(wxPropertyGrid& pg)
{
    pg.Append(new wxPropertyCategory("Shader Input"));

    wxPGChoices choices0;
    {
        choices0.Add("HLSL3");
        choices0.Add("HLSL4");
        choices0.Add("HLSL5");
    }
    pg.Append(new wxEnumProperty("Shader Version", "inputVersion", choices0, 2))->Enable(false);

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
    pg.Append(new wxEnumProperty("Shader Version", "outputVersion", choices0))->Enable(false);

    pg.Append(new wxStringProperty("Name Mangling Prefix", "prefix", "xsc_"));
}

void DebuggerView::CreateLayoutPropertyGridOptions(wxPropertyGrid& pg)
{
    pg.Append(new wxPropertyCategory("Options"));

    pg.Append(new wxBoolProperty("Allow Extensions", "extensions"));
    pg.Append(new wxBoolProperty("Explicit Binding", "binding"));
    pg.Append(new wxBoolProperty("Optimize", "optimize"));
    pg.Append(new wxBoolProperty("Prefer Wrappers", "wrappers"));
    pg.Append(new wxBoolProperty("Preprocess Only", "preprocess"));
    pg.Append(new wxBoolProperty("Preserve Comments", "comments"));
    pg.Append(new wxBoolProperty("Unroll Array Initializers", "unrollInitializers"));
}

void DebuggerView::CreateLayoutPropertyGridFormatting(wxPropertyGrid& pg)
{
    pg.Append(new wxPropertyCategory("Formatting"));

    pg.Append(new wxStringProperty("Indentation", "indent", "    "));
    pg.Append(new wxBoolProperty("Blanks", "blanks", true));
    pg.Append(new wxBoolProperty("Line Marks", "lineMarks"));
    pg.Append(new wxBoolProperty("Compact Wrappers", "compactWrappers", true));
    pg.Append(new wxBoolProperty("Always Braced Scopes", "alwaysBracedScopes"));
    pg.Append(new wxBoolProperty("New-Line Open Scope", "newLineOpenScope", true));
    pg.Append(new wxBoolProperty("Line Separation", "lineSeparation", true));
}

void DebuggerView::CreateLayoutSubSplitter()
{
    subSplitter_ = new wxSplitterWindow(mainSplitter_, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_LIVE_UPDATE);

    CreateLayoutReportView();
    CreateLayoutSourceSplitter();

    subSplitter_->SplitHorizontally(sourceSplitter_, reportView_, 600);
}

void DebuggerView::CreateLayoutReportView()
{
    reportView_ = new ReportView(subSplitter_, wxDefaultPosition, wxSize(400, 50));
}

void DebuggerView::CreateLayoutSourceSplitter()
{
    sourceSplitter_ = new wxSplitterWindow(subSplitter_, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_LIVE_UPDATE);

    CreateLayoutInputSourceView();
    CreateLayoutOutputSourceView();

    sourceSplitter_->SplitVertically(inputSourceView_, outputSourceView_);
}

void DebuggerView::CreateLayoutInputSourceView()
{
    inputSourceView_ = new SourceView(sourceSplitter_, wxDefaultPosition, wxSize(200, 600));
    inputSourceView_->SetLanguage(SourceViewLanguage::HLSL);
    inputSourceView_->SetCharEnterCallback(std::bind(&DebuggerView::OnInputSourceCharEnter, this, std::placeholders::_1));
}

void DebuggerView::CreateLayoutOutputSourceView()
{
    outputSourceView_ = new SourceView(sourceSplitter_, wxDefaultPosition, wxSize(200, 600));
    outputSourceView_->SetLanguage(SourceViewLanguage::GLSL);
    //outputSourceView_->SetReadOnly(true);
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

    if (name == "entry")
        shaderInput_.entryPoint = ValueStr();
    else if (name == "secondaryEntry")
        shaderInput_.secondaryEntryPoint = ValueStr();
    else if (name == "target")
        shaderInput_.shaderTarget = static_cast<ShaderTarget>(static_cast<long>(ShaderTarget::VertexShader) + ValueInt());
    else if (name == "prefix")
        shaderOutput_.nameManglingPrefix = ValueStr();
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

class DebuggerLog : public Log
{

    public:

        DebuggerLog(ReportView* reportView) :
            reportView_{ reportView }
        {
        }

        void SumitReport(const Report& report) override
        {
            reportView_->AddReport(report);
        }

    private:

        ReportView* reportView_ = nullptr;

};

void DebuggerView::TranslateInputToOutput()
{
    /* Initialize input source */
    auto inputSource = std::make_shared<std::stringstream>();
    *inputSource << inputSourceView_->GetText().ToStdString();
    shaderInput_.sourceCode = inputSource;

    /* Initialize output source */
    std::stringstream outputSource;
    shaderOutput_.sourceCode = (&outputSource);

    /* Compile shader */
    reportView_->Clear();

    DebuggerLog log(reportView_);
    if (Xsc::CompileShader(shaderInput_, shaderOutput_, &log))
    {
        /* Show output */
        outputSourceView_->SetTextAndRefresh(outputSource.str());
    }
}


} // /namespace Xsc



// ================================================================================
