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
#include "SourceView.h"
#include <Xsc/Xsc.h>


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
        void CreateLayoutSubSplitter();
        void CreateLayoutInputSourceView();
        void CreateLayoutOutputSourceView();

        void OnInputSourceCharEnter(char chr);

        void TranslateInputToOutput();

        wxSplitterWindow*   mainSplitter_       = nullptr;
        wxSplitterWindow*   subSplitter_        = nullptr;

        wxPropertyGrid*     propGrid_           = nullptr;

        SourceView*         inputSourceView_    = nullptr;
        SourceView*         outputSourceView_   = nullptr;

        ShaderInput         shaderInput_;
        ShaderOutput        shaderOutput_;

};


} // /namespace Xsc


#endif



// ================================================================================