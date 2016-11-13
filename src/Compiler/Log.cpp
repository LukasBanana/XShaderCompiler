/*
 * Log.cpp
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2016 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <Xsc/Log.h>
#include <Xsc/ConsoleManip.h>
#include <iostream>


namespace Xsc
{


/*
 * StdLog class
 */

void StdLog::SumitReport(const Report& report)
{
    switch (report.Type())
    {
        case Report::Types::Info:
            infos_.push_back({ FullIndent(), report });
            break;
        case Report::Types::Warning:
            warnings_.push_back({ FullIndent(), report });
            break;
        case Report::Types::Error:
            errors_.push_back({ FullIndent(), report });
            break;
    }
}

void StdLog::PrintAll(bool verbose)
{
    PrintAndClearReports(infos_, verbose);
    PrintAndClearReports(warnings_, verbose, "WARNING(S)");
    PrintAndClearReports(errors_, verbose, "ERROR(S)");
}

using Colors = ConsoleManip::ColorFlags;

static void PrintMultiLineString(const std::string& s, const std::string& indent)
{
    /* Determine at which position the actual text begins (excluding the "error (X:Y) : " or the like) */
    auto textStartPos = s.find(" : ");

    if (textStartPos != std::string::npos)
        textStartPos += 3;
    else
        textStartPos = 0;

    std::string newLineIndent(textStartPos, ' ');

    std::size_t start = 0;
    bool useNewLineIndent = false;

    while (start < s.size())
    {
        /* Print indentation */
        std::cout << indent;

        if (useNewLineIndent)
            std::cout << newLineIndent;

        /* Print next line */
        auto end = s.find('\n', start);

        if (end != std::string::npos)
        {
            std::cout << s.substr(start, end - start);
            start = end + 1;
        }
        else
        {
            std::cout << s.substr(start);
            start = end;
        }

        std::cout << std::endl;

        useNewLineIndent = true;
    }
}

void StdLog::PrintReport(const IndentReport& r, bool verbose)
{
    /* Print optional context description */
    if (!r.report.Context().empty())
        PrintMultiLineString(r.report.Context(), r.indent);

    /* Print report message */
    auto type = r.report.Type();
    const auto& msg = r.report.Message();

    if (type == Report::Types::Error)
    {
        ConsoleManip::ScopedColor highlight(std::cout, Colors::Red | Colors::Intens);
        PrintMultiLineString(msg, r.indent);
    }
    else if (type == Report::Types::Warning)
    {
        ConsoleManip::ScopedColor highlight(std::cout, Colors::Yellow);
        PrintMultiLineString(msg, r.indent);
    }
    else
        PrintMultiLineString(msg, r.indent);

    /* Print optional line and line-marker */
    if (verbose && r.report.HasLine())
    {
        const auto& line    = r.report.Line();
        const auto& marker  = r.report.Marker();

        /* Print line with color highlight for the occurrence */
        {
            ConsoleManip::ScopedColor highlight(std::cout, Colors::Green | Colors::Blue);

            std::cout << r.indent;

            auto pos = marker.find('^');
            if (pos != std::string::npos && pos < marker.size())
            {
                std::cout << line.substr(0, pos);
                {
                    ConsoleManip::ScopedColor highlight(std::cout, Colors::Cyan);
                    std::cout << line.substr(pos, marker.size() - pos);
                }
                if (marker.size() < line.size())
                    std::cout << line.substr(marker.size());
            }
            else
                std::cout << line;

            std::cout << std::endl;
        }

        /* Print line marker */
        {
            ConsoleManip::ScopedColor highlight(std::cout, Colors::Cyan);
            std::cout << r.indent << marker << std::endl;
        }
    }
}

void StdLog::PrintAndClearReports(IndentReportList& reports, bool verbose, const std::string& headline)
{
    if (!reports.empty())
    {
        /* Print headline */
        if (!headline.empty())
        {
            auto s = std::to_string(reports.size()) + " " + headline;
            std::cout << s << std::endl;
            std::cout << std::string(s.size(), '-') << std::endl;
        }

        /* Print and clear reports */
        for (const auto& r : reports)
            PrintReport(r, verbose);

        reports.clear();
    }
}


} // /namespace Xsc



// ================================================================================