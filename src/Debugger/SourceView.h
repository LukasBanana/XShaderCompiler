/*
 * SourceView.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_SOURCE_VIEW_H
#define XSC_SOURCE_VIEW_H


#include <wx/stc/stc.h>
#include <functional>


namespace Xsc
{


enum class SourceViewLanguage
{
    HLSL,
    GLSL,
};

class SourceView : public wxStyledTextCtrl
{

        DECLARE_EVENT_TABLE();

    public:

        using CharEnterCallback = std::function<void(char)>;

        SourceView(wxWindow* parent, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);

        void SetLanguage(const SourceViewLanguage language);

        void SetTextAndRefresh(const wxString& text);

        void SetCharEnterCallback(const CharEnterCallback& callback);

    private:

        void OnCharAdded(wxStyledTextEvent& event);
        void OnKeyDown(wxKeyEvent& event);

        CharEnterCallback charEnterCallback_;

};


} // /namespace Xsc


#endif



// ================================================================================