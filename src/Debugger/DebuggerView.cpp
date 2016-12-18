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
    pg.Append(new wxEnumProperty("Shader Version", "Shader Input Version", choices0, 2));

    wxPGChoices choices1;
    {
        choices1.Add("Vertex Shader");
        choices1.Add("Tessellation-Control Shader");
        choices1.Add("Tessellation-Evaluation Shader");
        choices1.Add("Geometry Shader");
        choices1.Add("Fragment Shader");
        choices1.Add("Compute Shader");
    }
    pg.Append(new wxEnumProperty("Shader Target", wxPG_LABEL, choices1));

    pg.Append(new wxStringProperty("Entry Point", wxPG_LABEL, ""));
    pg.Append(new wxStringProperty("Secondary Entry Point", wxPG_LABEL, ""));
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
    pg.Append(new wxEnumProperty("Shader Version", "Shader Output Version", choices0));

    pg.Append(new wxStringProperty("Name Mangling Prefix", wxPG_LABEL, "xsc_"));
}

void DebuggerView::CreateLayoutPropertyGridOptions(wxPropertyGrid& pg)
{
    pg.Append(new wxPropertyCategory("Options"));

    pg.Append(new wxBoolProperty("Allow Extensions"));
    pg.Append(new wxBoolProperty("Explicit Binding"));
    pg.Append(new wxBoolProperty("Optimize"));
    pg.Append(new wxBoolProperty("Prefer Wrappers"));
    pg.Append(new wxBoolProperty("Preprocess Only"));
    pg.Append(new wxBoolProperty("Preserve Comments"));
}

void DebuggerView::CreateLayoutPropertyGridFormatting(wxPropertyGrid& pg)
{
    pg.Append(new wxPropertyCategory("Formatting"));

    pg.Append(new wxStringProperty("Indentation", wxPG_LABEL, "    "));
    pg.Append(new wxBoolProperty("Blanks", wxPG_LABEL, true));
    pg.Append(new wxBoolProperty("Line Marks"));
    pg.Append(new wxBoolProperty("Compact Wrappers", wxPG_LABEL, true));
    pg.Append(new wxBoolProperty("Always Braced Scopes"));
    pg.Append(new wxBoolProperty("New-Line Open Scope", wxPG_LABEL, true));
    pg.Append(new wxBoolProperty("Line Separation", wxPG_LABEL, true));
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

void DebuggerView::OnInputSourceCharEnter(char chr)
{
    TranslateInputToOutput();
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

    #if 1
    shaderInput_.entryPoint = "VS";
    shaderInput_.shaderTarget = ShaderTarget::VertexShader;
    #endif

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
