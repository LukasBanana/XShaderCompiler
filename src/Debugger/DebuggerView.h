/*
 * DebuggerView.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_DEBUGGER_VIEW_H
#define XSC_DEBUGGER_VIEW_H


#include <Xsc/Xsc.h>
#include <wx/frame.h>
#include <wx/splitter.h>
#include <wx/propgrid/propgrid.h>
#include "SourceView.h"
#include "ReportView.h"


namespace Xsc
{


class DebuggerView : public wxFrame
{

    public:

        DebuggerView(const wxPoint& pos, const wxSize& size);

        void SaveSettings();
        void LoadSettings();

    private:

        void CreateLayout();
        void CreateLayoutPropertyGrid();
        void CreateLayoutPropertyGridShaderInput(wxPropertyGrid& pg);
        void CreateLayoutPropertyGridShaderOutput(wxPropertyGrid& pg);
        void CreateLayoutPropertyGridOptions(wxPropertyGrid& pg);
        void CreateLayoutPropertyGridFormatting(wxPropertyGrid& pg);
        void CreateLayoutSubSplitter();
        void CreateLayoutReportView();
        void CreateLayoutSourceSplitter();
        void CreateLayoutInputSourceView();
        void CreateLayoutOutputSourceView();

        void OnInputSourceCharEnter(char chr);
        void OnPropertyGridChange(wxPropertyGridEvent& event);
        void OnClose(wxCloseEvent& event);

        void TranslateInputToOutput();

        wxSplitterWindow*   mainSplitter_       = nullptr;
        wxSplitterWindow*   subSplitter_        = nullptr;
        wxSplitterWindow*   sourceSplitter_        = nullptr;

        wxPropertyGrid*     propGrid_           = nullptr;

        ReportView*         reportView_         = nullptr;

        SourceView*         inputSourceView_    = nullptr;
        SourceView*         outputSourceView_   = nullptr;

        ShaderInput         shaderInput_;
        ShaderOutput        shaderOutput_;

};


} // /namespace Xsc


#endif



// ================================================================================