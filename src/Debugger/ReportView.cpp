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

void ReportView::ClearAll()
{
    Clear();
    reportedErrors_.clear();
}

void ReportView::AddReport(const Report& r, const std::string& indent)
{
    /* Append context information */
    WriteLine(indent, r.Context());

    /* Append actual message */
    if (r.Type() == Report::Types::Error)
    {
        WriteLine(indent, r.Message(), wxColour(255, 30, 30));
        if (r.HasLine())
            AddReportedError(r.Message());
    }
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
            BeginTextColour(wxColour(0, 180, 180));
            WriteText(indent + line.substr(0, start));
            EndTextColour();

            if (end <= line.size())
            {
                BeginTextColour(wxColour(50, 255, 255));
                WriteText(line.substr(start, end - start));
                EndTextColour();

                BeginTextColour(wxColour(0, 180, 180));
                WriteText(line.substr(end) + '\n');
                EndTextColour();
            }
            else
                WriteText('\n');
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

void ReportView::AddReportedError(const std::string& message)
{
    auto posLBracket = message.find('(');
    if (posLBracket != std::string::npos)
    {
        auto posRBracket = message.find(')', posLBracket);
        if (posRBracket != std::string::npos)
        {
            auto posColon0 = message.find(':', posLBracket);
            if (posColon0 != std::string::npos && posLBracket < posColon0 && posColon0 < posRBracket)
            {
                ++posColon0;
                auto posColon1 = message.find(':', posColon0);
                if (posColon1 != std::string::npos && posColon1 < posRBracket)
                {
                    auto lineNoStr = message.substr(posColon0, posColon1 - posColon0);
                    auto lineNo = std::stoi(lineNoStr);

                    auto posColon2 = message.find(':', posRBracket);
                    if (posColon2 != std::string::npos && posRBracket < posColon2)
                    {
                        posColon2 += 2;
                        reportedErrors_.push_back({ lineNo, message.substr(posColon2) });
                    }
                }
            }
        }
    }
}


} // /namespace Xsc



// ================================================================================
