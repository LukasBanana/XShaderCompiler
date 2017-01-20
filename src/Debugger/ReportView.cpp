/*
 * ReportView.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "ReportView.h"


namespace Xsc
{


ReportView::ReportView(wxWindow* parent, const wxPoint& pos, const wxSize& size) :
    wxListBox{ parent, wxID_ANY, pos, size }
{
    wxFont font(wxFontInfo(8).Family(wxFONTFAMILY_MODERN));
    SetFont(font);
}

void ReportView::AddReport(const Report& r, const std::string& indent)
{
    Add(indent, r.Context());
    Add(indent, r.Message());

    if (r.HasLine())
    {
        Add(indent, r.Line());
        Add(indent, r.Marker());
    }

    for (const auto& s : r.GetHints())
        Add(indent, s);
}


/*
 * ======= Private: =======
 */

void ReportView::Add(const std::string& indent, const std::string& s)
{
    if (!s.empty())
        Append(indent + s);
}


} // /namespace Xsc



// ================================================================================
