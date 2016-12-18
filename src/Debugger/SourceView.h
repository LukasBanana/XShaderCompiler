/*
 * SourceView.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_SOURCE_VIEW_H
#define XSC_SOURCE_VIEW_H


#include <wx/stc/stc.h>


namespace Xsc
{


enum class SourceViewLanguage
{
    HLSL,
    GLSL,
};

class SourceView : public wxStyledTextCtrl
{

    public:

        SourceView(wxWindow* parent, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);

        void SetLanguage(const SourceViewLanguage language);

    private:



};


} // /namespace Xsc


#endif



// ================================================================================