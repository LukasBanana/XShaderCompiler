/*
 * DebuggerView.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_DEBUGGER_VIEW_H
#define XSC_DEBUGGER_VIEW_H


#include <wx/frame.h>
#include <wx/splitter.h>
#include <wx/propgrid/propgrid.h>
#include <wx/stc/stc.h>


namespace Xsc
{


class DebuggerView : public wxFrame
{

    public:

        DebuggerView(const wxPoint& pos, const wxSize& size);

    private:

        void CreateLayout();
        void CreateLayoutPropertyGrid();
        void CreateLayoutPropertyGridShaderInput(wxPropertyGrid& pg);
        void CreateLayoutPropertyGridShaderOutput(wxPropertyGrid& pg);
        void CreateLayoutPropertyGridOptions(wxPropertyGrid& pg);
        void CreateLayoutPropertyGridFormatting(wxPropertyGrid& pg);

        wxSplitterWindow*   mainSplitter_   = nullptr;
        wxSplitterWindow*   subSplitter_    = nullptr;

        wxPropertyGrid*     propGrid_       = nullptr;

};


} // /namespace Xsc


#endif



// ================================================================================