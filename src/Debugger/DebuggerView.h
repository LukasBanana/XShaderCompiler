/*
 * DebuggerView.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2018 by Lukas Hermanns)
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
        void CreateLayoutPropertyGridUniformPacking(wxPropertyGrid& pg);
        void CreateLayoutPropertyGridOptions(wxPropertyGrid& pg);
        void CreateLayoutPropertyGridFormatting(wxPropertyGrid& pg);
        void CreateLayoutPropertyGridNameMangling(wxPropertyGrid& pg);
        void CreateLayoutSubSplitter();
        void CreateLayoutReportView();
        void CreateLayoutSourceSplitter();
        void CreateLayoutInputSourceView();
        void CreateLayoutOutputSourceView();
        void CreateLayoutStatusBar();
        void CreateLayoutMenuBar();

        void OnInputSourceCharEnter(char chr);
        void OnPropertyGridChange(wxPropertyGridEvent& event);
        void OnClose(wxCloseEvent& event);

        void OnAbout(wxCommandEvent& event);
        void OnHelp(wxCommandEvent& event);
        void OnQuit(wxCommandEvent& event);

        void TranslateInputToOutput();

        void SetStatusReady(bool isReady);
        void SetStatusLine(int line);
        void SetStatusColumn(int column);

        wxStatusBar*        statusBar_          = nullptr;
        wxMenuBar*          menuBar_            = nullptr;

        wxSplitterWindow*   mainSplitter_       = nullptr;
        wxSplitterWindow*   subSplitter_        = nullptr;
        wxSplitterWindow*   sourceSplitter_     = nullptr;

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