/*
 * ReportView.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "ReportView.h"


namespace Xsc
{


ReportView::ReportView(wxWindow* parent, const wxPoint& pos, const wxSize& size) :
    wxRichTextCtrl { parent, wxID_ANY, wxEmptyString, pos, size, wxRE_MULTILINE | wxRE_READONLY }
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

void ReportView::ClearAll()
{
    Clear();
    reportedErrors_.clear();
}

void ReportView::AddReport(const Report& r, const std::string& indent)
{
    /* Append context information */
    WriteLn(indent, r.Context());

    /* Append source location */
    Write(indent);

    if (!r.Source().empty())
    {
        Write(r.Source() + ": ");
        if (r.Type() == ReportTypes::Error && r.HasLine())
            AddReportedError(r.Source(), r.Message());
    }

    /* Append name of report type */
    if (!r.TypeName().empty())
    {
        if (r.Type() == ReportTypes::Error)
            Write(r.TypeName() + ": ", wxColour(255, 30, 30));
        else if (r.Type() == ReportTypes::Warning)
            Write(r.TypeName() + ": ", wxColour(255, 255, 0));
        else
            Write(r.TypeName() + ": ");
    }

    /* Append actual message */
    Write(r.Message());
    Write("\n");

    /* Append line marker with multi-color */
    if (r.HasLine())
    {
        auto line = ReplaceTabs(r.Line());
        auto mark = ReplaceTabs(r.Marker());

        Write(indent, wxColour(0, 180, 180));

        std::size_t start = 0, end = 0;

        while ( end < mark.size() && ( start = mark.find_first_not_of(' ', end) ) != std::string::npos )
        {
            /* Write unhighlighted text */
            Write(line.substr(end, start - end), wxColour(0, 180, 180));

            /* Write highlighted text */
            end = mark.find(' ', start);

            if (end == std::string::npos)
                end = std::min(line.size(), mark.size());

            Write(line.substr(start, end - start), wxColour(50, 255, 255));
        }

        BeginTextColour(wxColour(0, 180, 180));
        {
            if (end < line.size())
                WriteText(line.substr(end));
            WriteText('\n');
        }
        EndTextColour();

        WriteLn(indent, mark, wxColour(50, 255, 255));
    }

    /* Append all hints */
    for (const auto& s : r.GetHints())
        WriteLn(indent, s);
}


/*
 * ======= Private: =======
 */

void ReportView::Write(const std::string& s, const wxColour& color)
{
    if (!s.empty())
    {
        BeginTextColour(color);
        WriteText(s);
        EndTextColour();
    }
}

void ReportView::WriteLn(const std::string& indent, const std::string& s, const wxColour& color)
{
    if (!s.empty())
        Write(indent + s + '\n', color);
}

void ReportView::AddReportedError(const std::string& sloc, const std::string& msg)
{
    auto posColon0 = sloc.find_last_of(':');
    if (posColon0 != std::string::npos)
    {
        auto posColon1 = sloc.find_last_of(':', posColon0 - 1);
        if (posColon1 != std::string::npos)
        {
            auto lineNoStr = sloc.substr(posColon1 + 1, posColon0 - posColon1 - 1);
            auto lineNo = std::stoi(lineNoStr);

            reportedErrors_.push_back({ lineNo, msg });
        }
    }
}


} // /namespace Xsc



// ================================================================================
