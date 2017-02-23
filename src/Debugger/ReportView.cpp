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
    wxRichTextCtrl{ parent, wxID_ANY, wxEmptyString, pos, size, wxRE_MULTILINE | wxRE_READONLY }
{
    wxFont font(wxFontInfo(8).Family(wxFONTFAMILY_MODERN).FaceName("Lucida Console"));
    SetFont(font);
    SetBackgroundColour(wxColour(20, 20, 80));
}

static std::string ReplaceTabs(const std::string& s)
{
    auto ns = s;

    std::size_t pos = 0;
    while ( ( pos = ns.find('\t', pos) ) != std::string::npos )
        ns.replace(pos, 1, "    ");

    return ns;
}

void ReportView::AddReport(const Report& r, const std::string& indent)
{
    /* Append context information */
    WriteLine(indent, r.Context());

    /* Append actual message */
    if (r.Type() == Report::Types::Error)
        WriteLine(indent, r.Message(), wxColour(255, 30, 30));
    else if (r.Type() == Report::Types::Warning)
        WriteLine(indent, r.Message(), wxColour(255, 255, 0));
    else
        WriteLine(indent, r.Message());

    /* Append line marker with multi-color */
    if (r.HasLine())
    {
        auto line = ReplaceTabs(r.Line());
        auto mark = ReplaceTabs(r.Marker());
        auto start = mark.find_first_not_of(' ');
        auto end = mark.size();
        
        if (start != std::string::npos && !mark.empty())
        {
            auto a = indent + line.substr(0, start);
            auto b = line.substr(start, end - start);
            auto c = line.substr(end);

            BeginTextColour(wxColour(0, 180, 180));
            WriteText(indent + line.substr(0, start));
            EndTextColour();

            BeginTextColour(wxColour(50, 255, 255));
            WriteText(line.substr(start, end - start));
            EndTextColour();

            BeginTextColour(wxColour(0, 180, 180));
            WriteText(line.substr(end) + '\n');
            EndTextColour();
        }
        else
            WriteLine(indent, line, wxColour(50, 255, 255));

        WriteLine(indent, mark, wxColour(50, 255, 255));
    }

    /* Append all hints */
    for (const auto& s : r.GetHints())
        WriteLine(indent, s);
}


/*
 * ======= Private: =======
 */

void ReportView::WriteLine(const std::string& indent, const std::string& s, const wxColour& color)
{
    if (!s.empty())
    {
        BeginTextColour(color);
        WriteText(indent + s + '\n');
        EndTextColour();
    }
}


} // /namespace Xsc



// ================================================================================
